#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

struct FavoriteItem {
    std::string system;
    std::string rom;
    std::string path;
};

struct HistoryItem {
    std::string system;
    std::string rom;
    std::string path;
};

class FavoritesManager {
public:
    static const int HISTORY_MAX = 50;

    FavoritesManager() {}

    void load(const std::string& favPath, const std::string& histPath) {
        favoritesPath = favPath;
        historyPath   = histPath;
        loadFavorites();
        loadHistory();
    }

    // --- Favorites ---

    bool isFavorite(const std::string& path) const {
        return std::any_of(favorites.begin(), favorites.end(),
            [&](const FavoriteItem& f){ return f.path == path; });
    }

    void toggleFavorite(const std::string& system, const std::string& rom, const std::string& path) {
        auto it = std::find_if(favorites.begin(), favorites.end(),
            [&](const FavoriteItem& f){ return f.path == path; });
        if (it != favorites.end())
            favorites.erase(it);
        else
            favorites.push_back({system, rom, path});
        saveFavorites();
    }

    const std::vector<FavoriteItem>& getFavorites() const { return favorites; }

    // --- History ---

    void addHistory(const std::string& system, const std::string& rom, const std::string& path) {
        // Remove duplicate if already in history
        history.erase(std::remove_if(history.begin(), history.end(),
            [&](const HistoryItem& h){ return h.path == path; }), history.end());
        // Insert at front
        history.insert(history.begin(), {system, rom, path});
        // Trim to max
        if ((int)history.size() > HISTORY_MAX)
            history.resize(HISTORY_MAX);
        saveHistory();
    }

    const std::vector<HistoryItem>& getHistory() const { return history; }

private:
    std::string favoritesPath;
    std::string historyPath;
    std::vector<FavoriteItem> favorites;
    std::vector<HistoryItem>  history;

    void loadFavorites() {
        favorites.clear();
        FILE* fp = fopen(favoritesPath.c_str(), "r");
        if (!fp) return;
        char buf[65536];
        rapidjson::FileReadStream is(fp, buf, sizeof(buf));
        rapidjson::Document doc;
        doc.ParseStream(is);
        fclose(fp);
        if (!doc.IsArray()) return;
        for (auto& v : doc.GetArray()) {
            FavoriteItem item;
            item.system = v["system"].GetString();
            item.rom    = v["rom"].GetString();
            item.path   = v["path"].GetString();
            favorites.push_back(item);
        }
    }

    void saveFavorites() {
        FILE* fp = fopen(favoritesPath.c_str(), "w");
        if (!fp) return;
        rapidjson::Document doc;
        doc.SetArray();
        auto& alloc = doc.GetAllocator();
        for (auto& f : favorites) {
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("system", rapidjson::Value(f.system.c_str(), alloc), alloc);
            obj.AddMember("rom",    rapidjson::Value(f.rom.c_str(),    alloc), alloc);
            obj.AddMember("path",   rapidjson::Value(f.path.c_str(),   alloc), alloc);
            doc.PushBack(obj, alloc);
        }
        char buf[65536];
        rapidjson::FileWriteStream os(fp, buf, sizeof(buf));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        doc.Accept(writer);
        fclose(fp);
    }

    void loadHistory() {
        history.clear();
        FILE* fp = fopen(historyPath.c_str(), "r");
        if (!fp) return;
        char buf[65536];
        rapidjson::FileReadStream is(fp, buf, sizeof(buf));
        rapidjson::Document doc;
        doc.ParseStream(is);
        fclose(fp);
        if (!doc.IsArray()) return;
        for (auto& v : doc.GetArray()) {
            HistoryItem item;
            item.system = v["system"].GetString();
            item.rom    = v["rom"].GetString();
            item.path   = v["path"].GetString();
            history.push_back(item);
        }
    }

    void saveHistory() {
        FILE* fp = fopen(historyPath.c_str(), "w");
        if (!fp) return;
        rapidjson::Document doc;
        doc.SetArray();
        auto& alloc = doc.GetAllocator();
        for (auto& h : history) {
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("system", rapidjson::Value(h.system.c_str(), alloc), alloc);
            obj.AddMember("rom",    rapidjson::Value(h.rom.c_str(),    alloc), alloc);
            obj.AddMember("path",   rapidjson::Value(h.path.c_str(),   alloc), alloc);
            doc.PushBack(obj, alloc);
        }
        char buf[65536];
        rapidjson::FileWriteStream os(fp, buf, sizeof(buf));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        doc.Accept(writer);
        fclose(fp);
    }
};