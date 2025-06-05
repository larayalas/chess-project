#include "../include/GameManager.hpp"
#include <iostream>
#include "../include/ConfigReader.hpp"
// burada portallar init edilir. init edilen portallar shared pointer ile mape verilir.
// portal init edilirken portal için config okunur.

GameManager::GameManager() : boardSize(8), currentPlayer("white"), gameOver(false) {
    configReader = std::make_shared<ConfigReader>();
}

bool GameManager::initialize(const std::string& configFilePath) {
    // Konfigürasyon dosyasını yükle
    if (!configReader->loadFromFile(configFilePath)) {
        std::cerr << "Konfigürasyon dosyası yüklenemedi: " << configFilePath << std::endl;
        return false;
    }
    
    // Konfigürasyonu doğrula
    if (!configReader->validateConfig()) {
        std::cerr << "Konfigürasyon dosyası geçerli değil." << std::endl;
        return false;
    }
    
    // Oyun ayarlarını al
    const GameConfig& config = configReader->getConfig();
    boardSize = config.game_settings.board_size;
    
    // Taşları oluştur
    std::vector<std::shared_ptr<chessPieces>> pieces;
    
    // Standart taşları ekle
    for (const auto& pieceConfig : config.pieces) {
        for (const auto& colorPositions : pieceConfig.positions) {
            std::string color = colorPositions.first;
            for (const auto& pos : colorPositions.second) {
                auto piece = std::make_shared<chessPieces>(
                    pieceConfig.type,
                    color,
                    pos,
                    pieceConfig.special_abilities
                );
                piece->setMovement(pieceConfig.movement);
                pieces.push_back(piece);
            }
        }
    }
    
    // Özel taşları ekle
    for (const auto& pieceConfig : config.custom_pieces) {
        for (const auto& colorPositions : pieceConfig.positions) {
            std::string color = colorPositions.first;
            for (const auto& pos : colorPositions.second) {
                auto piece = std::make_shared<chessPieces>(
                    pieceConfig.type,
                    color,
                    pos,
                    pieceConfig.special_abilities
                );
                piece->setMovement(pieceConfig.movement);
                pieces.push_back(piece);
            }
        }
    }
    
    // Tahta oluştur
    board = std::make_shared<ChessBoard>(boardSize, pieces);
    
    // MoveValidator oluştur ve ChessBoard ile ilişkilendir
    validator = std::make_shared<MoveValidator>(board);
    
    // ChessBoard'un moveValidator'ünü ayarla
    board->setMoveValidator(validator);
    
    return true;
}

MoveResult GameManager::makeMove(const Position& from, const Position& to) {
    if (gameOver) {
        std::cerr << "Oyun zaten bitti!" << std::endl;
        return MoveResult::InvalidMovePattern;
    }
    
    // Taşın rengini kontrol et
    auto piece = board->getPieceAt(from);
    if (!piece) {
        return MoveResult::NoPieceAtSource;
    }
    
    if (piece->getColor() != currentPlayer) {
        std::cerr << "Sıra " << currentPlayer << " oyuncusunda!" << std::endl;
        return MoveResult::InvalidMovePattern;
    }
    
    // Hareketi yap
    MoveResult result = board->movePiece(from, to);
    
    // Geçerli bir hareket yapıldıysa oyuncuyu değiştir
    if (result == MoveResult::ValidMove || result == MoveResult::EnemyPieceCapturable) {
        // Şah mat kontrolü
        // yeniden hamleleri oluştur
        //board->moveValidator->updateMoveCache();
        std::string oppositeColor = (currentPlayer == "white") ? "black" : "white";
        if (isCheckmate(oppositeColor)) {
            std::cout << oppositeColor << " şah mat oldu! " << currentPlayer << " kazandı!" << std::endl;
            gameOver = true;
        }
        
        // Oyuncuyu değiştir
        switchPlayer();
    }
    
    return result;
}

bool GameManager::isGameOver() const {
    return gameOver;
}

std::string GameManager::getCurrentPlayer() const {
    return currentPlayer;
}

void GameManager::switchPlayer() {
    currentPlayer = (currentPlayer == "white") ? "black" : "white";
}

std::string GameManager::getBoardString() const {
    return board->toString();
}

std::shared_ptr<ChessBoard> GameManager::getBoard() const {
    return board;
}

std::shared_ptr<MoveValidator> GameManager::getValidator() const {
    return validator;
}

bool GameManager::isCheckmate(const std::string& color) {
    return validator->isCheckmate(color);
} 

void GameManager::printRunLog(int x, int y) {
    std::cout << "Oyun logu:" << std::endl;
    std::cout << "Oyuncu: " << currentPlayer << std::endl;
    std::cout << "Tahta:" << std::endl;
    std::cout << "x: " << x << " y: " << y << std::endl;

    // gidebileceği tüm pozisyonları al
    auto validMoves = this->board->moveValidator->moveCache[std::to_string(x) + "," + std::to_string(y)];
    std::cout << "Gidebileceği tüm pozisyonlar:" << std::endl;
    for (const auto& move : validMoves) {
        if(move.result == MoveResult::ValidMove) {
            std::cout << move.from << " -> " << move.to << " " << int(move.type) << " " << int(move.result) << std::endl;
        }
    }
}