#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <boost/filesystem.hpp>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "Application.h"
#include "Exception.h"
#include "platform.h"

#define SCREEN_TIMEOUT_MS 60000 // 60s

#ifdef POWKIDDY
Application::Application() : Application(".", "./.state") {}
#else
Application::Application() : Application("/userdata/system/configs/simplermenu_plus", "/userdata/system/configs/simplermenu_plus/.state") {}
#endif
Application::Application(const std::string& szBasePath, const std::string& szStateFile) 
    : i18n(szBasePath + "/i18n.ini"),
      cfg(szBasePath + "/config.ini", 
          szStateFile),
      theme(cfg.get(Configuration::HOME_PATH), cfg.get(Configuration::THEME_PATH), cfg.get(Configuration::THEME)),
      controlMapping(cfg),
      renderComponent(cfg, theme, favManager),
      appSettings(cfg, i18n, 0, 100, 5),
      systemSettings(cfg, i18n, 0, 100, 5),
      romSettings(cfg, i18n, 0, 100, 5)
 {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        exit(1);
    }
    const SDL_VideoInfo* vi;
    vi = SDL_GetVideoInfo();
    int width = vi->current_w;
    int height = vi->current_h;
#ifdef _WIN32
    width = cfg.getInt(Configuration::SCREEN_WIDTH);
    height = cfg.getInt(Configuration::SCREEN_HEIGHT);
#endif
    theme.setScreenSize(width, height);
    renderComponent.setScreenSize(width, height);
    isApplicationStarted = false;
    // Observe settings changes
    appSettings.attach(this);
    systemSettings.attach(this);
    romSettings.attach(this);

    // Language observers
    attach(&romSettings);

    romSettings.initializeSettings();
    systemSettings.initializeSettings();
    appSettings.initializeSettings();

    //initialize display at the begin to be able to display a building rom list during cache generation
    renderComponent.initialize();

#ifndef POWKIDDY
    bool rebuildCache = false;
    try {
        state = cfg.loadState();
        std::cout << "State loaded: " << state.currentMenuLevel << std::endl;
        std::cout << "Folder: " << state.currentSystemIndex << std::endl;
        std::cout << "Rom: " << state.currentRomIndex << std::endl;

        // Restore cache if coming back from a launcher callback
        if (!state.launcherCallback) {
            std::cout << "Regular launch (no callback)" << std::endl;
            rebuildCache = true;
        }

    } catch (const StateNotFoundException& e) {
        std::cout << "State not found, using default values" << std::endl;
        state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
        state.currentSystemIndex = 0;
        state.currentRomIndex = 0;
        state.launcherCallback = false;

        cfg.saveState(state);

        rebuildCache = true;
    }

    if (rebuildCache) {
        // Initialize/Rebuild the cache - always create a new cache 
        // from disk contents on startup
        std::cout << "Rebuilding cache" << std::endl; 
        loadCache(true);

    } else {
        std::cout << "Loading cache from disk" << std::endl;
        loadCache(false);
        
    }
#else
    try {
        state = cfg.loadState();
        std::cout << "State loaded: " << state.currentMenuLevel << std::endl;
        std::cout << "Folder: " << state.currentSystemIndex << std::endl;
        std::cout << "Rom: " << state.currentRomIndex << std::endl;

    } catch (const StateNotFoundException& e) {
        std::cout << "State not found, using default values" << std::endl;
        state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
        state.currentSystemIndex = 0;
        state.currentRomIndex = 0;
        state.launcherCallback = false;
        cfg.saveState(state);
    }
    loadCache(false);

#endif
    if (state.launcherCallback) {
        // If we are coming from a launcher callback, we need to reset the state
        std::cout << "Launcher callback processed" << std::endl;
        state.launcherCallback = false;
        cfg.saveState(state);
        
    }
    favManager.load(
        cfg.get(Configuration::HOME_PATH) + "favorites.json",
        cfg.get(Configuration::HOME_PATH) + "history.json"
    );

    populateMenu(menu);

    theme.loadTheme(cfg.get(Configuration::HOME_PATH), cfg.get(Configuration::THEME_PATH), cfg.get(Configuration::THEME));
    


    // Initialize joystick
