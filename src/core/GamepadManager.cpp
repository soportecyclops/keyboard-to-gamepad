#include "core/GamepadManager.h"
#include "utils/Logger.h"

namespace ktg {

bool GamepadManager::initialize() {
    m_client = vigem_alloc();
    if (!m_client) {
        Logger::error("vigem_alloc failed");
        return false;
    }

    const auto retval = vigem_connect(m_client);
    if (!VIGEM_SUCCESS(retval)) {
        Logger::error("vigem_connect failed: {}", retval);
        vigem_free(m_client);
        m_client = nullptr;
        return false;
    }

    Logger::info("GamepadManager initialized (ViGEmClient connected)");
    return true;
}

void GamepadManager::shutdown() {
    for (int i = 0; i < 4; ++i) {
        destroyGamepad(i);
    }
    if (m_client) {
        vigem_disconnect(m_client);
        vigem_free(m_client);
        m_client = nullptr;
    }
    Logger::info("GamepadManager shutdown");
}

bool GamepadManager::createGamepad(int playerIndex) {
    if (playerIndex < 0 || playerIndex >= 4) return false;
    if (!m_client || m_connected[playerIndex]) return false;

    m_targets[playerIndex] = vigem_target_x360_alloc();
    if (!m_targets[playerIndex]) {
        Logger::error("vigem_target_x360_alloc failed for player {}", playerIndex);
        return false;
    }

    const auto retval = vigem_target_add(m_client, m_targets[playerIndex]);
    if (!VIGEM_SUCCESS(retval)) {
        Logger::error("vigem_target_add failed for player {}: {}", playerIndex, retval);
        vigem_target_free(m_targets[playerIndex]);
        m_targets[playerIndex] = nullptr;
        return false;
    }

    m_connected[playerIndex] = true;
    Logger::info("Gamepad created for Player {}", playerIndex + 1);
    return true;
}

void GamepadManager::destroyGamepad(int playerIndex) {
    if (playerIndex < 0 || playerIndex >= 4) return;
    if (!m_connected[playerIndex]) return;

    vigem_target_remove(m_client, m_targets[playerIndex]);
    vigem_target_free(m_targets[playerIndex]);
    m_targets[playerIndex] = nullptr;
    m_connected[playerIndex] = false;

    Logger::info("Gamepad destroyed for Player {}", playerIndex + 1);
}

bool GamepadManager::updateGamepad(int playerIndex, const XUSB_REPORT& report) {
    if (playerIndex < 0 || playerIndex >= 4) return false;
    if (!m_connected[playerIndex]) return false;

    const auto retval = vigem_target_x360_update(m_client, m_targets[playerIndex], report);
    return VIGEM_SUCCESS(retval);
}

bool GamepadManager::isConnected(int playerIndex) const {
    if (playerIndex < 0 || playerIndex >= 4) return false;
    return m_connected[playerIndex];
}

int GamepadManager::getConnectedCount() const {
    int count = 0;
    for (bool c : m_connected) if (c) ++count;
    return count;
}

} // namespace ktg
