#include "Configuration.h"
#include "Exception.h"
#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <cstdio>

/////////
// CONFIG.INI
////////

const std::string Configuration::GLOBAL = std::string("GLOBAL");
const std::string Configuration::APPLICATION = std::string("APPLICATION");
const std::string Configuration::SYSTEM = std::string("SYSTEM");
const std::string Configuration::GAME = std::string("GAME");

// CONFIG . GLOBAL section
const std::string Configuration::ALIAS_PATH = std::string("GLOBAL.aliasPath");
const std::string Configuration::HOME_PATH = std::string("GLOBAL.homePath");
const std::string Configuration::THEME_PATH = std::string("GLOBAL.themePath");
const std::string Configuration::ROMS_PATH = std::string("GLOBAL.romsPath");
const std::string Configuration::IMAGES_PATH = std::string("GLOBAL.imagesPath");
const std::string Configuration::SCREEN_DEPTH = std::string("GLOBAL.screenDepth");
const std::string Configuration::GLOBAL_CACHE = std::string("GLOBAL.globalCacheJSON");

#ifdef _WIN32
const std::string Configuration::SCREEN_WIDTH = std::string("GLOBAL.screenWidth");
const std::string Configuration::SCREEN_HEIGHT = std::string("GLOBAL.screenHeight");
#endif

// CONFIG . APPLICATION section
const std::string Configuration::VOLUME = std::string("APPLICATION.volume");
const std::string Configuration::BRIGHTNESS = std::string("APPLICATION.brightness");
const std::string Configuration::SCREEN_REFRESH = std::string("APPLICATION.screenRefresh");
const std::string Configuration::SHOW_FPS = std::string("APPLICATION.showFPS");
const std::string Configuration::OVERCLOCK = std::string("APPLICATION.overclock");
const std::string Configuration::OVERCLOCK_VALUES = std::string("GLOBAL.overclockValues");
const std::string Configuration::THEME = std::string("APPLICATION.theme");
const std::string Configuration::THUMBNAIL_TYPE = std::string("APPLICATION.thumbnailType");
const std::string Configuration::THUMBNAIL_TYPE_VALUES = std::string("GLOBAL.thumbnailTypeValues");
const std::string Configuration::USB_MODE = std::string("APPLICATION.usbMode");
const std::string Configuration::USB_MODE_VALUES = std::string("GLOBAL.usbModeValues");
const std::string Configuration::WIFI = std::string("APPLICATION.wifi");
const std::string Configuration::ROTATION = std::string("APPLICATION.rotation");
const std::string Configuration::LANGUAGE = std::string("APPLICATION.language");
const std::string Configuration::UPDATE_CACHES = std::string("APPLICATION.updateCaches");
const std::string Configuration::CORE_SETTINGS = std::string("APPLICATION.coreSettings");
const std::string Configuration::RESTART = std::string("APPLICATION.restart");
const std::string Configuration::QUIT = std::string("APPLICATION.quit");

// CONFIG . SYSTEM section
const std::string Configuration::CORE_SELECTION = std::string("SYSTEM.coreSelection");

// CONFIG . GAME section
const std::string Configuration::ROM_OVERCLOCK = std::string("GAME.romOverclock");
const std::string Configuration::ROM_AUTOSTART = std::string("GAME.romAutostart");
const std::string Configuration::CORE_OVERRIDE = std::string("GAME.coreOverride");

/////////
// THEME.INI
/////////

