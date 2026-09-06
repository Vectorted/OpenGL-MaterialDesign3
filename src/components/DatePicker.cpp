/**
 * @file DatePicker.cpp
 * @brief Implementation of calendar grid arithmetic, month navigation, and rendering passes.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "DatePicker.hpp"
#include <ctime>
#include <string>
#include <cmath>
#include <GLFW/glfw3.h>

DatePicker::DatePicker() : View(0, 0, 342.0f, 448.0f) { 
    layout_width = 342.0f;
    layout_height = 448.0f;
}

int DatePicker::getDaysInMonth(int y, int m) {
    if (m == 2) return ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) ? 29 : 28;
    if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
    return 31;
}

int DatePicker::getStartWeekday(int y, int m) {
    std::tm time_in = { 0, 0, 0, 1, m - 1, y - 1900 };
    std::time_t time_temp = std::mktime(&time_in);
    return std::localtime(&time_temp)->tm_wday;
}

bool DatePicker::isInsideRect(float mx, float my, float rx, float ry, float rw, float rh) {
    return mx >= rx && mx <= rx + rw && my >= ry && my <= ry + rh;
}

void DatePicker::render(MaterialShader& renderer, MaterialTheme& theme) {
    renderer.drawM3UI(x, y, width, height, dp(20), dp(20), dp(20), dp(20), theme.surfaceContainerHigh);

    renderer.drawText("Select date", x + dp(24), y + dp(28) - dp(13) * 0.45f, dp(13), theme.onSurfaceVariant);

    const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    std::string dateStr = "Sun, " + std::string(months[selMonth - 1]) + " " + std::to_string(selDay);
    renderer.drawText(dateStr, x + dp(24), y + dp(65) - dp(32) * 0.45f, dp(32), theme.onSurface);

    renderer.drawM3UI(x, y + dp(100), width, dp(1), 0, 0, 0, 0, theme.outlineVariant);

    std::string myStr = std::string(months[viewMonth - 1]) + " " + std::to_string(viewYear);
    renderer.drawText(myStr, x + dp(24), y + dp(130) - dp(14) * 0.45f, dp(14), theme.onSurface);

    btnSize = dp(32.0f);
    prevBtnX = x + width - dp(88.0f); prevBtnY = y + dp(116.0f);
    nextBtnX = x + width - dp(44.0f); nextBtnY = y + dp(116.0f);

    renderer.drawIcon(prevBtnX + dp(4), prevBtnY + dp(4), dp(22), "m3:arrow_back", theme.onSurface);
    renderer.drawIcon(nextBtnX + dp(4), nextBtnY + dp(4), dp(22), "m3:arrow_forward", theme.onSurface);

    const char* wdays[] = { "S", "M", "T", "W", "T", "F", "S" };

    cellSize = dp(42.0f);
    float totalGridWidth = 7 * cellSize;
    gridStartX = x + (width - totalGridWidth) / 2.0f;
    gridStartY = y + dp(160.0f);

    for (int i = 0; i < 7; i++) {
        float cx = gridStartX + i * cellSize + cellSize / 2.0f;
        float tw = renderer.getTextWidth(wdays[i], dp(13));
        renderer.drawText(wdays[i], cx - tw / 2.0f, gridStartY + dp(10.0f) - dp(13.0f) * 0.45f, dp(13), theme.onSurface);
    }

    gridStartY += dp(24.0f);
    int startIdx = getStartWeekday(viewYear, viewMonth);
    int totalDays = getDaysInMonth(viewYear, viewMonth);

    for (int d = 1; d <= totalDays; d++) {
        int r = (startIdx + d - 1) / 7;
        int c = (startIdx + d - 1) % 7;
        float cx = gridStartX + c * cellSize;
        float cy = gridStartY + r * cellSize;
        float cellCenterX = cx + cellSize / 2.0f;
        float cellCenterY = cy + cellSize / 2.0f;

        bool isSel = (d == selDay && viewMonth == selMonth && viewYear == selYear);
        float p = dp(4.0f); 

        if (isSel) {
            renderer.drawM3UI(cx + p, cy + p, cellSize - p * 2, cellSize - p * 2, (cellSize - p * 2) / 2, (cellSize - p * 2) / 2, (cellSize - p * 2) / 2, (cellSize - p * 2) / 2, theme.primary);
        }
        else if (d == hoverDay) {
            renderer.drawM3UI(cx + p, cy + p, cellSize - p * 2, cellSize - p * 2, (cellSize - p * 2) / 2, (cellSize - p * 2) / 2, (cellSize - p * 2) / 2, (cellSize - p * 2) / 2, theme.surfaceVariant);
        }

        std::string dayStr = std::to_string(d);
        float tw = renderer.getTextWidth(dayStr, dp(14));
        M3Color tColor = isSel ? theme.onPrimary : theme.onSurface;

        renderer.drawText(dayStr, cellCenterX - tw / 2.0f, cellCenterY - dp(14.0f) * 0.45f, dp(14), tColor);
    }
}

bool DatePicker::handleMouseMove(float mx, float my) {
    if (!isVisible()) return false;
    hoverDay = -1;

    if (mx >= gridStartX && mx <= gridStartX + 7 * cellSize && my >= gridStartY && my <= gridStartY + 6 * cellSize) {
        int c = (int)((mx - gridStartX) / cellSize);
        int r = (int)((my - gridStartY) / cellSize);

        int startIdx = getStartWeekday(viewYear, viewMonth);
        int d = (r * 7 + c) - startIdx + 1;
        if (d >= 1 && d <= getDaysInMonth(viewYear, viewMonth)) {
            hoverDay = d;
            return true;
        }
    }
    return View::handleMouseMove(mx, my);
}

bool DatePicker::handleMouseButton(int button, int action, float mx, float my) {
    if (!isVisible()) return false;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (isInsideRect(mx, my, prevBtnX, prevBtnY, btnSize, btnSize)) {
            viewMonth--; if (viewMonth < 1) { viewMonth = 12; viewYear--; } return true;
        }
        if (isInsideRect(mx, my, nextBtnX, nextBtnY, btnSize, btnSize)) {
            viewMonth++; if (viewMonth > 12) { viewMonth = 1; viewYear++; } return true;
        }
        if (hoverDay != -1) {
            selDay = hoverDay; selMonth = viewMonth; selYear = viewYear;
            if (onDateSelected) onDateSelected(selYear, selMonth, selDay);
            return true;
        }
    }
    return View::handleMouseButton(button, action, mx, my);
}