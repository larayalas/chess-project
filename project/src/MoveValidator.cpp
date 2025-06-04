#include "../include/MoveValidator.hpp"
#include "../include/ChessBoard.hpp"
#include <iostream>
#include <array>

static std::unordered_map<std::string, std::unordered_map<std::string, bool>> visitedPaths;

MoveValidator::MoveValidator(std::shared_ptr<ChessBoard> chessBoard) {
    board = chessBoard;
    updateMoveCache();
}

MoveResult MoveValidator::isValidMove(const Position &from, const Position &to)
{
    // Kaynak pozisyonda taş var mı kontrol et
    std::shared_ptr<chessPieces> piece = board->getPieceAt(from);
    if (!piece) {
        return MoveResult::NoPieceAtSource;
    }

    // Taşın özelliklerini yazdır (debug için)
//    std::cout << piece->getType() << std::endl;
//    std::cout << piece->getColor() << std::endl;
//    std::cout << piece->getPosition().x << std::endl;
//    std::cout << piece->getPosition().y << std::endl;

    // Taşın hareket özelliklerini yazdır (debug için)
//    Movement mov = piece->getMovement();
//    std::cout << "  Hareket: ";
//    if (mov.forward > 0)
//        std::cout << "İleri: " << mov.forward << " ";
//    if (mov.sideways > 0)
//        std::cout << "Yana: " << mov.sideways << " ";
//    if (mov.diagonal > 0)
//        std::cout << "Çapraz: " << mov.diagonal << " ";
//    if (mov.l_shape)
//        std::cout << "L-şekli: Evet ";
//    std::cout << std::endl;
//
//    // taşın özel yeteneklerini yazdır
//    SpecialAbilities special = piece->getSpecialAbilities();
//    std::cout << "  Özel Yetenekler: ";
//    for (const auto &ability : special.custom_abilities)
//    {
//        std::cout << ability.first << ": " << ability.second << " ";
//    }
//    if (special.castling)
//        std::cout << "Castling: Evet ";
//    if (special.royal)
//        std::cout << "Royal: Evet ";
//    if (special.jump_over)
//        std::cout << "Jump Over: Evet ";
//    if (special.promotion)
//        std::cout << "Promotion: Evet ";
//    std::cout << std::endl;

    // Tahtanın sınırları içinde mi kontrol et
    if (!board->isValidPosition(to)) {
        return MoveResult::OutOfBounds;
    }

    // Cache'den edge bilgisini al
    std::string fromStr = from.toString();
    std::string toStr = to.toString();
    
    auto edge = edgeSquareCache[toStr];
    std::cout << "edge: " << edge.size() << std::endl;
    for(auto e : edge) {
        std::cout << "edge: " << e.from << " " << int(e.result) << std::endl;
        if(e.from == fromStr) {
            return e.result;
        }
    }
    // Eğer edge bulunamadıysa, geçersiz hareket
    return MoveResult::InvalidMovePattern;
}

std::vector<Edge> MoveValidator::calculateEdge(const Position &from ) {
    std::vector<Edge> edges;
    // mapten kendi konumumum piecesini al 
    std::shared_ptr<chessPieces> piece = board->getPieceAt(from);
    std::string color = piece->getColor();

    // map sınırlarını al
    int boardSize = board->getBoardSize();
    // from'a göre tüm edge'leri hesapla
    for (int dx = 0; dx < boardSize; dx++) {
        for (int dy = 0; dy < boardSize; dy++) {
            if (dx == 0 && dy == 0) {
                edges.push_back({from.toString(), from.toString(), EdgeType::is_me, MoveResult::FriendlyPieceBlocking});
                continue;
            }

            Position to = {dx, dy};
            std::shared_ptr<chessPieces> targetPiece = board->getPieceAt(to);
            bool isPortal = board->getPortals().find(to.toString()) != board->getPortals().end();

            if (!targetPiece) {
                if (isPortal)
                    edges.push_back({from.toString(), to.toString(), EdgeType::is_portal, MoveResult::Free});
                else
                    edges.push_back({from.toString(), to.toString(), EdgeType::is_free, MoveResult::Free});
                continue;
            }

            std::string targetColor = targetPiece->getColor();

            if (targetColor == color)
                edges.push_back({from.toString(), to.toString(), EdgeType::is_friend, MoveResult::Free});
            else
                edges.push_back({from.toString(), to.toString(), EdgeType::is_enemy, MoveResult::Free});

        }
    }
    return edges;
}

