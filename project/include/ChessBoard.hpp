#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <set>
#include <functional>
#include "chessPieces.hpp"
#include "ConfigReader.hpp"
#include "Portal.hpp"
#include "PortalManager.hpp"

// Forward declarations
class MoveValidator;
class Portal;

enum class MoveResult;

struct MoveRecord {
    Position from;
    Position to;
    std::string color;
    std::string type_piece;
    MoveResult result;
    int turn;
};

// Graf düğümü olarak satranç tahtasındaki kare
struct ChessNode {
    Position pos;
    std::shared_ptr<chessPieces> piece;
    std::vector<Position> neighbors; // Bu kareden gidilebilecek diğer kareler
};

// Graf yapısı olarak satranç tahtası
class ChessBoard : public std::enable_shared_from_this<ChessBoard> {
private:
    int boardSize;
    // x,y koordinatlarından string key oluşturan yardımcı fonksiyon
    std::string positionToKey(const Position& pos) const;
    
    // Graf yapısı - her kare bir düğüm
    std::unordered_map<std::string, ChessNode> graph;
    
    // Kareleri ve içlerindeki taşları tutacak hashmap
    std::unordered_map<std::string, std::shared_ptr<chessPieces>> board;
    // Tüm taşların listesi
    std::vector<std::shared_ptr<chessPieces>> allPieces;
    // Portal
    std::unordered_map<std::string, std::shared_ptr<Portal>> portals;
    
    std::vector<MoveRecord> move_history;

    // Hareket doğrulayıcı
    std::shared_ptr<MoveValidator> moveValidator;
    // move 7,6 7,7
    // Cache yapıları
    // Hareket geçmişi için cache
    std::unordered_map<std::string, std::vector<Position>> moveHistoryCache;
    // Taşların gidebileceği pozisyonlar için cache
    std::unordered_map<std::string, std::vector<Position>> validMovesCache;
    // DFS/BFS hesaplamaları için path cache
    std::unordered_map<std::string, bool> pathFindingCache;
    
    // belirli bir isimdeki taşları return etme
    std::vector<std::shared_ptr<chessPieces>> getPiecesByName(const std::string& name);
    // Cache'i temizleme (tahtada değişiklik olduğunda)
    void clearCache();
    
public:
    int turn;
    // Hazır taşlarla constructor
    ChessBoard(int size, const std::vector<std::shared_ptr<chessPieces>>& pieces);
    // Destructor
    ~ChessBoard();

    // MoveValidator'ü ayarlama (GameManager tarafından çağrılacak)
    void setMoveValidator(std::shared_ptr<MoveValidator> validator);
    
    // Satranç tahtasını init etme
    void init();
    // Dışarıdan taşları alarak init etme
    void initWithPieces(const std::vector<std::shared_ptr<chessPieces>>& pieces);

    Position getKingPosition(const std::string& color);
    
    // Tahtaya taş ekleme
    void addPiece(std::shared_ptr<chessPieces> piece);
    // Belirli konumdaki taşı alma
    std::shared_ptr<chessPieces> getPieceAt(const Position& pos) const;
    // Tüm taşları alma
    const std::vector<std::shared_ptr<chessPieces>>& getAllPieces() const;
    
    // Tahtanın boyutunu alma
    int getBoardSize() const;
    // Tahtayı kontrol etme (pozisyon geçerli mi)
    bool isValidPosition(const Position& pos) const;
    
    // Bir taşın gidebileceği tüm pozisyonları hesaplama
    std::vector<Position> getValidMoves(const Position& pos);
    // Taşın hareket kurallarına göre geçerli komşu pozisyonları alma
    std::vector<Position> getNeighbors(const Position& pos);
    
    // Portal
    std::unordered_map<std::string, std::shared_ptr<Portal>> getPortals() const;
    
    // Taş hareket ettirme
    MoveResult movePiece(const Position& from, const Position& to, int turn);
    
    std::vector<MoveRecord> getMoveHistoryPosition(Position pos);

    // Şah ve şahmat kontrolleri
    bool isCheckmate(const std::string& color);

    // Tahtanın güncel durumunu string olarak gösterme
    std::string toString() const;
    

    bool isRuningPiece(const Position& pos);

    bool undoMove();
    void setPortals(std::vector<std::shared_ptr<Portal>> portals);
    // GameManager sınıfının erişimine izin ver
    friend class GameManager;
};