#ifndef POWKIDDY
//disable joystick at this time, to be enabled again once SDL1.2 open all joystick when 1 is opened from client app
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "Failed to initialize SDL joystick subsystem: " << SDL_GetError() << std::endl;
    }

    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
        if (!joystick) {
            std::cerr << "Failed to open joystick: " << SDL_GetError() << std::endl;
            // Handle the error appropriately.
        } else {
            std::cout << "Joystick Name: " << SDL_JoystickName(0) << std::endl;
            std::cout << "Number of Axes: " << SDL_JoystickNumAxes(joystick) << std::endl;
            std::cout << "Number of Buttons: " << SDL_JoystickNumButtons(joystick) << std::endl;
        }
    }
#endif
    
}

void Application::drawCurrentState() {
    std::stringstream ss;
    switch (state.currentMenuLevel) {
        case MENU_SYSTEM:
        {
            std::string systemName = menu.getSystems()[state.currentSystemIndex].getTitle();
            std::string systemPath = "";
            int numberOfRoms = menu.getSystems()[state.currentSystemIndex].getRoms().size();
            renderComponent.drawSystem(systemName, systemPath, numberOfRoms);
            break;
        }
        case MENU_ROM:
        {
            std::vector<std::pair<std::string, std::string>> romData;
            for (const Rom& rom : menu.getSystems()[state.currentSystemIndex].getRoms()) {
                romData.push_back({rom.getTitle(), rom.getPath()});
            }
            if(state.currentRomIndex < 0) state.currentRomIndex = 0;
            if(state.currentRomIndex >= menu.getSystems()[state.currentSystemIndex].getRoms().size() - 1) state.currentRomIndex = menu.getSystems()[state.currentSystemIndex].getRoms().size() - 1;
            renderComponent.drawRomList(menu.getSystems()[state.currentSystemIndex].getTitle(), romData, state.currentRomIndex);
            break;
        }
        case APP_SETTINGS:
        {
            renderComponent.drawSettingsMenu(i18n.get("appSettings"), appSettings.getAppSettings(), currentSettingsIndex);
            break;
        }
        case SYSTEM_SETTINGS:
        {
            std::vector<Settings::I18nSetting> systemData;
            Settings::I18nSetting setting;
            setting.title = systemSettings.currentSystem;
            setting.value = systemSettings.getDefaultCore(systemSettings.currentSystem, cache);
            systemData.push_back(setting);
            renderComponent.drawSettingsMenu(i18n.get("systemSettings"), systemData, currentSystemSettingsIndex);
            break;
        }
        case ROM_SETTINGS:
        {
            std::vector<Settings::I18nSetting> romData;
            Settings::I18nSetting setting;
            setting.title = i18n.get("coreOverride");
            setting.value = romSettings.getDefaultCore(cache); //get selected core for this rom
            romData.push_back(setting);
            renderComponent.drawSettingsMenu(i18n.get("romSettings"), romData, currentRomSettingsIndex);
            break;
        }
    }
}

