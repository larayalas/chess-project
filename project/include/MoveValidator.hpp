#pragma once

// Forward declaration
class ChessBoard;
#include <memory>
#include <vector>
#include <iostream>
#include "Portal.hpp"
#include <unordered_map>
#include <stack>

// Position sınıfını içeren başlık dosyasını dahil et
#include "ConfigReader.hpp"

// Hareket sonucu durumu
enum class MoveResult
{
    Free,
    ValidMove,
    OutOfBounds,
    FriendlyPieceBlocking,
    EnemyPieceCapturable,
    InvalidMovePattern,
    NoPieceAtSource,
    Portal
};
enum class EdgeType
{
    FREE,
    is_friend,
    is_enemy,
    is_portal,
    is_free,
    is_me
};

struct Edge {
    std::string from;
    std::string to;
    EdgeType type;
    MoveResult result;
};

// hedef yer kendisidir
struct EdgeSquare {
    std::string color;
    std::string pieceType;
    std::string from;
    EdgeType type;
    MoveResult result;
};

// Hareket doğrulayıcı sınıf
class MoveValidator
{
private:
    std::shared_ptr<ChessBoard> board;
    // map için cache
    std::unordered_map<std::string, std::vector<Edge>> moveCache; // pieces için
    std::unordered_map<std::string, std::vector<EdgeSquare>> edgeSquareCache; // tahta için
public:
    // Constructor
    MoveValidator(std::shared_ptr<ChessBoard> chessBoard);

    // Bir hareketin geçerli olup olmadığını kontrol etme
    MoveResult isValidMove(const Position &from, const Position &to);

    void updateMoveCache();
    // Bir taşın gidebileceği tüm geçerli konumları hesaplama ve cache'e kaydetme
    std::vector<Edge> calculateEdge(const Position &from);
    void updateEdgeMoveCache(const Position &from, const Position &to, int depth);

    // Şah mat kontrolü
    bool isCheckmate(const std::string &color);

    bool setBoard(std::shared_ptr<ChessBoard> chessBoard);
    
    // GameManager sınıfının erişimine izin ver
    friend class GameManager;
};
