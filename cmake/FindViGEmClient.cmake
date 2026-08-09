# FindViGEmClient.cmake
# Busca ViGEmClient SDK en deps/vigem-client o en VIGEMCLIENT_ROOT

set(VIGEMCLIENT_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/vigem-client
    ${VIGEMCLIENT_ROOT}
    $ENV{VIGEMCLIENT_ROOT}
    "C:/Program Files/ViGEm"
    "C:/ViGEm"
)

find_path(VIGEMCLIENT_INCLUDE_DIR
    NAMES ViGEm/Client.h
    PATHS ${VIGEMCLIENT_SEARCH_PATHS}
    PATH_SUFFIXES include
)

find_library(VIGEMCLIENT_LIBRARY
    NAMES ViGEmClient
    PATHS ${VIGEMCLIENT_SEARCH_PATHS}
    PATH_SUFFIXES lib lib/x64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ViGEmClient DEFAULT_MSG
    VIGEMCLIENT_LIBRARY VIGEMCLIENT_INCLUDE_DIR)

if(ViGEmClient_FOUND)
    message(STATUS "ViGEmClient found: ${VIGEMCLIENT_LIBRARY}")
    message(STATUS "ViGEmClient include: ${VIGEMCLIENT_INCLUDE_DIR}")
endif()