void Application::handleCommand(ControlMap cmd) {
    switch (state.currentMenuLevel) {
        case MenuLevel::MENU_SYSTEM:
            if (cmd == CMD_ENTER) { // KEY_A/ENTER
                state.currentMenuLevel = MenuLevel::MENU_ROM;
                state.currentRomIndex = 0;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP || cmd == CMD_LEFT) { // UP
                const System& system = menu.getSystems()[state.currentSystemIndex];
                if (state.currentSystemIndex > 0) state.currentSystemIndex--;
                else state.currentSystemIndex = menu.getSystems().size() - 1;
            } else if (cmd == CMD_DOWN || cmd == CMD_RIGHT) { // DOWN
                const System& system = menu.getSystems()[state.currentSystemIndex];
                state.currentSystemIndex = (state.currentSystemIndex + 1) % menu.getSystems().size();
            } else if (cmd == CMD_ROM_SETTINGS) {
                if (!menu.getSystems()[state.currentSystemIndex].isVirtual())
                {
                    state.currentMenuLevel = MenuLevel::SYSTEM_SETTINGS;
                    renderComponent.resetValues();
                    systemSettings.currentSystem = menu.getSystems()[state.currentSystemIndex].getTitle();
                    systemSettings.applyCurrentKey();
                    systemSettings.getCores(systemSettings.currentSystem, cache);
                }
            }

            // Save state after navigating, but not when entering the ROM settings
            if (cmd != CMD_ROM_SETTINGS) {
                cfg.saveState(state);
            }

            break;
        case MenuLevel::MENU_ROM:
            if (cmd == CMD_BACK) { // ESC
                state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                const System& system = menu.getSystems()[state.currentSystemIndex];
                if (state.currentRomIndex > 0) state.currentRomIndex--;
                else state.currentRomIndex = system.getRoms().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                const System& system = menu.getSystems()[state.currentSystemIndex];
                state.currentRomIndex = (state.currentRomIndex + 1) % system.getRoms().size();
            }
            else if (cmd == CMD_PREV_PAGE) { // PREV PAGE
                const System& system = menu.getSystems()[state.currentSystemIndex];
                int items = theme.getIntValue(Configuration::ITEMS);
                if (state.currentRomIndex >= items)
                    state.currentRomIndex -= items;
                else
                    state.currentRomIndex = 0;
            } else if (cmd == CMD_NEXT_PAGE) { // DOWN
                const System& system = menu.getSystems()[state.currentSystemIndex];
                state.currentRomIndex = (state.currentRomIndex + theme.getIntValue(Configuration::ITEMS)) % system.getRoms().size();
            } else if (cmd == CMD_ENTER) { // ENTER
                std::cout << "execute rom" << std::endl;
                launchRom();
                renderComponent.resetValues();
            } else if (cmd == CMD_ROM_SETTINGS) {
                state.currentMenuLevel = MenuLevel::ROM_SETTINGS;
                renderComponent.resetValues();
                const Rom& rom = menu.getSystems()[state.currentSystemIndex].getRoms()[state.currentRomIndex];
                romSettings.currentRom    = rom.getTitle();
                romSettings.currentPath   = rom.getPath();
                romSettings.currentSystem = rom.getOriginalSystem().empty() ? menu.getSystems()[state.currentSystemIndex].getTitle() : rom.getOriginalSystem();
                romSettings.applyCurrentKey();
                romSettings.getCores(romSettings.currentSystem, cache);
            } else if (cmd == CMD_TOGGLE_FAVORITE) {
                int currentRomIndex = state.currentRomIndex;
                const Rom& rom = menu.getSystems()[state.currentSystemIndex].getRoms()[state.currentRomIndex];
                const std::string sysName = menu.getSystems()[state.currentSystemIndex].getTitle();
                favManager.toggleFavorite(sysName, rom.getTitle(), rom.getPath());
                //redo menu generation
                menu = Menu();
                populateMenu(menu);
                state.currentRomIndex = currentRomIndex;
                state.currentSystemIndex = menu.getSystemIndexByName(sysName);
                //renderComponent.resetValues();
            }

            // Save state after navigating, but not when entering the ROM settings
            // When launching a rom we should never get back here in any case
            if (cmd != CMD_ROM_SETTINGS) {
                cfg.saveState(state);
            }

            break;
        case APP_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                state = cfg.loadState();
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (currentSettingsIndex > 0) currentSettingsIndex--;
                else currentSettingsIndex = appSettings.getEnabledKeys().size() - 1;
                std::cout << "currentSettingsIndex: " << currentSettingsIndex << std::endl;
            } else if (cmd == CMD_DOWN) { // DOWN
                currentSettingsIndex = (currentSettingsIndex + 1) % (appSettings.getEnabledKeys().size());
                std::cout << "currentSettingsIndex: " << currentSettingsIndex << std::endl;
            }
            break;
        case SYSTEM_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (state.currentSystemIndex > 0) state.currentSystemIndex--;
                else state.currentSystemIndex = systemSettings.getEnabledKeys().size() - 1;
                std::cout << "currentSettingsIndex: " << state.currentSystemIndex << std::endl;
            } else if (cmd == CMD_DOWN) { // DOWN
                state.currentSystemIndex = (state.currentSystemIndex + 1) % (systemSettings.getEnabledKeys().size());
                std::cout << "currentSettingsIndex: " << state.currentSystemIndex << std::endl;
            }
            break;
        case ROM_SETTINGS:
            if (cmd == CMD_BACK) { // ESC
                // Flush pending core override to disk now
                if (hasPendingCoreOverride && !pendingCoreOverridePath.empty())
                {
                    cache.menuCacheUpdateItem(
                        cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE),
                        pendingCoreOverridePath, pendingCoreOverrideValue);
                    hasPendingCoreOverride = false;
                    pendingCoreOverridePath.clear();
                    pendingCoreOverrideValue.clear();
                }
                state.currentMenuLevel = MenuLevel::MENU_ROM;
                renderComponent.resetValues();
            } else if (cmd == CMD_UP) { // UP
                if (currentRomSettingsIndex > 0) currentRomSettingsIndex--;
                else currentRomSettingsIndex = romSettings.getEnabledKeys().size() - 1;
            } else if (cmd == CMD_DOWN) { // DOWN
                currentRomSettingsIndex = (currentRomSettingsIndex + 1) % (romSettings.getEnabledKeys().size() );
            } 
            break;
    }

    if (cmd == CMD_SYS_SETTINGS) {
        if(state.currentMenuLevel != MenuLevel::APP_SETTINGS) {
            cfg.saveState(state);
        }
        state.currentMenuLevel = MenuLevel::APP_SETTINGS;
        renderComponent.resetValues();
    }

    if(state.currentMenuLevel == MenuLevel::APP_SETTINGS) {
        if (cmd == CMD_UP) {
            appSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            appSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            appSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            appSettings.navigateRight();
        } else if (cmd == CMD_ENTER) {
            appSettings.navigateEnter();
        }

        std::string currentKey = appSettings.getCurrentKey();
        std::string currentValue = appSettings.getCurrentValue();

    }

    if(state.currentMenuLevel == SYSTEM_SETTINGS) {
        if (cmd == CMD_UP) {
            systemSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            systemSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            systemSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            systemSettings.navigateRight();
        } else if (cmd == CMD_ENTER) {
            systemSettings.navigateEnter();
        }

        std::string currentKey = systemSettings.getCurrentKey();
        std::string currentValue = systemSettings.getCurrentValue();

    }

    if(state.currentMenuLevel == ROM_SETTINGS) {
        if (cmd == CMD_UP) {
            romSettings.navigateUp();
        } else if (cmd == CMD_DOWN) {
            romSettings.navigateDown();
        } else if (cmd == CMD_LEFT) {
            romSettings.navigateLeft();
        } else if (cmd == CMD_RIGHT) {
            romSettings.navigateRight();
        }// } else if (cmd == CMD_ENTER) {
        //     romSettings.navigateEnter();
        // }

        std::string currentKey = romSettings.getCurrentKey();
        std::string currentValue = romSettings.getCurrentValue();

    }
}

