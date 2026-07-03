#include "SectionManager.h"
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

// ─── load() ──────────────────────────────────────────────────────────────────

bool SectionManager::load(const std::string& filePath) {
    sections.clear();

    FILE* fp = fopen(filePath.c_str(), "r");
    if (!fp) {
        std::cerr << "SectionManager: cannot open " << filePath << std::endl;
        return false;
    }

    char buf[32768];
    rapidjson::FileReadStream is(fp, buf, sizeof(buf));
    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (!doc.IsArray()) {
        std::cerr << "SectionManager: " << filePath << " is not a JSON array" << std::endl;
        return false;
    }

    for (const auto& v : doc.GetArray()) {
        SectionItem item;

        if (v.HasMember("name"))   item.name   = v["name"].GetString();
        if (v.HasMember("action")) item.action  = v["action"].GetString();


        if (!item.name.empty() && !item.action.empty())
            sections.push_back(item);
    }

    std::cout << "SectionManager: loaded " << sections.size() << " sections from " << filePath << std::endl;
    return !sections.empty();
}

// ─── parseAction() ───────────────────────────────────────────────────────────

SectionAction SectionManager::parseAction(const std::string& action) {
    SectionAction sa;
    sa.type  = SectionActionType::UNKNOWN;
    sa.param = "";

    if (action == "SETTINGS") {
        sa.type = SectionActionType::SETTINGS;
    } else if (action == "FAVORITES") {
        sa.type = SectionActionType::FAVORITES;
    } else if (action == "HISTORY") {
        sa.type = SectionActionType::HISTORY;
    } else if (action.substr(0, 8) == "SYSTEMS-") {
        sa.type  = SectionActionType::SYSTEMS;
        sa.param = action.substr(8);   // "SYSTEMS-Arcade" → "Arcade"
    } else if (action.substr(0, 8) == "ROMLIST-") {
        sa.type  = SectionActionType::ROMLIST;
        sa.param = action.substr(8);   // "ROMLIST-apps" → "apps"
    }

    return sa;
}