// THEME . DEFAULT section
const std::string Configuration::SEL_ITEM_FONT_COLOR = std::string("GENERAL.selected_item_font_color");
const std::string Configuration::ITEMS_FONT_COLOR = std::string("GENERAL.items_font_color");
const std::string Configuration::ROM_SEL_ITEM_FONT_COLOR = std::string("GENERAL.rom_selected_item_font_color");
const std::string Configuration::ROM_ITEMS_FONT_COLOR = std::string("GENERAL.rom_items_font_color");
const std::string Configuration::THEME_BACKGROUND = std::string("GENERAL.background");
const std::string Configuration::SETTINGS_BACKGROUND = std::string("GENERAL.settings_background");
const std::string Configuration::SETTINGS_FONT = std::string("GENERAL.settings_font");
const std::string Configuration::THEME_GRID = std::string("GENERAL.is_grid");
// THEME . GENERAL section
const std::string Configuration::ART_X = std::string("GENERAL.art_x");
const std::string Configuration::ART_Y = std::string("GENERAL.art_y");
const std::string Configuration::ART_MAX_W = std::string("GENERAL.art_max_w");
const std::string Configuration::ART_MAX_H = std::string("GENERAL.art_max_h");
const std::string Configuration::ART_TXT_X = std::string("GENERAL.art_text_x");
const std::string Configuration::ART_TXT_Y = std::string("GENERAL.art_text_y");
const std::string Configuration::DISPLAY_GAME_COUNT = std::string("GENERAL.display_game_count");
const std::string Configuration::GAME_COUNT_ALIGNMENT = std::string("GENERAL.game_count_alignment");
const std::string Configuration::GAME_COUNT_FONT_COLOR = std::string("GENERAL.game_count_font_color");
const std::string Configuration::GAME_COUNT_X = std::string("GENERAL.game_count_x");
const std::string Configuration::GAME_COUNT_Y = std::string("GENERAL.game_count_y");
const std::string Configuration::GAME_LIST_X = std::string("GENERAL.game_list_x");
const std::string Configuration::GAME_LIST_Y = std::string("GENERAL.game_list_y");
const std::string Configuration::GAME_LIST_BG_ITEM = std::string("GENERAL.game_list_bg_item");
const std::string Configuration::GAME_LIST_BG_ITEM_SELECTED = std::string("GENERAL.game_list_bg_item_selected");
const std::string Configuration::GAME_LIST_BG_ITEM_W= std::string("GENERAL.game_list_bg_item_w");
const std::string Configuration::GAME_LIST_BG_ITEM_H= std::string("GENERAL.game_list_bg_item_h");
const std::string Configuration::ITEMS = std::string("GENERAL.items");
const std::string Configuration::ITEMS_SEPARATION = std::string("GENERAL.items_separation");
const std::string Configuration::TEXT1_X= std::string("GENERAL.text1_x");
const std::string Configuration::TEXT1_Y= std::string("GENERAL.text1_y");
const std::string Configuration::TEXT1_ALIGNMENT= std::string("GENERAL.text1_alignment");
const std::string Configuration::TEXT2_X= std::string("GENERAL.text2_x");
const std::string Configuration::TEXT2_Y = std::string("GENERAL.text2_y");
const std::string Configuration::TEXT2_ALIGNMENT = std::string("GENERAL.text2_alignment");
const std::string Configuration::THEME_FONT = std::string("GENERAL.font");
const std::string Configuration::ICON_SCALE = std::string("GENERAL.icon_scale");

const std::string Configuration::BATT_X = std::string("GENERAL.batt_x");
const std::string Configuration::BATT_Y = std::string("GENERAL.batt_y");
const std::string Configuration::BATT_1 = std::string("GENERAL.batt_1");
const std::string Configuration::BATT_2 = std::string("GENERAL.batt_2");
const std::string Configuration::BATT_3 = std::string("GENERAL.batt_3");
const std::string Configuration::BATT_4 = std::string("GENERAL.batt_4");
const std::string Configuration::BATT_5 = std::string("GENERAL.batt_5");
const std::string Configuration::BATT_CHARGING = std::string("GENERAL.batt_charging");

const std::string Configuration::FAVORITE_INDICATOR = std::string("GENERAL.favorite_indicator");

const std::string Configuration::SETTINGS_ITEM_FONT_SIZE = std::string("GENERAL.settings_item_size");
const std::string Configuration::SETTINGS_TITLE_FONT_SIZE = std::string("GENERAL.settings_title_size");
const std::string Configuration::SETTINGS_ITEM_START_X = std::string("GENERAL.settings_item_start_x");
const std::string Configuration::SETTINGS_ITEM_START_Y = std::string("GENERAL.settings_item_start_y");
const std::string Configuration::SETTINGS_ITEM_STEP_Y = std::string("GENERAL.settings_item_step_y");
const std::string Configuration::SETTINGS_ITEM_PER_PAGE = std::string("GENERAL.settings_item_per_page");



