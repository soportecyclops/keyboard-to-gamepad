cmake_minimum_required(VERSION 3.25)
project(KeyboardToGamepad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opciones
option(KTG_BUILD_INSTALLER "Build Inno Setup installer" OFF)

# Directorios de output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Dear ImGui
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

# nlohmann/json
set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/nlohmann_json)

# spdlog
set(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog)

# ViGEmClient (opcional)
set(VIGEMCLIENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/vigem-client)
if(EXISTS ${VIGEMCLIENT_DIR}/CMakeLists.txt)
    add_subdirectory(${VIGEMCLIENT_DIR} ${CMAKE_BINARY_DIR}/vigem-client)
    set(VIGEMCLIENT_INCLUDE_DIR ${VIGEMCLIENT_DIR}/include)
    set(VIGEMCLIENT_LIBRARY ViGEmClient)
    add_compile_definitions(KTG_HAS_VIGEM=1)
    message(STATUS "ViGEmClient found and enabled")
else()
    set(VIGEMCLIENT_INCLUDE_DIR "")
    set(VIGEMCLIENT_LIBRARY "")
    message(WARNING "ViGEmClient not found. Gamepad support disabled.")
endif()

# Generar Version.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h
    @ONLY
)

# Fuentes
file(GLOB_RECURSE KTG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE KTG_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Ejecutable
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
)

if(VIGEMCLIENT_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${VIGEMCLIENT_INCLUDE_DIR})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    d3dcompiler
    shell32
    user32
    gdi32
    hid
    setupapi
)

if(VIGEMCLIENT_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${VIGEMCLIENT_LIBRARY})
endif()
