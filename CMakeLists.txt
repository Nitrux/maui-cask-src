cmake_minimum_required(VERSION 3.21)

project(valenz VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "valenz supports Linux builds only.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(REQUIRED_QT_VERSION 6.9)
set(REQUIRED_KF_VERSION 6.13.0)
set(REQUIRED_MAUI_VERSION 4.0.3)
set(REQUIRED_MAUI_CALENDAR_VERSION 4.0.3)

include(CheckCXXCompilerFlag)
include(FeatureSummary)

string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" VALENZ_HOST_PROCESSOR)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" VALENZ_TARGET_PROCESSOR)

if(NOT CMAKE_CROSSCOMPILING
   AND VALENZ_HOST_PROCESSOR MATCHES "^(x86_64|amd64)$"
   AND VALENZ_TARGET_PROCESSOR MATCHES "^(x86_64|amd64)$")
    check_cxx_compiler_flag("-march=x86-64-v3" VALENZ_HAS_X86_64_V3_CXX)

    if(VALENZ_HAS_X86_64_V3_CXX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-march=x86-64-v3>)
    endif()
endif()

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core DBus Gui Network Qml Quick QuickControls2)
find_package(KF6CoreAddons ${REQUIRED_KF_VERSION} REQUIRED)
find_package(KF6I18n ${REQUIRED_KF_VERSION} REQUIRED)
find_package(MauiKit4 ${REQUIRED_MAUI_VERSION} REQUIRED)
find_package(MauiKitCalendar4 ${REQUIRED_MAUI_CALENDAR_VERSION} REQUIRED)

# Qt 6.8+ policy: generate qmldir files for extra QML subdirectories (e.g. src/modules).
qt_policy(SET QTP0004 NEW)

add_subdirectory(src)
