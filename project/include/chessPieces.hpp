#pragma once

#include <string>
#include <vector>
#include "ConfigReader.hpp"

// forward declaration
struct Position;
struct Movement;
struct SpecialAbilities;
using namespace std;


class chessPieces
{
private:
    string type;
    string color;
    Position pos; // değişken adını değiştirdim
    SpecialAbilities special_abilities;
    Movement movement;
public:
    chessPieces(string type, string color, Position pos, SpecialAbilities special_abilities);
    ~chessPieces();

    void setType(string type);
    string getType() const;

    void setColor(string color);
    string getColor() const;

    void setPosition(Position pos); // tip adını büyük harfle, parametre adını değiştirdim
    Position getPosition() const; // tip adını büyük harfle

    void setSpecialAbilities(SpecialAbilities special_abilities);
    SpecialAbilities getSpecialAbilities() const;

    void setMovement(Movement movement);
    Movement getMovement() const;
};

// piecesler oluştur all piece olarak chessboard ekle.