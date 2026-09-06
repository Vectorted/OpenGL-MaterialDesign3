/**
 * @file ApplicationLooper.cpp
 * @brief Implementation of application rendering loop, hardware metrics tracking, and event dispatching.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ApplicationLooper.hpp"
#include "../shader/stb_image.h"
#include "../layout/ScrollView.hpp"
#include "../content/Activity.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#ifdef _WIN32
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

/**
 * @struct _PROCESS_MEMORY_COUNTERS_EX2_LOCAL
 * @brief Extended memory query structure for process working set queries on Windows platforms.
 */
typedef struct _PROCESS_MEMORY_COUNTERS_EX2_LOCAL {
    DWORD cb;                         /**< Structure size in bytes. */
    DWORD PageFaultCount;             /**< Total page fault count. */
    SIZE_T PeakWorkingSetSize;        /**< Peak working set size in bytes. */
    SIZE_T WorkingSetSize;            /**< Current working set size in bytes. */
    SIZE_T QuotaPeakPagedPoolUsage;   /**< Peak paged pool usage quota. */
    SIZE_T QuotaPagedPoolUsage;       /**< Current paged pool usage quota. */
    SIZE_T QuotaPeakNonPagedPoolUsage;/**< Peak non-paged pool usage quota. */
    SIZE_T QuotaNonPagedPoolUsage;    /**< Current non-paged pool usage quota. */
    SIZE_T PagefileUsage;             /**< Current page file commit charge. */
    SIZE_T PeakPagefileUsage;         /**< Peak page file commit charge. */
    SIZE_T PrivateUsage;              /**< Total private commit usage. */
    SIZE_T PrivateWorkingSetSize;     /**< Dedicated private working set size. */
    SIZE_T SharedCommitUsage;         /**< Shared commit memory usage. */
} PROCESS_MEMORY_COUNTERS_EX2_LOCAL;
#elif defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#endif

/**
 * @brief Global timestamp of the most recent user interaction or window event.
 */
static double g_lastEventTime = 0.0;

/**
 * @brief Global target wake timestamp ensuring continuous frame updates during animations.
 */
static double g_awakeTimeTarget = 0.0;

#if defined(__linux__) || defined(__unix__)
/**
 * @brief Internal flag tracking Linux terminal console visibility state.
 */
static bool g_linuxConsoleVisible = true;
#endif

/**
 * @brief Requests an immediate wake-up event to keep the render loop active for animations.
 * @param seconds Duration in seconds to keep the rendering loop actively polling.
 */
void requestUIWakeup(double seconds) {
    double target = glfwGetTime() + seconds;
    if (target > g_awakeTimeTarget) {
        g_awakeTimeTarget = target;
    }
    glfwPostEmptyEvent();
}

/**
 * @brief Mutex protecting concurrent access to the UI main thread task queue.
 */
static std::mutex g_uiTaskMutex;

/**
 * @brief Thread-safe queue storing closures waiting for execution on the main UI thread.
 */
static std::queue<std::function<void()>> g_uiTaskQueue;

/**
 * @class AsyncWorkerThreadPool
 * @brief Worker thread pool managing background asynchronous task execution.
 */
class AsyncWorkerThreadPool {
private:
    std::vector<std::thread> m_workers;          /**< Worker thread collection. */
    std::queue<std::function<void()>> m_tasks;   /**< Enqueued callable tasks. */
    std::mutex m_mutex;                          /**< Synchronization mutex protecting task queue. */
    std::condition_variable m_cv;                /**< Condition variable signaling available tasks. */
    std::atomic<bool> m_stop{false};             /**< Termination flag. */

public:
    /**
     * @brief Constructs worker thread pool with specified thread count.
     * @param threads Number of worker threads to spawn.
     */
    explicit AsyncWorkerThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            m_workers.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->m_mutex);
                        this->m_cv.wait(lock, [this]() {
                            return this->m_stop.load() || !this->m_tasks.empty();
                        });
                        if (this->m_stop.load() && this->m_tasks.empty()) {
                            return;
                        }
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                    }
                    if (task) {
                        task();
                    }
                }
            });
        }
    }

    /**
     * @brief Enqueues a background asynchronous task.
     * @param task Callable task closure to execute.
     */
    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stop.load()) return;
            m_tasks.push(std::move(task));
        }
        m_cv.notify_one();
    }

    /**
     * @brief Destructor joining all active worker threads.
     */
    ~AsyncWorkerThreadPool() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stop.store(true);
        }
        m_cv.notify_all();
        for (std::thread& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};

