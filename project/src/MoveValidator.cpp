#include "../include/MoveValidator.hpp"
#include "../include/ChessBoard.hpp"
#include <iostream>
#include <array>

// Bu MoveDepth yapısını kaldırıyorum çünkü header dosyasında tanımladık
// Farklı hareket yönleri için ayrı derinlik değerlerini tutacak yapı
//struct MoveDepth {
//    int forward = 0;
//    int sideways = 0;
//    int diagonal = 0;
//    int l_shape = 0;
//    int first_move = 0;
//    
//    MoveDepth() : forward(0), sideways(0), diagonal(0), l_shape(0), first_move(0) {}
//};

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
            {
                bool en_passant = false;
                if(piece->getSpecialAbilities().en_passant && targetPiece->getSpecialAbilities().en_passant){
                    if( piece->getPosition().x -  to.x == (color == "white" ? -1 : 1) && piece->getPosition().y == to.y){
                        auto move_history = this->board->getMoveHistoryPosition(to);
                        for(auto move : move_history){
                            if(move.result == MoveResult::ValidMove){
                                if(this->board->turn == move.turn ){
                                    en_passant = true;
                                }
                            }
                        }
                    }
                }
                if(en_passant){
                    edges.push_back({from.toString(), to.toString(), EdgeType::en_passant, MoveResult::EnPassant});
                    // edgeSquare ekle
                    EdgeSquare edgeSquare;
                    edgeSquare.color = piece->getColor();
                    edgeSquare.pieceType = piece->getType();
                    edgeSquare.from = from.toString();
                    edgeSquare.type = EdgeType::en_passant;
                    edgeSquare.result = MoveResult::EnPassant;
                    edgeSquareCache[Position{to.x, to.y + (color == "white" ? 1 : -1)}.toString()].push_back(edgeSquare);
                    std::cout << "edgeSquareCache'e en_passant hareketi eklendi " << Position{to.x, to.y + (color == "white" ? 1 : -1)}.toString() << "  "  << from.toString() << " " << int(edgeSquare.result) << " " << edgeSquare.pieceType << " " << edgeSquare.color << std::endl;
                }
                else{
                    edges.push_back({from.toString(), to.toString(), EdgeType::is_enemy, MoveResult::Free});
                    
                }
            }
        }
    }

    // piece al mapten 
    if(piece->getType() == "Rook") {
        // şimdi rengine göre bakalım castling için
        if(this->castlling_valid(color, from)) {
            std::shared_ptr<chessPieces> king = this->board->getPieceAt(this->board->getKingPosition(color));
            if(from.x < king->getPosition().x) {
                // sol castling
                for(auto &edge : edges) {
                    if(edge.from == from.toString() && edge.to == Position{3,from.y}.toString()) {
                        std::cout << "Rook sol castling" << " " << from.toString() << " " << Position{3,from.y}.toString() << std::endl;

                        edge.result = MoveResult::Castling;
                    }
                }
            } else {
                // sağ castling
                // doğru edge i değiştir
                for(auto &edge : edges) {
                    if(edge.from == from.toString() && edge.to == Position{5,from.y}.toString()) {
                        std::cout << "Rook sağ castling" << " " << from.toString() << " " << Position{5,from.y}.toString() << std::endl;
                        edge.result = MoveResult::Castling;
                    }
                }
            }
        }
    }
    // tok için edge'leri hesapla
    return edges;
}

bool MoveValidator::isDepthBranchActive(const MoveDepth& d, const std::string& current) {

    if (current == "forward" &&  d.sideways == 0 && d.diagonal == 0 && d.l_shape == 0 && d.first_move == 0)
        return true;
    if (current == "sideways" &&  d.forward == 0 && d.diagonal == 0 && d.l_shape == 0 && d.first_move == 0)
        return true;
    if (current == "diagonal" &&  d.forward == 0 && d.sideways == 0 && d.l_shape == 0 && d.first_move == 0)
        return true;
    if (current == "l_shape" &&  d.forward == 0 && d.sideways == 0 && d.diagonal == 0 && d.first_move == 0)
        return true;
    if (current == "first_move"  && d.forward == 0 && d.sideways == 0 && d.diagonal == 0 && d.l_shape == 0)
        return true;

    return false;
}

