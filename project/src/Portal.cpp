#include "Portal.hpp"

Portal::Portal(Position entry, Position exit, bool preserve_direction, std::vector<std::string> allowed_colors, int cooldown, std::string id)
    : entry(entry), exit(exit), preserve_direction(preserve_direction), allowed_colors(allowed_colors), cooldown(cooldown),
      current_cooldown(cooldown), is_active(false), id(id) {}

Portal::~Portal() {}

Position Portal::getEntryPosition() const { return entry; }
Position Portal::getExitPosition() const { return exit; }
bool Portal::getPreserveDirection() const { return preserve_direction; }
std::vector<std::string> Portal::getAllowedColors() const { return allowed_colors; }

int Portal::getCooldown() const { return cooldown; }
int Portal::getCurrentCooldown() const { return current_cooldown; }
void Portal::setCurrentCooldown(int c) { current_cooldown = c; }

bool Portal::isActive() const { return is_active; }
void Portal::setActive(bool active) { is_active = active; }

bool Portal::canUse(const std::string& piece_color) const {
    if (!is_active || current_cooldown > 0) return false;
    if (allowed_colors.empty()) return true;
    for (const auto& color : allowed_colors)
        if (color == piece_color) return true;
    return false;
}

void Portal::use() {
    current_cooldown = cooldown;
    is_active = false;
}