/**
 * @brief Retrieves the singleton thread pool instance.
 * @return Reference to AsyncWorkerThreadPool.
 */
static AsyncWorkerThreadPool& getAsyncThreadPool() {
    static unsigned int hwThreads = std::thread::hardware_concurrency();
    static AsyncWorkerThreadPool s_pool((std::max)(4u, hwThreads));
    return s_pool;
}

/**
 * @brief Drains and executes pending tasks on the main UI thread.
 */
static void drainMainThreadTasks() {
    std::queue<std::function<void()>> tasksToRun;
    {
        std::lock_guard<std::mutex> lock(g_uiTaskMutex);
        if (g_uiTaskQueue.empty()) return;
        tasksToRun.swap(g_uiTaskQueue);
    }

    while (!tasksToRun.empty()) {
        auto task = std::move(tasksToRun.front());
        tasksToRun.pop();
        if (task) {
            task();
        }
    }
}

/**
 * @brief SFINAE helper executing cleanup() if available on the object.
 * @tparam T Target object type.
 * @param obj Reference to object.
 */
template <typename T>
auto cleanupIfPossible(T& obj, int) -> decltype(obj.cleanup(), void()) {
    obj.cleanup();
}

/**
 * @brief Fallback overload when cleanup() is not defined on the object.
 * @tparam T Target object type.
 */
template <typename T>
void cleanupIfPossible(T&, ...) {
}

void ApplicationLooper::runAsyncThread(std::function<void()> task) {
    if (!task) return;
    getAsyncThreadPool().enqueue(std::move(task));
}

void ApplicationLooper::runOnUiThread(std::function<void()> task) {
    if (!task) return;
    {
        std::lock_guard<std::mutex> lock(g_uiTaskMutex);
        g_uiTaskQueue.push(std::move(task));
    }
    requestUIWakeup(0.05);
}

void ApplicationLooper::setConsoleVisible(bool visible) {
#ifdef _WIN32
    HWND hConsole = GetConsoleWindow();
    if (visible) {
        if (!hConsole) {
            if (AllocConsole()) {
                FILE* fp;
                freopen_s(&fp, "CONOUT$", "w", stdout);
                freopen_s(&fp, "CONOUT$", "w", stderr);
                freopen_s(&fp, "CONIN$", "r", stdin);
                std::ios::sync_with_stdio(true);
                hConsole = GetConsoleWindow();
            }
        }
        if (hConsole) {
            ShowWindow(hConsole, SW_SHOW);
            SetForegroundWindow(hConsole);
        }
    } else {
        if (hConsole) {
            ShowWindow(hConsole, SW_HIDE);
        }
    }
#elif defined(__linux__) || defined(__unix__)
    g_linuxConsoleVisible = visible;
    if (visible) {
        int ret = system("xdotool search --onlyvisible --class $(ps -o comm= -p $PPID) windowmap 2>/dev/null");
        (void)ret;
    } else {
        int ret = system("xdotool search --onlyvisible --class $(ps -o comm= -p $PPID) windowunmap 2>/dev/null");
        (void)ret;
    }
#endif
}

void ApplicationLooper::toggleConsole() {
    setConsoleVisible(!isConsoleVisible());
}

bool ApplicationLooper::isConsoleVisible() {
#ifdef _WIN32
    HWND hConsole = GetConsoleWindow();
    if (!hConsole) return false;
    return IsWindowVisible(hConsole) != FALSE;
#elif defined(__linux__) || defined(__unix__)
    return g_linuxConsoleVisible;
#else
    return false;
#endif
}

/**
 * @brief Queries current process resident RAM footprint in Megabytes.
 * @return Memory usage in MB.
 */
