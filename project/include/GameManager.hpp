#pragma once

#include <memory>
#include "ChessBoard.hpp"
#include "MoveValidator.hpp"
#include "chessPieces.hpp"
#include "ConfigReader.hpp"

class GameManager {
private:
    std::shared_ptr<ChessBoard> board;
    std::shared_ptr<MoveValidator> validator;
    std::shared_ptr<ConfigReader> configReader;
    int boardSize;
    std::string currentPlayer;
    bool gameOver;

public:
    // Constructor
    GameManager();
    
    // Oyunu başlat
    bool initialize(const std::string& configFilePath);
    
    // Taş hareketi
    MoveResult makeMove(const Position& from, const Position& to);
    
    // Oyun durumu kontrolü
    bool isGameOver() const;
    
    // Mevcut oyuncuyu al
    std::string getCurrentPlayer() const;
    
    // Oyuncu değiştir
    void switchPlayer();
    
    // Tahtayı string olarak al
    std::string getBoardString() const;
    
    // Getter fonksiyonları
    std::shared_ptr<ChessBoard> getBoard() const;
    std::shared_ptr<MoveValidator> getValidator() const;
    
    // Şah mat kontrolü
    bool isCheckmate(const std::string& color);

    void printRunLog(int x, int y);
}; 