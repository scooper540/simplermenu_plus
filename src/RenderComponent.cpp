#include <string>
#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <boost/filesystem.hpp>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>
#include <fstream>

#include "RenderComponent.h"
#include "Configuration.h"

std::unordered_map<std::string, std::string> RenderComponent::aliasMap;

RenderComponent::RenderComponent(Configuration& cfg, Theme& theme, FavoritesManager& fav) 
    : cfg(cfg), theme(theme), fav(fav) {

    lastLoadedBackground = "";
    lastRom = -1;
    // Implementation
    loadAliases();
}

RenderComponent::~RenderComponent() {
    if(background) {
        SDL_FreeSurface(background);
    }
    if(thumbnail) {
        SDL_FreeSurface(thumbnail);
    }
    if(tmpThumbnail) {
        SDL_FreeSurface(tmpThumbnail);
    }
    if(favoritePicture) {
        SDL_FreeSurface(favoritePicture);
    }
    if(font) {
        TTF_CloseFont(font);
    }
    if(battSurface) {
        SDL_FreeSurface(battSurface);
    }
    if(screen) {
        SDL_FreeSurface(screen);
    }
    if(titleFont)
        TTF_CloseFont(titleFont);
    if(settingsFont)
        TTF_CloseFont(settingsFont);
    // Implementation
}

void RenderComponent::drawSystem(const std::string& name, const std::string& path, int numRoms) {
    std::string backgroundPath = cfg.getThemePath() + theme.getValue(name + ".logo");

    if(theme.getValue(name + ".logo") != "NOT FOUND") {
	    if (background == nullptr || lastLoadedBackground != backgroundPath) {
       	    setBackground(backgroundPath);
    	}
    	SDL_BlitSurface(background, NULL, screen, NULL);
    } else {
        clearScreen();
        renderText(name, screenWidth / 2 , screenHeight / 2 , {255, 255, 255}, 1); 
    }

    // As before, determine x, y positions and styles
    //renderText(name, 50, 50, {255, 255, 255}); 
    //renderText(path, 50, 100, {200, 200, 200}); 

    if(theme.getIntValue(Configuration::DISPLAY_GAME_COUNT) == 1 ) {
        int x = theme.getIntValue(Configuration::GAME_COUNT_X);
        int y = theme.getIntValue(Configuration::GAME_COUNT_Y);
        SDL_Color color = theme.getColor(Configuration::GAME_COUNT_FONT_COLOR);
        renderText(std::to_string(numRoms) + (numRoms == 1 ? " GAME" : " GAMES"), x, y, color, theme.getIntValue(Configuration::GAME_COUNT_ALIGNMENT));
    }

}