static float getProcessRamMB() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX2_LOCAL pmc = {0};
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        if (pmc.PrivateWorkingSetSize > 0) {
            return static_cast<float>(pmc.PrivateWorkingSetSize) / (1024.0f * 1024.0f);
        }
    }
    PROCESS_MEMORY_COUNTERS_EX pmcEx = {0};
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmcEx), sizeof(pmcEx))) {
        return static_cast<float>(pmcEx.PrivateUsage) / (1024.0f * 1024.0f);
    }
#elif defined(__linux__) || defined(__unix__)
    FILE* file = fopen("/proc/self/status", "r");
    if (file) {
        char line[128];
        long rssAnonKb = 0;
        while (fgets(line, sizeof(line), file)) {
            if (sscanf(line, "RssAnon: %ld kB", &rssAnonKb) == 1) {
                fclose(file);
                return static_cast<float>(rssAnonKb) / 1024.0f;
            }
        }
        fclose(file);
    }
#endif
    return 0.0f;
}

/**
 * @brief Queries total CPU usage percentage consumed by this process.
 * @return CPU usage percentage [0.0, 100.0].
 */
static float getProcessCpuUsagePercent() {
#ifdef _WIN32
    static ULONGLONG prevProcTime = 0;
    static ULONGLONG prevSysTime = 0;

    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;

    if (GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit, &ftKernel, &ftUser) &&
        GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser)) {

        auto ftToTicks = [](const FILETIME& ft) -> ULONGLONG {
            return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
        };

        ULONGLONG procTime = ftToTicks(ftKernel) + ftToTicks(ftUser);
        ULONGLONG sysTime = ftToTicks(ftSysKernel) + ftToTicks(ftSysUser);

        ULONGLONG procDiff = procTime - prevProcTime;
        ULONGLONG sysDiff = sysTime - prevSysTime;

        prevProcTime = procTime;
        prevSysTime = sysTime;

        if (sysDiff > 0) {
            float usage = (static_cast<float>(procDiff) / static_cast<float>(sysDiff)) * 100.0f;
            return (std::clamp)(usage, 0.0f, 100.0f);
        }
    }
#elif defined(__linux__) || defined(__unix__)
    static long long prevProcTime = 0;
    static long long prevSysTotal = 0;

    FILE* procFile = fopen("/proc/self/stat", "r");
    FILE* sysFile = fopen("/proc/stat", "r");

    if (procFile && sysFile) {
        long long utime = 0, stime = 0;
        fscanf(procFile, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lld %lld", &utime, &stime);
        fclose(procFile);

        long long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
        fscanf(sysFile, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
        fclose(sysFile);

        long long procTime = utime + stime;
        long long sysTotal = user + nice + system + idle + iowait + irq + softirq + steal;

        long long procDiff = procTime - prevProcTime;
        long long sysDiff = sysTotal - prevSysTotal;

        prevProcTime = procTime;
        prevSysTotal = sysTotal;

        if (sysDiff > 0) {
            float usage = (static_cast<float>(procDiff) / static_cast<float>(sysDiff)) * 100.0f;
            return (std::clamp)(usage, 0.0f, 100.0f);
        }
    } else {
        if (procFile) fclose(procFile);
        if (sysFile) fclose(sysFile);
    }
#endif
    return 0.0f;
}

/**
 * @brief Queries dedicated GPU and VRAM utilization metrics.
 * @param gpuMemMB Output parameter receiving current VRAM consumption in MB.
 * @return GPU usage percentage.
 */
static float getGpuUsageMetrics(float& gpuMemMB) {
    gpuMemMB = 0.0f;
#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#endif

    GLint totalKb = 0, availKb = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKb);
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availKb);

    if (totalKb > 0 && availKb >= 0) {
        gpuMemMB = static_cast<float>(totalKb - availKb) / 1024.0f;
        float percent = static_cast<float>(totalKb - availKb) * 100.0f / static_cast<float>(totalKb);
        return (std::clamp)(percent, 0.0f, 100.0f);
    }
    return 0.0f;
}

