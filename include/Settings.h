#pragma once

#include <string>
#include <map>
#include <vector>
#include "I18n.h"
#include "IObservers.h"
#include "Cache.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

class Configuration;

class Settings : public ISettingsSubject {
public: 
    struct Setting {
        std::string key;
        std::string value;
        bool enabled;
    };

    struct I18nSetting {
        std::string title;
        std::string value;
    };

    virtual void navigateUp();
    virtual void navigateDown();
    virtual void navigateLeft() = 0;
    virtual void navigateRight() = 0;
    virtual void navigateEnter() = 0;

private:
    std::vector<ISettingsObserver *> observers;
    
protected:
    int currentIndex;

    std::string currentKey;
    std::string currentValue;
    
    std::set<std::string> themeFolders;

    Configuration& cfg;
    I18n& i18n;
    std::vector<I18nSetting> i18nSettings;

    std::vector<std::string> enabledKeys;

    std::map<std::string, Setting> settingsMap;

    int minValue;
    int maxValue;
    int delta;

    void updateInt(bool increase, std::string setting, 
                   int min, int max, int inc);
    
    std::set<std::string> cores;

    

    /**
     * ISettingsSubject methods
     */
    void notifySettingsChange(const std::string &key, const std::string &value) override;


public:
    Settings(Configuration& cfg, I18n& i18n, 
             int minValue, int maxValue, int delta);
    // ~Settings();

    // Define default settings with their keys
    std::vector<std::string> defaultKeys;

    void initializeSettings();

    void updateListSetting(const std::set<std::string>& values, bool increase);
    void updateBoolSetting();

    std::string getCurrentKey();
    std::string getCurrentValue();
    std::vector<std::string> getEnabledKeys();
    /**
     * ISettingsSubject methods
    */
    void attach(ISettingsObserver *observer) override;
    void detach(ISettingsObserver *observer) override;
    
    /**
     * ISettingsSubject and ILanguageObserver common methods
     */
    virtual std::string getName() = 0;
};

class AppSettings : public Settings {
public:

    AppSettings(Configuration& cfg, I18n& i18n, 
                   int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getAppSettings();

    void reloadI18nSettings();

    // these methods should just call the parent class's methods. Is it possible to defined them as pure virtual?
    void navigateUp() { Settings::navigateUp(); };
    void navigateDown() { Settings::navigateDown();};
    void navigateLeft() override {
        std::cout << "navigate Left" << std::endl;
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::BRIGHTNESS) {
                updateBrightness(false);
            
            } else if (currentKey == Configuration::VOLUME) {
                updateInt(false, currentKey, minValue, maxValue, delta);
            
            } else if (currentKey == Configuration::SCREEN_REFRESH) {
                updateInt(false, currentKey, minValue + delta, maxValue, delta);
            
            } else if (currentKey == Configuration::THEME) {
                updateTheme(false);
            
            } else if (currentKey == Configuration::THUMBNAIL_TYPE) {
                updateThumbnailType(false);
            
            } else if (currentKey == Configuration::WIFI) {
                updateWifi();
            
            } else if (currentKey == Configuration::ROTATION) {
                updateRotation();

            } else if (currentKey == Configuration::USB_MODE) {
                updateUSBMode(false);
            
            } else if (currentKey == Configuration::OVERCLOCK) {
                updateOverclock(true);
            
            } else if (currentKey == Configuration::SHOW_FPS) {
                updateShowFPS();
            
            } else if (currentKey == Configuration::LANGUAGE) {
                updateLanguage(false);
            
            }
        }
        notifySettingsChange(currentKey, currentValue);
    }

    void navigateRight() override {
        std::cout << "navigate Right" << std::endl;
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::BRIGHTNESS) {
                updateBrightness(true);

            } else if (currentKey == Configuration::VOLUME) {
                updateInt(true, currentKey, minValue, maxValue, delta);

            } else if (currentKey == Configuration::SCREEN_REFRESH) {
                updateInt(true, currentKey, minValue + delta, maxValue, delta);

            } else if (currentKey == Configuration::THEME) {
                updateTheme(true);

            } else if (currentKey == Configuration::THUMBNAIL_TYPE) {
                updateThumbnailType(true);

            } else if (currentKey == Configuration::WIFI) {
                updateWifi();

            } else if (currentKey == Configuration::ROTATION) {
                updateRotation();

            } else if (currentKey == Configuration::USB_MODE) {
                updateUSBMode(false);

            } else if (currentKey == Configuration::OVERCLOCK) {
                updateOverclock(false);

            } else if (currentKey == Configuration::SHOW_FPS) {
                updateShowFPS();

            } else if (currentKey == Configuration::LANGUAGE) {
                updateLanguage(true);
            }  
        }
        notifySettingsChange(currentKey, currentValue);
    }


    void navigateEnter() override {
        std::cout << "navigate Enter" << std::endl;
        // TODO this should notify the observers that the current setting
        //      has been changed
        if (settingsMap[currentKey].enabled) {
            if (currentKey == Configuration::RESTART) {
                restartApplication();
            } else if (currentKey == Configuration::QUIT) {
                quitApplication();
            } else if (currentKey == Configuration::CORE_SELECTION) {
                // TODO this should open the core selection menu
                std::cout << "CORE SELECTION" << std::endl;
                coreSelectionMenu();
            }   
        }
    }

    void updateTheme(bool increase);
    void updateThumbnailType(bool increase);
    void updateUSBMode(bool increase);
    void updateLanguage(bool increase);
    void updateOverclock(bool increase);
    void updateBrightness(bool increase);
    void updateShowFPS();
    void coreSelectionMenu();
    void restartApplication();
    void quitApplication();
    void updateWifi();
    void updateRotation();

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;
};


