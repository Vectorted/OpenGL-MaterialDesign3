/**
 * @file main.cpp
 * @brief Comprehensive showcase catalog and integration test suite demonstrating all Material Design 3 components.
 * 
 * Part of the Material 3 OpenGL ES Component Library.
 * 
 * @author Vectorted
 * @repository https://github.com/Vectorted
 * @license Open-source
 * @copyright Copyright (c) 2026 Vectorted. All rights reserved.
 */

#include "content/Activity.hpp"
#include "components/MaterialComponents.hpp"    
#include "components/StarrySkyFormation.hpp"
#include "components/StarrySkyNotification.hpp"
#include "components/MaterialPrimaryTab.hpp"
#include "components/FloatingImageView.hpp"
#include "components/PageContainer.hpp"
#include "event/ApplicationLooper.hpp"    
#include "theme/MaterialTheme.hpp"

#include "io/Io.hpp"
#include "io/Json.hpp"
#include "io/IoJson.hpp"
#include "net/NetworkOkhttp.hpp"

#include <iostream>    
#include <string>
#include <sstream>

using namespace content;

/**
 * @class MainActivity
 * @brief Main application entry activity demonstrating the full Material Design 3 component catalog.
 * 
 * Sets up the window lifecycle, initializes resource assets, builds the multi-tab interface
 * with interactive forms, cards, virtualized lists, data tables, charts, navigation elements, 
 * and dedicated layout alignment showcases.
 */
class MainActivity : public Activity {
private:
    Snackbar* snackbar = nullptr;                               /**< Transient snackbar notification bar. */
    NavigationDrawer* navigationDrawer = nullptr;               /**< Modal slide-out navigation drawer. */
    MaterialDialog* materialDialog = nullptr;                   /**< Modal confirmation dialog. */
    NotificationOverlay* notificationOverlay = nullptr;         /**< In-app dynamic toast/notification layer. */
    StarrySkyNotification* windowNotification = nullptr;        /**< In-app floating kinetic notification banner queue. */
    StarrySkyNotification* screenNotification = nullptr;        /**< Global desktop overlay notification banners. */
    StarrySkyFormation* starrySkyFormation = nullptr;           /**< Interactive procedural particle formation widget. */
    FloatingImageView* floatingImageView = nullptr;             /**< Interactive detachable picture-in-picture floating media card. */
    FloatingActionButton* floatingActionButton = nullptr;       /**< Anchored multi-action speed dial FAB. */
    PageContainer* pageContainer = nullptr;                     /**< Material 3 Animated Page Switcher container. */

public:
    /**
     * @brief Configures initial window title and default viewport dimensions.
     */
    void onInit() override {
        setTitle("Material Design 3 Component Catalog — OpenGL ES");
        setSize(1950, 850);
    }

    /**
     * @brief Performs pre-composition configuration such as custom font loading and window icon assignment.
     * 
     * @param app Reference to the running ApplicationLooper context.
     */
    void onCreate(ApplicationLooper& app) override {
        app.setConsoleVisible(false);
        app.setEnableDebugTitle(false);

        app.setIcon("./resources/vector.png");
        app.getShader().loadFont("./resources/Edition.ttf");
    }

    /**
     * @brief Builds the dedicated showcase page for demonstrating Gravity alignments and centering mechanics.
     * 
     * @param sb Pointer to the active Snackbar instance for triggering feedback toasts.
     * @return ScrollView* Configured scrollable container presenting the alignment matrix.
     */
    ScrollView* createGravityShowcase(Snackbar* sb) {
        ScrollView* scrollView = Components::scrollView();
        scrollView->setLayoutParams(MATCH_PARENT, MATCH_PARENT);

        LinearLayout* scrollList = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        scrollList->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        scrollList->setGap(32.0f);
        scrollList->setPadding(20.0f, 20.0f, 20.0f, 32.0f);

        /* Header typography */
        LinearLayout* headerLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        headerLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        headerLayout->setGap(8.0f);
        headerLayout->addView(Components::textView().text("Layout Gravity & Alignment Showcase").textSize(36.0f).textColor("primary").build());
        headerLayout->addView(Components::textView().text("Demonstrating child positioning, axis gravity, and center alignments across various Material 3 components.").textSize(13.0f).textColor("secondary").build());
        scrollList->addView(headerLayout);

        /* 1. Dual-Axis Center Alignment Hero Card */
        scrollList->addView(Components::textView().text("1. Dual-Axis Center Alignment (Gravity::CENTER)").textSize(16.0f).textColor("primary").build());

        CardView* heroCard = Components::card()
            .type(CardType::Elevated)
            .cornerRadius(24.0f)
            .layoutWidth(MATCH_PARENT)
            .layoutHeight(320.0f)
            .padding(32.0f, 32.0f, 32.0f, 32.0f)
            .build();

        LinearLayout* heroContent = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        heroContent->setLayoutParams(MATCH_PARENT, MATCH_PARENT);
        heroContent->setGravity(Gravity::CENTER);
        heroContent->setGap(14.0f);

        heroContent->addView(Components::textView()
            .text("OPENGL ES 3.0 / ACCELERATED")
            .textSize(12.0f)
            .alignment(TextAlignment::Center)
            .textColor("primary")
            .build());

        heroContent->addView(Components::textView()
            .text("Centered Card View Container")
            .textSize(26.0f)
            .alignment(TextAlignment::Center)
            .textColor("onSurface")
            .build());

        heroContent->addView(Components::textView()
            .text("All child components inside this card are automatically centered along both horizontal and vertical axes.")
            .textSize(14.0f)
            .alignment(TextAlignment::Center)
            .textColor("onSurfaceVariant")
            .build());

        heroContent->addView(Components::button()
            .filled()
            .text("Explore Interactions")
            .padding(20.0f, 12.0f, 20.0f, 12.0f)
            .setOnClickListener([sb]() {
                sb->show("Hero Card button clicked!", "DISMISS", nullptr, 2.5f);
            })
            .build());

        heroCard->addView(heroContent);
        scrollList->addView(heroCard);

        /* 2. Horizontal Alignment Matrix (Left / Center / Right) */
        scrollList->addView(Components::textView().text("2. Horizontal Alignment Matrix (Left / Center / Right)").textSize(16.0f).textColor("primary").build());

        CardView* hAlignCard = Components::card()
            .type(CardType::Outlined)
            .cornerRadius(20.0f)
            .layoutWidth(MATCH_PARENT)
            .layoutHeight(WRAP_CONTENT)
            .padding(24.0f, 24.0f, 24.0f, 24.0f)
            .build();

        LinearLayout* hAlignCol = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        hAlignCol->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        hAlignCol->setGap(20.0f);

        // Left
        LinearLayout* leftBox = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        leftBox->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        leftBox->setGravity(Gravity::LEFT);
        leftBox->addView(Components::textView().text("Gravity::LEFT").textSize(13.0f).textColor("secondary").build());
        leftBox->addView(Components::button().tonal().text("Aligned to Left").icon("icon:home").build());
        hAlignCol->addView(leftBox);

        // Center
        LinearLayout* centerBox = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        centerBox->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        centerBox->setGravity(Gravity::CENTER_HORIZONTAL);
        centerBox->addView(Components::textView().text("Gravity::CENTER_HORIZONTAL").textSize(13.0f).textColor("secondary").build());
        centerBox->addView(Components::button().filled().text("Aligned to Center").icon("icon:star").build());
        hAlignCol->addView(centerBox);

        // Right
        LinearLayout* rightBox = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        rightBox->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        rightBox->setGravity(Gravity::RIGHT);
        rightBox->addView(Components::textView().text("Gravity::RIGHT").textSize(13.0f).textColor("secondary").build());
        rightBox->addView(Components::button().outlined().text("Aligned to Right").icon("icon:settings").build());
        hAlignCol->addView(rightBox);

        hAlignCard->addView(hAlignCol);
        scrollList->addView(hAlignCard);

        /* 3. Vertical Gravity Alignment (Top / Center / Bottom) */
        scrollList->addView(Components::textView().text("3. Vertical Gravity Alignment (Top / Center / Bottom)").textSize(16.0f).textColor("primary").build());

        LinearLayout* vRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        vRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        vRow->setGap(20.0f);

        // Box 1: TOP
        CardView* topCard = Components::card().type(CardType::Filled).layoutWidth(MATCH_PARENT).layoutHeight(180.0f).cornerRadius(16.0f).build();
        LinearLayout* topCol = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        topCol->setLayoutParams(MATCH_PARENT, MATCH_PARENT);
        topCol->setGravity(Gravity::TOP | Gravity::CENTER_HORIZONTAL);
        topCol->setPadding(12.0f, 16.0f, 12.0f, 16.0f);
        topCol->addView(Components::textView().text("Gravity::TOP").alignment(TextAlignment::Center).textSize(13.0f).textColor("secondary").build());
        Chip* chip = new Chip(ChipType::Filter);
        chip->setText("Chip Button")->setSelected(true);
        topCol->addView(chip);
        topCard->addView(topCol);

        // Box 2: CENTER_VERTICAL
        CardView* centerVCard = Components::card().type(CardType::Filled).layoutWidth(MATCH_PARENT).layoutHeight(180.0f).cornerRadius(16.0f).build();
        LinearLayout* centerVCol = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        centerVCol->setLayoutParams(MATCH_PARENT, MATCH_PARENT);
        centerVCol->setGravity(Gravity::CENTER);
        centerVCol->setPadding(12.0f, 16.0f, 12.0f, 16.0f);
        centerVCol->addView(Components::textView().text("Gravity::CENTER").alignment(TextAlignment::Center).textSize(13.0f).textColor("secondary").build());
        centerVCol->addView(Components::switchButton().checked(true).build());
        centerVCard->addView(centerVCol);

        // Box 3: BOTTOM
        CardView* bottomCard = Components::card().type(CardType::Filled).layoutWidth(MATCH_PARENT).layoutHeight(180.0f).cornerRadius(16.0f).build();
        LinearLayout* bottomCol = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        bottomCol->setLayoutParams(MATCH_PARENT, MATCH_PARENT);
        bottomCol->setGravity(Gravity::BOTTOM | Gravity::CENTER_HORIZONTAL);
        bottomCol->setPadding(12.0f, 16.0f, 12.0f, 16.0f);
        bottomCol->addView(Components::textView().text("Gravity::BOTTOM").alignment(TextAlignment::Center).textSize(13.0f).textColor("secondary").build());
        bottomCol->addView(Components::button().tonal().text("Bottom Action").build());
        bottomCard->addView(bottomCol);

        vRow->addView(topCard);
        vRow->addView(centerVCard);
        vRow->addView(bottomCard);
        scrollList->addView(vRow);

        /* 4. Mixed Centered Form Controls & Widgets */
        scrollList->addView(Components::textView().text("4. Mixed Centered Form Controls & Widgets").textSize(16.0f).textColor("primary").build());

        CardView* controlsCard = Components::card()
            .type(CardType::Outlined)
            .cornerRadius(20.0f)
            .layoutWidth(MATCH_PARENT)
            .layoutHeight(WRAP_CONTENT)
            .padding(24.0f, 24.0f, 24.0f, 24.0f)
            .build();

        LinearLayout* controlsLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        controlsLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        controlsLayout->setGravity(Gravity::CENTER_HORIZONTAL);
        controlsLayout->setGap(20.0f);

        // Centered SearchBar
        controlsLayout->addView(Components::searchBar()
            .hint("Centered Search Bar...")
            .leadingIcon("icon:search")
            .width(420.0f)
            .build());

        // Centered Segmented Buttons
        MaterialSegmentedButton* segBtn = new MaterialSegmentedButton({ "Start", "Pause", "Stop" }, 0);
        controlsLayout->addView(segBtn);

        // Centered Slider
        MediaSlider* centeredSlider = new MediaSlider(0.65f);
        centeredSlider->layout_width = 460.0f;
        centeredSlider->setStyle(SliderStyle::Standard);
        controlsLayout->addView(centeredSlider);

        // Centered Checkboxes & Switch row
        LinearLayout* formRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        formRow->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        formRow->setGap(24.0f);
        formRow->addView(new Checkbox("Auto-sync", true));
        formRow->addView(Components::switchButton().checked(true).build());
        controlsLayout->addView(formRow);

        controlsCard->addView(controlsLayout);
        scrollList->addView(controlsCard);

        scrollView->addView(scrollList);
        return scrollView;
    }

