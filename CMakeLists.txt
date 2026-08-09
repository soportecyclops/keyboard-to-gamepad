cmake_minimum_required(VERSION 3.25)
project(KeyboardToGamepad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Opciones ──
option(KTG_BUILD_INSTALLER "Build Inno Setup installer" OFF)

# ── Directorios de output ──
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Dependencias ──

# Dear ImGui (docking branch, header-only friendly)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# nlohmann/json (header-only)
set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/nlohmann_json)

# spdlog (header-only)
set(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog)

# ViGEmClient (compilar como subdirectorio)
set(VIGEMCLIENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/vigem-client)
if(EXISTS ${VIGEMCLIENT_DIR}/CMakeLists.txt)
    add_subdirectory(${VIGEMCLIENT_DIR} ${CMAKE_BINARY_DIR}/vigem-client)
    set(VIGEMCLIENT_INCLUDE_DIR ${VIGEMCLIENT_DIR}/include)
    set(VIGEMCLIENT_LIBRARY ViGEmClient)
else()
    message(FATAL_ERROR "ViGEmClient not found. Run setup.ps1 first.")
endif()

# ── Generar Version.h ──
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h
    @ONLY
)

# ── Fuentes ──
file(GLOB_RECURSE KTG_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE KTG_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

# ── Ejecutable ──
add_executable(${PROJECT_NAME} WIN32
    ${KTG_SOURCES}
    ${KTG_HEADERS}
    ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${NLOHMANN_JSON_DIR}/include
    ${SPDLOG_DIR}/include
    ${VIGEMCLIENT_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    shell32.lib
    user32.lib
    gdi32.lib
    hid.lib
    setupapi.lib
    ${VIGEMCLIENT_LIBRARY}
)

# ── Instalador (Inno Setup) ──
if(KTG_BUILD_INSTALLER)
    find_program(ISCC_EXECUTABLE iscc PATHS "C:/Program Files (x86)/Inno Setup 6" "C:/Program Files/Inno Setup 6")
    if(NOT ISCC_EXECUTABLE)
        message(FATAL_ERROR "Inno Setup compiler (iscc.exe) no encontrado. Instalalo desde jrsoftware.org/isinfo.php")
    endif()
    add_custom_target(package_installer
        COMMAND ${ISCC_EXECUTABLE}
            /O${CMAKE_BINARY_DIR}
            /F"Setup_${PROJECT_NAME}_${PROJECT_VERSION}"
            ${CMAKE_CURRENT_SOURCE_DIR}/installer/KeyboardToGamepad.iss
        COMMENT "Generando instalador con Inno Setup..."
    )
endif()