class SystemSettings : public Settings {
public:

    SystemSettings(Configuration& cfg, I18n& i18n, 
                   int minValue, int maxValue, int delta);
    std::string currentSystem;
    std::vector<Settings::I18nSetting> getSystemSettings();
    void applyCurrentKey() {
        std::replace(currentSystem.begin(), currentSystem.end(), '.', '_');
        currentKey = "SYSTEM." + currentSystem;
    }
    void updateCoreOverride(bool increase);
    void navigateUp() { /*Settings::navigateUp();*/ };
    void navigateDown() { /*Settings::navigateDown();*/};
    void navigateLeft() override {
        std::cout << "navigate Left" << std::endl;
        updateCoreOverride(false);
        notifySettingsChange(currentKey, currentValue);
    };
    void navigateRight() override {
        std::cout << "navigate Right" << std::endl;
        updateCoreOverride(true);
        notifySettingsChange(currentKey, currentValue);
    };
    void navigateEnter() override {};

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;
    std::string getDefaultCore(std::string systemName, Cache& cache) {
        //check if we have an override in ini file, if not return the SelectedExec from json
        if(!settingsMap[currentKey].value.empty())
            return settingsMap[currentKey].value;
        else
        {
            std::map<std::string, ConsoleData> consoleDataMap = 
                cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");

            // Check if the parentTitle exists in the consoleDataMap
            if (consoleDataMap.find(systemName) != consoleDataMap.end()) {
                return (consoleDataMap[systemName].selectedExec.empty()) ? *cores.begin() : consoleDataMap[systemName].selectedExec;
            }
            else
                return "NOT FOUND! Define selectedExec in systems.json";
        }
    }
    void getCores(std::string systemName, Cache& cache) {

        // Retrieve the systems cache (from memory, if was already read, or file
        // if this is the first time we are reading it)
        std::map<std::string, ConsoleData> consoleDataMap = 
            cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");

        cores.clear();

        // Check if the parentTitle exists in the consoleDataMap
        if (consoleDataMap.find(systemName) != consoleDataMap.end()) {
            // Access the ConsoleData for the parentTitle
            ConsoleData consoleData = consoleDataMap[systemName];

            // Check if the execs vector is not empty
            if (!consoleData.execs.empty()) {
                for(auto exec: consoleData.execs) {
                    cores.insert(exec);
                }
            }
        }
        // By default we select the first core from the list
        std::string currentCore = (consoleDataMap[systemName].selectedExec.empty()) ? *cores.begin() : consoleDataMap[systemName].selectedExec;
        // Sync currentValue so updateListSetting finds the right position
        currentValue = currentCore;
        settingsMap[currentKey] = {currentKey, currentCore, true};
        notifySettingsChange(currentKey, currentCore);
    }
};