void RenderComponent::drawRomList(const std::string& systemName, const std::vector<std::pair<std::string, std::string>>& romData, int currentRomIndex) {

    std::string backgroundPath =  cfg.getThemePath() + theme.getValue(Configuration::THEME_BACKGROUND);

	if (background == nullptr || lastLoadedBackground != backgroundPath) {
        setBackground(backgroundPath);
    }
    SDL_BlitSurface(background, NULL, screen, NULL);

    // Set rom list starting position and item separation
    int startX = theme.getIntValue(Configuration::GAME_LIST_X);
    int startY = theme.getIntValue(Configuration::GAME_LIST_Y);
    int stepY = theme.getIntValue(Configuration::ITEMS_SEPARATION);

    int itemsPerPage = theme.getIntValue(Configuration::ITEMS);

    // Calculate number of pages FIXME: move that to the constructor
    int total_pages = (romData.size() + itemsPerPage - 1)/ itemsPerPage;

    int currentPage = currentRomIndex / itemsPerPage;
    int startIndex = currentPage * itemsPerPage;
    int endIndex = std::min<int>(startIndex + itemsPerPage, romData.size());

    // for (int i = 0; i < theme.getIntValue(Configuration::ITEMS); i++) {
    for (int i = startIndex; i < endIndex; i++) {
        SDL_Color color = (i == currentRomIndex) ? 
            theme.getColor(Configuration::SEL_ITEM_FONT_COLOR) :
            theme.getColor(Configuration::ITEMS_FONT_COLOR);
        std::string alias = getAlias(romData[i].first);

        // Determine text width
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, alias.c_str(), color);
        int titleWidth = textSurface->w;

        // TODO replace clipWidth the correct width based on theme.ini settings
        int clipWidth = theme.getIntValue("GENERAL.game_list_w");

        // Create the scrolling view for titles that are too wide
        if(i == currentRomIndex) {
            if (i == currentRomIndex && SDL_GetTicks() - selectTime > SCROLL_TIMEOUT) {
                if (scrollPixelPosition < titleWidth - clipWidth) {
                    scrollPixelPosition += 1;  // Increment by 1 pixel. Adjust for faster scrolling.
                    if (scrollPixelPosition == titleWidth - clipWidth) {
                        // Record the time when scrolling completes
                        scrollEndTime = SDL_GetTicks();
                    }
                } else if (SDL_GetTicks() - scrollEndTime > END_SCROLL_PAUSE) {
                    // Reset the scroll position after the timeout period has elapsed
                    scrollPixelPosition = 0;
                    selectTime = SDL_GetTicks();
                }
            }
            SDL_Rect destRect = {static_cast<Sint16>(startX - scrollPixelPosition), startY, 0, 0};  // Adjust x position by scrollPixelPosition
            SDL_Rect clipRect = {startX, startY, clipWidth, static_cast<Uint16>(textSurface->h)}; // Ensure text doesn't spill over the intended area

            SDL_SetClipRect(screen, &clipRect);
            SDL_BlitSurface(textSurface, nullptr, screen, &destRect);
        } else {
            SDL_Rect clipRect = {startX, startY, clipWidth, static_cast<Uint16>(textSurface->h)}; // Ensure text doesn't spill over the intended area

            SDL_SetClipRect(screen, &clipRect);
            SDL_BlitSurface(textSurface, nullptr, screen, &clipRect);
        }

        SDL_SetClipRect(screen, NULL);  // Reset the clip rect
        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
        if (i == currentRomIndex) {
            // Add Rom title 
            int x = theme.getIntValue(Configuration::ART_X) + theme.getIntValue(Configuration::ART_MAX_W)/2;
            int y = theme.getIntValue(Configuration::ART_Y) +
                    theme.getIntValue(Configuration::ART_MAX_H) +
                    theme.getIntValue(Configuration::ART_TXT_DIST_FROM_PIC) +
                    theme.getIntValue(Configuration::ART_TXT_DIST_FROM_PIC);
        
            if(alias.find("/") != std::string::npos) {
                size_t position = alias.find("/");
                std::string firstLine = alias.substr(0, position - 1);
                std::string secondLine = alias.substr(position + 2);
                int separation = theme.getIntValue(Configuration::ART_TXT_DIST_FROM_PIC) / 2;

                renderText(firstLine, x, y - separation, {255, 255, 255},1);
                renderText(secondLine, x, y + separation, {255, 255, 255},1);
            } else {
                renderText(alias, x, y, {255, 255, 255},1);
            }
        }



        // Display pagination page number / total_pages at the bottom
        std::string pageInfo = std::to_string(currentPage + 1) + " / " + std::to_string(total_pages);
        int x = theme.getIntValue(Configuration::TEXT2_X);
        int y = theme.getIntValue(Configuration::TEXT2_Y);

        renderText(pageInfo, x, y, {255, 255, 255}, theme.getIntValue(Configuration::TEXT2_ALIGNMENT));

        startY += stepY;
    }

    // Load Thumbnail
    if(thumbnail == nullptr || lastRom != currentRomIndex) {
        loadThumbnail(romData[currentRomIndex].second);
        lastRom = currentRomIndex;
    }
    if(thumbnail != nullptr)
    {
        Sint16 x = theme.getIntValue(Configuration::ART_X); 
        Sint16 y = theme.getIntValue(Configuration::ART_Y); 
        Uint16 w = theme.getIntValue(Configuration::ART_MAX_W); 
        Uint16 h = theme.getIntValue(Configuration::ART_MAX_H); 
        SDL_Rect destRect = {x, y, w, h};
        SDL_BlitSurface(thumbnail, nullptr, screen, &destRect);
    }

    if(fav.isFavorite(romData[currentRomIndex].second))
    {
        //draw favorite picture   
        if (favoritePicture == nullptr)
        {
            //load picture
            std::string favImg = theme.getValue(Configuration::FAVORITE_INDICATOR);
            std::string imgPath = cfg.getThemePath() + favImg;
            if (imgPath.empty() || favImg == "NOT FOUND")
            {
            }
            else
            {
                SDL_Surface* raw = IMG_Load(imgPath.c_str());
                if (raw) {
                    //printf("iamge loaded\n");
                    favoritePicture = SDL_DisplayFormatAlpha(raw);
                    float img_scaling = theme.getFloatValue(Configuration::ICON_SCALE);
                    if(img_scaling != 1)
                    {
                        SDL_Rect dest = {0, 0, favoritePicture->w * img_scaling, favoritePicture->h * img_scaling};
                        SDL_Surface* temp = SDL_CreateRGBSurface(
                        SDL_SWSURFACE,        // surface logicielle
                        favoritePicture->w * img_scaling,
                        favoritePicture->h * img_scaling,
                        favoritePicture->format->BitsPerPixel,
                        favoritePicture->format->Rmask,
                        favoritePicture->format->Gmask,
                        favoritePicture->format->Bmask,
                        favoritePicture->format->Amask
                        );
                        SDL_SoftStretch(favoritePicture, NULL, temp, &dest);
                        SDL_FreeSurface(favoritePicture); 
                        favoritePicture = nullptr;
                        favoritePicture = SDL_DisplayFormatAlpha(temp);
                        SDL_FreeSurface(temp);
                        temp = nullptr;
                    }
                    SDL_FreeSurface(raw);
                }
            }
        }
        if(favoritePicture)
        {
            //draw favorite on thumbnail position
            Sint16 x = theme.getIntValue(Configuration::ART_X); 
            Sint16 y = theme.getIntValue(Configuration::ART_Y); 
            SDL_Rect destRect = {x, y, 0, 0};
            SDL_BlitSurface(favoritePicture, nullptr, screen, &destRect);
        }
    }

    // Add Folder Title
    renderText(systemName, theme.getIntValue(Configuration::TEXT1_X), theme.getIntValue(Configuration::TEXT1_Y), {255, 255, 255}, theme.getIntValue(Configuration::TEXT2_ALIGNMENT)); 
}