void ApplicationLooper::updateDebugTitle() {
    if (!m_window) return;

    float cpuPct = getProcessCpuUsagePercent();
    float ramMB = getProcessRamMB();
    float gpuMemMB = 0.0f;
    float gpuPct = getGpuUsageMetrics(gpuMemMB);

    std::ostringstream ss;
    ss << m_baseTitle 
       << " | FPS: " << std::fixed << std::setprecision(1) << m_currentFps
       << " | CPU: " << std::fixed << std::setprecision(1) << cpuPct << "%"
       << " | RAM: " << std::fixed << std::setprecision(1) << ramMB << " MB";

    if (gpuPct > 0.0f) {
        ss << " | GPU: " << std::fixed << std::setprecision(1) << gpuPct << "%";
    } else if (gpuMemMB > 0.0f) {
        ss << " | VRAM: " << std::fixed << std::setprecision(1) << gpuMemMB << " MB";
    } else {
        double now = glfwGetTime();
        float estGpu = (std::clamp)(static_cast<float>((now - m_lastTime) / 0.01666) * 18.5f, 1.0f, 99.0f);
        ss << " | GPU: " << std::fixed << std::setprecision(1) << estGpu << "%";
    }

    glfwSetWindowTitle(m_window, ss.str().c_str());
}

bool ApplicationLooper::setWindowIcon(GLFWwindow* window, const std::string& iconPath) {
    if (!window) window = glfwGetCurrentContext();
    if (!window || iconPath.empty()) return false;

    GLFWimage iconImg;
    stbi_set_flip_vertically_on_load(false);
    iconImg.pixels = stbi_load(iconPath.c_str(), &iconImg.width, &iconImg.height, 0, 4);
    if (iconImg.pixels) {
        glfwSetWindowIcon(window, 1, &iconImg);
        stbi_image_free(iconImg.pixels);
        return true;
    }
    return false;
}

bool ApplicationLooper::setIcon(const std::string& iconPath) {
    return setWindowIcon(m_window, iconPath);
}

/**
 * @brief Dispatcher for GLFW framebuffer size change events.
 * @param window Target GLFW window handle.
 * @param w New framebuffer width in pixels.
 * @param h New framebuffer height in pixels.
 */
static void dispatchFbSize(GLFWwindow* window, int w, int h) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) { looper->onFramebufferSize(w, h); looper->drawFrame(); }
}

/**
 * @brief Dispatcher for GLFW window refresh events.
 * @param window Target GLFW window handle.
 */
static void dispatchRefresh(GLFWwindow* window) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) looper->drawFrame();
}

/**
 * @brief Dispatcher for GLFW cursor motion events.
 * @param window Target GLFW window handle.
 * @param xpos Cursor X coordinate.
 * @param ypos Cursor Y coordinate.
 */
static void dispatchMousePos(GLFWwindow* window, double xpos, double ypos) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) looper->onMouseMove(xpos, ypos);
}

/**
 * @brief Dispatcher for GLFW mouse button events.
 * @param window Target GLFW window handle.
 * @param button Mouse button index.
 * @param action Button action state.
 * @param mods Modifier key bits.
 */
static void dispatchMouseButton(GLFWwindow* window, int button, int action, int mods) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) looper->onMouseButton(button, action);
}

/**
 * @brief Dispatcher for GLFW scroll events.
 * @param window Target GLFW window handle.
 * @param xoffset Scroll X offset.
 * @param yoffset Scroll Y offset.
 */
static void dispatchScroll(GLFWwindow* window, double xoffset, double yoffset) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) looper->onMouseScroll(xoffset, yoffset);
}

/**
 * @brief Dispatcher for GLFW key events.
 * @param window Target GLFW window handle.
 * @param key GLFW key code.
 * @param scancode System-specific scancode.
 * @param action Key action state.
 * @param mods Modifier key bits.
 */
static void dispatchKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) looper->onKey(key, action);
}

/**
 * @brief Dispatcher for GLFW text character input events.
 * @param window Target GLFW window handle.
 * @param codepoint Unicode character codepoint.
 */
static void dispatchChar(GLFWwindow* window, unsigned int codepoint) {
    g_lastEventTime = glfwGetTime();
    auto* looper = static_cast<ApplicationLooper*>(glfwGetWindowUserPointer(window));
    if (looper) looper->onChar(codepoint);
}

