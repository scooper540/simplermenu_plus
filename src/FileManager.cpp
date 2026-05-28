#include "FileManager.h"
#include "Configuration.h"
#include <vector>
#include <string>
#include <set>
#include <boost/filesystem.hpp>
#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

std::vector<std::string> FileManager::getFolders(const std::string& path) {
    std::vector<std::string> systems;
    
    for (const auto& entry : boost::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            if(entry.path().filename().string() != "bios") {
                systems.push_back(entry.path().filename().string());
            }
        }
    }

    std::sort(systems.begin(), systems.end());
    return systems;
}

// Retrieve a list of files from a given system (recursive)
std::vector<std::string> FileManager::getFiles(const std::string& system, const std::vector<std::string>& allowedExts) {
	std::cerr << "ENTER" << std::endl;
    std::vector<std::string> files;
    std::set<std::string> excludedExtensions = 
        cfg.getList("GLOBAL.excludedExtensions");

#if defined(_WIN32) || !defined(POWKIDDY)
    try {
        for (const auto& entry : boost::filesystem::directory_iterator(system)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();

                // Exclude files starting with . or ._ (hidden files in UNIX-based systems)
                if (filename[0] == '.' || (filename.length() > 1 && filename[0] == '.' && filename[1] == '_')) {
                    continue;
                }

                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                if (std::find(allowedExts.begin(), allowedExts.end(), ext) != allowedExts.end()) {
                    files.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const boost::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory " << system << ": " << e.what() << std::endl;
    }
#else
//this part should be faster
// todo measure timing on powkiddy to verify
// Fallback for filesystems that return DT_UNKNOWN (FAT32, exFAT...)
    auto getType = [](const std::string& fullPath, unsigned char d_type) -> unsigned char {
        if (d_type != DT_UNKNOWN) return d_type;
        struct ::stat st;
        if (::stat(fullPath.c_str(), &st) != 0) return DT_UNKNOWN;
        if (S_ISREG(st.st_mode)) return DT_REG;
        if (S_ISDIR(st.st_mode)) return DT_DIR;
        return DT_UNKNOWN;
    };
   // Extract extension from filename (returns "" if none)
    auto getExt = [](const std::string& filename) -> std::string {
        size_t dot = filename.rfind('.');
        if (dot == std::string::npos || dot == 0) return "";
        return filename.substr(dot);
    };

    DIR* topDir = opendir(system.c_str());
    if (!topDir) {
        std::cerr << "Cannot open: " << system << std::endl;
        return files;
    }

    struct dirent* topEntry;
    while ((topEntry = readdir(topDir)) != nullptr) {
        // Skip hidden files/dirs and . ..
        if (topEntry->d_name[0] == '.') continue;

        std::string topName(topEntry->d_name);
        std::string topPath = system + "/" + topName;
        unsigned char topType = getType(topPath, topEntry->d_type);

        if (topType == DT_REG) {
            // ROM directly in system folder
            std::string ext = getExt(topName);
            std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c){ return std::tolower(c); });
            if (std::find(allowedExts.begin(), allowedExts.end(), ext) != allowedExts.end()) {
                files.push_back(topName);
            }

        } else if (topType == DT_DIR) {
            // Subfolder � grab first valid ROM file inside
            DIR* subDir = opendir(topPath.c_str());
            if (!subDir) continue;

            struct dirent* subEntry;
            while ((subEntry = readdir(subDir)) != nullptr) {
                if (subEntry->d_name[0] == '.') continue;

                std::string subName(subEntry->d_name);
                std::string subPath = topPath + "/" + subName;
                unsigned char subType = getType(subPath, subEntry->d_type);

                if (subType != DT_REG) continue;

                std::string ext = getExt(subName);
                std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                if (std::find(allowedExts.begin(), allowedExts.end(), ext) != allowedExts.end()) {
                    files.push_back(topName + "/" + subName);
                }
            }
            closedir(subDir);
        }
    }

    closedir(topDir);
#endif
    std::sort(files.begin(), files.end());
    return files;
}