void MoveValidator::updateEdgeMoveCache(const Position &from, const Position &to, int depth) {
    // Maksimum derinlik kontrolü ekleyelim
    if (depth > 1000) {
        std::cout << "Maksimum derinliğe ulaşıldı: " << from.toString() << " -> " << to.toString() << std::endl;
        return;
    }
    
    // Taşı al
    std::shared_ptr<chessPieces> piece = board->getPieceAt(from);
    if (!piece) return;

    if(to.x < 0 || to.x >= board->getBoardSize() || to.y < 0 || to.y >= board->getBoardSize()) return;

    std::string fromStr = from.toString();
    std::string toStr = to.toString();
    
    // Ziyaret edilen pozisyonları takip edelim
    
    // Bu yol daha önce ziyaret edilmiş mi?
    if (visitedPaths[fromStr][toStr]) {
        //std::cout << "Bu yol daha önce ziyaret edilmiş: " << fromStr << " -> " << toStr << std::endl;
        return;
    }
    
    // Yolu ziyaret edildi olarak işaretle
    visitedPaths[fromStr][toStr] = true;
    
    std::unordered_map<std::string, bool> visited;
    std::stack<Position> stack;
    
    // Başlangıç noktasını stack'e ekle
    stack.push(from);
    
    // moveCache'de from için edge'ler var mı kontrol et
    if (moveCache.find(fromStr) == moveCache.end()) {
        std::cout << "moveCache'de " << fromStr << " bulunamadı" << std::endl;
        return;
    }
    
//    std::cout << "moveCache[" << fromStr << "].size(): " << moveCache[fromStr].size() << std::endl;
    
    // to pozisyonuna karşılık gelen edge'i bul
    Edge* edge_node = nullptr;
    
    // Referansı doğrudan moveCache'den al
    for (size_t i = 0; i < moveCache[fromStr].size(); i++) {
        if (moveCache[fromStr][i].to == toStr) {
            edge_node = &moveCache[fromStr][i];
            break;
        }
    }
    
    if (edge_node == nullptr) {
        std::cout << "Edge bulunmadı " << fromStr << " -> " << toStr << std::endl;
        return;
    }
    
    // edgeSquareCache'e eklenecek kayıt için temel bilgiler
    if(edge_node->type == EdgeType::is_free) {
        
        // EdgeSquare oluştur - bu kareye hangi taşın gelebileceğini gösterir
        EdgeSquare edgeSquare;
        edgeSquare.color = piece->getColor();
        edgeSquare.pieceType = piece->getType();
        edgeSquare.from = fromStr;
        edgeSquare.type = EdgeType::is_free;
        edgeSquare.result = MoveResult::ValidMove;
        
        std::cout << "edgeSquareCache'e eklendi " << toStr << "  "  << fromStr   << " " << int(edgeSquare.result) << " " << edgeSquare.pieceType << " " << edgeSquare.color << std::endl;
        
        // Hedef karede bizi tehdit eden veya koruyan taşları izlemek için edgeSquareCache'e ekle
        edgeSquareCache[toStr].push_back(edgeSquare);
    }
    
    if(edge_node->type == EdgeType::is_friend) {
        EdgeSquare edgeSquare;
        edgeSquare.color = piece->getColor();
        edgeSquare.pieceType = piece->getType();
        edgeSquare.from = fromStr;
        edgeSquare.type = EdgeType::is_friend;
        edgeSquare.result = MoveResult::FriendlyPieceBlocking;
        
        // Dost taşı koruma
        edgeSquareCache[toStr].push_back(edgeSquare);
    }
    
    if(edge_node->type == EdgeType::is_enemy) {
        EdgeSquare edgeSquare;
        edgeSquare.color = piece->getColor();
        edgeSquare.pieceType = piece->getType();
        edgeSquare.from = fromStr;
        edgeSquare.type = EdgeType::is_enemy;
        edgeSquare.result = MoveResult::EnemyPieceCapturable;
        
        // Düşman taşını tehdit etme
        edgeSquareCache[toStr].push_back(edgeSquare);
    }
    
    // Taşın hareket tipleri için rekürsif çağrıları yeniden düzenleyelim
    // ve sonsuz döngüye girmemek için kontrol ekleyelim
    Movement mov = piece->getMovement();
    
    // Recursive çağrıları mevcut pozisyondan değil, hedef pozisyondan yapıyoruz
    Position nextPos = to;
    
    // İleri hareket 
    if (mov.forward > depth) {
        Position newPos = {nextPos.x , nextPos.y + 1};
        Position newPos2 = {nextPos.x , nextPos.y - 1};
        if (board->isValidPosition(newPos) && !visitedPaths[toStr][newPos.toString()]) {
            updateEdgeMoveCache(to, newPos, depth + 1);
        }
        if (board->isValidPosition(newPos2) && !visitedPaths[toStr][newPos2.toString()]) {
            updateEdgeMoveCache(to, newPos2, depth + 1);
        }
    }
    // Yanlara hareket
    if (mov.sideways > depth) {
        Position newPosRight = {nextPos.x + 1, nextPos.y };
        Position newPosLeft = {nextPos.x - 1, nextPos.y };
        if (board->isValidPosition(newPosRight) && !visitedPaths[toStr][newPosRight.toString()]) {
            updateEdgeMoveCache(to, newPosRight, depth + 1);
        }
        if (board->isValidPosition(newPosLeft) && !visitedPaths[toStr][newPosLeft.toString()]) {
            updateEdgeMoveCache(to, newPosLeft, depth + 1);
        }
    }
    
    // Çapraz hareket
    if (mov.diagonal > depth) {
        Position newPosDiag1 = {nextPos.x + 1, nextPos.y + 1};
        Position newPosDiag2 = {nextPos.x + 1, nextPos.y - 1};
        Position newPosDiag3 = {nextPos.x - 1, nextPos.y + 1};
        Position newPosDiag4 = {nextPos.x - 1, nextPos.y - 1};
        
        if (board->isValidPosition(newPosDiag1) && !visitedPaths[toStr][newPosDiag1.toString()]) {
            updateEdgeMoveCache(to, newPosDiag1, depth + 1);
        }
        if (board->isValidPosition(newPosDiag2) && !visitedPaths[toStr][newPosDiag2.toString()]) {
            updateEdgeMoveCache(to, newPosDiag2, depth + 1);
        }
        if (board->isValidPosition(newPosDiag3) && !visitedPaths[toStr][newPosDiag3.toString()]) {
            updateEdgeMoveCache(to, newPosDiag3, depth + 1);
        }
        if (board->isValidPosition(newPosDiag4) && !visitedPaths[toStr][newPosDiag4.toString()]) {
            updateEdgeMoveCache(to, newPosDiag4, depth + 1);
        }
    }
    
    // L şekli hareket (at)
    if (mov.l_shape && depth < 1) {
        const std::array<std::pair<int, int>, 8> knightMoves = {{
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
            {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
        }};
        
        for (const auto& move : knightMoves) {
            Position newPos = {nextPos.x + move.first, nextPos.y + move.second};
            if (board->isValidPosition(newPos) && !visitedPaths[toStr][newPos.toString()]) {
                updateEdgeMoveCache(to, newPos, depth + 1);
            }
        }
    }

    if(mov.first_move_forward && (piece->getColor() == "white" ? nextPos.y == 1 : nextPos.y == 6)) {
        Position newPos = {nextPos.x , nextPos.y + 2 * (piece->getColor() == "white" ? 1 : -1)};
        if (board->isValidPosition(newPos) && !visitedPaths[toStr][newPos.toString()]) {
            updateEdgeMoveCache(to, newPos, depth + 1);
        }
    }
    
}

// Hareket cache'ini güncelle
void MoveValidator::updateMoveCache() {
    // Her cache güncellemede ziyaret yollarını temizle
    visitedPaths.clear();
    
    // Cache'leri temizle
    moveCache.clear();
    edgeSquareCache.clear();
    
    std::cout << "Cache güncelleniyor..." << std::endl;
    
    // Taşların edge'lerini hesapla
    for (const auto &piece : board->getAllPieces()) {
        if (piece) {
            moveCache[piece->getPosition().toString()] = calculateEdge(piece->getPosition());
        }
    }
    
    // Edge'leri tahtaya işle
    for (const auto &piece : board->getAllPieces()) {
        if (piece) {
            updateEdgeMoveCache(piece->getPosition(), piece->getPosition(), 0);
        }
    }

    // Debug için 1,2 pozisyonundaki edgeSquareCache içeriğini yazdır
    std::cout << "Cache güncelleme tamamlandı" << std::endl;
}

bool MoveValidator::isCheckmate(const std::string &color) {
    std::cout << "isCheckmate kontrolü başlıyor: " << color << " için" << std::endl;
    // taşların hedef yerlerini al
    Position kingPosition = board->getKingPosition(color);
    std::cout << "Şah pozisyonu: " << kingPosition.toString() << std::endl;
    
    // hedef yerleri için edgeSquareCache'i al
    std::vector<EdgeSquare> edgeSquares = edgeSquareCache[kingPosition.toString()];
    std::cout << "Şahın bulunduğu karedeki tehditler: " << edgeSquares.size() << " adet" << std::endl;

    bool kingNotSafe = false;

    for(auto edgeSquare : edgeSquares) {
        if(edgeSquare.color == color && edgeSquare.result == MoveResult::EnemyPieceCapturable) {
            std::cout << "Kendi renginde tehdit: " << edgeSquare.pieceType << " taşı " 
                      << edgeSquare.from << " konumundan" << std::endl;
            kingNotSafe = true;
        }
    }
    if(kingNotSafe) {
        std::cout << "Şah tehdit altında değil, şah mat yok" << std::endl;
        return false;
    }
    
    auto kingMoves = moveCache[kingPosition.toString()];
    bool isCheckmate = true;
    std::cout << "Şahın gidebileceği " << kingMoves.size() << " adet kare kontrol ediliyor" << std::endl;
    
    // Şahın gidebileceği tüm yerler için kontrol
    for(auto edge : kingMoves) {
        std::cout << "Kontrol edilen kare: " << edge.to << ", Tip: " 
                  << (edge.type == EdgeType::is_free ? "Boş" : 
                     (edge.type == EdgeType::is_enemy ? "Düşman" : 
                     (edge.type == EdgeType::is_friend ? "Dost" : "Diğer"))) << std::endl;
        
        if(edge.type == EdgeType::is_free || edge.type == EdgeType::is_enemy) {
            // eğerki düşman taş ise yenilebilirmi bakıyoruz.
            std::cout << "Potansiyel güvenli kare bulundu" << std::endl;
            
            edgeSquares = edgeSquareCache[edge.to];
            bool squareIsSafe = true;
            
            if(edge.type == EdgeType::is_enemy) {
                std::cout << "Düşman taş bulundu, yenilebilir mi kontrol ediliyor" << std::endl;
                edgeSquares = edgeSquareCache[edge.to];
                // Başlangıçta düşman taşı yiyebileceğimizi varsayalım
                bool canCapture = true;
                
                // Düşman taşı başka bir düşman taş koruyor mu?
                for(auto edgeSquare : edgeSquares) {
                    if(edgeSquare.color != color && 
                       edgeSquare.result == MoveResult::EnemyPieceCapturable) {
                        std::cout << "Düşman taşı " << edgeSquare.pieceType << " tarafından " 
                                  << edgeSquare.from << " konumundan korunuyor" << std::endl;
                        // Eğer düşman taşı başka bir düşman koruyor ise, yemek güvenli değil
                        canCapture = false;
                        break;
                    }
                }
                
                // Eğer düşman taşı güvenli bir şekilde yiyemezsek, bu kare güvenli değil
                if(!canCapture) {
                    std::cout << "Düşman taşı güvenli bir şekilde yiyemiyoruz" << std::endl;
                    squareIsSafe = false;
                } else {
                    std::cout << "Düşman taşı güvenli bir şekilde yiyebiliriz" << std::endl;
                }
            }

            // Bu kareye düşman taşlar ulaşabilir mi?
            std::cout << "Kareye tehdit kontrolü yapılıyor: " << edge.to << std::endl;
            for(auto edgeSquare : edgeSquares) {
                if(edgeSquare.color != color && 
                   (edgeSquare.result == MoveResult::ValidMove || 
                    edgeSquare.result == MoveResult::EnemyPieceCapturable)) {
                    std::cout << "Kare tehdit altında: " << edgeSquare.pieceType << " taşı " 
                              << edgeSquare.from << " konumundan" << std::endl;
                    squareIsSafe = false;
                    break;
                }
            }
            
            // Eğer güvenli bir kare bulduysan, şah mat değildir
            if(squareIsSafe) {
                std::cout << "Güvenli kare bulundu: " << edge.to << ", şah mat değil" << std::endl;
                isCheckmate = false;
                break;
            } else {
                std::cout << "Kare güvenli değil: " << edge.to << std::endl;
            }
        }
    }
    
    if(isCheckmate) {
        std::cout << "Şah mat tespit edildi: " << color << " için" << std::endl;
    } else {
        std::cout << "Şah mat değil: " << color << " için" << std::endl;
    }
    
    return isCheckmate;
}

bool MoveValidator::setBoard(std::shared_ptr<ChessBoard> chessBoard) {
    board = chessBoard;
    return true;
}