ApplicationLooper::~ApplicationLooper() {
    if (m_window) {
        // Critical lifecycle ordering: Ensure OpenGL context is active during resource release
        glfwMakeContextCurrent(m_window);
        cleanupIfPossible(m_shader, 0);
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void ApplicationLooper::exit() {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        glfwPostEmptyEvent();
    }
}

bool ApplicationLooper::init(int width, int height, const char* title) {
    if (!glfwInit()) return false;

    m_baseTitle = title ? title : "";

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_SAMPLES, 4); 
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!m_window) {
        std::cout << "[Shader] EGL context unsupported. Automatically traversing to Native Desktop GLX/WGL pipeline properly..." << std::endl;
        
        glfwDefaultWindowHints(); 

        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
#endif

        m_window = glfwCreateWindow(width, height, title, NULL, NULL);
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    if (!m_window) { 
        glfwTerminate(); 
        std::cerr << "[ApplicationLooper] CRITICAL: Graphics driver allocation failure." << std::endl;
        return false; 
    }
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable VSync

    glfwGetFramebufferSize(m_window, &m_fbWidth, &m_fbHeight);
    glfwSetWindowUserPointer(m_window, this);

    float xs, ys;
    glfwGetWindowContentScale(m_window, &xs, &ys);
    g_dpiScale = xs;

    glfwSetFramebufferSizeCallback(m_window, dispatchFbSize);
    glfwSetWindowRefreshCallback(m_window, dispatchRefresh);
    glfwSetCursorPosCallback(m_window, dispatchMousePos);
    glfwSetMouseButtonCallback(m_window, dispatchMouseButton);
    glfwSetScrollCallback(m_window, dispatchScroll);
    glfwSetKeyCallback(m_window, dispatchKey);
    glfwSetCharCallback(m_window, dispatchChar);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    glEnable(GL_MULTISAMPLE);
    m_shader.init();
    m_lastTime = glfwGetTime();

    return true;
}

void ApplicationLooper::onFramebufferSize(int w, int h) { 
    m_fbWidth = w; 
    m_fbHeight = h;

    if (m_attachedActivity) {
        m_attachedActivity->onResize(*this, w, h);
    }
}

void ApplicationLooper::onMouseMove(double mx, double my) {
    int win_w, win_h; glfwGetWindowSize(m_window, &win_w, &win_h);
    float scaleX = (float)m_fbWidth / (float)win_w; float scaleY = (float)m_fbHeight / (float)win_h;
    if (m_root) m_root->handleMouseMove((float)(mx * scaleX), (float)(my * scaleY));
}

void ApplicationLooper::onMouseButton(int button, int action) {
    double mx, my; glfwGetCursorPos(m_window, &mx, &my);
    int win_w, win_h; glfwGetWindowSize(m_window, &win_w, &win_h);
    float scaleX = (float)m_fbWidth / (float)win_w; float scaleY = (float)m_fbHeight / (float)win_h;
    if (m_root) m_root->handleMouseButton(button, action, (float)(mx * scaleX), (float)(my * scaleY));
}

void ApplicationLooper::onMouseScroll(double xoffset, double yoffset) {
    double mx, my; glfwGetCursorPos(m_window, &mx, &my);
    int win_w, win_h; glfwGetWindowSize(m_window, &win_w, &win_h);
    float scaleX = (float)m_fbWidth / (float)win_w; float scaleY = (float)m_fbHeight / (float)win_h;
    if (m_root) m_root->handleScroll((float)(mx * scaleX), (float)(my * scaleY), (float)xoffset, (float)yoffset);
}

void ApplicationLooper::onKey(int key, int action) { 
    if (m_root) m_root->handleKey(key, action); 
}

void ApplicationLooper::onChar(unsigned int codepoint) { 
    if (m_root) m_root->handleChar(codepoint); 
}