const std::string Configuration::THEME_GRID_COL_COUNT= std::string("GENERAL.theme_grid_col_count");
const std::string Configuration::THEME_GRID_ROW_COUNT= std::string("GENERAL.theme_grid_row_count");
const std::string Configuration::THEME_GRID_TEXT_SYSTEM_FONT_PATH = std::string("GENERAL.theme_grid_text_system_font_path");
const std::string Configuration::THEME_GRID_TEXT_SYSTEM_FONT_SIZE = std::string("GENERAL.theme_grid_text_system_font_size");
const std::string Configuration::THEME_GRID_TEXT_SYSTEM_COLOR= std::string("GENERAL.theme_grid_text_system_color");
const std::string Configuration::THEME_GRID_TEXT_SYSTEM_COLOR_SELECTED= std::string("GENERAL.theme_grid_text_system_color_selected");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_W= std::string("GENERAL.theme_grid_img_system_w");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_H= std::string("GENERAL.theme_grid_img_system_h");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_START_X= std::string("GENERAL.theme_grid_img_system_start_x");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_START_Y= std::string("GENERAL.theme_grid_img_system_start_y");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_INCREMENT_COL= std::string("GENERAL.theme_grid_img_system_increment_col");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_INCREMENT_ROW= std::string("GENERAL.theme_grid_img_system_increment_row");
const std::string Configuration::THEME_GRID_IMG_BG= std::string("GENERAL.theme_grid_img_system_bg");
const std::string Configuration::THEME_GRID_IMG_BACKGROUND= std::string("GENERAL.theme_grid_img_system_background");
const std::string Configuration::THEME_GRID_IMG_BG_SELECTED= std::string("GENERAL.theme_grid_img_system_bg_selected");
const std::string Configuration::THEME_GRID_IMG_BG_W= std::string("GENERAL.theme_grid_img_system_bg_w");
const std::string Configuration::THEME_GRID_IMG_BG_H= std::string("GENERAL.theme_grid_img_system_bg_h");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_X= std::string("GENERAL.theme_grid_img_system_x");
const std::string Configuration::THEME_GRID_IMG_SYSTEM_Y= std::string("GENERAL.theme_grid_img_system_y");
const std::string Configuration::THEME_GRID_TEXT_SYSTEM_X= std::string("GENERAL.theme_grid_text_system_x");
const std::string Configuration::THEME_GRID_TEXT_SYSTEM_Y= std::string("GENERAL.theme_grid_text_system_y");


/////////
// <SECTION>.INI
/////////

const std::string Configuration::CONSOLES = std::string("CONSOLES");

// <SECTION> . CONSOLES section
const std::string Configuration::CONSOLES_LIST = std::string("CONSOLES.consoleList");

// <SECTION> . <CONSOLE_NAME> section
const std::string Configuration::CONSOLE_EXECS = std::string(".execs");
const std::string Configuration::CONSOLE_ROM_DIRS = std::string(".romDirs");
const std::string Configuration::CONSOLE_ROM_EXTS = std::string(".romExts");
const std::string Configuration::CONSOLE_ALIAS_FILE = std::string(".aliasFile");


//////////
// Savestate.json
//////////
const std::string Configuration::CURRENT_MENU_LEVEL = std::string("currentMenuLevel");
const std::string Configuration::CURRENT_SYSTEM_INDEX = std::string("currentSystemIndex");
const std::string Configuration::CURRENT_ROM_INDEX = std::string("currentRomIndex");
const std::string Configuration::LAUNCHER_CALLBACK = std::string("launcherCallback");


Configuration::Configuration(const std::string& configIniFilepath, 
                             const std::string& stateFilepath) 
    : configIniFilepath(configIniFilepath), stateFilepath(stateFilepath) {

    // Load values from .ini file using Boost.PropertyTree
    boost::property_tree::ini_parser::read_ini(configIniFilepath, mainPt);
}

void Configuration::set(const std::string& id, const std::string& value) {
    mainPt.put(id, value);
}

std::string Configuration::get(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error 
    return mainPt.get<std::string>(id);
}

bool Configuration::getBool(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error
    return mainPt.get<bool>(id);

    // std::string value = boost::algorithm::to_lower_copy(get(id));
    // try {
    //     return value == "on";
    // } catch (const std::exception& e) {
    //     throw ValueConversionException("Error converting value for id '"
    //         + id + "' to boolean: " + e.what());
    // }
}

int Configuration::getInt(const std::string& id) const {
    // TODO exceptions  ptree_bad_path, ptree_bad_data, both from ptree_error
    return mainPt.get<int>(id);
    // std::string value = getValue(id);
    // try {
    //     return std::stoi(value);
    // } catch (const std::exception& e) {
    //     throw ValueConversionException("Error converting value for id '"
    //         + this->getIniKey(id) + "' to integer: " + e.what());
    // }
}