////
void RenderComponent::drawSettingsMenu(
    const std::string& settingsTitle,
    const std::vector<Settings::I18nSetting>& settingList,
    int currentSettingIndex
) {
    //verify we are not out of bounds
    int sectionSize = settingList.size();
    if (!settingList.empty()) {
        currentSettingIndex = std::min(currentSettingIndex, (int)settingList.size() - 1);
    } else {
        currentSettingIndex = 0;
    }
    std::string backgroundPath =  cfg.getThemePath() + theme.getValue(Configuration::SETTINGS_BACKGROUND);
    std::string settingsFontPath =  cfg.getThemePath() + theme.getValue(Configuration::SETTINGS_FONT);
    int settingsFontSize = theme.getIntValue(Configuration::SETTINGS_ITEM_FONT_SIZE); 
    if(settingsFont == nullptr)
    {
        std::cout << "new font" <<std::endl;
        settingsFont = TTF_OpenFont(settingsFontPath.c_str(), settingsFontSize);
    }
    if (background == nullptr || lastLoadedBackground != backgroundPath) {
        std::cout << "new background" <<std::endl;
        setBackground(backgroundPath);
    }
    SDL_BlitSurface(background, NULL, screen, NULL);

    int titleFontSize = theme.getIntValue(Configuration::SETTINGS_TITLE_FONT_SIZE);
    if(titleFont == nullptr) 
    {
        std::cout << "new font" <<std::endl;
        titleFont = TTF_OpenFont(settingsFontPath.c_str(), titleFontSize);
    }
    SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(titleFont, settingsTitle.c_str(), {255,255,255});
    SDL_Rect titlePos = {screenWidth / 2 - titleSurface->w /2 , 5, 0,0};
    SDL_BlitSurface(titleSurface, nullptr, screen, &titlePos);
    SDL_FreeSurface(titleSurface);
    titleSurface = nullptr;

    int startX = theme.getIntValue(Configuration::SETTINGS_ITEM_START_X);
    int startY = theme.getIntValue(Configuration::SETTINGS_ITEM_START_Y);
    int stepY = theme.getIntValue(Configuration::SETTINGS_ITEM_STEP_Y);
    int itemsPerPage = theme.getIntValue(Configuration::SETTINGS_ITEM_PER_PAGE);
    
    int total_pages = (sectionSize + itemsPerPage - 1) / itemsPerPage;
    int currentPage = currentSettingIndex / itemsPerPage;
    int startIndex = currentPage * itemsPerPage;
    if (startIndex >= settingList.size())
        startIndex = 0;
    int endIndex = std::min<int>(startIndex + itemsPerPage, settingList.size());

    for (int i = startIndex; i < endIndex; i++) {
        SDL_Color color = (i == currentSettingIndex) ? 
            theme.getColor(Configuration::SEL_ITEM_FONT_COLOR) :
            theme.getColor(Configuration::ITEMS_FONT_COLOR);

        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(
            settingsFont, 
            settingList[i].title.c_str(),
            color);

        int clipWidth = (int)screenWidth * 0.8;
        SDL_Rect clipRect = {startX, startY, clipWidth, static_cast<Uint16>(textSurface->h)};
        SDL_SetClipRect(screen, &clipRect);
        SDL_BlitSurface(textSurface, nullptr, screen, &clipRect);
        SDL_SetClipRect(screen, NULL);
        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
        
        std::string pageInfo = std::to_string(currentPage + 1) + " / " + std::to_string(total_pages);
        int x = theme.getIntValue(Configuration::TEXT2_X);
        int y = theme.getIntValue(Configuration::TEXT2_Y);
        renderText(pageInfo, x, y, {255, 255, 255}, theme.getIntValue(Configuration::TEXT2_ALIGNMENT));

        std::string settingsValue = settingList[i].value;
        if (settingsValue == "INTERNAL" || settingsValue.empty()) { 
            settingsValue = ". . .";
        }

        SDL_Surface* valueSurface = TTF_RenderUTF8_Blended(settingsFont, settingsValue.c_str(), color);

        SDL_Rect valueDestRect = {static_cast<Sint16>(screenWidth - valueSurface->w - 10), startY, 0, 0};
        SDL_BlitSurface(valueSurface, nullptr, screen, &valueDestRect);
        SDL_FreeSurface(valueSurface);
        valueSurface = nullptr;

        startY += stepY;
    }
}
void RenderComponent::drawMessage(const std::string& msg)
{
    clearScreen();

    std::string settingsFontPath = cfg.getThemePath() + theme.getValue(Configuration::SETTINGS_FONT);

    int settingsFontSize = theme.getIntValue(Configuration::SETTINGS_ITEM_FONT_SIZE);

    if(settingsFont == nullptr)
        settingsFont = TTF_OpenFont(settingsFontPath.c_str(), settingsFontSize);

    if (!settingsFont)
        return;

    std::vector<std::string> lines = wrapText(msg);

    int offsetY = 0;
    int lineHeight = TTF_FontLineSkip(settingsFont);

    for (const std::string& line : lines)
    {
        if (line.empty())
        {
            offsetY += lineHeight;
            continue;
        }

        SDL_Surface* text =
            TTF_RenderUTF8_Blended(settingsFont, line.c_str(), {255,255,255});

        if (!text)
            continue;

        SDL_Rect dst;

        dst.x = (screenWidth - text->w) / 2;
        dst.y = (screenHeight - text->h) / 2 + offsetY;

        SDL_BlitSurface(text, NULL, screen, &dst);

        SDL_FreeSurface(text);

        offsetY += lineHeight;
    }
}
void RenderComponent::loadThumbnail(const std::string& romPath) 
{
    boost::filesystem::path path(romPath);
    std::string romName = path.stem().string();
    std::string parentFolderName = path.parent_path().filename().string();

    // Dossiers à scanner : [Dossier actuel]/images et [Dossier parent]/images
    std::vector<boost::filesystem::path> searchDirs;
    searchDirs.push_back((path.parent_path() / cfg.get(Configuration::IMAGES_PATH)).lexically_normal());
    searchDirs.push_back((path.parent_path() / ".." / cfg.get(Configuration::IMAGES_PATH)).lexically_normal());

    boost::filesystem::path romImage;
    bool found = false;

    for (const auto& imagesDir : searchDirs) {
        if (boost::filesystem::exists(imagesDir) && boost::filesystem::is_directory(imagesDir)) {
            for (const auto& entry : boost::filesystem::directory_iterator(imagesDir)) {
                std::string entryStem = entry.path().stem().string();
                
                // Vérifie si l'image correspond au nom de la ROM OU au nom du dossier parent
                if (entryStem == romName || entryStem == parentFolderName) {
                    romImage = entry.path();
                    found = true;
                    break;
                }
            }
        }
        if (found) break;
    }

    if (!found) 
    {
        if (thumbnail != nullptr)
            SDL_FreeSurface(thumbnail);
        thumbnail = nullptr;
        return;
    }
    if(thumbnail != nullptr)
    {
        SDL_FreeSurface(thumbnail);
        thumbnail = nullptr;
    }
    
    if(tmpThumbnail != nullptr) //error when loading thumbnail image
    { 
        SDL_FreeSurface(tmpThumbnail);
        tmpThumbnail = nullptr;
    }
    std::string pathStr = romImage.string();
    tmpThumbnail = IMG_Load(pathStr.c_str());
    if(tmpThumbnail == nullptr) //error when loading thumbnail image
    { 
        return;
    }
    int thumbnailWidth = theme.getIntValue(Configuration::ART_MAX_W);
    int thumbnailHeight = theme.getIntValue(Configuration::ART_MAX_H);

    // Check if the thumbnail needs to be resized
    if (tmpThumbnail->w != thumbnailWidth || tmpThumbnail->h != thumbnailHeight) 
    {
        SDL_Surface* formatted = SDL_DisplayFormat(tmpThumbnail);
        SDL_FreeSurface(tmpThumbnail);
        tmpThumbnail = nullptr;
        //tmpThumbnail = SDL_DisplayFormat(tmpThumbnail);
        SDL_Rect dest = {0, 0, thumbnailWidth, thumbnailHeight};
        SDL_Surface* temp = SDL_CreateRGBSurface(
                SDL_SWSURFACE,        // surface logicielle
                thumbnailWidth,
                thumbnailHeight,
                screen->format->BitsPerPixel,
                screen->format->Rmask,
                screen->format->Gmask,
                screen->format->Bmask,
                screen->format->Amask
        );
        SDL_SoftStretch(formatted, NULL, temp, &dest);
        //SDL_BlitSurface(formatted, NULL, temp, &dest);
        if (thumbnail) {
            SDL_FreeSurface(thumbnail);
            thumbnail = nullptr;
        }
        thumbnail = SDL_DisplayFormat(temp);

        if (temp) {
            SDL_FreeSurface(temp);
            temp = nullptr;
        }
        if (tmpThumbnail) {
            SDL_FreeSurface(tmpThumbnail);
            tmpThumbnail = nullptr;
        }
        if (formatted) {
            SDL_FreeSurface(formatted);
            formatted = nullptr;
        }
    }
    else
    {
        if (thumbnail != nullptr) 
            SDL_FreeSurface(thumbnail);
        thumbnail = nullptr;
        thumbnail = SDL_DisplayFormat(tmpThumbnail);
        SDL_FreeSurface(tmpThumbnail);
        tmpThumbnail = nullptr;
    }
}

