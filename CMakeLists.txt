cmake_minimum_required(VERSION 4.2)
project(stapikcalendar)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(stapikcalendar
        src/main.cpp
        src/ui/window/MainWindow.cpp
        src/ui/style/AppStyleProvider.cpp
        src/infrastructure/storage/AppPaths.cpp
        src/core/util/UrlTitleFetcher.cpp
        src/ui/action/MenuActionHandler.cpp
        src/ui/widget/MainMenu.cpp
        src/ui/dialog/AboutDialog.cpp
        src/infrastructure/network/CloudStorageClient.cpp
        src/infrastructure/network/CloudStorageConfigStorage.cpp
        src/ui/dialog/ConnectDialog.cpp
        src/core/locale/LocalizationEngine.cpp
        src/core/locale/LocaleManager.cpp
        src/core/command/AddEntryCommand.cpp
        src/core/command/EditEntryCommand.cpp
        src/core/command/DeleteEntryCommand.cpp
        src/core/command/CalendarCommandHistory.cpp
        src/core/command/CalendarCommand.cpp
        src/ui/view/CalendarCell.cpp
        src/ui/view/CalendarGrid.cpp
        src/ui/view/CalendarHeader.cpp
        src/ui/view/CalendarView.cpp
        src/infrastructure/storage/CalendarStorage.cpp
        src/core/util/ClipboardUrlDetector.cpp
        src/ui/dialog/CalendarEntryDialog.cpp
        src/ui/view/CalendarNavBar.cpp
        src/ui/widget/CalendarEntryWidget.cpp)

target_include_directories(stapikcalendar PRIVATE ${GTKMM_INCLUDE_DIRS})
target_compile_options(stapikcalendar PRIVATE ${GTKMM_CFLAGS_OTHER})
target_link_libraries(stapikcalendar PRIVATE ${GTKMM_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)

add_custom_command(TARGET stapikcalendar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:stapikcalendar>/resources)