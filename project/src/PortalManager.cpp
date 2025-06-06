#include "../include/PortalManager.hpp"
#include <algorithm>
#include <iostream>

PortalManager::PortalManager() {}
PortalManager::~PortalManager() {}

void PortalManager::init(const std::vector<PortalConfig>& portal_configs) {
    portals.clear();
    active_portals.clear();
    while (!portal_cooldown.empty()) portal_cooldown.pop();

    for (const auto& config : portal_configs) {
        auto portal = std::make_shared<Portal>(
            config.positions.entry,
            config.positions.exit,
            config.properties.preserve_direction,
            config.properties.allowed_colors,
            config.properties.cooldown,
            config.id
        );
        addPortal(portal);
    }
}

void PortalManager::addPortal(const std::shared_ptr<Portal>& portal) {
    portals.push_back(portal);

    if (portal->getCurrentCooldown() == 0) {
        portal->setActive(true);
        active_portals.push_back(portal);
    } else {
        portal->setActive(false);
        portal_cooldown.push({portal, portal->getCurrentCooldown()});
    }
}

void PortalManager::usePortal(std::shared_ptr<Portal> portal) {
    if (!portal) return;

    portal->use();
    portal_cooldown.push({portal, portal->getCooldown()});

    active_portals.erase(std::remove(active_portals.begin(), active_portals.end(), portal), active_portals.end());
}

void PortalManager::update(int /*turn*/) {
    decrementCooldowns();
}

void PortalManager::decrementCooldowns() {
    processCooldowns();
}

void PortalManager::processCooldowns() {
    int size = portal_cooldown.size();
    for (int i = 0; i < size; ++i) {
        auto [portal, cooldown] = portal_cooldown.front();
        portal_cooldown.pop();

        int newCooldown = cooldown - 1;
        portal->setCurrentCooldown(newCooldown);

        if (newCooldown > 0) {
            portal_cooldown.push({portal, newCooldown});
        } else {
            portal->setActive(true);
            active_portals.push_back(portal);
        }
    }
}

std::shared_ptr<Portal> PortalManager::getPortalAtPosition(const Position& pos) {
    for (auto& portal : portals)
        if (portal->getEntryPosition().x == pos.x && portal->getEntryPosition().y == pos.y)
            return portal;
    return nullptr;
}

std::vector<std::shared_ptr<Portal>> PortalManager::getActivePortals() {
    return active_portals;
}

void PortalManager::removePortal(const Position& entry) {
    for (auto it = portals.begin(); it != portals.end(); ++it) {
        if ((*it)->getEntryPosition().x == entry.x && (*it)->getEntryPosition().y == entry.y) {
            active_portals.erase(
                std::remove_if(active_portals.begin(), active_portals.end(),
                               [&](const std::shared_ptr<Portal>& p) { return p == *it; }),
                active_portals.end()
            );
            portals.erase(it);
            return;
        }
    }
}

bool PortalManager::isPortalInCooldown(std::shared_ptr<Portal> portal) {
    return portal && portal->getCurrentCooldown() > 0;
}