void RenderComponent::printFPS(int fps) {
    // Display FPS page number / total_pages at the bottom
    if(cfg.getBool(Configuration::SHOW_FPS)) {

        std::string fpsText = "FPS: " + std::to_string(fps);

        SDL_Surface* rawTextSurface = TTF_RenderUTF8_Blended(font, fpsText.c_str(), {255,255,0});
        if (!rawTextSurface) {
            return;
        }

        SDL_Surface* textSurface = SDL_DisplayFormatAlpha(rawTextSurface);
        SDL_FreeSurface(rawTextSurface);
        rawTextSurface = nullptr;
        if(!textSurface) {
            return;
        }

        SDL_Rect destRect = {screenWidth - textSurface->w - 10, 10, 0, 0};  // Position for page counter
	    SDL_BlitSurface(textSurface, NULL, screen, &destRect);

        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
    }
}

void RenderComponent::loadAliases() {
    std::ifstream infile(cfg.get(Configuration::HOME_PATH) + cfg.get(Configuration::ALIAS_PATH));
    std::string line;
    while (std::getline(infile, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string filename = line.substr(0, pos);
            std::string alias = line.substr(pos + 1);
            aliasMap[filename] = alias;
        }
    }
}

std::string RenderComponent::getAlias(const std::string& title) {
    std::string displayTitle = title;
	boost::filesystem::path romPath(title);
    std::string filenameWithoutExt = romPath.stem().string();
    // Check if the rom name exists in the alias map
    if (aliasMap.find(filenameWithoutExt) != aliasMap.end()) {
        displayTitle = aliasMap[filenameWithoutExt];
    } else {
        // If no alias is available, remove the file extension
        displayTitle = filenameWithoutExt;
    }
    
    return displayTitle;
}

