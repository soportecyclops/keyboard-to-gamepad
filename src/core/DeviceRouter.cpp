#include "core/DeviceRouter.h"
#include "utils/Logger.h"
#include "utils/Win32Utils.h"

#include <sstream>
#include <iomanip>

namespace ktg {

void DeviceRouter::onInputEvent(HANDLE hDevice, USHORT vKey, bool pressed) {
    std::string pid = buildPersistentId(hDevice);

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_devices.find(pid);
    if (it == m_devices.end()) {
        // Nuevo dispositivo detectado por input
        DeviceInfo dev = probeDevice(hDevice);
        dev.persistentId = pid;
        dev.firstSeen = std::chrono::steady_clock::now();
        dev.lastSeen = dev.firstSeen;
        dev.connected = true;

        // Asignar automáticamente el primer slot libre
        for (int i = 0; i < 4 && dev.assignedPlayer < 0; ++i) {
            bool taken = false;
            for (const auto& [k, d] : m_devices) {
                if (d.assignedPlayer == i) { taken = true; break; }
            }
            if (!taken) dev.assignedPlayer = i;
        }

        m_devices[pid] = dev;
        logDeviceEvent(dev, "DETECTED (via input)");

        Logger::info("[DeviceRouter] New keyboard: {} | VID/PID: {} | Assigned to Player {}",
                     std::string(dev.name.begin(), dev.name.end()),
                     std::string(dev.hardwareId.begin(), dev.hardwareId.end()),
                     dev.assignedPlayer + 1);
    } else {
        it->second.lastSeen = std::chrono::steady_clock::now();
        it->second.connected = true;

        // Log de input (solo en debug, no spamear)
        static std::map<std::string, std::chrono::steady_clock::time_point> lastInputLog;
        auto now = std::chrono::steady_clock::now();
        auto& last = lastInputLog[pid];
        if (now - last > std::chrono::seconds(5)) {
            Logger::debug("[DeviceRouter] Input from Player {} ({}): VK=0x{:04X} {}",
                          it->second.assignedPlayer + 1,
                          std::string(it->second.name.begin(), it->second.name.end()),
                          vKey, pressed ? "DOWN" : "UP");
            last = now;
        }
    }
}

void DeviceRouter::refreshDeviceList() {
    Logger::info("[DeviceRouter] Refreshing device list...");

    UINT numDevices = 0;
    GetRawInputDeviceList(nullptr, &numDevices, sizeof(RAWINPUTDEVICELIST));
    if (numDevices == 0) {
        Logger::warn("[DeviceRouter] No Raw Input devices found");
        return;
    }

    std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
    if (GetRawInputDeviceList(deviceList.data(), &numDevices, sizeof(RAWINPUTDEVICELIST)) == static_cast<UINT>(-1)) {
        Logger::error("[DeviceRouter] GetRawInputDeviceList failed: {}", GetLastError());
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Marcar todos como potencialmente desconectados
    for (auto& [pid, dev] : m_devices) {
        dev.connected = false;
    }

    // Re-escanear
    for (const auto& d : deviceList) {
        if (d.dwType != RIM_TYPEKEYBOARD) continue;

        std::string pid = buildPersistentId(d.hDevice);
        auto it = m_devices.find(pid);

        if (it == m_devices.end()) {
            DeviceInfo dev = probeDevice(d.hDevice);
            dev.persistentId = pid;
            dev.hDevice = d.hDevice;
            dev.firstSeen = std::chrono::steady_clock::now();
            dev.lastSeen = dev.firstSeen;
            dev.connected = true;

            // Auto-assign
            for (int i = 0; i < 4 && dev.assignedPlayer < 0; ++i) {
                bool taken = false;
                for (const auto& [k, existing] : m_devices) {
                    if (existing.assignedPlayer == i) { taken = true; break; }
                }
                if (!taken) dev.assignedPlayer = i;
            }

            m_devices[pid] = dev;
            logDeviceEvent(dev, "DETECTED (scan)");
            Logger::info("[DeviceRouter] Keyboard found: {} | {} | Player {}",
                         std::string(dev.name.begin(), dev.name.end()),
                         std::string(dev.hardwareId.begin(), dev.hardwareId.end()),
                         dev.assignedPlayer + 1);
        } else {
            it->second.connected = true;
            it->second.hDevice = d.hDevice;  // Actualizar handle (puede cambiar)
            it->second.lastSeen = std::chrono::steady_clock::now();
        }
    }

    // Reportar desconexiones
    for (auto it = m_devices.begin(); it != m_devices.end(); ) {
        if (!it->second.connected) {
            Logger::info("[DeviceRouter] Keyboard disconnected: {} (Player {})",
                         std::string(it->second.name.begin(), it->second.name.end()),
                         it->second.assignedPlayer + 1);
            it = m_devices.erase(it);
        } else {
            ++it;
        }
    }

    Logger::info("[DeviceRouter] Active keyboards: {}/4", m_devices.size());
}

std::vector<DeviceInfo> DeviceRouter::getDevices() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<DeviceInfo> result;
    for (const auto& [pid, dev] : m_devices) {
        result.push_back(dev);
    }
    return result;
}

bool DeviceRouter::assignPlayer(const std::string& persistentId, int playerIndex) {
    if (playerIndex < 0 || playerIndex >= 4) return false;

    std::lock_guard<std::mutex> lock(m_mutex);

    // Verificar que no esté tomado
    for (auto& [pid, dev] : m_devices) {
        if (dev.assignedPlayer == playerIndex && pid != persistentId) {
            Logger::warn("[DeviceRouter] Player {} already assigned to {}, unassigning first",
                         playerIndex + 1,
                         std::string(dev.name.begin(), dev.name.end()));
            dev.assignedPlayer = -1;
        }
    }

    auto it = m_devices.find(persistentId);
    if (it == m_devices.end()) return false;

    it->second.assignedPlayer = playerIndex;
    Logger::info("[DeviceRouter] {} assigned to Player {}",
                 std::string(it->second.name.begin(), it->second.name.end()),
                 playerIndex + 1);
    return true;
}

void DeviceRouter::unassignPlayer(const std::string& persistentId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_devices.find(persistentId);
    if (it != m_devices.end()) {
        Logger::info("[DeviceRouter] {} unassigned from Player {}",
                     std::string(it->second.name.begin(), it->second.name.end()),
                     it->second.assignedPlayer + 1);
        it->second.assignedPlayer = -1;
    }
}

const DeviceInfo* DeviceRouter::findDevice(HANDLE hDevice) const {
    std::string pid = buildPersistentId(hDevice);
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_devices.find(pid);
    if (it != m_devices.end()) return &it->second;
    return nullptr;
}

size_t DeviceRouter::getConnectedCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_devices.size();
}

