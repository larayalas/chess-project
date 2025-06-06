#pragma once

#include "ConfigReader.hpp"
#include "Portal.hpp"
#include <vector>
#include <queue>
#include <memory>

class PortalManager {
private:
    std::vector<std::shared_ptr<Portal>> portals;
    std::vector<std::shared_ptr<Portal>> active_portals;
    std::queue<std::pair<std::shared_ptr<Portal>, int>> portal_cooldown;

public:
    PortalManager();
    ~PortalManager();

    void init(const std::vector<PortalConfig>& portal_configs);
    void update(int turn);

    void addPortal(const std::shared_ptr<Portal>& portal);
    void removePortal(const Position& entry);
    std::shared_ptr<Portal> getPortalAtPosition(const Position& pos);
    std::vector<std::shared_ptr<Portal>> getActivePortals();

    void usePortal(std::shared_ptr<Portal> portal);
    void decrementCooldowns();
    void processCooldowns();

    bool isPortalInCooldown(std::shared_ptr<Portal> portal);
};