    /**
     * @brief Constructs and binds the main application component tree.
     * 
     * @param app Reference to active ApplicationLooper.
     * @return View* Root ViewGroup containing all initialized UI layers.
     */
    View* compose(ApplicationLooper& app) override {
        ViewGroup* root = new ViewGroup();
        root->setLayoutParams(MATCH_PARENT, MATCH_PARENT);

        screenNotification = new StarrySkyNotification(0, 4.0f); 
        screenNotification->setWindowMode(false); 
        screenNotification->setIcon("./resources/vector.png");
        screenNotification->setFont("./resources/Edition.ttf");

        windowNotification = new StarrySkyNotification(0, 3.5f); 
        windowNotification->setWindowMode(true); 

        starrySkyFormation = new StarrySkyFormation(90.0f);
        starrySkyFormation->x = 1100.0f;
        starrySkyFormation->y = 120.0f;

        StarrySkyFormation* formation = starrySkyFormation;
        StarrySkyNotification* screenNotif = screenNotification;

        starrySkyFormation->setOnClick([formation, screenNotif]() {
            int currentLevel = formation->getLevel();
            if (currentLevel == 0) {
                formation->setLevel(1);
                screenNotif->show(StarryNotifType::Green, "Formation Activated", "Standard energy level engaged.");
            } else {
                formation->setLevel(0);
                screenNotif->show(StarryNotifType::Green, "Formation Inactive", "Energy field restored to standby.");
            }
        });

        starrySkyFormation->setOnDoubleClick([formation, screenNotif]() {
            formation->setLevel(2);
            screenNotif->show(StarryNotifType::Red, "Security Alert", "High-threat interception protocol active.");
        });

        starrySkyFormation->setOnLongClick([formation, screenNotif]() {
            formation->setLevel(0);
            screenNotif->show(StarryNotifType::Green, "System Reset", "Formation parameters reset to default.");
        });

        snackbar = new Snackbar();
        materialDialog = new MaterialDialog();
        materialDialog->setTitle("Apply Configuration Changes?");
        materialDialog->setContent("Are you sure you want to hot-reload the rendering architecture? This will update active color tokens and layout parameters.");
        materialDialog->setLayoutParams(dp(300), View::WRAP_CONTENT);
        notificationOverlay = new NotificationOverlay();
        navigationDrawer = new NavigationDrawer();  

        Snackbar* sb = snackbar;
        NavigationDrawer* drawer = navigationDrawer;
        MaterialDialog* dialog = materialDialog;
        NotificationOverlay* notifOverlay = notificationOverlay;
        StarrySkyNotification* windowNotif = windowNotification;

        drawer->addHeader("Overview");  
        drawer->addItem(0, "Dashboard", "icon:home");  
        drawer->addItem(1, "Activity & Logs", "icon:file");  
        drawer->addItem(2, "Starred Items", "icon:favorite");  

        drawer->addDivider();  

        drawer->addHeader("Data & Connectivity");
        DrawerExpandableGroup* cloudGroup = drawer->addGroup("Cloud Services", true, "icon:cloud");
        cloudGroup->addItem(10, "Auto-Sync", "./resources/vector.png");
        cloudGroup->addItem(11, "Incident Monitor", "icon:warning");
        cloudGroup->addItem(12, "Offline Cache", "icon:download");

        drawer->addDivider();

        DrawerExpandableGroup* securityGroup = drawer->addGroup("Security & Access", false, "icon:lock");
        securityGroup->addItem(20, "API Credentials", "icon:diamond");
        securityGroup->addItem(21, "Terminal Console", "icon:terminal");
        securityGroup->addItem(22, "Audit Trail", "icon:history");

        drawer->addDivider();

        drawer->addHeader("Preferences");  
        drawer->addItem(3, "Account Settings", "icon:person");  
        drawer->addItem(4, "Toggle Dark Theme", "icon:settings");  

        drawer->setOnItemSelected([drawer, sb](int itemId) {  
            switch (itemId) {
            case 4: {  
                static bool isDark = false;   
                isDark = !isDark;  
                if (isDark) ThemeManager::setTheme(MaterialColorScheme::OCEAN_DARK);  
                else ThemeManager::setTheme(MaterialColorScheme::OCEAN_LIGHT);  
                sb->show("Theme updated successfully", "DISMISS", nullptr, 3.5f);  
                break;
            }
            case 11: {
                sb->show("Incident Monitor: All services operational", "DETAILS", nullptr, 3.5f);
                break;
            }
            case 21: {
                sb->show("Secure Shell session connected", "DISMISS", nullptr, 3.0f);
                break;
            }
            default: {
                sb->show("Navigated to destination item #" + std::to_string(itemId), "VIEW", nullptr, 3.0f);  
                break;
            }
            }
            drawer->close();  
        });

        dialog->setActions(
            "Cancel", [sb]() { sb->show("Action cancelled", "", nullptr, 2.0f); },
            "Confirm", [sb]() { sb->show("Configuration applied successfully", "UNDO", nullptr, 4.0f); }
        );
        dialog->setAnimationStyle(DialogAnimationStyle::FadeZoom);

        CardView* rootCard = Components::card()
                                .layoutWidth(MATCH_PARENT)
                                .layoutHeight(MATCH_PARENT)
                                .interactive(false)
                                .cornerRadius(0.0f)
                                .margins(0.0f, 0.0f, 0.0f, 0.0f)
                                .padding(20.0f, 20.0f, 20.0f, 20.0f)
                                .build();

        /* Page 1: Main Component Catalog (Dashboard) */
        ScrollView* homeScrollView = Components::scrollView();
        homeScrollView->setLayoutParams(MATCH_PARENT, MATCH_PARENT);

        LinearLayout* scrollList = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        scrollList->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        scrollList->setGap(36.0f);
        scrollList->setPadding(12.0f, 12.0f, 12.0f, 24.0f);

        LinearLayout* headerLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        headerLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        headerLayout->setGap(8.0f);
        headerLayout->addView(Components::textView().text("Material 3 Component Catalog").textSize(40.0f).textColor("primary").build());
        headerLayout->addView(Components::textView().text("Expressive, accessible, and adaptive UI components rendered with OpenGL ES 3.0").textSize(13.0f).textColor("secondary").build());
        scrollList->addView(headerLayout);

        View* title0 = Components::textView().text("0. Dialogs, Drawers & System Feedback").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title0);
        