bool Application::isInteger(const std::string &s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void Application::run() {
    isApplicationStarted = true;
    bool isRunning = true;
    SDL_Event event;

    int fps = 0;
    int frameCount = 0;
    Uint32 fpsTimer = 0;

    Uint32 frameStart = SDL_GetTicks();
    int screenRefresh = cfg.getInt(Configuration::SCREEN_REFRESH);
    Uint32 frameDelay = 1000 / screenRefresh;
    Uint32 now = SDL_GetTicks();
    bool screenOff = false;

    Uint32 elapsed = now - frameStart;
    while (isRunning) {
        screenRefresh = cfg.getInt(Configuration::SCREEN_REFRESH);
        frameDelay = 1000 / screenRefresh;
        
        now = SDL_GetTicks();
        elapsed = now - frameStart;
        if (elapsed < frameDelay) {
            SDL_Delay(frameDelay - elapsed);  // ← libère le CPU au lieu de busy-wait
            continue;
        }
/*
        frameStart = SDL_GetTicks();
        // Wait if last frame was drawn too fast
        if (SDL_GetTicks() - frameStart < frameDelay) {
            continue;
        }

        // // Fine tune FPS
        if (frameCount == screenRefresh && ((SDL_GetTicks() - fpsTimer) < 1000)) {
            continue;
        }
*/
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_KEYDOWN:
                case SDL_JOYAXISMOTION:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYHATMOTION:
                    if (screenOff) 
                    {
                        std::ofstream(SYS_BACKLIGHT_POWER) << "0";
                        screenOff = false;
                    }
                    lastInputTime = SDL_GetTicks();
                    isButtonHeld = true;
                    lastHeldEvent = event;
                    repeatStartTime = SDL_GetTicks() + 500;
                    repeatInterval = 100;
                    handleCommand(controlMapping.convertCommand(event));
                    break;
                case SDL_JOYBUTTONUP:
                case SDL_KEYUP:
                    isButtonHeld = false;
                    break;
            }
        }

        if (isButtonHeld && SDL_GetTicks() > repeatStartTime) {
            handleCommand(controlMapping.convertCommand(lastHeldEvent));
            repeatStartTime = SDL_GetTicks() + repeatInterval;
        }

        // Handle FPS information
        if (SDL_GetTicks() - fpsTimer >= 1000) {
            fps = frameCount;
            frameCount = 0;
            fpsTimer = SDL_GetTicks();
        }
        if (!screenOff && SDL_GetTicks() - lastInputTime > SCREEN_TIMEOUT_MS) 
        {
            std::ofstream(SYS_BACKLIGHT_POWER) << "1";
            screenOff = true;
        }
        drawCurrentState();

        renderComponent.printFPS(fps);
        renderComponent.printBattery();
        renderComponent.update();

        frameCount++;
    }
}