size_t DeviceRouter::getAssignedCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t count = 0;
    for (const auto& [pid, dev] : m_devices) {
        if (dev.assignedPlayer >= 0) ++count;
    }
    return count;
}

// ── Helpers privados ──

std::string DeviceRouter::buildPersistentId(HANDLE hDevice) {
    // Intentar obtener VID+PID+Serial como ID persistente
    RID_DEVICE_INFO rdi = {};
    rdi.cbSize = sizeof(RID_DEVICE_INFO);
    UINT cbSize = sizeof(RID_DEVICE_INFO);

    std::stringstream ss;

    if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &cbSize) > 0) {
        if (rdi.dwType == RIM_TYPEKEYBOARD) {
            ss << "VID_" << std::hex << std::setfill('0') << std::setw(4) << rdi.keyboard.dwVendorId
               << "_PID_" << std::setw(4) << rdi.keyboard.dwProductId;

            // Intentar obtener serial del nombre del dispositivo
            UINT nameSize = 0;
            GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, nullptr, &nameSize);
            if (nameSize > 0) {
                std::wstring name(nameSize, L'\0');
                if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, name.data(), &nameSize) > 0) {
                    // El nombre suele contener el GUID único del dispositivo
                    std::string narrow(name.begin(), name.end());
                    // Hashear el nombre para obtener un ID único
                    size_t hash = std::hash<std::string>{}(narrow);
                    ss << "_" << std::hex << hash;
                }
            }
            return ss.str();
        }
    }

    // Fallback: usar el puntero del handle (menos ideal, cambia entre sesiones)
    ss << "HANDLE_" << std::hex << reinterpret_cast<uintptr_t>(hDevice);
    return ss.str();
}

DeviceInfo DeviceRouter::probeDevice(HANDLE hDevice) {
    DeviceInfo info;
    info.hDevice = hDevice;
    info.lastSeen = std::chrono::steady_clock::now();

    // Nombre del dispositivo
    UINT nameSize = 0;
    GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, nullptr, &nameSize);
    if (nameSize > 0) {
        std::wstring name(nameSize, L'\0');
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, name.data(), &nameSize) > 0) {
            info.name = name;
        }
    }

    // Info de hardware
    RID_DEVICE_INFO rdi = {};
    rdi.cbSize = sizeof(RID_DEVICE_INFO);
    UINT cbSize = sizeof(RID_DEVICE_INFO);
    if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &cbSize) > 0) {
        if (rdi.dwType == RIM_TYPEKEYBOARD) {
            wchar_t buf[128];
            swprintf_s(buf, L"VID_%04X&PID_%04X",
                       rdi.keyboard.dwVendorId, rdi.keyboard.dwProductId);
            info.hardwareId = buf;
        }
    }

    // Si no tenemos nombre amigable, usar el hardwareId
    if (info.name.empty()) {
        info.name = info.hardwareId.empty() ? L"Unknown Keyboard" : info.hardwareId;
    }

    return info;
}

void DeviceRouter::logDeviceEvent(const DeviceInfo& dev, const std::string& event) {
    Logger::debug("[DEVICE] {} | {} | Player={} | {}",
                  event,
                  std::string(dev.name.begin(), dev.name.end()),
                  dev.assignedPlayer >= 0 ? std::to_string(dev.assignedPlayer + 1) : "UNASSIGNED",
                  dev.persistentId);
}

} // namespace ktg