        LinearLayout* systemRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        systemRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        systemRow->setGap(14.0f);

        Button* asyncButton = Components::button().tonal().text("Click Me").build();
        asyncButton->setOnClickListener([this, sb]() {
            sb->show("Asynchronous thread started, click Ok to start writing JSON!", "Ok", [sb](){
                ApplicationLooper::runAsyncThread([sb]() {
                    JSONObject authorJson("");
                    authorJson.put("@Repository", "https://github.com/Vectorted")
                              .put("@Author", "Vectorted");
                    Io::write("./Author.json", JSON::toJson(authorJson, JsonFormat::Tab));

                    Network::OKHTTP::Response networkResponse = Network::OKHTTP::HTTPS::get("https://baidu.com").execute();
                    if (networkResponse.isSuccessful()) {
                        Io::write("./baidu.com.html", networkResponse.string());

                        ApplicationLooper::runOnUiThread([sb]() {
                            sb->show("Network data request successful!");
                        });
                    }
                });
            }, 3.0f);
        });
        systemRow->addView(asyncButton);

        systemRow->addView(Components::button().tonal().text("Show Snackbar").padding(14.0f, 14.0f, 14.0f, 14.0f).setOnClickListener([sb]() { 
            sb->show("Message sent to recipient", "ACTION", nullptr, 3.5f); 
        }).build());
        systemRow->addView(Components::button().filled().text("Open Drawer").icon("icon:menu").iconSize(18.0f).padding(14.0f, 14.0f, 14.0f, 14.0f).setOnClickListener([drawer]() { 
            drawer->open(); 
        }).build());
        systemRow->addView(Components::button().outlined().text("Open Dialog").iconSize(18.0f).padding(14.0f, 14.0f, 14.0f, 14.0f).setOnClickListener([dialog]() { 
            dialog->show(); 
        }).build());
        systemRow->addView(Components::button().elevated().text("Post Notification").icon("icon:add").iconSize(18.0f).padding(14.0f, 14.0f, 14.0f, 14.0f).setOnClickListener([notifOverlay]() {
            static int noticeSequence = 1; 
            notifOverlay->push("System Notice #" + std::to_string(noticeSequence++), "Background synchronization finished successfully.", 4.5f);
        }).build());
        systemRow->addView(Components::button().outlined().text("Post Alert").icon("icon:info").iconSize(18.0f).padding(14.0f, 14.0f, 14.0f, 14.0f).setOnClickListener([notifOverlay]() {
            notifOverlay->pushError("Warning Detected", "High memory consumption reported.", 6.0f);
        }).build());
        scrollList->addView(systemRow);

        View* globalTitle = Components::textView().text("Global Desktop Screen Overlays (Notification Roles):").textSize(14.0f).textColor("onSurfaceVariant").build();
        scrollList->addView(globalTitle);

        LinearLayout* starryRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        starryRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        starryRow->setGap(10.0f);

