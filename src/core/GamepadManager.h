#pragma once
#include <ViGEm/Client.h>
#include <memory>
#include <array>

namespace ktg {

class GamepadManager {
public:
    GamepadManager() = default;
    ~GamepadManager() { shutdown(); }

    bool initialize();
    void shutdown();

    bool createGamepad(int playerIndex);
    void destroyGamepad(int playerIndex);
    bool updateGamepad(int playerIndex, const XUSB_REPORT& report);

    bool isConnected(int playerIndex) const;
    int getConnectedCount() const;

private:
    PVIGEM_CLIENT m_client = nullptr;
    std::array<PVIGEM_TARGET, 4> m_targets = {};
    std::array<bool, 4> m_connected = {};
};

} // namespace ktg
