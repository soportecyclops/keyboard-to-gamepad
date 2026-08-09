#include "ui/MainWindow.h"
#include "app/Application.h"
#include "app/Constants.h"
#include "app/Version.h"
#include "core/DeviceRouter.h"
#include "utils/Logger.h"

#include <imgui.h>
#include <shellapi.h>
#include <chrono>

namespace ktg {

void MainWindow::initialize() {
    Logger::info("[MainWindow] Initialized");
}

void MainWindow::shutdown() {
    Logger::info("[MainWindow] Shutdown");
}

void MainWindow::update() {
    showMenuBar();

    // Panel principal con docking
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
                           | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("MainDockSpace", nullptr, flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    showDevicePanel();
    showMappingPanel();
    if (m_showLogPanel) showLogPanel();
    showStatusBar();

    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    ImGui::End();
}

void MainWindow::showMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Archivo")) {
            if (ImGui::MenuItem("Salir")) {
                Application::get().requestQuit();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Ver")) {
            ImGui::MenuItem("Panel de Log", nullptr, &m_showLogPanel);
            ImGui::MenuItem("Demo Window", nullptr, &m_showDemoWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Dispositivos")) {
            if (ImGui::MenuItem("Refrescar lista")) {
                auto router = Application::get().getDeviceRouter();
                if (router) router->refreshDeviceList();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Ayuda")) {
            if (ImGui::MenuItem("Acerca de...")) {
                ImGui::OpenPopup("AboutPopup");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Popup About
    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("KeyboardToGamepad v%s", KTG_VERSION_STRING);
        ImGui::Separator();
        ImGui::Text("Convierte multiples teclados USB en gamepads XInput.");
        ImGui::Text("Licencia: MIT");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void MainWindow::showDevicePanel() {
    ImGui::Begin("Dispositivos Conectados");

    auto router = Application::get().getDeviceRouter();
    if (!router) {
        ImGui::TextDisabled("Router no disponible.");
        ImGui::End();
        return;
    }

    // Refrescar cache cada REFRESH_INTERVAL segundos
    m_refreshTimer += ImGui::GetIO().DeltaTime;
    if (m_refreshTimer >= REFRESH_INTERVAL) {
        m_cachedDevices = router->getDevices();
        m_refreshTimer = 0.0f;
    }

    size_t count = m_cachedDevices.size();

    ImGui::Text("Teclados detectados: %zu / %d", count, MAX_PLAYERS);
    ImGui::SameLine();
    if (ImGui::Button("Refrescar")) {
        router->refreshDeviceList();
        m_cachedDevices = router->getDevices();
    }

    ImGui::Separator();

    if (count == 0) {
        ImGui::TextDisabled("No hay teclados detectados.");
        ImGui::TextDisabled("Conecta un teclado USB para comenzar.");
        ImGui::Spacing();
        ImGui::TextWrapped("Cada teclado se asignara automaticamente como Player 1, 2, 3 o 4.");
    } else {
        for (size_t i = 0; i < m_cachedDevices.size(); ++i) {
            renderDeviceCard(m_cachedDevices[i], static_cast<int>(i));
        }
    }

    ImGui::End();
}

void MainWindow::renderDeviceCard(const DeviceInfo& dev, int index) {
    ImGui::PushID(index);

    bool isOpen = ImGui::CollapsingHeader(
        formatDeviceName(dev.name).c_str(),
        ImGuiTreeNodeFlags_DefaultOpen
    );

    if (isOpen) {
        ImGui::Indent(16.0f);

        // Estado de conexion
        ImVec4 statusColor = dev.connected ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
        const char* statusText = dev.connected ? "Conectado" : "Desconectado";

        ImGui::Text("Estado: "); 
        ImGui::SameLine();
        ImGui::TextColored(statusColor, "%s", statusText);

        // Player asignado
        if (dev.assignedPlayer >= 0) {
            ImGui::Text("Asignado a: Player %d", dev.assignedPlayer + 1);
        } else {
            ImGui::TextDisabled("Sin asignar");
        }

        // Hardware ID
        std::string hwid(dev.hardwareId.begin(), dev.hardwareId.end());
        if (!hwid.empty()) {
            ImGui::Text("Hardware: %s", hwid.c_str());
        }

        // ID persistente (truncado)
        if (!dev.persistentId.empty()) {
            std::string shortId = dev.persistentId;
            if (shortId.length() > 40) shortId = shortId.substr(0, 37) + "...";
            ImGui::TextDisabled("ID: %s", shortId.c_str());
        }

        // Boton para reasignar player
        ImGui::Spacing();
        ImGui::Text("Asignar a:");
        ImGui::SameLine();
        for (int p = 0; p < MAX_PLAYERS; ++p) {
            ImGui::PushID(p);
            bool isCurrent = (dev.assignedPlayer == p);
            if (isCurrent) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
            }
            if (ImGui::Button(std::to_string(p + 1).c_str(), ImVec2(28, 28))) {
                auto router = Application::get().getDeviceRouter();
                if (router) {
                    router->assignPlayer(dev.persistentId, p);
                    m_cachedDevices = router->getDevices();  // Refrescar inmediatamente
                }
            }
            if (isCurrent) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            ImGui::PopID();
        }
        // Boton desasignar
        if (ImGui::Button("X", ImVec2(28, 28))) {
            auto router = Application::get().getDeviceRouter();
            if (router) {
                router->unassignPlayer(dev.persistentId);
                m_cachedDevices = router->getDevices();
            }
        }

        ImGui::Unindent(16.0f);
    }

    ImGui::PopID();
}

void MainWindow::showMappingPanel() {
    ImGui::Begin("Mapeo de Controles");

    auto router = Application::get().getDeviceRouter();
    if (!router || router->getConnectedCount() == 0) {
        ImGui::TextDisabled("Conecta al menos un teclado para configurar el mapeo.");
        ImGui::End();
        return;
    }

    ImGui::Text("Selecciona un dispositivo en el panel izquierdo y asigna los botones.");
    ImGui::Separator();

    // Placeholder para futuro Milestone 3
    ImGui::TextDisabled("(Mapeo visual disponible en Milestone 3)");
    ImGui::TextWrapped("Por ahora, los perfiles se cargan desde archivos JSON en %%APPDATA%%\\KeyboardToGamepad\\profiles\\");

    ImGui::End();
}

void MainWindow::showLogPanel() {
    ImGui::Begin("Log del Sistema");

    ImGui::TextDisabled("Los logs se guardan en:");
    ImGui::TextDisabled("%%LOCALAPPDATA%%\\KeyboardToGamepad\\logs\\app.log");
    ImGui::Separator();

    // Instrucciones para ver logs
    ImGui::BulletText("Abre el archivo app.log con cualquier editor de texto.");
    ImGui::BulletText("Los niveles son: TRACE < DEBUG < INFO < WARN < ERROR < FATAL");
    ImGui::BulletText("Por defecto se loguea desde DEBUG en adelante.");

    ImGui::Spacing();
    ImGui::Text("Eventos recientes (desde el inicio):");
    ImGui::TextDisabled("  - Application initialized");
    ImGui::TextDisabled("  - RawInputManager registered");
    ImGui::TextDisabled("  - DeviceRouter ready");

    if (ImGui::Button("Abrir carpeta de logs")) {
        std::wstring logPath = getAppDataPath(false) + L"\\KeyboardToGamepad\\logs";
        ShellExecuteW(nullptr, L"open", logPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }

    ImGui::End();
}

void MainWindow::showStatusBar() {
    ImGui::Begin("Estado");

    auto router = Application::get().getDeviceRouter();
    size_t connected = router ? router->getConnectedCount() : 0;
    size_t assigned = router ? router->getAssignedCount() : 0;

    ImGui::Text("ViGEmBus: No verificado (Milestone 2)");

    ImVec4 devColor = (connected > 0) ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    ImGui::Text("Teclados: "); 
    ImGui::SameLine();
    ImGui::TextColored(devColor, "%zu conectados", connected);
    ImGui::SameLine();
    ImGui::Text("| %zu asignados", assigned);

    ImGui::Text("Gamepads: 0 / %d (Milestone 2)", MAX_PLAYERS);
    ImGui::Separator();
    ImGui::Text("Version: %s", KTG_VERSION_STRING);

    ImGui::End();
}

std::string MainWindow::formatDeviceName(const std::wstring& name) const {
    // Convertir wstring a string basico (para ImGui)
    std::string result(name.begin(), name.end());

    // Limpiar: quitar prefijos comunes de Windows
    static const char* prefixes[] = {
        "\\??\\", "\\Device\\", "HID#"
    };
    for (const auto* prefix : prefixes) {
        size_t pos = result.find(prefix);
        if (pos != std::string::npos) {
            result = result.substr(pos + strlen(prefix));
        }
    }

    // Si quedo muy largo, truncar
    if (result.length() > 60) {
        result = result.substr(0, 57) + "...";
    }

    // Si esta vacio, poner generico
    if (result.empty()) {
        result = "Teclado USB";
    }

    return result;
}

} // namespace ktg