        starryRow->addView(Components::button().filled().text("Success").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Green, "Device Verified", "Security check completed with zero issues."); 
        }).build());
        starryRow->addView(Components::button().outlined().text("Critical Alert").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Red, "Security Alert", "Unusual sign-in attempt intercepted."); 
        }).build());
        starryRow->addView(Components::button().tonal().text("Info").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Blue, "Update Available", "A new system patch is ready to install."); 
        }).build());
        starryRow->addView(Components::button().elevated().text("Expressive").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Pink, "Special Offer", "Custom seasonal theme unlocked."); 
        }).build());
        starryRow->addView(Components::button().outlined().text("Neutral").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::White, "Clipboard Synced", "Copied text synced across devices."); 
        }).build());
        starryRow->addView(Components::button().filled().text("Primary Role").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Primary, "Account Connected", "Linked with active primary credentials."); 
        }).build());
        starryRow->addView(Components::button().tonal().text("Secondary Role").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Secondary, "Sync Finished", "Secondary token synchronized."); 
        }).build());
        starryRow->addView(Components::button().elevated().text("Tertiary Role").setOnClickListener([screenNotif]() { 
            screenNotif->show(StarryNotifType::Tertiary, "Backup Created", "Cloud snapshot uploaded."); 
        }).build());

        scrollList->addView(starryRow);

        View* inAppTitle = Components::textView().text("0.1 In-App Floating Banners (Dynamic Queue)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(inAppTitle);

        CardView* inAppCard = Components::card()
            .type(CardType::Outlined)
            .layoutWidth(MATCH_PARENT)
            .layoutHeight(WRAP_CONTENT)
            .cornerRadius(20.0f)
            .padding(20.0f, 20.0f, 20.0f, 20.0f)
            .build();

        LinearLayout* inAppLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        inAppLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        inAppLayout->setGap(14.0f);
        inAppLayout->addView(Components::textView().text("In-App Notification Queue").textSize(18.0f).textColor("primary").build());
        inAppLayout->addView(Components::textView().text("Trigger real-time floating banners with kinetic vertical slot re-ordering and smooth exit physics.").textSize(13.0f).textColor("secondary").build());
        
        LinearLayout* inAppRow1 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        inAppRow1->setGap(10.0f);
        inAppRow1->addView(Components::button().filled().text("Success").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Green, "Changes Saved", "Your workspace settings have been updated."); 
        }).build());
        inAppRow1->addView(Components::button().outlined().text("Alert").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Red, "Connection Lost", "Unable to reach remote host server."); 
        }).build());
        inAppRow1->addView(Components::button().tonal().text("Stream").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Blue, "Downloading resources", "Receiving texture atlas payload..."); 
        }).build());
        inAppRow1->addView(Components::button().elevated().text("Accent").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Pink, "New Highlight", "Featured showcase item ready to view."); 
        }).build());

        LinearLayout* inAppRow2 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        inAppRow2->setGap(10.0f);
        inAppRow2->addView(Components::button().outlined().text("Neutral").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::White, "Item Archived", "Moved selected item to archive folder."); 
        }).build());
        inAppRow2->addView(Components::button().filled().text("Primary").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Primary, "Primary Event", "User authenticated with master key."); 
        }).build());
        inAppRow2->addView(Components::button().tonal().text("Secondary").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Secondary, "Secondary Event", "Background worker finished batch job."); 
        }).build());
        inAppRow2->addView(Components::button().elevated().text("Tertiary").setOnClickListener([windowNotif]() { 
            windowNotif->show(StarryNotifType::Tertiary, "Tertiary Event", "Cached resources refreshed."); 
        }).build());

        inAppLayout->addView(inAppRow1);
        inAppLayout->addView(inAppRow2);
        inAppCard->addView(inAppLayout);
        scrollList->addView(inAppCard);

        View* themeTitle = Components::textView().text("0.2 Dynamic Color Schemes & Theming").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(themeTitle);

        CardView* themeCard = Components::card()
            .type(CardType::Outlined)
            .layoutWidth(MATCH_PARENT)
            .layoutHeight(WRAP_CONTENT)
            .cornerRadius(20.0f)
            .padding(20.0f, 20.0f, 20.0f, 20.0f)
            .build();

        LinearLayout* themeLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        themeLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        themeLayout->setGap(16.0f);
        themeLayout->addView(Components::textView().text("Material Baseline & Expressive Color Roles").textSize(18.0f).textColor("primary").build());
        themeLayout->addView(Components::textView().text("Select a color palette below to dynamically transform all component tokens across the application.").textSize(13.0f).textColor("secondary").build());

        themeLayout->addView(Components::textView().text("Light Palettes:").textSize(14.0f).textColor("onSurfaceVariant").build());
        LinearLayout* lightThemeRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        lightThemeRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        lightThemeRow->setGap(12.0f);

        lightThemeRow->addView(Components::button().filled().text("Baseline Purple").icon("icon:settings").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::BASELINE_LIGHT);
            sb->show("Color scheme -> Baseline Light (Purple)", "OK", nullptr, 2.5f);
        }).build());

        lightThemeRow->addView(Components::button().tonal().text("Forest Emerald").icon("icon:check").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::FOREST_LIGHT);
            sb->show("Color scheme -> Forest Light (Emerald)", "OK", nullptr, 2.5f);
        }).build());

        lightThemeRow->addView(Components::button().elevated().text("Terracotta Clay").icon("icon:star").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::TERRACOTTA_LIGHT);
            sb->show("Color scheme -> Terracotta Light (Clay)", "OK", nullptr, 2.5f);
        }).build());

        lightThemeRow->addView(Components::button().outlined().text("Ocean Teal").icon("icon:home").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::OCEAN_LIGHT);
            sb->show("Color scheme -> Ocean Light (Teal)", "OK", nullptr, 2.5f);
        }).build());

        themeLayout->addView(lightThemeRow);

        themeLayout->addView(Components::textView().text("Dark Palettes:").textSize(14.0f).textColor("onSurfaceVariant").build());
        LinearLayout* darkThemeRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        darkThemeRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        darkThemeRow->setGap(12.0f);

        darkThemeRow->addView(Components::button().filled().text("Baseline Dark").icon("icon:settings").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::BASELINE_DARK);
            sb->show("Color scheme -> Baseline Dark (Violet)", "OK", nullptr, 2.5f);
        }).build());

        darkThemeRow->addView(Components::button().tonal().text("Forest Dark").icon("icon:check").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::FOREST_DARK);
            sb->show("Color scheme -> Forest Dark (Deep Green)", "OK", nullptr, 2.5f);
        }).build());

        darkThemeRow->addView(Components::button().elevated().text("Terracotta Dark").icon("icon:star").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::TERRACOTTA_DARK);
            sb->show("Color scheme -> Terracotta Dark (Warm Coral)", "OK", nullptr, 2.5f);
        }).build());

        darkThemeRow->addView(Components::button().outlined().text("Ocean Dark").icon("icon:home").iconSize(16.0f).setOnClickListener([sb]() {
            ThemeManager::setTheme(MaterialColorScheme::OCEAN_DARK);
            sb->show("Color scheme -> Ocean Dark (Midnight)", "OK", nullptr, 2.5f);
        }).build());

        themeLayout->addView(darkThemeRow);

        themeLayout->addView(Components::textView().text("Dynamic Color Role Previews:").textSize(14.0f).textColor("onSurfaceVariant").build());
        LinearLayout* previewRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        previewRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        previewRow->setGap(12.0f);

        previewRow->addView((new Chip(ChipType::Assist, "Primary Role"))->setLeadingIcon("icon:star"));
        previewRow->addView((new Chip(ChipType::Filter, "Secondary Role"))->setSelected(true));
        previewRow->addView((new Chip(ChipType::Suggestion, "Tertiary Role")));
        previewRow->addView(Components::switchButton().checked(true).build());

        themeLayout->addView(previewRow);
        themeCard->addView(themeLayout);
        scrollList->addView(themeCard);

        View* title1 = Components::textView().text("1. Text Fields (Outlined, Filled & Underlined)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title1);
        
        LinearLayout* textFieldRow1 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        textFieldRow1->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        textFieldRow1->setGap(24.0f);
        textFieldRow1->addView(Components::textField().label("Email Address").hint("user@example.com").style(TextFieldStyle::Outlined).leadingIcon("icon:search").clearIcon("icon:close").width(260.0f).build());
        textFieldRow1->addView(Components::textField().label("Display Name").hint("Enter your name").style(TextFieldStyle::Filled).trailingIcon("icon:settings").width(260.0f).build());
        textFieldRow1->addView(Components::textField().label("Document Title").text("Project Roadmap 2026").style(TextFieldStyle::Underlined).width(260.0f).build());
        scrollList->addView(textFieldRow1);
        
        LinearLayout* textFieldRow2 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        textFieldRow2->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        textFieldRow2->setGap(24.0f);
        textFieldRow2->addView(Components::textField().label("Password").text("secret_pass123").style(TextFieldStyle::Outlined).password(true).leadingIcon("icon:lock").width(260.0f).build());
        textFieldRow2->addView(Components::textField().label("Username").text("invalid#char!").style(TextFieldStyle::Filled).error("Invalid characters detected").leadingIcon("icon:error").width(260.0f).build());
        textFieldRow2->addView(Components::textField().label("Organization ID").text("ORG-8842-CORP").readOnly(true).style(TextFieldStyle::Outlined).width(260.0f).build());
        scrollList->addView(textFieldRow2);

        View* title2 = Components::textView().text("2. Cards (Elevated, Filled & Outlined)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title2);
        
        LinearLayout* cardRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        cardRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        cardRow->setGap(24.0f);
        
        CardView* elevatedCard = Components::card().type(CardType::Elevated).interactive(true).cornerRadius(16.0f).layoutWidth(MATCH_PARENT).layoutHeight(150.0f).padding(20.0f, 20.0f, 20.0f, 20.0f).build();
        LinearLayout* inCardLayoutA = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        inCardLayoutA->setLayoutParams(MATCH_PARENT, MATCH_PARENT); 
        inCardLayoutA->setGap(12.0f);
        inCardLayoutA->addView(Components::textView().text("Elevated Card").textSize(15.0f).build()); 
        inCardLayoutA->addView(Components::button().tonal().text("Explore Now").build()); 
        elevatedCard->addView(inCardLayoutA);
        
        CardView* filledCard = Components::card().type(CardType::Filled).interactive(false).cornerRadius(16.0f).layoutWidth(MATCH_PARENT).layoutHeight(150.0f).padding(20.0f, 20.0f, 20.0f, 20.0f).build();
        LinearLayout* inCardLayoutB = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        inCardLayoutB->setLayoutParams(MATCH_PARENT, MATCH_PARENT); 
        inCardLayoutB->setGap(16.0f);
        inCardLayoutB->addView(Components::textView().text("Filled Card").textSize(15.0f).build()); 
        inCardLayoutB->addView(Components::switchButton().checked(true).build()); 
        filledCard->addView(inCardLayoutB);
        
        CardView* outlinedCard = Components::card().type(CardType::Outlined).interactive(false).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(150.0f).padding(20.0f, 20.0f, 20.0f, 20.0f).build();
        LinearLayout* inCardLayoutC = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        inCardLayoutC->setLayoutParams(MATCH_PARENT, MATCH_PARENT); 
        inCardLayoutC->setGap(18.0f);
        inCardLayoutC->addView(Components::textView().text("Outlined Card").textSize(15.0f).build()); 
        inCardLayoutC->addView(Components::skeleton().size(160.0f, 14.0f).build()); 
        outlinedCard->addView(inCardLayoutC);
        
        cardRow->addView(elevatedCard); 
        cardRow->addView(filledCard); 
        cardRow->addView(outlinedCard); 
        scrollList->addView(cardRow);

        View* title3 = Components::textView().text("3. Selection Controls — Switches").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title3);
        
        LinearLayout* switchRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        switchRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        switchRow->setGap(28.0f);
        switchRow->addView(Components::switchButton().checked(false).build());
        switchRow->addView(Components::switchButton().checked(true).hasIcon(true).build());
        switchRow->addView(Components::switchButton().themeSelector(true).checked(false).setOnCheckedChangeListener([sb](bool isDark) {
            if (isDark) ThemeManager::setTheme(MaterialColorScheme::TERRACOTTA_LIGHT); 
            else ThemeManager::setTheme(MaterialColorScheme::BASELINE_LIGHT);
            sb->show("Theme updated", "", nullptr, 3.5f);
        }).build());
        switchRow->addView(Components::switchButton().checked(true).checkedIcon("./resources/vector.png").uncheckedIcon("icon:close").iconSize(14.0f).build());
        switchRow->addView(Components::switchButton().checked(true).build());
        switchRow->addView(Components::switchButton().checked(false).build());
        scrollList->addView(switchRow);

        View* title4 = Components::textView().text("4. Common Buttons (Filled, Tonal, Elevated, Outlined & Text)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title4);
        
        LinearLayout* buttonRow1 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        buttonRow1->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        buttonRow1->setGap(16.0f);
        buttonRow1->addView(Components::button().filled().text("Filled Button").textSize(15.0f).padding(16.0f, 16.0f, 16.0f, 16.0f).build());
        buttonRow1->addView(Components::button().tonal().text("Tonal Button").icon("icon:settings").iconSize(16.0f).build());
        buttonRow1->addView(Components::button().elevated().text("Elevated Button").icon("./resources/vector.png").iconSize(18.0f).build());
        scrollList->addView(buttonRow1);
        
        LinearLayout* buttonRow2 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        buttonRow2->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        buttonRow2->setGap(16.0f);
        buttonRow2->addView(Components::button().outlined().text("Outlined Button").icon("icon:edit").iconSize(18.0f).build());
        buttonRow2->addView(Components::button().text().text("Text Button").icon("icon:check").iconSize(16.0f).build());
        buttonRow2->addView(Components::button().filled().text("Fixed Dimension").width(180.0f).height(48.0f).build());
        scrollList->addView(buttonRow2);
        
        LinearLayout* buttonRow3 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        buttonRow3->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        buttonRow3->setGap(16.0f);
        buttonRow3->addView(Components::button().filled().text("Disabled Filled").icon("icon:close").build());
        buttonRow3->addView(Components::button().outlined().text("Disabled Outlined").build());
        buttonRow3->addView(Components::button().tonal().text("Disabled Tonal").build());
        scrollList->addView(buttonRow3);

        View* title5 = Components::textView().text("5. Floating Action Buttons (FAB & Speed Dial)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title5);

        scrollList->addView(Components::textView().text("Standard Container Shapes (Squircle):").textSize(14.0f).textColor("onSurfaceVariant").build());
        LinearLayout* fabRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        fabRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        fabRow->setGap(35.0f);
        fabRow->addView(Components::fabButton().size(FabSize::Small).type(FabType::Secondary).icon("icon:search").iconSize(20.0f).build());
        fabRow->addView(Components::fabButton().size(FabSize::Standard).type(FabType::Primary).icon("icon:add").build());
        fabRow->addView(Components::fabButton().size(FabSize::Large).type(FabType::Surface).icon("icon:home").iconSize(32.0f).build());
        fabRow->addView(Components::fabButton().size(FabSize::Extended).type(FabType::Primary).icon("./resources/vector.png").iconSize(22.0f).text("Extended Action").build());
        scrollList->addView(fabRow);

        scrollList->addView(Components::textView().text("Circular Shapes (FabShape::Circle):").textSize(14.0f).textColor("onSurfaceVariant").build());
        LinearLayout* circleFabRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        circleFabRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        circleFabRow->setGap(35.0f);
        circleFabRow->addView(Components::fabButton().shape(FabShape::Circle).size(FabSize::Small).type(FabType::Secondary).icon("icon:search").iconSize(20.0f).build());
        circleFabRow->addView(Components::fabButton().shape(FabShape::Circle).size(FabSize::Standard).type(FabType::Primary).icon("icon:add").build());
        circleFabRow->addView(Components::fabButton().shape(FabShape::Circle).size(FabSize::Large).type(FabType::Surface).icon("./resources/vector.png").iconSize(32.0f).build());
        circleFabRow->addView(Components::fabButton().shape(FabShape::Circle).size(FabSize::Extended).type(FabType::Secondary).icon("icon:star").iconSize(22.0f).text("Circle Extended").build());
        scrollList->addView(circleFabRow);

        scrollList->addView(Components::textView().text("Interactive Speed Dial (Pop-up Actions):").textSize(14.0f).textColor("onSurfaceVariant").build());
        LinearLayout* speedDialRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        speedDialRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        speedDialRow->setPadding(0.0f, 10.0f, 0.0f, 10.0f); 

        FloatingActionButton* speedDialRight = Components::fabButton()
            .shape(FabShape::Container)
            .size(FabSize::Standard)
            .type(FabType::Secondary)
            .icon("icon:edit")
            .expandDirection(ExpandDirection::Right)
            .showTooltips(false)
            .addSubItem("New Note", "icon:edit", [sb]() { sb->show("Created note draft", "OK", nullptr, 2.5f); }, false, FabType::Secondary)
            .addSubItem("Attach Media", "./resources/vector.png", [sb]() { sb->show("Media attachment added", "OK", nullptr, 2.5f); }, false, FabType::Primary)
            .build();

        speedDialRow->addView(speedDialRight);
        scrollList->addView(speedDialRow);

        View* title6 = Components::textView().text("6. Progress Indicators & Typography Styles").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title6);
        
        LinearLayout* utilRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        utilRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        utilRow->setGap(64.0f);
        
        LinearLayout* skeletonCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        skeletonCol->layout_width = 300.0f; 
        skeletonCol->layout_height = WRAP_CONTENT; 
        skeletonCol->setGap(14.0f);
        skeletonCol->addView(Components::skeleton().size(280.0f, 22.0f).build());
        skeletonCol->addView(Components::skeleton().size(220.0f, 14.0f).build());
        skeletonCol->addView(Components::skeleton().size(180.0f, 12.0f).build());
        
        LinearLayout* typographyCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        typographyCol->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        typographyCol->setGap(8.0f);
        typographyCol->addView(Components::textView().text("[ Compact Density — 1.0x ]\n[rgb(0,150,136)] High-density data presentation\n[rgb(0,150,136)] Optimized for analytical dashboards\n[rgb(0,150,136)] Consistent vertical rhythm").lineSpacing(-4.0f, 1.0f).textColor("rgb(0, 150, 136)").build());
        typographyCol->addView(Components::textView().text("[ Expressive Body — 1.8x ]\n+ Elegant editorial reading experience\n+ Generous line spacing for long-form content\n+ Adaptive typeface baseline alignment").lineSpacing(0.0f, 1.8f).textColor("secondary").build());
        
        utilRow->addView(skeletonCol); 
        utilRow->addView(typographyCol); 
        scrollList->addView(utilRow);

        View* title7 = Components::textView().text("7. Segmented Buttons (Single & Multi-Select)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title7);
        
        CardView* segmentCard = Components::card().type(CardType::Filled).interactive(false).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).padding(24.0f, 24.0f, 24.0f, 24.0f).build();
        LinearLayout* segmentRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        segmentRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        segmentRow->setGap(48.0f);
        
        LinearLayout* segCol1 = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        segCol1->setGap(16.0f);
        segCol1->addView(Components::textView().text("Outlined Style").textSize(14.0f).textColor("onSurfaceVariant").build());
        MaterialSegmentedButton* outlinedSegment = new MaterialSegmentedButton({ "Attending", "Declined", "Maybe" }, 0);
        outlinedSegment->setOnSelectionChanged([sb](int index) { 
            std::string options[] = { "Attending", "Declined", "Maybe" }; 
            sb->show("RSVP status: " + options[index], "UNDO", nullptr, 2.5f); 
        });
        segCol1->addView(outlinedSegment);
        
        LinearLayout* segCol2 = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        segCol2->setGap(16.0f);
        segCol2->addView(Components::textView().text("Multi-Select Tonal").textSize(14.0f).textColor("onSurfaceVariant").build());
        MaterialSegmentedButton* tonalSegment = new MaterialSegmentedButton({ "Wi-Fi", "Bluetooth", "Location" }, std::vector<bool>{true, false, true});
        tonalSegment->setOnMultiSelectionChanged([sb](const std::vector<bool>& states) {
            std::string displayText = "Active Sensors: ";
            if (states[0]) displayText += "[Wi-Fi] "; 
            if (states[1]) displayText += "[Bluetooth] "; 
            if (states[2]) displayText += "[Location]";
            if (!states[0] && !states[1] && !states[2]) displayText += "None";
            sb->show(displayText, "LOG", nullptr, 2.5f);
        });
        segCol2->addView(tonalSegment);
        
        LinearLayout* segCol3 = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        segCol3->setGap(16.0f);
        segCol3->addView(Components::textView().text("Single-Select Range").textSize(14.0f).textColor("onSurfaceVariant").build());
        MaterialSegmentedButton* rangeSegment = new MaterialSegmentedButton({ "Day", "Week", "Month", "Year" }, 1);
        rangeSegment->setOnSelectionChanged([sb](int index) { 
            std::string options[] = { "Day", "Week", "Month", "Year" }; 
            sb->show("Timeline view: " + options[index], "RELOAD", nullptr, 2.5f); 
        });
        segCol3->addView(rangeSegment);
        
        segmentRow->addView(segCol1); 
        segmentRow->addView(segCol2); 
        segmentRow->addView(segCol3); 
        segmentCard->addView(segmentRow); 
        scrollList->addView(segmentCard);

        View* title7_1 = Components::textView().text("7.1 Primary Tabs (Active Indicator Animation)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title7_1);

        CardView* tabCard = Components::card().type(CardType::Filled).interactive(false).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).padding(24.0f, 24.0f, 24.0f, 24.0f).build();
        LinearLayout* tabLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        tabLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        tabLayout->setGap(16.0f);
        tabLayout->addView(Components::textView().text("Primary Tabs with Smooth Sliding & Scaling Line Indicator").textSize(14.0f).textColor("onSurfaceVariant").build());

        MaterialTabRow* primaryTabRow = new MaterialTabRow();
        primaryTabRow->addTab("Overview");
        primaryTabRow->addTab("Repositories");
        primaryTabRow->addTab("Favorites");
        primaryTabRow->addTab("Settings");

        primaryTabRow->setOnTabSelectedListener([sb](size_t index) {
            std::string tabTitles[] = { "Overview", "Repositories", "Favorites", "Settings" };
            sb->show("Switched to tab: " + tabTitles[index], "OK", nullptr, 2.0f);
        });

        tabLayout->addView(primaryTabRow);
        tabCard->addView(tabLayout);
        scrollList->addView(tabCard);

        View* title8 = Components::textView().text("8. Chips (Assist, Filter, Input & Suggestion)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title8);
        
        LinearLayout* chipCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        chipCol->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chipCol->setGap(16.0f);
        
        LinearLayout* chipRow1 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        chipRow1->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chipRow1->setGap(12.0f);
        chipRow1->addView((new Chip(ChipType::Assist, "Deploy Package"))->setLeadingIcon("icon:home")->setOnClickListener([sb]() { 
            sb->show("Package queued for deployment", "VIEW", nullptr, 3.0f); 
        }));
        chipRow1->addView((new Chip(ChipType::Assist, "Add Reminder"))->setElevated(true)->setLeadingIcon("icon:settings"));
        chipRow1->addView((new Chip(ChipType::Suggestion, "Suggested completion..."))->setOnClickListener([sb]() { 
            sb->show("Suggestion inserted into prompt", "DISCARD", nullptr, 4.0f); 
        }));
        chipCol->addView(chipRow1);
        
        LinearLayout* chipRow2 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        chipRow2->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chipRow2->setGap(12.0f);
        chipRow2->addView((new Chip(ChipType::Filter, "OpenGL ES"))->setSelected(true)->setOnSelectedChangeListener([sb](bool) { sb->show("Graphics API filter changed", "", nullptr, 2.0f); }));
        chipRow2->addView((new Chip(ChipType::Filter, "Vulkan SDK"))->setSelected(false)->setOnSelectedChangeListener([](bool) {}));
        chipRow2->addView((new Chip(ChipType::Filter, "DirectX 12"))->setSelected(false)->setElevated(true));
        chipCol->addView(chipRow2);
        
        LinearLayout* chipRow3 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        chipRow3->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chipRow3->setGap(12.0f);
        Chip* inputTag1 = new Chip(ChipType::Input, "Administrator"); 
        inputTag1->setLeadingIcon("icon:person"); 
        inputTag1->setOnTrailingIconClickListener([inputTag1, chipRow3, sb]() { 
            chipRow3->removeView(inputTag1); 
            sb->show("Role removed", "UNDO", nullptr, 3.0f); 
        });
        Chip* inputTag2 = new Chip(ChipType::Input, "Release v3.2"); 
        inputTag2->setLeadingIcon("icon:file"); 
        inputTag2->setOnTrailingIconClickListener([inputTag2, chipRow3, sb]() { 
            chipRow3->removeView(inputTag2); 
        });
        chipRow3->addView(inputTag1); 
        chipRow3->addView(inputTag2); 
        chipCol->addView(chipRow3); 
        scrollList->addView(chipCol);

        View* title9 = Components::textView().text("9. Sliders & Continuous Progress Indicators").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title9);
        
        CardView* sliderCard = Components::card()
            .type(CardType::Outlined)
            .cornerRadius(20.0f)
            .layoutWidth(MATCH_PARENT)
            .layoutHeight(WRAP_CONTENT)
            .padding(24.0f, 24.0f, 24.0f, 24.0f)
            .build();

        LinearLayout* sliderCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        sliderCol->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        sliderCol->setGap(20.0f); 

        sliderCol->addView(Components::textView().text("Material 3 Capsule Media Slider (with Gap, Handle Bar & Time Labels):").textSize(13.0f).textColor("secondary").build());
        MediaSlider* m3CapsuleTimeSlider = new MediaSlider(0.72f);
        m3CapsuleTimeSlider->layout_width = MATCH_PARENT;
        m3CapsuleTimeSlider->setStyle(SliderStyle::Thick)
                           ->setShowTime(true)
                           ->setTime(180.0f, 202.0f) // 3:00 / 3:22
                           ->setOnValueChanged([sb](float val) {
                               int cur = static_cast<int>(val * 202.0f);
                               char buf[32];
                               std::snprintf(buf, sizeof(buf), "Seek: %d:%02d / 3:22", cur / 60, cur % 60);
                               sb->show(buf, "SEEK", nullptr, 1.5f);
                           });
        sliderCol->addView(m3CapsuleTimeSlider);

        sliderCol->addView(Components::textView().text("Material 3 Capsule Slider (Discrete Handle & Gap):").textSize(13.0f).textColor("secondary").build());
        MediaSlider* m3CapsuleSlider = new MediaSlider(0.48f);
        m3CapsuleSlider->layout_width = MATCH_PARENT;
        m3CapsuleSlider->setStyle(SliderStyle::Thick);
        sliderCol->addView(m3CapsuleSlider);

        sliderCol->addView(Components::textView().text("Animated Squiggly Waveform Slider (Media Playback Active):").textSize(13.0f).textColor("secondary").build());
        MediaSlider* waveSlider = new MediaSlider(0.65f);     
        waveSlider->layout_width = MATCH_PARENT; 
        waveSlider->setStyle(SliderStyle::Squiggly); 
        waveSlider->setPlaying(true);
        sliderCol->addView(waveSlider);

        sliderCol->addView(Components::textView().text("Standard Continuous Slider (Thin Track + Circular Grab Thumb):").textSize(13.0f).textColor("secondary").build());
        MediaSlider* standardSlider = new MediaSlider(0.85f); 
        standardSlider->layout_width = MATCH_PARENT; 
        standardSlider->setStyle(SliderStyle::Standard);
        sliderCol->addView(standardSlider);

        sliderCol->addView(Components::textView().text("Material 3 Discrete Slider (Tick Marks + Snap to Steps + Value Tooltip):").textSize(13.0f).textColor("secondary").build());
        MediaSlider* discreteSlider = new MediaSlider(0.4f);
        discreteSlider->layout_width = MATCH_PARENT;
        discreteSlider->setStyle(SliderStyle::Thick)
                      ->setDiscreteConfig(5, true, true, true)
                      ->setOnValueChanged([sb](float val) {
                          int step = static_cast<int>(std::round(val * 5.0f));
                          sb->show("Step selected: " + std::to_string(step) + " / 5", "OK", nullptr, 1.5f);
                      });
        sliderCol->addView(discreteSlider);

        sliderCol->addView(Components::textView().text("Linear Progress Indicator (Read-only):").textSize(13.0f).textColor("secondary").build());
        MediaSlider* linearProgress = new MediaSlider(0.55f); 
        linearProgress->layout_width = MATCH_PARENT; 
        linearProgress->setStyle(SliderStyle::LinearProgress);
        sliderCol->addView(linearProgress);

        sliderCol->addView(Components::textView().text("Disabled Slider:").textSize(13.0f).textColor("secondary").build());
        MediaSlider* disabledSlider = new MediaSlider(0.35f); 
        disabledSlider->layout_width = MATCH_PARENT; 
        disabledSlider->setStyle(SliderStyle::Thick); 
        disabledSlider->setEnabled(false);
        sliderCol->addView(disabledSlider);

        sliderCard->addView(sliderCol);
        scrollList->addView(sliderCard);

        View* title10 = Components::textView().text("10. Virtualized Recycler List View (Extreme Node Pool)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title10);
        
        CardView* virtualListCard = Components::card().type(CardType::Elevated).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(384.0f).padding(12.0f, 12.0f, 12.0f, 12.0f).build();
        ListView* virtualListView = new ListView(
            1000000000,
            []() -> ListItem* { 
                return (new ListItem(""))->setLeadingIcon("icon:info")->setTrailingIcon("icon:add")->setInteractive(true); 
            },
            [sb](ListItem* item, auto position) {
                item->setHeadline("Virtualized Stream Record #" + std::to_string((long long)position + 1))
                    ->setSupportingText("Constant O(1) memory allocation footprint with high-performance native scissor clip.")
                    ->setTrailingText("ID " + std::to_string((long long)position + 1))
                    ->setOnClickListener([sb, position]() {
                        sb->show("Selected stream record #" + std::to_string((long long)position + 1), "DETAILS", nullptr, 2.5f);
                    });
            }
        );
        virtualListView->layout_width = MATCH_PARENT; 
        virtualListView->layout_height = 360.0f;
        virtualListCard->addView(virtualListView); 
        scrollList->addView(virtualListCard);

        View* title11 = Components::textView().text("11. Pickers (Time Dial & Date Calendar)").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title11);
        
        LinearLayout* pickersRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        pickersRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        pickersRow->setGap(64.0f); 
        pickersRow->setPadding(24.0f, 12.0f, 12.0f, 24.0f);
        
        TimePickerDial* timePicker = new TimePickerDial(260.0f, 360.0f); 
        timePicker->mode = TimePickerMode::Hour; 
        timePicker->selectedHour = 10; 
        timePicker->selectedMinute = 30;
        timePicker->onTimeSelected = [sb](int hour, int minute) {
            std::string hourStr = std::to_string(hour); 
            std::string minuteStr = (minute < 10 ? "0" : "") + std::to_string(minute); 
            sb->show("Time selected: " + hourStr + ":" + minuteStr, "OK", nullptr, 2.5f);
        };
        
        LinearLayout* timeControlCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        timeControlCol->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT); 
        timeControlCol->setGap(20.0f);
        timeControlCol->addView(Components::textView().text("Time Selection").textSize(24.0f).textColor("primary").build());
        timeControlCol->addView(Components::textView().text("Interactive dual-mode dial with hour and minute views.").textSize(13.0f).textColor("secondary").build());
        
        LinearLayout* toggleHourMinuteRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        toggleHourMinuteRow->setGap(12.0f);
        toggleHourMinuteRow->addView(Components::button().tonal().text("Select Hour").setOnClickListener([timePicker]() { timePicker->mode = TimePickerMode::Hour; }).build());
        toggleHourMinuteRow->addView(Components::button().outlined().text("Select Minute").setOnClickListener([timePicker]() { timePicker->mode = TimePickerMode::Minute; }).build());
        timeControlCol->addView(toggleHourMinuteRow);
        
        LinearLayout* timePickerContainer = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        timePickerContainer->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT); 
        timePickerContainer->setGap(24.0f);
        timePickerContainer->addView(timePicker); 
        timePickerContainer->addView(timeControlCol); 
        pickersRow->addView(timePickerContainer);
        
        DatePicker* datePicker = new DatePicker();
        datePicker->onDateSelected = [sb](int year, int month, int day) { 
            sb->show("Date selected: " + std::to_string(year) + "/" + std::to_string(month) + "/" + std::to_string(day), "SYNC", nullptr, 2.5f); 
        };
        pickersRow->addView(datePicker);
        
        CardView* pickersCard = Components::card().type(CardType::Outlined).interactive(false).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        pickersCard->addView(pickersRow); 
        scrollList->addView(pickersCard);

        View* title12 = Components::textView().text("12. Selection Controls — Checkboxes & Radio Buttons").textSize(16.0f).textColor("primary").build(); 
        scrollList->addView(title12);
        
        CardView* formCard = Components::card().type(CardType::Outlined).interactive(false).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).padding(24.0f, 24.0f, 24.0f, 24.0f).build();
        LinearLayout* formRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL); 
        formRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        formRow->setGap(64.0f);
        
        LinearLayout* cbCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        cbCol->setGap(12.0f);
        cbCol->addView(Components::textView().text("Preferences").textSize(14.0f).textColor("onSurfaceVariant").build());
        cbCol->addView(new Checkbox("Send push notifications", true));
        cbCol->addView(new Checkbox("Allow automatic updates", false));
        Checkbox* developerModeCb = new Checkbox("Enable developer options (Experimental)", false);
        developerModeCb->onCheckedChange = [sb](bool isChecked) { 
            sb->show(isChecked ? "Developer options enabled" : "Developer options disabled", "OK", nullptr, 2.5f); 
        };
        cbCol->addView(developerModeCb);
        
        LinearLayout* rbCol = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        rbCol->setGap(12.0f);
        rbCol->addView(Components::textView().text("Network Protocol").textSize(14.0f).textColor("onSurfaceVariant").build());
        RadioButton* rb1 = new RadioButton("Fast RPC / TCP", true); 
        RadioButton* rb2 = new RadioButton("Real-Time UDP Stream", false); 
        RadioButton* rb3 = new RadioButton("Secure WebSocket (WSS)", false);
        rb1->onCheckedChange = [rb2, rb3](bool isChecked) { if (isChecked) { rb2->checked = false; rb3->checked = false; } };
        rb2->onCheckedChange = [rb1, rb3](bool isChecked) { if (isChecked) { rb1->checked = false; rb3->checked = false; } };
        rb3->onCheckedChange = [rb1, rb2](bool isChecked) { if (isChecked) { rb1->checked = false; rb2->checked = false; } };
        rbCol->addView(rb1); 
        rbCol->addView(rb2); 
        rbCol->addView(rb3);
        
        formRow->addView(cbCol); 
        formRow->addView(rbCol); 
        formCard->addView(formRow); 
        scrollList->addView(formCard);

        View* title13 = Components::textView().text("13. Search Bars (Interactive Pill Containers)").textSize(16.0f).textColor("primary").build();
        scrollList->addView(title13);

        LinearLayout* searchRow = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        searchRow->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        searchRow->setGap(24.0f);

        searchRow->addView(Components::searchBar()
            .hint("Search files, packages, and methods...")
            .leadingIcon("icon:search")
            .trailingIcon("icon:settings") 
            .width(460.0f)
            .onSubmit([sb](const std::string& queryText) {
                if (!queryText.empty()) sb->show("Searching: " + queryText, "ABORT", nullptr, 4.0f);
            })
            .onTrailingClick([sb]() {
                sb->show("Filter options opened", "", nullptr, 3.0f);
            }).build());

        searchRow->addView(Components::searchBar()
            .hint("Find in file...")
            .text("drawM3UI") 
            .leadingIcon("icon:search")
            .trailingIcon("icon:person")
            .width(300.0f)
            .build());
        scrollList->addView(searchRow);

        View* title14 = Components::textView().text("14. Data Tables (Single & Multi-Column Grids)").textSize(16.0f).textColor("primary").build();
        scrollList->addView(title14);

        LinearLayout* tableCol = new LinearLayout(LinearLayout::Orientation::VERTICAL);
        tableCol->setLayoutParams(MATCH_PARENT, WRAP_CONTENT);
        tableCol->setGap(20.0f);

        CardView* singleTableCard = Components::card().type(CardType::Outlined).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).build();
        DataTable* singleDataTable = new DataTable();
        singleDataTable->addColumn("File Name", 2.0f);
        singleDataTable->addColumn("Size", 1.0f);
        singleDataTable->addColumn("Status", 1.0f);
        singleDataTable->addRow({"main.cpp", "42 KB", "Compiled"});
        singleDataTable->addRow({"DataTable.cpp", "6 KB", "Hot Reloading"});
        singleDataTable->addRow({"MaterialChart.hpp", "2 KB", "Cached"});
        singleDataTable->addRow({"CMakeLists.txt", "1 KB", "Success"});
        singleDataTable->setOnRowClicked([sb](int rowIndex){
            if (sb) sb->show("Selected row index: " + std::to_string(rowIndex), "OK", nullptr, 2.0f);
        });
        singleTableCard->addView(singleDataTable);
        tableCol->addView(singleTableCard);

        CardView* multiTableCard = Components::card().type(CardType::Outlined).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).build();
        DataTable* multiDataTable = new DataTable();
        multiDataTable->addDimensionGroup("Quarterly Revenue (USD)", {
            {"q1_val", "Q1 Sales", 1.2f},
            {"q2_val", "Q2 Sales", 1.2f}
        });
        multiDataTable->addDimensionGroup("Operations & Profit", {
            {"cost", "R&D Cost", 1.0f},
            {"profit", "Net Profit", 1.0f}
        });
        multiDataTable->addRow({"$ 128,000", "$ 165,000", "$ 42,000", "$ 123,000"});
        multiDataTable->addRow({"$ 95,400",  "$ 110,200", "$ 31,000", "$ 79,200"});
        multiDataTable->addRow({"$ 210,000", "$ 280,000", "$ 65,000", "$ 215,000"});
        multiDataTable->setOnRowClicked([sb](int rowIndex){
            if (sb) sb->show("Selected financial record: row " + std::to_string(rowIndex), "OK", nullptr, 2.0f);
        });
        multiTableCard->addView(multiDataTable);
        tableCol->addView(multiTableCard);
        scrollList->addView(tableCol);

        View* title15 = Components::textView().text("15. Data Visualization (Line, Area, Step & Bar Charts)").textSize(16.0f).textColor("primary").build();
        scrollList->addView(title15);

        LinearLayout* chartRow1 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        chartRow1->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chartRow1->setGap(24.0f);

        CardView* singleLineCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* singleLineChart = new MaterialChart(ChartType::Line);
        singleLineChart->setAxisPointerStyle(AxisPointerStyle::Dashed);
        singleLineChart->setData({ 65.0f, 40.0f, 85.0f, 30.0f, 95.989f, 50.0f }, { "Jan", "Feb", "Mar", "Apr", "May", "Jun" });
        singleLineCard->addView(Components::textView().text("Single-Series Line (Seamless Nodes + Dashed Pointer)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        singleLineCard->addView(singleLineChart);
        chartRow1->addView(singleLineCard);

        CardView* multiLineCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* multiLineChart = new MaterialChart(ChartType::Line);
        multiLineChart->setAxisPointerStyle(AxisPointerStyle::Dashed);
        multiLineChart->setMultiSeries({
            { "Direct Access", { 320, 332, 301, 334, 390, 330, 320 } },
            { "Search Engine", { 820, 932, 901, 934, 1290, 1330, 1320 } },
            { "Affiliate Ads", { 220, 182, 191, 234, 290, 330, 310 } }
        }, {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        multiLineCard->addView(Components::textView().text("Multi-Series Line (Custom Palette + Dynamic Tooltip)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        multiLineCard->addView(multiLineChart);
        chartRow1->addView(multiLineCard);

        scrollList->addView(chartRow1);

        LinearLayout* chartRow2 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        chartRow2->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chartRow2->setGap(24.0f);

        CardView* areaLineCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* areaLineChart = new MaterialChart(ChartType::AreaLine);
        areaLineChart->setAxisPointerStyle(AxisPointerStyle::Solid);
        areaLineChart->setMultiSeries({
            { "Revenue", { 400, 550, 480, 720, 890, 1100 } },
            { "Operating Cost", { 200, 310, 290, 400, 450, 520 } }
        }, { "2021", "2022", "2023", "2024", "2025", "2026" });
        areaLineCard->addView(Components::textView().text("Multi-Series Area Line (Smooth Gradient Fill)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        areaLineCard->addView(areaLineChart);
        chartRow2->addView(areaLineCard);

        CardView* stepLineCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* stepLineChart = new MaterialChart(ChartType::StepLine);
        stepLineChart->setAxisPointerStyle(AxisPointerStyle::Dashed);
        stepLineChart->setData({ 120, 132, 101, 134, 90, 230, 210 }, {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"});
        stepLineCard->addView(Components::textView().text("Step Line (Pixel-Aligned Turning Nodes)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        stepLineCard->addView(stepLineChart);
        chartRow2->addView(stepLineCard);

        scrollList->addView(chartRow2);

        LinearLayout* chartRow3 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        chartRow3->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chartRow3->setGap(24.0f);

        CardView* standardBarCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* standardBarChart = new MaterialChart(ChartType::Bar);
        standardBarChart->setBarStyle(ChartBarStyle::Standard);
        standardBarChart->setAxisPointerStyle(AxisPointerStyle::Dashed);
        standardBarChart->setData({ 120.0f, 200.0f, 150.0f, 80.0f, 70.0f, 110.0f }, { "Q1", "Q2", "Q3", "Q4", "Q5", "Q6" });
        standardBarCard->addView(Components::textView().text("Standard Bar Chart (Flat Top Columns)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        standardBarCard->addView(standardBarChart);
        chartRow3->addView(standardBarCard);

        CardView* pillBarCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* pillBarChart = new MaterialChart(ChartType::PillBar);
        pillBarChart->setAxisPointerStyle(AxisPointerStyle::Solid);
        pillBarChart->setMultiSeries({
            { "2025 Actual", { 120, 200, 150, 80, 70, 110 } },
            { "2026 Target", { 180, 250, 210, 140, 120, 190 } }
        }, {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6"});
        pillBarCard->addView(Components::textView().text("Grouped Pill Bar Chart (Rounded Capsule Top)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        pillBarCard->addView(pillBarChart);
        chartRow3->addView(pillBarCard);

        scrollList->addView(chartRow3);

        LinearLayout* chartRow4 = new LinearLayout(LinearLayout::Orientation::HORIZONTAL);
        chartRow4->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        chartRow4->setGap(24.0f);

        CardView* gappedLineCard = Components::card().type(CardType::Filled).layoutWidth(500.0f).layoutHeight(WRAP_CONTENT).padding(16.0f, 16.0f, 16.0f, 16.0f).build();
        MaterialChart* gappedLineChart = new MaterialChart(ChartType::Line);
        gappedLineChart->setLineStyle(ChartLineStyle::Gapped);
        gappedLineChart->setAxisPointerStyle(AxisPointerStyle::Solid);
        gappedLineChart->setData({ 45.0f, 82.5f, 34.0f, 105.0f, 66.0f }, {"Mon", "Tue", "Wed", "Thu", "Fri"});
        gappedLineCard->addView(Components::textView().text("Gapped Nodes Line (Solid Axis Pointer)").textSize(13.0f).margins(0,0,0,8).textColor("secondary").build());
        gappedLineCard->addView(gappedLineChart);
        chartRow4->addView(gappedLineCard);

        scrollList->addView(chartRow4);

        CardView* polarCard = Components::card()
            .type(CardType::Filled)
            .layoutWidth(540.0f)
            .layoutHeight(400.0f)
            .padding(8.0f, 8.0f, 8.0f, 8.0f)
            .build(); 

        MaterialPieChart* polarPieChart = new MaterialPieChart();
        polarPieChart->setLayoutParams(MATCH_PARENT, MATCH_PARENT); 

        polarPieChart->setUnit("LOC");
        polarPieChart->setLegend("Code Range", "Avg Complexity"); 
        polarPieChart->setAvgColor("#5C6BC0");

        std::vector<PolarBarItem> customData = {
            { "GL Context",       2000, 4500, 3200.0f, "#2196F3" },
            { "Input Dispatcher", 1500, 3800, 2600.0f, "#03A9F4" },
            { "VAO Memory Pool",  1200, 3000, 2100.0f, "#00BCD4" },
            { "View Base",        2500, 6000, 4100.0f, "#4CAF50" },
            { "Box Layout",       3000, 7500, 5200.0f, "#8BC34A" },
            { "Spring Animator",  1800, 4200, 2900.0f, "#CDDC39" },
            { "SDF Shaders",      3500, 8500, 5800.0f, "#FF9800" },
            { "Text Atlas",       2200, 5000, 3600.0f, "#FF5722" },
            { "M3 Palette",       1000, 2500, 1800.0f, "#E91E63" },
            { "Card Container",   2800, 6500, 4600.0f, "#9C27B0" },
            { "FAB & Buttons",    3200, 7200, 5100.0f, "#673AB7" },
            { "Form Controls",    2600, 5800, 4200.0f, "#3F51B5" },
            { "Nested Scroll",    3000, 6800, 4800.0f, "#5C6BC0" },
            { "Polar Bar Chart",  4000, 9500, 6800.0f, "#009688" },
            { "Area Line Chart",  3800, 8800, 6200.0f, "#26A69A" },
            { "DSL Builder",      2000, 4800, 3400.0f, "#00ACC1" },
            { "State Bindings",   1600, 3600, 2500.0f, "#1E88E5" },
            { "ECharts API",      1500, 3200, 2300.0f, "#5E35B1" }
        }; 

        polarPieChart->setData(customData);
        polarCard->addView(polarPieChart); 
        scrollList->addView(polarCard);

        CardView* footerCard = Components::card().type(CardType::Filled).interactive(false).cornerRadius(20.0f).layoutWidth(MATCH_PARENT).layoutHeight(WRAP_CONTENT).padding(24.0f, 32.0f, 24.0f, 32.0f).build();
        LinearLayout* footerLayout = new LinearLayout(LinearLayout::Orientation::VERTICAL); 
        footerLayout->setLayoutParams(MATCH_PARENT, WRAP_CONTENT); 
        footerLayout->setGap(16.0f);
        footerLayout->addView(Components::textView().text("Material Design 3 Architecture Complete").textSize(20.0f).textColor("primary").build());
        footerLayout->addView(Components::textView().text("Built with modern M3 guidelines, featuring adaptive color tokens, fluid spring physics, and hardware-accelerated rendering.").textSize(14.0f).lineSpacing(0.0f, 1.6f).textColor("secondary").build());
        footerCard->addView(footerLayout); 
        scrollList->addView(footerCard);

        homeScrollView->addView(scrollList); 

        /* Page 2: Layout & Gravity Alignment Lab */
        ScrollView* gravityScrollView = createGravityShowcase(sb);

        /* Material 3 Animated Page Switcher Container */
        pageContainer = new PageContainer();
        pageContainer->setLayoutParams(MATCH_PARENT, MATCH_PARENT);
        pageContainer->setMargins(0.0f, 0.0f, 0.0f, 92.0f); // Reserve space for bottom navigation bar

        pageContainer->addPage(homeScrollView);    // Page 0
        pageContainer->addPage(gravityScrollView); // Page 1

        rootCard->addView(pageContainer);

        PageContainer* pc = pageContainer;

        NavigationBar* navigationBar = new NavigationBar();
        navigationBar->addTab("Home", "icon:home");
        navigationBar->addTab("Layout & Gravity", NavigationIcon::Grid);
        navigationBar->addTab("Favorites", "icon:star");
        navigationBar->addTab("Profile", "./resources/vector.png");
        navigationBar->setCornerRadius(24.0f); 
        navigationBar->setDrawBackground(false); 
        navigationBar->setTopDivider(true);
        
        navigationBar->setOnTabSelected([pc, sb](int tabIndex) { 
            if (tabIndex < static_cast<int>(pc->getPageCount())) {
                pc->switchToPage(tabIndex, TransitionType::FadeThrough);
                if (tabIndex == 0) {
                    sb->show("Switched to Home Catalog", "", nullptr, 2.0f);
                } else if (tabIndex == 1) {
                    sb->show("Switched to Layout & Gravity Showcase", "", nullptr, 2.0f);
                }
            } else {
                std::string tabNames[] = { "Dashboard", "Layout & Gravity", "Favorites", "Profile" };
                sb->show("Page: " + tabNames[tabIndex] + " (Under Construction)", "", nullptr, 2.0f); 
            }
        });

        rootCard->addView(navigationBar); 
        rootCard->addView(drawer);

        floatingImageView = new FloatingImageView("./resources/Image.jpg", 300.0f, 600.0f, 20.0f);
        floatingImageView->setPosition(1200.0f, 100.0f);
        floatingImageView->setSize(260.0f, 500.0f);

        FloatingImageView* floatingImg = floatingImageView;

        floatingImageView->setOnClickListener([sb]() {
            sb->show("Media Card: Left Clicked", "INFO", nullptr, 2.0f);
        });

        floatingImageView->setOnRightClickListener([sb]() {
            sb->show("Tip: Hold Right Mouse Button for 0.8s to detach to Desktop", "OK", nullptr, 3.0f);
        });

        floatingImageView->setOnLongClickListener([floatingImg, sb](bool) {  
            if (!floatingImg->isDetached()) {
                floatingImg->detachToDesktop();
                sb->show("Media detached to Desktop Floating Window", "DISMISS", nullptr, 2.5f);
            } else {
                floatingImg->dockToApp();
                sb->show("Media docked back into Application Viewport", "DISMISS", nullptr, 2.5f);
            }
        });

        floatingActionButton = Components::fabButton()
            .shape(FabShape::Circle)
            .size(FabSize::Standard)
            .type(FabType::Primary)
            .icon("./resources/vector.png")
            .expandDirection(ExpandDirection::Up)
            .menuAlignment(MenuAlignment::Center)
            .anchorBottomRight(60.0f, 150.0f)
            .addSubItem("Developer Profile", "./resources/vector.png", [sb]() { sb->show("@github.com/Vectorted", "OK", nullptr, 2.5f); }, true, FabType::Secondary)
            .addSubItem("Capture Photo", "icon:home", [sb]() { sb->show("Camera opened", "OK", nullptr, 2.5f); }, true, FabType::Primary)
            .addSubItem("Browse Files", "", [sb]() { sb->show("File manager opened", "OK", nullptr, 2.5f); }, true, FabType::Surface)
            .build();

        root->addView(rootCard); 
        root->addView(snackbar); 
        root->addView(materialDialog); 
        root->addView(floatingImageView); 
        root->addView(notificationOverlay);
        root->addView(starrySkyFormation);       
        root->addView(windowNotification);  
        root->addView(screenNotification);  
        root->addView(floatingActionButton);

        return root;
    }

    /**
     * @brief Handles window resize events.
     * 
     * @param app Reference to running ApplicationLooper.
     * @param width New window width in pixels.
     * @param height New window height in pixels.
     */
    void onResize(ApplicationLooper& app, int width, int height) override {}

    /**
     * @brief Cleanup handler invoked when the activity lifecycle terminates.
     * 
     * @param app Reference to terminating ApplicationLooper.
     */
    void onDestroy(ApplicationLooper& app) override {}
};

MAIN_ACTIVITY(MainActivity)
