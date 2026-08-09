#pragma once
#include <vector>
#include <string>

namespace ktg {

struct DeviceInfo;

class MainWindow {
public:
    MainWindow() = default;
    ~MainWindow() = default;

    void initialize();
    void shutdown();
    void update();  // Llamado cada frame, renderiza UI ImGui

private:
    void showMenuBar();
    void showDevicePanel();
    void showMappingPanel();
    void showLogPanel();
    void showStatusBar();

    void renderDeviceCard(const DeviceInfo& dev, int index);
    std::string formatDeviceName(const std::wstring& name) const;

    bool m_showDemoWindow = false;
    bool m_showLogPanel = true;

    // Cache de dispositivos para evitar lock en cada frame
    std::vector<DeviceInfo> m_cachedDevices;
    float m_refreshTimer = 0.0f;
    static constexpr float REFRESH_INTERVAL = 0.5f;  // Segundos
};

} // namespace ktg
