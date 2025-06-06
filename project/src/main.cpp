#include "ConfigReader.hpp"
#include "chessPieces.hpp"
#include "ChessBoard.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "../include/GameManager.hpp"
#include "../include/MoveValidator.hpp"

// Helper function to display special abilities
void displaySpecialAbilities(const SpecialAbilities &abilities) {
  if (abilities.castling)
    std::cout << "Castling ";
  if (abilities.royal)
    std::cout << "Royal ";
  if (abilities.jump_over)
    std::cout << "Jump-over ";
  if (abilities.promotion)
    std::cout << "Promotion ";
  if (abilities.en_passant)
    std::cout << "En-passant ";

  // Display any custom abilities
  for (const auto &[key, value] : abilities.custom_abilities) {
    if (value) {
      std::cout << key << " ";
    }
  }
}

// Helper function to display piece information
void displayPieceInfo(const PieceConfig &piece) {
  std::cout << "Type: " << piece.type << " (Count: " << piece.count << ")"
            << std::endl;

  // Show movement info
  std::cout << "  Movement: ";
  if (piece.movement.forward > 0)
    std::cout << "Forward: " << piece.movement.forward << " ";
  if (piece.movement.sideways > 0)
    std::cout << "Sideways: " << piece.movement.sideways << " ";
  if (piece.movement.diagonal > 0)
    std::cout << "Diagonal: " << piece.movement.diagonal << " ";
  if (piece.movement.l_shape)
    std::cout << "L-shape: Yes ";
  if (piece.movement.diagonal_capture > 0)
    std::cout << "Diagonal Capture: " << piece.movement.diagonal_capture << " ";
  if (piece.movement.first_move_forward > 0)
    std::cout << "First Move Forward: " << piece.movement.first_move_forward
              << " ";
  std::cout << std::endl;

  // Show special abilities
  std::cout << "  Special Abilities: ";
  displaySpecialAbilities(piece.special_abilities);
  std::cout << std::endl;

  // Display positions
  if (piece.positions.count("white") > 0) {
    std::cout << "  White positions: ";
    for (const auto &pos : piece.positions.at("white")) {
      std::cout << "(" << pos.x << "," << pos.y << ") ";
    }
    std::cout << std::endl;
  }

  if (piece.positions.count("black") > 0) {
    std::cout << "  Black positions: ";
    for (const auto &pos : piece.positions.at("black")) {
      std::cout << "(" << pos.x << "," << pos.y << ") ";
    }
    std::cout << std::endl;
  }
}

// Helper function to display portal information
void displayPortalInfo(const std::vector<PortalConfig> &portals) {
  std::cout << "\n==== Portals ====" << std::endl;
  for (const auto &portal : portals) {
    std::cout << "Portal ID: " << portal.id << std::endl;
    std::cout << "  Entry: (" << portal.positions.entry.x << ","
              << portal.positions.entry.y << ")" << std::endl;
    std::cout << "  Exit: (" << portal.positions.exit.x << ","
              << portal.positions.exit.y << ")" << std::endl;
    std::cout << "  Preserve direction: "
              << (portal.properties.preserve_direction ? "Yes" : "No")
              << std::endl;
    std::cout << "  Cooldown: " << portal.properties.cooldown << " turns"
              << std::endl;

    std::cout << "  Allowed colors: ";
    for (const auto &color : portal.properties.allowed_colors) {
      std::cout << color << " ";
    }
    std::cout << std::endl;
  }
}

// Helper function to display game configuration information
void displayGameConfig(const GameConfig &config) {
  std::cout << "==== Game Configuration ====" << std::endl;
  std::cout << "Game: " << config.game_settings.name << std::endl;
  std::cout << "Board size: " << config.game_settings.board_size << "x"
            << config.game_settings.board_size << std::endl;
  std::cout << "Turn limit: " << config.game_settings.turn_limit << std::endl;
}

// Helper function to display all pieces information
void displayAllPieces(const GameConfig &config) {
  // Display standard pieces information
  std::cout << "\n==== Standard Pieces ====" << std::endl;
  for (const auto &piece : config.pieces) {
    displayPieceInfo(piece);
  }

  // Display custom pieces information if any
  if (!config.custom_pieces.empty()) {
    std::cout << "\n==== Custom Pieces ====" << std::endl;
    for (const auto &piece : config.custom_pieces) {
      displayPieceInfo(piece);
    }
  }
}

void displayAllConfigValues(const GameConfig &config) {
  // Display some information from the config
  displayGameConfig(config);

  // Display all pieces information
  displayAllPieces(config);

  // Display portals information
  displayPortalInfo(config.portals);
}


