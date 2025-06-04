#include "../include/Portal.hpp"

Portal::Portal(Position entry, Position exit, bool preserve_direction, std::vector<std::string> allowed_colors, int cooldown)
{
    this->entry = entry;
    this->exit = exit;
    this->preserve_direction = preserve_direction;
    this->allowed_colors = allowed_colors;
    this->cooldown = cooldown;
}

Portal::~Portal()
{
}

