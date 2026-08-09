#include "app/Application.h"
#include "utils/Logger.h"
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    auto& app = ktg::Application::get();

    if (!app.initialize(hInstance, nCmdShow)) {
        return 1;
    }

    int result = app.run();
    app.shutdown();
    return result;
}