void RenderComponent::update() {
    if (SDL_Flip(screen) == -1) {
        std::cerr << "SDL_Flip failed: " << SDL_GetError() << std::endl;
        return;  // or handle the error as appropriate
    }
}

void RenderComponent::printBattery() {
    // Refresh battery level every 30 seconds
    Uint32 now = SDL_GetTicks();
    if (now - battTimer >= 30000 || battTimer == 0) {
        battTimer = now;

        // Read charging status
        std::ifstream statusFile("/sys/class/power_supply/battery/status");
        std::string status;
        if (statusFile >> status)
            battCharging = (status == "Charging");

        // Read capacity
        std::ifstream capFile("/sys/class/power_supply/battery/capacity");
        if (capFile >> battLevel) {
            if (battLevel < 0)   battLevel = 0;
            if (battLevel > 100) battLevel = 100;
        }
    }

    // Pick the right image key
    std::string imgKey;
    if (battCharging) {
        imgKey = theme.getValue(Configuration::BATT_CHARGING);
    } else if (battLevel <= 20) {
        imgKey = theme.getValue(Configuration::BATT_1);
    } else if (battLevel <= 40) {
        imgKey = theme.getValue(Configuration::BATT_2);
    } else if (battLevel <= 60) {
        imgKey = theme.getValue(Configuration::BATT_3);
    } else if (battLevel <= 80) {
        imgKey = theme.getValue(Configuration::BATT_4);
    } else {
        imgKey = theme.getValue(Configuration::BATT_5);
    }

    std::string imgPath = cfg.getThemePath() + imgKey;
    //printf("imgPath = %s\n", imgPath.c_str());
    if (imgPath.empty() || imgKey == "NOT FOUND") return;
    
    // Load image only if it changed
    if (imgPath != lastBattImage) {
        if (battSurface) { SDL_FreeSurface(battSurface); battSurface = nullptr; }
        //printf("imgPath = %s\n", imgPath.c_str());
        SDL_Surface* raw = IMG_Load(imgPath.c_str());
        if (raw) {
            //printf("iamge loaded\n");
            battSurface = SDL_DisplayFormat(raw);
            float img_scaling = theme.getFloatValue(Configuration::ICON_SCALE);
            if(img_scaling != 1)
            {
                SDL_Rect dest = {0, 0, battSurface->w * img_scaling, battSurface->h * img_scaling};
                SDL_Surface* temp = SDL_CreateRGBSurface(
                SDL_SWSURFACE,        // surface logicielle
                battSurface->w * img_scaling,
                battSurface->h * img_scaling,
                battSurface->format->BitsPerPixel,
                battSurface->format->Rmask,
                battSurface->format->Gmask,
                battSurface->format->Bmask,
                battSurface->format->Amask
                );
                SDL_SoftStretch(battSurface, NULL, temp, &dest);
                SDL_FreeSurface(battSurface); 
                battSurface = nullptr;
                battSurface = SDL_DisplayFormat(temp);
                SDL_FreeSurface(temp);
                temp = nullptr;
            }

            SDL_FreeSurface(raw);
        }
        lastBattImage = imgPath;
    }

    if (!battSurface) return;

    int x = theme.getIntValue(Configuration::BATT_X);
    int y = theme.getIntValue(Configuration::BATT_Y);
    SDL_Rect dest = {(Sint16)x, (Sint16)y, 0, 0};
    //printf("display at %d %d\n",x,y);
    SDL_BlitSurface(battSurface, NULL, screen, &dest);
}