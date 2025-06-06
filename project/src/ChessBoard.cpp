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

std::unordered_map<std::string, std::shared_ptr<Portal>> ChessBoard::getPortals() const {
    return portals;
}


MoveResult ChessBoard::movePiece(const Position& from, const Position& to, int turn) {
    MoveResult finalResult = MoveResult::InvalidMovePattern;

    if (!moveValidator) {
        return finalResult;
    }

    this->turn = turn;
    finalResult = moveValidator->isValidMove(from, to);
    std::cout << "result: " << int(finalResult) << std::endl;

    if (finalResult == MoveResult::Castling) {
        auto piece = getPieceAt(from);
        if (!piece) {
            finalResult = MoveResult::NoPieceAtSource;
        } else {
            board.erase(positionToKey(from));
            board[positionToKey(to)] = piece;
            piece->setPosition(to);

            auto kingPosition = this->getKingPosition(piece->getColor());
            auto king = getPieceAt(kingPosition);

            if (!king) {
                finalResult = MoveResult::NoPieceAtSource;
            } else {
                board.erase(positionToKey(kingPosition));
                Position newKingPos = (from.x < to.x) ? Position{2, to.y} : Position{6, to.y};
                board[positionToKey(newKingPos)] = king;
                king->setPosition(newKingPos);

                move_history.push_back({from, to, piece->getColor(), piece->getType(), finalResult, turn});
            }
        }
    }
    else if (finalResult == MoveResult::EnPassant) {
        auto piece = getPieceAt(from);
        // from ile to arasındaki taşı sil
        board.erase(positionToKey(Position{to.x, from.y}));
        // fromdan to ya taşı kopyala
        board.erase(positionToKey(from));
        board[positionToKey(to)] = piece;
        piece->setPosition(to);

        move_history.push_back({from, to, piece->getColor(), piece->getType(), finalResult, turn});
    }
    else if (finalResult == MoveResult::ValidMove || finalResult == MoveResult::EnemyPieceCapturable) {
        std::string toKey = positionToKey(to);
        std::string fromKey = positionToKey(from);

        auto piece = getPieceAt(from);
        if (!piece) {
            finalResult = MoveResult::NoPieceAtSource;
        } else {
            if (board.find(toKey) != board.end()) {
                auto targetPiece = board[toKey];
                auto it = std::find(allPieces.begin(), allPieces.end(), targetPiece);
                if (it != allPieces.end()) {
                    allPieces.erase(it);
                }
            }

            board.erase(fromKey);
            board[toKey] = piece;
            piece->setPosition(to);

            if (to.y == (piece->getColor() == "white" ? boardSize - 1 : 0) && piece->getSpecialAbilities().promotion) {
                std::string promotion;
                std::set<std::string> uniqueTypes;
                for (const auto& p : allPieces) {
                    uniqueTypes.insert(p->getType());
                }

                std::cout << "Promotion: Hangi tip taşa çevirmek istiyorsunuz? (";
                for (const auto& type : uniqueTypes) {
                    std::cout << type << ", ";
                }
                std::cout << ")" << std::endl;
                std::cin >> promotion;

                chessPieces tempPiece = {promotion, piece->getColor(), piece->getPosition(), piece->getSpecialAbilities()};
                for (const auto& p : allPieces) {
                    if (p->getType() == promotion) {
                        tempPiece.setMovement(p->getMovement());
                        tempPiece.setType(p->getType());
                        tempPiece.setSpecialAbilities(p->getSpecialAbilities());
                        break;
                    }
                }

                piece->setType(tempPiece.getType());
                piece->setSpecialAbilities(tempPiece.getSpecialAbilities());
                piece->setMovement(tempPiece.getMovement());

                finalResult = MoveResult::Promotion;
            }
            // aktif protallardan birinde bitti ise hamle exitine gider
            for(const auto& portal : portals) {
                auto portal_entry = portal.second->getEntryPosition();
                if(portal_entry == to) {
                    finalResult = MoveResult::Portal;
                    auto portal_exit = portal.second->getExitPosition();
                    board.erase(positionToKey(to));
                    board[positionToKey(portal_exit)] = piece;
                    piece->setPosition(portal_exit);
                }
            }
            move_history.push_back({from, to, piece->getColor(), piece->getType(), finalResult, turn});

            clearCache();
        }

    }

    return finalResult;
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


bool ChessBoard::isRuningPiece(const Position& pos) {
    for (const auto& move : move_history) {
        if(move.from == pos) {
            return true;
        }
    }
    return false;
}

bool ChessBoard::undoMove() {
    if(move_history.empty()) {
        return false;
    }
    MoveRecord moveRecord = move_history.back();
    move_history.pop_back();
    auto piece = getPieceAt(moveRecord.to);
    if(piece) {
        piece->setPosition(moveRecord.from);
    }
    return true;
}

std::vector<std::shared_ptr<chessPieces>> ChessBoard::getPiecesByName(const std::string& name) {
    std::vector<std::shared_ptr<chessPieces>> pieces;
    for(const auto& piece : allPieces) {
        if(piece->getType() == name) {
            pieces.push_back(piece);
        }
    }
    return pieces;
}

std::vector<MoveRecord> ChessBoard::getMoveHistoryPosition(Position pos) {
    std::vector<MoveRecord> moves;
    for(const auto& move : move_history) {
        std::cout << "move.to: " << move.to.toString() << " " << pos.toString() << std::endl;
        if(move.to == pos) {
            moves.push_back(move);
        }
    }
    return moves;
}

void ChessBoard::setPortals(std::vector<std::shared_ptr<Portal>> portals) {
    this->portals.clear();
    for(const auto& portal : portals) {
        this->portals.insert(std::make_pair(portal->getEntryPosition().toString(), portal));
    }
}
