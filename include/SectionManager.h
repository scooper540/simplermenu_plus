#pragma once
#include <string>
#include <vector>

// ─── Data structures ─────────────────────────────────────────────────────────

struct SectionItem {
    std::string  name;    // display name
    std::string  action;  // SYSTEMS-xxx | ROMLIST-xxx | FAVORITES | HISTORY | MENU-xxx
};

// Resolved action type after parsing
enum class SectionActionType {
    SYSTEMS,    // → filter menu by group tag, enter MENU_SYSTEM
    ROMLIST,    // → go directly to MENU_ROM for a named system
    SETTINGS,   // → open settings
    UNKNOWN
};

struct SectionAction {
    SectionActionType type;
    std::string       param;  // group name for SYSTEMS, system name for ROMLIST
};

// ─── SectionManager ──────────────────────────────────────────────────────────

class SectionManager {
public:
    SectionManager() = default;

    bool load(const std::string& filePath);

    const std::vector<SectionItem>& getSections() const { return sections; }
    int size() const { return (int)sections.size(); }

    // Parse the action string into a typed SectionAction
    static SectionAction parseAction(const std::string& action);

private:
    std::vector<SectionItem> sections;
};