void Application::print_list() {
    for (const auto& system : menu.getSystems()) {
        std::cout << "  System: " << system.getTitle() << std::endl;
        for (const auto& rom : system.getRoms()) {
            std::cout << "  System: " << system.getTitle() <<  " -> Rom: " << rom.getTitle() << std::endl;
        }
    }
}

void Application::launchRom() {


    // Save application state first and mark it as a launcher callback
    state.launcherCallback = true;
    cfg.saveState(state);

    const Rom& currentRom = menu.getSystems()[state.currentSystemIndex].getRoms()[state.currentRomIndex];
    std::string romName = currentRom.getTitle();
    std::string romPath = currentRom.getPath();
    std::string systemName = currentRom.getOriginalSystem().empty() ? menu.getSystems()[state.currentSystemIndex].getTitle() : currentRom.getOriginalSystem();

    //add to history
    favManager.addHistory(systemName, romName, romPath);

    //find core either default -> get it from cache defaultexec
    std::string coreName = cache.getMenuItemByPath(romPath).core;
    if (coreName.empty() || coreName == "default") {
        cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");
        ConsoleData sysData = cache.getSystemData(systemName);
        if (!sysData.selectedExec.empty())
            coreName = sysData.selectedExec;
        else if (!sysData.execs.empty())
            coreName = sysData.execs.front();
    }
    std::cout << "Launching rom: " << systemName << " -> " << romName << std::endl;

    std::string execLauncher = cfg.get(Configuration::HOME_PATH) + "launchers/" + coreName;

    // Launch emulator
    std::string command = "launcher.sh " + execLauncher + " '" + romPath + "'";
    std::cout << "Executing: " << command << std::endl;
#ifndef _WIN32
    setenv("SDL_NOMOUSE", "1", 1);

    pid_t pid = fork();
    if (pid == 0) {
            execlp("launcher.sh","launcher.sh", execLauncher.c_str(), romPath.c_str(), NULL);
            exit(1);
    } else if (pid > 0) {
            SDL_Quit();
            exit(0);
    } else {
            std::cerr << "Fork failed" << std::endl;
    }
#endif
    // Exit the application to free all resources
    SDL_Quit();
    exit(0);
}