// Oluşturulan taşları ekrana yazdıran fonksiyon
void displayChessPieces(const std::vector<chessPieces> &pieces) {
  std::cout << "\n==== Oluşturulan Taşlar ====" << std::endl;
  for (const auto &piece : pieces) {
    Position pos = piece.getPosition();
    std::cout << "Tür: " << piece.getType() << ", Renk: " << piece.getColor() 
              << ", Konum: (" << pos.x << "," << pos.y << ")" << std::endl;
    
    // Hareket özelliklerini göster
    Movement mov = piece.getMovement();
    std::cout << "  Hareket: ";
    if (mov.forward > 0)
      std::cout << "İleri: " << mov.forward << " ";
    if (mov.sideways > 0)
      std::cout << "Yana: " << mov.sideways << " ";
    if (mov.diagonal > 0)
      std::cout << "Çapraz: " << mov.diagonal << " ";
    if (mov.l_shape)
      std::cout << "L-şekli: Evet ";
    std::cout << std::endl;
    
    // Özel yetenekleri göster
    std::cout << "  Özel Yetenekler: ";
    displaySpecialAbilities(piece.getSpecialAbilities());
    std::cout << std::endl;
  }
}

// 8x8'lik satranç tahtasında taşları gösteren fonksiyon
void displayChessBoard(const std::vector<chessPieces> &pieces, int boardSize = 8) {
  // Tahtayı temsil eden 2D dizi oluştur
  std::vector<std::vector<std::string>> board(boardSize, std::vector<std::string>(boardSize, "  "));
  
  // Taşları tahtaya yerleştir
  for (const auto &piece : pieces) {
    Position pos = piece.getPosition();
    
    // Pozisyon tahta sınırları içinde mi kontrol et
    if (pos.x >= 0 && pos.x < boardSize && pos.y >= 0 && pos.y < boardSize) {
      // Taş kodunu belirle (İlk harf türü, ikinci harf rengi temsil eder)
      std::string pieceCode = piece.getType().substr(0, 1);
      pieceCode += piece.getColor() == "white" ? "b" : "s"; // b=beyaz, s=siyah
      board[pos.y][pos.x] = pieceCode;
    }
  }
  
  // Tahtayı ekrana yazdır
  std::cout << "\n==== Satranç Tahtası (8x8) ====" << std::endl;
  
  // Üst kenar
  std::cout << "   ";
  for (int x = 0; x < boardSize; x++) {
    std::cout << " " << static_cast<char>('a' + x) << " ";
  }
  std::cout << std::endl;
  
  // Üst çizgi
  std::cout << "  +" << std::string(boardSize * 3, '-') << "+" << std::endl;
  
  // Satırlar
  for (int y = 0; y < boardSize; y++) {
    std::cout << " " << (boardSize - y) << "|";
    
    // Hücreler
    for (int x = 0; x < boardSize; x++) {
      std::cout << " " << board[y][x] << "";
    }
    
    std::cout << " |" << (boardSize - y) << std::endl;
  }
  
  // Alt çizgi
  std::cout << "  +" << std::string(boardSize * 3, '-') << "+" << std::endl;
  
  // Alt kenar
  std::cout << "   ";
  for (int x = 0; x < boardSize; x++) {
    std::cout << " " << static_cast<char>('a' + x) << " ";
  }
  std::cout << std::endl;
  
  // Taş kodlarının açıklaması
  std::cout << "\nTaş Kodları:" << std::endl;
  std::cout << "İlk harf taş türünü gösterir (P=Piyon, R=Kale, N=At, B=Fil, Q=Vezir, K=Şah)" << std::endl;
  std::cout << "İkinci harf rengi gösterir (b=beyaz, s=siyah)" << std::endl;
}

