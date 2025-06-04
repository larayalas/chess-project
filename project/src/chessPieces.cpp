#include "../include/chessPieces.hpp"
#include "../include/ConfigReader.hpp"

chessPieces::chessPieces(string type, string color, Position pos, SpecialAbilities special_abilities) : type(type), color(color), pos(pos), special_abilities(special_abilities)
{

}

chessPieces::~chessPieces()
{
}

string chessPieces::getType() const
{
    return type;
}

string chessPieces::getColor() const
{
    return color;
}

Position chessPieces::getPosition() const
{
    return pos;
}

SpecialAbilities chessPieces::getSpecialAbilities() const
{
    return special_abilities;
}

void chessPieces::setType(string type)
{
    this->type = type;
}

void chessPieces::setColor(string color)
{
    this->color = color;
}

void chessPieces::setPosition(Position pos)
{
    this->pos = pos;
}

void chessPieces::setSpecialAbilities(SpecialAbilities special_abilities)
{
    this->special_abilities = special_abilities;
}

void chessPieces::setMovement(Movement movement)
{
    this->movement = movement;
}

Movement chessPieces::getMovement() const
{
    return movement;
} 