/////////////////
// ISettingsObserver methods

void Application::settingsChanged(const std::string& key, const std::string& value) {
    std::cout << key << " changed to " << value << std::endl;
    if (key == Configuration::LANGUAGE) {
        i18n.setLang(value);
        notifyLanguageChange();

    } else if (key == Configuration::THEME) {
        theme.loadTheme(cfg.get(Configuration::HOME_PATH), cfg.get(Configuration::THEME_PATH), value);
    }
    else if (key == Configuration::QUIT) {
        if(value != "INTERNAL") {
            SDL_Quit();
            execlp("shutdown.sh", "shutdown.sh", NULL);
            exit(0);
        }
    }
    else if(key == Configuration::BRIGHTNESS)
    {            
        int maxBrightness = 0;
        std::ifstream(SYS_MAX_BRIGHTNESS) >> maxBrightness;
        //align currnet key value with max brightness
        int newBrightness = std::stoi(value) * maxBrightness / 100;
        std::ofstream(SYS_CURRENT_BRIGHTNESS) << newBrightness;
        std::cout << "BRIGHTNESS SET TO " << newBrightness << std::endl;
    }
    else if(isApplicationStarted && state.currentMenuLevel == MenuLevel::ROM_SETTINGS)
    {
         // Store pending override in memory — flushed to disk on CMD_BACK
        std::string romPath = menu.getSystems()[state.currentSystemIndex].getRoms()[state.currentRomIndex].getPath();
        if (romPath != "")
        {
            hasPendingCoreOverride = true;
            pendingCoreOverridePath = romPath;
            pendingCoreOverrideValue = value;
        }
        //change of Core override for a specific ROM. update cache and ini file to have this setting persistant accross new cache generation
    /*    std::string romPath = menu.getSystems()[state.currentSystemIndex].getRoms()[state.currentRomIndex].getPath();
        if (romPath != "") 
        {
            cache.menuCacheUpdateItem(
                    cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE), 
                    romPath, value);
        }*/
    }
    else if(isApplicationStarted && state.currentMenuLevel == MenuLevel::SYSTEM_SETTINGS)
    {
        cache.systemCacheUpdateSelectedExec(
                    cfg.get(Configuration::HOME_PATH) + "systems.json", 
                    menu.getSystems()[state.currentSystemIndex].getTitle(), value);
        return; //don't save ini file in that case
    }
    else if(isApplicationStarted && key == Configuration::UPDATE_CACHES) //renew the cache
    {
        loadCache(true);
        menu = Menu();
        populateMenu(menu);
        return;
    }
    if(isApplicationStarted)
    {
        cfg.set(key, value);
        cfg.saveConfigIni();
    }
}

/////////////////
// ILanguageSubject methods

void Application::attach(ILanguageObserver *observer) {
    langObservers.push_back(observer);
    std::cout << "LangObserver added to " << getName() 
              << " object: " << observer->getName() << "\n";
}

void Application::detach(ILanguageObserver *observer) {
    langObservers.erase(std::remove(langObservers.begin(), 
                                    langObservers.end(), 
                                    observer), 
                        langObservers.end());
}

void Application::notifyLanguageChange() {
    for (ILanguageObserver *observer : langObservers) {
        observer->languageChanged();
        std::cout << "LangObserver " << observer->getName() << " notified by " 
                  << getName() << std::endl;
    }
}

/////////////////
// ISettingsObserver and ILanguageSubject common methods

std::string Application::getName() {
    return "Application::" + std::to_string((unsigned long long)(void**)this);
}