// Shared_ptr kullanarak taşları oluşturan fonksiyon
std::vector<std::shared_ptr<chessPieces>> createChessPiecesShared(const GameConfig &config) {
  std::vector<std::shared_ptr<chessPieces>> allPieces;
  
  // Standart taşları oluştur
  for (const auto &pieceConfig : config.pieces) {
    // Beyaz taşları oluştur
    if (pieceConfig.positions.count("white") > 0) {
      for (const auto &pos : pieceConfig.positions.at("white")) {
        auto piece = std::make_shared<chessPieces>(pieceConfig.type, "white", pos, pieceConfig.special_abilities);
        piece->setMovement(pieceConfig.movement);
        allPieces.push_back(piece);
      }
    }
    
    // Siyah taşları oluştur
    if (pieceConfig.positions.count("black") > 0) {
      for (const auto &pos : pieceConfig.positions.at("black")) {
        auto piece = std::make_shared<chessPieces>(pieceConfig.type, "black", pos, pieceConfig.special_abilities);
        piece->setMovement(pieceConfig.movement);
        allPieces.push_back(piece);
      }
    }
  }
  
  // Özel taşları oluştur
  for (const auto &pieceConfig : config.custom_pieces) {
    // Beyaz taşları oluştur
    if (pieceConfig.positions.count("white") > 0) {
      for (const auto &pos : pieceConfig.positions.at("white")) {
        auto piece = std::make_shared<chessPieces>(pieceConfig.type, "white", pos, pieceConfig.special_abilities);
        piece->setMovement(pieceConfig.movement);
        allPieces.push_back(piece);
      }
    }
    
    // Siyah taşları oluştur
    if (pieceConfig.positions.count("black") > 0) {
      for (const auto &pos : pieceConfig.positions.at("black")) {
        auto piece = std::make_shared<chessPieces>(pieceConfig.type, "black", pos, pieceConfig.special_abilities);
        piece->setMovement(pieceConfig.movement);
        allPieces.push_back(piece);
      }
    }
  }
  
  return allPieces;
}

int main(int argc, char* argv[]) {
    // GameManager'ı oluştur
    GameManager gameManager;
    
    // Konfigürasyon dosyasının yolunu al
    std::string configPath = "data/chess_pieces.json";
    if (argc > 1) {
        configPath = argv[1];
    }
    
    // Oyunu başlat
    if (!gameManager.initialize(configPath)) {
        std::cerr << "Oyun başlatılamadı!" << std::endl;
        return 1;
    }
    
    std::cout << "Oyun başlatıldı!" << std::endl;
    std::cout << "Tahta:" << std::endl;
    std::cout << gameManager.getBoardString() << std::endl;
    
    // Oyun döngüsü
    while (!gameManager.isGameOver()) {
        std::string currentPlayer = gameManager.getCurrentPlayer();
        std::cout << "Sıra " << currentPlayer << " oyuncusunda." << std::endl;
        
        // Kullanıcıdan hareket al
        std::string movename, from_str, to_str;
        std::cin >> movename >> from_str >> to_str;
        // move x,y x1,y1
        int fromX = std::stoi(from_str.substr(0,from_str.find(',')));
        int fromY = std::stoi(from_str.substr(from_str.find(',')+1,from_str.length()));
        int toX = std::stoi(to_str.substr(0,to_str.find(',')));
        int toY = std::stoi(to_str.substr(to_str.find(',')+1,to_str.length()));
        
        // Hareketi yap
        Position from = {fromX, fromY};
        gameManager.printRunLog(toX, toY);
        Position to = {toX, toY};
        MoveResult result = gameManager.makeMove(from, to);
        
        // Hareketin sonucunu kontrol et
        switch (result) {
            case MoveResult::ValidMove:
                std::cout << "Geçerli hareket!" << std::endl;
                break;
            case MoveResult::EnemyPieceCapturable:
                std::cout << "Düşman taş yendi!" << std::endl;
                break;
            case MoveResult::FriendlyPieceBlocking:
                std::cout << "Dost taş yolu kapatıyor!" << std::endl;
                continue;
            case MoveResult::InvalidMovePattern:
                std::cout << "Geçersiz hareket!" << std::endl;
                continue;
            case MoveResult::NoPieceAtSource:
                std::cout << "Belirtilen konumda taş yok!" << std::endl;
                continue;
            case MoveResult::OutOfBounds:
                std::cout << "Hedef konum tahtanın dışında!" << std::endl;
                continue;
            case MoveResult::Portal:
                std::cout << "Portal üzerinden geçildi!" << std::endl;
                break;
            case MoveResult::Castling:
                std::cout << "Castling yapıldı!" << std::endl;
                break;
            case MoveResult::Promotion:
                std::cout << "Taş yükseltildi!" << std::endl;
                break;
            case MoveResult::EnPassant:
                std::cout << "En passant yapıldı!" << std::endl;
                break;
            default:
                std::cout << "Bilinmeyen sonuç!" << std::endl;
                continue;
        }
        
        // Tahtayı yazdır
        std::cout << "Tahta:" << std::endl;
        std::cout << gameManager.getBoardString() << std::endl;
    }
    
    std::cout << "Oyun bitti!" << std::endl;
    
    return 0;
}
