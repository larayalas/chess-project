#pragma once
#include "ConfigReader.hpp"
#include <vector>
#include <string>

class Portal
{
private:
    Position entry;
    Position exit;
    bool preserve_direction;
    std::vector<std::string> allowed_colors;
    int cooldown;
public:
    Portal(Position entry, Position exit, bool preserve_direction, std::vector<std::string> allowed_colors, int cooldown);
    ~Portal();
};

