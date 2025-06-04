#include "../include/ChessBoard.hpp"
#include "../include/MoveValidator.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <functional>
#include <iostream>

// Hazır taşlarla constructor
ChessBoard::ChessBoard(int size, const std::vector<std::shared_ptr<chessPieces>>& pieces) : boardSize(size) {
    // Graf yapısını oluştur ve taşları ekle
    initWithPieces(pieces);
    // MoveValidator'ü başlangıçta nullptr olarak ayarla
    moveValidator = nullptr;
}

// Destructor
ChessBoard::~ChessBoard() {
    // Shared pointer kullanıldığı için özel bir temizleme gerekmiyor
}

// MoveValidator'ü ayarlama
void ChessBoard::setMoveValidator(std::shared_ptr<MoveValidator> validator) {
    moveValidator = validator;
}

// Cache'i temizleme
void ChessBoard::clearCache() {
    // Cache temizlendiğinde moveValidator'ı da güncelle
    if (moveValidator) {
        moveValidator->updateMoveCache();
    }
}

Position ChessBoard::getKingPosition(const std::string& color) {
    // Tahtadaki tüm taşları kontrol et
    for (const auto& piece : allPieces) {
        if (piece->getColor() == color && piece->getType() == "King") {
            return piece->getPosition();
        }
    }
    return {-1,-1};
}

// Position'ı string anahtara dönüştürme
std::string ChessBoard::positionToKey(const Position& pos) const {
    return std::to_string(pos.x) + "," + std::to_string(pos.y);
}

// Belirli konumdaki taşı alma
std::shared_ptr<chessPieces> ChessBoard::getPieceAt(const Position& pos) const {
    std::string key = positionToKey(pos);
    
    if (board.find(key) != board.end()) {
        return board.at(key);
    }
    
    return nullptr;
}

// Tüm taşları alma
const std::vector<std::shared_ptr<chessPieces>>& ChessBoard::getAllPieces() const {
    return allPieces;
}

// Tahtanın boyutunu alma
int ChessBoard::getBoardSize() const {
    return boardSize;
}

// Tahtayı kontrol etme (pozisyon geçerli mi)
bool ChessBoard::isValidPosition(const Position& pos) const {
    return (pos.x >= 0 && pos.x < boardSize && pos.y >= 0 && pos.y < boardSize);
}

// Satranç tahtasını init etme (temel init)
void ChessBoard::init() {
    // Init fonksiyonu boş implementasyon
}

// Dışarıdan taşları alarak init etme
void ChessBoard::initWithPieces(const std::vector<std::shared_ptr<chessPieces>>& pieces) {
    // Tüm taşları tahtaya ekle
    for (const auto& piece : pieces) {
        addPiece(piece);
    }
}

// Tahtaya taş ekleme
void ChessBoard::addPiece(std::shared_ptr<chessPieces> piece) {
    Position pos = piece->getPosition();
    std::string key = positionToKey(pos);
    
    // Konumu kontrol et
    if (!isValidPosition(pos)) {
        throw std::invalid_argument("Geçersiz pozisyon: (" + 
            std::to_string(pos.x) + "," + std::to_string(pos.y) + ")");
    }
    
    // Taşı tahtaya ve listeye ekle
    board[key] = piece;
    allPieces.push_back(piece);
        
    // Cache'i temizle - tahta değişti
    clearCache();
}

// Tahtanın güncel durumunu string olarak gösterme
std::string ChessBoard::toString() const {
    std::stringstream ss;
    
    ss << " ";
    for (int x = 0; x < boardSize; x++) {
        ss << "  " << x << " ";
    }
    ss << std::endl;
    
    for (int y = boardSize - 1; y >= 0; y--) {
        ss << y << " ";
        
        for (int x = 0; x < boardSize; x++) {
            Position pos = {x, y};
            std::string key = positionToKey(pos);
            
            if (board.find(key) != board.end()) {
                auto piece = board.at(key);
                std::string pieceSymbol ;
                if (piece->getType() == "Pawn") {
                    pieceSymbol = "P";
                } else if (piece->getType() == "Rook") {
                    pieceSymbol = "R";
                } else if (piece->getType() == "Knight") {
                    pieceSymbol = "N";
                } else if (piece->getType() == "Bishop") {
                    pieceSymbol = "B";
                } else if (piece->getType() == "Queen") {
                    pieceSymbol = "Q";
                } else if (piece->getType() == "King") {
                    pieceSymbol = "K";
                } else {
                    pieceSymbol = "X"; // Özel taşlar için
                }
                if (piece->getColor() == "white") {
                    pieceSymbol += "W"; // Beyaz taşlar için biçimlendirme
                } else if (piece->getColor() == "black") {
                    pieceSymbol += "B"; // Siyah taşlar için biçimlendirme
                }
                
                ss << "[" << pieceSymbol << "]";
            } else {
                ss << "[  ]";
            }
        }
        
        ss << " " << y << std::endl;
    }
    
    ss << " ";
    for (int x = 0; x < boardSize; x++) {
        ss << "  " << x << " ";
    }
    
    return ss.str();
}

std::unordered_map<std::string, Portal> ChessBoard::getPortals() const {
    return portals;
}


// Taş hareket ettirme
MoveResult ChessBoard::movePiece(const Position& from, const Position& to) {
    if (!moveValidator) {
        return MoveResult::InvalidMovePattern;
    }
    
    MoveResult result = moveValidator->isValidMove(from, to);
    
    if (result == MoveResult::ValidMove || result == MoveResult::EnemyPieceCapturable) {
        // Hedef konumdaki taşı kaldır (eğer varsa)
        std::string toKey = positionToKey(to);
        std::string fromKey = positionToKey(from);
        
        auto piece = getPieceAt(from);
        if (!piece) {
            return MoveResult::NoPieceAtSource;
        }
        
        // Hedefte düşman taşı varsa, tüm taşlar listesinden kaldır
        if (board.find(toKey) != board.end()) {
            auto targetPiece = board[toKey];
            auto it = std::find(allPieces.begin(), allPieces.end(), targetPiece);
            if (it != allPieces.end()) {
                allPieces.erase(it);
            }
        }
        
        // Taşı yeni konuma taşı
        board.erase(fromKey);
        board[toKey] = piece;
        piece->setPosition(to);
        
        // Cache'i temizle - tahta değişti
        clearCache();
        
        return result;
    }
    
    return result;
}

// Bir taşın gidebileceği tüm pozisyonları hesaplama
std::vector<Position> ChessBoard::getValidMoves(const Position& pos) {
    std::vector<Position> validMoves;
    
    if (!moveValidator) {
        return validMoves;
    }
    
    auto piece = getPieceAt(pos);
    if (!piece) {
        return validMoves;
    }
    
    // Tahtadaki tüm pozisyonları kontrol et
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            Position target = {x, y};
            MoveResult result = moveValidator->isValidMove(pos, target);
            
            if (result == MoveResult::ValidMove || result == MoveResult::EnemyPieceCapturable) {
                validMoves.push_back(target);
            }
        }
    }
    
    return validMoves;
}

// Taşın hareket kurallarına göre geçerli komşu pozisyonları alma
std::vector<Position> ChessBoard::getNeighbors(const Position& pos) {
    
    return getValidMoves(pos); // Şu an için aynı fonksiyonu kullanıyoruz
}


// Şah mat kontrolü
bool ChessBoard::isCheckmate(const std::string& color) {
    if (!moveValidator) {
        return false;
    }
    
    return moveValidator->isCheckmate(color);
}
