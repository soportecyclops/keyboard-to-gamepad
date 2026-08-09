#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

namespace ktg {

struct DeviceInfo {
    HANDLE hDevice;                    // Handle Raw Input (volátil)
    std::wstring name;                 // Nombre del dispositivo
    std::wstring hardwareId;           // VID_XXXX&PID_XXXX
    std::wstring serial;               // Número de serie (si disponible)
    std::string persistentId;          // ID único persistente para perfil
    bool connected = true;
    std::chrono::steady_clock::time_point lastSeen;
    std::chrono::steady_clock::time_point firstSeen;
    int assignedPlayer = -1;           // -1 = sin asignar, 0-3 = Player 1-4
};

class DeviceRouter {
public:
    DeviceRouter() = default;

    // Llamado cuando llega un evento Raw Input
    void onInputEvent(HANDLE hDevice, USHORT vKey, bool pressed);

    // Llamado cuando Windows notifica cambio de dispositivos
    void refreshDeviceList();

    // Obtener lista de dispositivos conocidos (thread-safe)
    std::vector<DeviceInfo> getDevices() const;

    // Asignar/desasignar jugador
    bool assignPlayer(const std::string& persistentId, int playerIndex);
    void unassignPlayer(const std::string& persistentId);

    // Obtener dispositivo por handle (para routing)
    const DeviceInfo* findDevice(HANDLE hDevice) const;

    // Contadores
    size_t getConnectedCount() const;
    size_t getAssignedCount() const;

private:
    mutable std::mutex m_mutex;
    std::map<std::string, DeviceInfo> m_devices;  // key = persistentId

    std::string buildPersistentId(HANDLE hDevice);
    DeviceInfo probeDevice(HANDLE hDevice);
    void logDeviceEvent(const DeviceInfo& dev, const std::string& event);
};

} // namespace ktg