std::set<std::string> Configuration::getList(const std::string& id, 
                                             const char delimiter) const {
    std::set<std::string> result;
    std::string value = get(id);
    
    size_t start = 0;
    size_t end = value.find(delimiter);
    while (end != std::string::npos) {
        result.insert(value.substr(start, end - start));
        start = end + 1;
        end = value.find(delimiter, start);
    }
    result.insert(value.substr(start, end));

    return result;
}

std::string Configuration::getThemePath() const {

    // TODO Do we really need to convert to int and then again to string?
    std::string themePath = 
        get(Configuration::HOME_PATH) +
        get(Configuration::THEME_PATH) +
        + "/" + 
        get(Configuration::THEME) + "/";
        
    return themePath;
}

void Configuration::saveConfigIni() {
    
    boost::property_tree::ini_parser::write_ini(configIniFilepath, mainPt);

    std::cout << configIniFilepath << " written.\n";
}

State Configuration::loadState() {

    boost::property_tree::ptree statePt;
    try {
        boost::property_tree::json_parser::read_json(stateFilepath, statePt);
    } catch (const boost::property_tree::json_parser_error& e) {
        throw StateNotFoundException(
            "Error loading state: " + std::string(e.what()));
    }

    State state;
    std::string currentMenuLevelStr = 
        statePt.get<std::string>(Configuration::CURRENT_MENU_LEVEL);

    if (currentMenuLevelStr == "MENU_SYSTEM") {
        state.currentMenuLevel = MenuLevel::MENU_SYSTEM;
    } else if (currentMenuLevelStr == "MENU_ROM") {
        state.currentMenuLevel = MenuLevel::MENU_ROM;
    } else if (currentMenuLevelStr == "APP_SETTINGS") {
        state.currentMenuLevel = MenuLevel::APP_SETTINGS;
    } else if (currentMenuLevelStr == "SYSTEM_SETTINGS") {
        state.currentMenuLevel = MenuLevel::SYSTEM_SETTINGS;
    } else if (currentMenuLevelStr == "ROM_SETTINGS") {
        state.currentMenuLevel = MenuLevel::ROM_SETTINGS;
    } else {
        throw ValueConversionException(
            "Error loading state: invalid currentMenuLevel value: " 
            + currentMenuLevelStr);
    }

    state.currentSystemIndex = statePt.get<int>(Configuration::CURRENT_SYSTEM_INDEX);
    state.currentRomIndex = statePt.get<int>(Configuration::CURRENT_ROM_INDEX);
    state.launcherCallback = statePt.get<bool>(Configuration::LAUNCHER_CALLBACK);

    std::cout << stateFilepath << " load.\n";

    return state;
}


void Configuration::saveState(const State& state) {
    
        boost::property_tree::ptree statePt;
    
        std::string currentMenuLevelStr;
        switch (state.currentMenuLevel) {
            case MenuLevel::MENU_SYSTEM:
                currentMenuLevelStr = "MENU_SYSTEM";
                break;
            case MenuLevel::MENU_ROM:
                currentMenuLevelStr = "MENU_ROM";
                break;
            case MenuLevel::APP_SETTINGS:
                currentMenuLevelStr = "APP_SETTINGS";
                break;
            case MenuLevel::SYSTEM_SETTINGS:
                currentMenuLevelStr = "SYSTEM_SETTINGS";
                break;
            case MenuLevel::ROM_SETTINGS:
                currentMenuLevelStr = "ROM_SETTINGS";
                break;
            default:
                throw ValueConversionException(
                    "Error saving state: invalid currentMenuLevel value: " 
                    + std::to_string(state.currentMenuLevel));
        }
    
        statePt.put(Configuration::CURRENT_MENU_LEVEL, currentMenuLevelStr);
        statePt.put(Configuration::CURRENT_SYSTEM_INDEX, state.currentSystemIndex);
        statePt.put(Configuration::CURRENT_ROM_INDEX, state.currentRomIndex);
        statePt.put(Configuration::LAUNCHER_CALLBACK, state.launcherCallback);
    
        boost::property_tree::json_parser::write_json(stateFilepath, statePt);
    
        std::cout << stateFilepath << " written.\n";
}