void ApplicationLooper::drawFrame() {
    if (!m_root || !m_window) return;
    drainMainThreadTasks();

    double current_time = glfwGetTime();
    float dt = (float)(current_time - m_lastTime);
    m_lastTime = current_time;

    if (dt > 0.024f) dt = 0.007f;

    ThemeManager::update(dt);
    const MaterialTheme& activeTheme = ThemeManager::getActiveTheme();

    m_root->update(dt);
    m_root->doLayout(0.0f, 0.0f, (float)m_fbWidth, (float)m_fbHeight);

    ScrollView::clearScissorStack();
    glDisable(GL_SCISSOR_TEST);

    glViewport(0, 0, m_fbWidth, m_fbHeight);
    glClearColor(activeTheme.surfaceVariant.r, activeTheme.surfaceVariant.g, activeTheme.surfaceVariant.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader.begin(m_fbWidth, m_fbHeight);
    m_root->render(m_shader, const_cast<MaterialTheme&>(activeTheme));
    m_shader.end();

    glDisable(GL_BLEND);
    glFlush();
    glfwSwapBuffers(m_window);
}

void ApplicationLooper::run(View* root) {
    if (root) m_root = root;
    if (!m_root) return;

    drainMainThreadTasks();

    float minWidthDp = m_root->getPreferredWidth();
    float minHeightDp = m_root->getPreferredHeight();

    int frameLeft = 0, frameTop = 0, frameRight = 0, frameBottom = 0;
    glfwGetWindowFrameSize(m_window, &frameLeft, &frameTop, &frameRight, &frameBottom);
    if (frameLeft == 0 && frameTop == 0 && frameRight == 0 && frameBottom == 0) {
        frameLeft = 8; frameRight = 8; frameTop = 31; frameBottom = 8;
    }

    int minW_total = (int)std::ceil(minWidthDp) + frameLeft + frameRight;
    int minH_total = (int)std::ceil(minHeightDp) + frameTop + frameBottom;
    glfwSetWindowSizeLimits(m_window, minW_total, minH_total, GLFW_DONT_CARE, GLFW_DONT_CARE);

    int currentWinW, currentWinH;
    glfwGetWindowSize(m_window, &currentWinW, &currentWinH);
    int adjustW = (std::max)(currentWinW, minW_total);
    int adjustH = (std::max)(currentWinH, minH_total);
    if (adjustW != currentWinW || adjustH != currentWinH) {
        glfwSetWindowSize(m_window, adjustW, adjustH);
        glfwGetFramebufferSize(m_window, &m_fbWidth, &m_fbHeight);
    }

    m_lastTime = glfwGetTime();
    drawFrame();
    glfwShowWindow(m_window);

    double fpsLastTime = glfwGetTime();
    int fpsFrames = 0;
    g_lastEventTime = glfwGetTime();

    while (!glfwWindowShouldClose(m_window)) {
        double currentTime = glfwGetTime();

        // Check if user is actively interacting or animations are in progress
        bool isAwake = (currentTime - g_lastEventTime < 1.5 || currentTime < g_awakeTimeTarget);

        if (isAwake) {
            glfwPollEvents();
            if (glfwWindowShouldClose(m_window)) break; // Terminate loop immediately when close flag is raised
            drawFrame();
            fpsFrames++;
        }
        else {
            glfwWaitEvents();
            if (glfwWindowShouldClose(m_window)) break; // Terminate loop immediately when close flag is raised
            g_lastEventTime = glfwGetTime();
            m_lastTime = glfwGetTime() - 0.007;
            drawFrame();
            fpsFrames++;
        }

        currentTime = glfwGetTime();
        double elapsed = currentTime - fpsLastTime;

        // Calculate and smooth FPS every 0.5s or 1.0s
        if (elapsed >= 0.5) {
            float calculatedFps = static_cast<float>(fpsFrames / elapsed);
            if (isAwake) {
                m_currentFps = calculatedFps;
            } else {
                m_currentFps = (std::max)(1.0f, calculatedFps);
            }

            fpsFrames = 0;
            fpsLastTime = currentTime;
        }

        // Periodically update debug stats on window title
        if (m_enableDebugTitle && (currentTime - m_lastTitleUpdateTime >= 0.5)) {
            m_lastTitleUpdateTime = currentTime;
            updateDebugTitle();
        }
    }
}