class RomSettings : public Settings, public ILanguageObserver {
public:
    RomSettings(Configuration& cfg, I18n& i18n, 
                int minValue, int maxValue, int delta);

    std::vector<Settings::I18nSetting> getRomSettings();
    std::string currentRom;
    std::string currentSystem;
    std::string currentPath;
    void applyCurrentKey() {
        currentKey = RomSettings::getKey(currentSystem, currentRom);
    }

    static std::string getKey(std::string system, std::string rom)
    {
        std::replace(system.begin(), system.end(), '.', '_');
        std::replace(rom.begin(), rom.end(), '.', '_');
        return "ROM." + system + "-" + rom;
    }
    void updateRomOverclock(bool increase);
    void updateAutoStart(bool increase);
    void updateCoreOverride(bool increase);

    void navigateUp() { /* Settings::navigateUp(); */};
    void navigateDown() { /* Settings::navigateDown();*/};
    void navigateEnter() override {
        std::cout << "RomSettings navigate Enter" << std::endl;
    };
    void navigateLeft() override {
        std::cout << "navigate Left" << std::endl;

        updateCoreOverride(false);
        notifySettingsChange(currentKey, currentValue);
    }

    void navigateRight() override {
        std::cout << "navigate Right" << std::endl;
        
        updateCoreOverride(true);
        notifySettingsChange(currentKey, currentValue);
    }
    /**
     * ILanguageObserver methods
     */
    void languageChanged() override;

    /**
     * ISettingsSubject methods 
     */
    std::string getName() override;

public:
    std::string getDefaultCore(Cache& cache) {
        //check if we have an override in ini file, if not return the SelectedExec from json
        if(!settingsMap[currentKey].value.empty())
            return settingsMap[currentKey].value;
        else
        {

            // 1. Check in-memory menu cache first (most up-to-date, includes pending overrides)
            if (!currentPath.empty()) {
                CachedMenuItem item = cache.getMenuItemByPath(currentPath);
                if (!item.core.empty()) {
                    return item.core;
                }
            }
            // 2. Fallback to system default from systems.json
            std::map<std::string, ConsoleData> consoleDataMap =
                cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");
            if (consoleDataMap.find(currentSystem) != consoleDataMap.end()) {
                return (consoleDataMap[currentSystem].selectedExec.empty()) ? *cores.begin() : consoleDataMap[currentSystem].selectedExec;
            }
            return "NOT FOUND! Define selectedExec in systems.json";
        }
    }
    void getCores(std::string systemName, Cache& cache) {

        // Retrieve the systems cache (from memory, if was already read, or file
        // if this is the first time we are reading it)
        std::map<std::string, ConsoleData> consoleDataMap = 
            cache.systemsCacheLoad(cfg.get(Configuration::HOME_PATH) + "systems.json");

        cores.clear();

        // Check if the parentTitle exists in the consoleDataMap
        if (consoleDataMap.find(systemName) != consoleDataMap.end()) {
            // Access the ConsoleData for the parentTitle
            ConsoleData consoleData = consoleDataMap[systemName];

            // Check if the execs vector is not empty
            if (!consoleData.execs.empty()) {
                for(auto exec: consoleData.execs) {
                    cores.insert(exec);
                }
            }
        }

        // 1. Start with system default (selectedExec or first core)
        std::string currentCore = (consoleDataMap[systemName].selectedExec.empty()) ? *cores.begin() : consoleDataMap[systemName].selectedExec;
        // 2. Check in-memory menu cache for a ROM-level override (most up-to-date)
        if (!currentPath.empty()) {
            CachedMenuItem item = cache.getMenuItemByPath(currentPath);
            if (!item.core.empty()) {
                currentCore = item.core;
            }
        }
        // Sync currentValue so updateListSetting can find the right position in the list
        currentValue = currentCore;
        settingsMap[currentKey] = {currentKey, currentCore, true};
        notifySettingsChange(currentKey, currentCore);
    }
};