void MoveValidator::updateEdgeMoveCache(const Position &from, const Position &to, const MoveDepth &moveDepth, bool first_block) {
    // harita dışı ise direk return
    if(to.x < 0 || to.x >= board->getBoardSize() || to.y < 0 || to.y >= board->getBoardSize()) return;
    // Maksimum derinlik kontrolü ekleyelim
    if (moveDepth.forward > 1000 || moveDepth.sideways > 1000 || moveDepth.diagonal > 1000) {
        std::cout << "Maksimum derinliğe ulaşıldı: " << from.toString() << " -> " << to.toString() << std::endl;
        return;
    }
    
    // Taşı al
    std::shared_ptr<chessPieces> piece = board->getPieceAt(from);
    if (!piece) return;

    // harita dışı ise direk return
    if(to.x < 0 || to.x >= board->getBoardSize() || to.y < 0 || to.y >= board->getBoardSize()) return;

    std::string fromStr = from.toString();
    std::string toStr = to.toString();
    
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
    if(first_block) return; 

    // edgeSquareCache'e eklenecek kayıt için temel bilgiler
    if(edge_node->type == EdgeType::is_free) {
        EdgeSquare edgeSquare;
        edgeSquare.color = piece->getColor();
        edgeSquare.pieceType = piece->getType();
        edgeSquare.from = fromStr;
        edgeSquare.type = EdgeType::is_free;

        if(edge_node->result == MoveResult::Castling) {
            edgeSquare.result = MoveResult::Castling;
        }
        else
            edgeSquare.result = MoveResult::ValidMove;

        // EdgeSquare oluştur - bu kareye hangi taşın gelebileceğini gösterir
        
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
        std::cout << "edgeSquareCache'e yeme hareketi eklendi " << toStr << "  "  << fromStr   << " " << int(edgeSquare.result) << " " << edgeSquare.pieceType << " " << edgeSquare.color << std::endl;        
        // Düşman taşını tehdit etme
        edgeSquareCache[toStr].push_back(edgeSquare);
    }
    if(edge_node->type == EdgeType::is_portal) {
        EdgeSquare edgeSquare;
        edgeSquare.color = piece->getColor();
        edgeSquare.pieceType = piece->getType();
        edgeSquare.from = fromStr;
        edgeSquare.type = EdgeType::is_portal;
        edgeSquare.result = MoveResult::Portal;
        std::cout << "edgeSquareCache'e portal hareketi eklendi " << toStr << "  "  << fromStr   << " " << int(edgeSquare.result) << " " << edgeSquare.pieceType << " " << edgeSquare.color << std::endl;        
        // Düşman taşını tehdit etme
        edgeSquareCache[toStr].push_back(edgeSquare);
    }


    // Taşın hareket tipleri için rekürsif çağrıları yeniden düzenleyelim
    // ve sonsuz döngüye girmemek için kontrol ekleyelim
    Movement mov = piece->getMovement();
    if(piece->getType() == "Rook" && edge_node->type == EdgeType::is_enemy) {
        std::cout << "Rook hareketi: " << from.toString() << " " << to.toString() << std::endl;
        std::cout << "moveDepth: " << moveDepth.forward << " " << moveDepth.sideways << " " << moveDepth.diagonal << " " << moveDepth.l_shape << " " << moveDepth.first_move << std::endl;
        std::cout << "edge_node->type: " << int(edge_node->type) << std::endl;

       std::cout << "mov.forward > moveDepth.forward: " << (mov.forward > moveDepth.forward) << std::endl;
       std::cout << "isDepthBranchActive(moveDepth, \"forward\"): " << isDepthBranchActive(moveDepth, "forward") << std::endl;
       std::cout << "edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy: " << int(edge_node->type)  << " " << int(edge_node->type == EdgeType::is_free) << " " << int(edge_node->type == EdgeType::is_enemy) << std::endl;
       std::cout << "first_block: " << first_block << std::endl;
    }

    if(piece->getType() == "Pawn") {
  //      std::cout << "Pawn hareketi: " << from.toString() << " " << to.toString() << std::endl;
  //      std::cout << "moveDepth: " << moveDepth.forward << " " << moveDepth.sideways << " " << moveDepth.diagonal << " " << moveDepth.l_shape << " " << moveDepth.first_move << std::endl;
  //      std::cout << "edge_node->type: " << int(edge_node->type) << std::endl;
  //      // mov.forward > moveDepth.forward && isDepthBranchActive(moveDepth, "forward") && (edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy)
  //     // yazdır
  //     std::cout << "mov.forward > moveDepth.forward: " << (mov.forward > moveDepth.forward) << std::endl;
  //     std::cout << "isDepthBranchActive(moveDepth, \"forward\"): " << isDepthBranchActive(moveDepth, "forward") << std::endl;
  //     std::cout << "edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy: " << (edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy) << std::endl;
    }

    // portalsa
    // İleri hareket - sadece forward derinliği 0 ise ve diğer derinlikler 0 değilse
    if (mov.forward > moveDepth.forward && isDepthBranchActive(moveDepth, "forward") && (edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy || edge_node->type == EdgeType::is_me)) {
        Position newPos = {from.x , from.y + moveDepth.forward + 1};
        Position newPos2 = {from.x , from.y - moveDepth.forward - 1};
        // Yeni derinlik değerlerini hesapla
        MoveDepth newDepth = moveDepth;
        newDepth.forward += 1;
        if (board->isValidPosition(newPos) && !visitedPaths[fromStr][newPos.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }

            updateEdgeMoveCache(from, newPos, newDepth, first_block);
        }
        if (board->isValidPosition(newPos2) && !visitedPaths[fromStr][newPos2.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            updateEdgeMoveCache(from, newPos2, newDepth, first_block);
        }
    }
    // Yanlara hareket - sadece sideways derinliği 0 ise ve diğer derinlikler 0 değilse
    if (mov.sideways > moveDepth.sideways && isDepthBranchActive(moveDepth, "sideways") && (edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy || edge_node->type == EdgeType::is_me)) {
        Position newPosRight = {from.x + moveDepth.sideways + 1, from.y };
        Position newPosLeft = {from.x - moveDepth.sideways - 1, from.y };
        
        // Yeni derinlik değerlerini hesapla
        MoveDepth newDepth = moveDepth;
        newDepth.sideways += 1;

        if (board->isValidPosition(newPosRight) && !visitedPaths[fromStr][newPosRight.toString()]) {
        if(edge_node->type == EdgeType::is_enemy) {
            first_block = true;
        }
            updateEdgeMoveCache(from, newPosRight, newDepth, first_block);
        }
        if (board->isValidPosition(newPosLeft) && !visitedPaths[fromStr][newPosLeft.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            updateEdgeMoveCache(from, newPosLeft, newDepth, first_block);
        }
    }
    
    // Çapraz hareket - sadece diagonal derinliği 0 ise ve diğer derinlikler 0 değilse
    if (mov.diagonal > moveDepth.diagonal && isDepthBranchActive(moveDepth, "diagonal") && (edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy || edge_node->type == EdgeType::is_me)) {
        Position newPosDiag1 = {from.x + moveDepth.diagonal + 1, from.y + moveDepth.diagonal + 1};
        Position newPosDiag2 = {from.x + moveDepth.diagonal + 1, from.y - moveDepth.diagonal - 1};
        Position newPosDiag3 = {from.x - moveDepth.diagonal - 1, from.y + moveDepth.diagonal + 1};
        Position newPosDiag4 = {from.x - moveDepth.diagonal - 1, from.y - moveDepth.diagonal - 1};
        
        // Yeni derinlik değerlerini hesapla
        MoveDepth newDepth = moveDepth;
        newDepth.diagonal += 1;
        
        if (board->isValidPosition(newPosDiag1) && !visitedPaths[fromStr][newPosDiag1.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            // Çapraz hareket için derinliği artır
            updateEdgeMoveCache(from, newPosDiag1, newDepth, first_block);
        }
        if (board->isValidPosition(newPosDiag2) && !visitedPaths[fromStr][newPosDiag2.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            updateEdgeMoveCache(from, newPosDiag2, newDepth, first_block);
        }
        if (board->isValidPosition(newPosDiag3) && !visitedPaths[fromStr][newPosDiag3.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            updateEdgeMoveCache(from, newPosDiag3, newDepth, first_block);
        }
        if (board->isValidPosition(newPosDiag4) && !visitedPaths[fromStr][newPosDiag4.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            updateEdgeMoveCache(from, newPosDiag4, newDepth, first_block);
        }
    }
    
    // L şekli hareket (at) - sadece l_shape derinliği 0 ise ve diğer derinlikler 0 değilse
    if (mov.l_shape && moveDepth.l_shape < 1 && isDepthBranchActive(moveDepth, "l_shape") && (edge_node->type == EdgeType::is_free || edge_node->type == EdgeType::is_enemy || edge_node->type == EdgeType::is_me)) {
        const std::array<std::pair<int, int>, 8> knightMoves = {{
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
            {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
        }};
        
        // Yeni derinlik değerlerini hesapla
        MoveDepth newDepth = moveDepth;
        newDepth.l_shape += 1;
        
        for (const auto& move : knightMoves) {
            Position newPos = {from.x + move.first, from.y + move.second};
            if (board->isValidPosition(newPos) && !visitedPaths[fromStr][newPos.toString()]) {
                if(edge_node->type == EdgeType::is_enemy) {
                    first_block = true;
                }
                // At hareketi için derinliği artır
                updateEdgeMoveCache(from, newPos, newDepth, first_block);
            }
        }
    }

    if(mov.first_move_forward && (piece->getColor() == "white" ? from.y == 1 : from.y == 6)) {
        Position newPos = {from.x , from.y + (piece->getColor() == "white" ? 2 : -2)};
        MoveDepth newDepth = moveDepth;
        newDepth.first_move += 1;
        if (board->isValidPosition(newPos) && !visitedPaths[fromStr][newPos.toString()]) {
            if(edge_node->type == EdgeType::is_enemy) {
                first_block = true;
            }
            updateEdgeMoveCache(from, newPos, newDepth, first_block);
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
            // MoveDepth nesnesi oluştur, tüm derinlikleri 0 olarak başlat
            MoveDepth initialDepth;
            
            // Yeni parametre yapısıyla fonksiyonu çağır
            updateEdgeMoveCache(piece->getPosition(), piece->getPosition(), initialDepth, false);
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

bool MoveValidator::castlling_valid(std::string color,  Position position) {
    // şimdi ilk olarak kral konumunu al
    Position kingPosition = board->getKingPosition(color);
    // kralın gidebileceği yerleri al
    if(this->board->isRuningPiece(kingPosition)) {
        return false;
    }
    if(this->board->isRuningPiece(position)) {
        return false;
    }

    // position ile kingPosition arasında bir taş var mı bak
    // min ve max değerleri al
    int minX = std::min(position.x, kingPosition.x);
    int maxX = std::max(position.x, kingPosition.x);
    int minY = std::min(position.y, kingPosition.y);
    int maxY = std::max(position.y, kingPosition.y);
    
    // bu arada kalan taşları taramak gerekiyor
    for(int x = minX; x <= maxX; x++) {
        for(int y = minY; y <= maxY; y++) {
            std::cout << "x: " << x << " y: " << y << std::endl;
            if(x == position.x && y == position.y) {
                continue;
            }
            if(x == kingPosition.x && y == kingPosition.y) {
                continue;
            }
            if(board->getPieceAt({x, y})) {
                return false;
            }
            // kralın solundaki kareyi edgeSqureCache'ten al
            auto edgeSquares = edgeSquareCache[std::to_string(x) + "," + std::to_string(y)];

            // şimdi bakalım  kareleri gören düşman taş var mı
            for(auto edgeSquare : edgeSquares) {
                if(edgeSquare.color != color && edgeSquare.result == MoveResult::ValidMove) {
                    return false;
                }
            }
        }
    }

    return true;
}
