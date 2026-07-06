enum MenuLevel {
    MENU_SECTION,
    MENU_SYSTEM,
    MENU_ROM,
    APP_SETTINGS,
    SYSTEM_SETTINGS,
    ROM_SETTINGS,
    FILTER_SYSTEM_SETTINGS
};

struct State {
    MenuLevel currentMenuLevel;
    MenuLevel previousMenuLevel;
    int currentSectionIndex;
    int currentSystemIndex;
    int currentRomIndex;
    int currentFilterCategory;
    bool launcherCallback;
};