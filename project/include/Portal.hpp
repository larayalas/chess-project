#pragma once

#include "ConfigReader.hpp"
#include <string>
#include <vector>
#include <queue>

class Portal {
private:
    Position entry;
    Position exit;
    bool preserve_direction;
    std::vector<std::string> allowed_colors;
    int cooldown;
    int current_cooldown;
    bool is_active;
    std::string id;

public:
    Portal(Position entry, Position exit, bool preserve_direction, std::vector<std::string> allowed_colors, int cooldown, std::string id);
    ~Portal();

    Position getEntryPosition() const;
    Position getExitPosition() const;
    bool getPreserveDirection() const;
    std::vector<std::string> getAllowedColors() const;

    int getCooldown() const;
    int getCurrentCooldown() const;
    void setCurrentCooldown(int cooldown);

    bool isActive() const;
    void setActive(bool active);

    bool canUse(const std::string& piece_color) const;
    void use();
};
