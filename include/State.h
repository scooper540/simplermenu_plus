enum MenuLevel {
    MENU_SECTION,
    MENU_SYSTEM,
    MENU_ROM,
    APP_SETTINGS,
    SYSTEM_SETTINGS,
    ROM_SETTINGS
};

struct State {
    MenuLevel currentMenuLevel;
    int currentSectionIndex;
    int currentSystemIndex;
    int currentRomIndex;
    bool launcherCallback;
};