//
/////////////////


/////////////////
// Private methods

void Application::loadCache(bool force) {
    // Get the path to the cache file from config.ini file
    std::string cacheFilePath = cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE);

    if (force || !cache.menuCacheExists(cacheFilePath)) {
        // Cache does not exist or force update is requested:
        // Read all sections and create a new cache


        std::cout << "Force cache update" << std::endl;
        
        // get the path to the cache file by removing the filename
        // from the cacheFilePath
        boost::filesystem::path cacheFilePathObj(cacheFilePath);
        cacheFilePathObj.remove_filename();
        // create the directories if they do not exist
        boost::filesystem::create_directories(cacheFilePathObj.string());

        cache.menuCacheSave(cacheFilePath, populateCache());
        // check if we have any override for ROM in the ini file
        for (const auto& cachedItem : cache.menuCacheLoad(cacheFilePath)) {
            std::string iniKey = RomSettings::getKey(cachedItem.system, boost::filesystem::path(cachedItem.rom).stem().string());
            if(cfg.existsKey(iniKey))
            {
                std::string savedCore = cfg.get(iniKey);
                if (!savedCore.empty() && savedCore != "default") {
                    std::cout << "Restoring core override: " << iniKey << " = " << savedCore << std::endl;
                    cache.menuCacheUpdateItem(cacheFilePath, cachedItem.path, savedCore);
                }
            }
        }
    } else {

        std::cout << "Cache exists, loading from cache file" << std::endl;
        // Ignore the return value as we are not using it here, just
        // load the cache file contents into the in-memory cache
        cache.menuCacheLoad(cacheFilePath);
    }
}

std::vector<CachedMenuItem> Application::populateCache() {
    FileManager fileManager(cfg);

    // Load systems from systems.json
    auto consoleDataMap = cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");
    std::string romsPath = cfg.get(Configuration::ROMS_PATH);

    std::vector<CachedMenuItem> allCachedItems;
    std::string cacheGeneration = i18n.get("cacheGenerationProgress");
    boost::replace_all(cacheGeneration, "\\r", "\r");
    boost::replace_all(cacheGeneration, "\\n", "\n");
    for (const auto& [consoleName, data] : consoleDataMap) {
        lastInputTime = SDL_GetTicks();
        renderComponent.drawMessage(cacheGeneration + " " + consoleName);
        renderComponent.update();
        for (const auto& romDir : data.romDirs) {
            auto files = fileManager.getFiles(romsPath + romDir, data.romExts);
            for (const auto& file : files) {
                std::string romPath = romsPath + "/" + romDir + "/" + file;
                allCachedItems.push_back({consoleName, file, romPath});
            }
        }
    }

    return allCachedItems;
}

void Application::populateMenu(Menu& menu) {
    
    //virtual systems favorites and history
    System favSystem("Favorites");
    for (const auto& f : favManager.getFavorites()) {
        favSystem.addRom(Rom(f.rom, f.path, f.system));
    }
    if (!favManager.getFavorites().empty())
        menu.addSystem(favSystem);

    System histSystem("History");
    for (const auto& h : favManager.getHistory()) {
        histSystem.addRom(Rom(h.rom, h.path, h.system));
    }
    if (!favManager.getHistory().empty())
        menu.addSystem(histSystem);

// Loop through the cached items and populate the Menu structure
    for (const auto& cachedItem : cache.menuCacheLoad(cfg.get(Configuration::HOME_PATH) + "/" + cfg.get(Configuration::GLOBAL_CACHE))) {
        // cachedItem should have members: system, filename, path.

        // Check if the System already exists in the menu
        System* system = menu.getSystemByName(cachedItem.system);
        if (!system) {
            System newSystem(cachedItem.system);
            menu.addSystem(newSystem);
            system = menu.getSystemByName(cachedItem.system);
        }

        // Add the file to the system
        Rom rom(cachedItem.rom, cachedItem.path);
        system->addRom(rom);
    }
}