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
        src/MainWindow.cpp
        src/AppStyleProvider.cpp
        src/AppPaths.cpp
        src/util/UrlTitleFetcher.cpp
        src/menu/MenuActionHandler.cpp
        src/menu/MainMenu.cpp
        src/dialog/AboutDialog.cpp
        src/cloud/CloudStorageClient.cpp
        src/cloud/CloudStorageConfigStorage.cpp
        src/cloud/ConnectDialog.cpp
        src/locale/LocalizationEngine.cpp
        src/locale/LocaleManager.cpp
        src/calendar/command/AddEntryCommand.cpp
        src/calendar/command/EditEntryCommand.cpp
        src/calendar/command/DeleteEntryCommand.cpp
        src/calendar/command/CalendarCommandHistory.cpp
        src/calendar/command/CalendarCommand.cpp
        src/calendar/CalendarCell.cpp
        src/calendar/CalendarGrid.cpp
        src/calendar/CalendarHeader.cpp
        src/calendar/CalendarView.cpp
        src/calendar/storage/CalendarStorage.cpp
        src/calendar/detector/ClipboardUrlDetector.cpp
        src/calendar/dialog/CalendarEntryDialog.cpp
        src/calendar/navbar/CalendarNavBar.cpp
        src/calendar/widget/CalendarEntryWidget.cpp)

target_include_directories(stapikcalendar PRIVATE ${GTKMM_INCLUDE_DIRS})
target_compile_options(stapikcalendar PRIVATE ${GTKMM_CFLAGS_OTHER})
target_link_libraries(stapikcalendar PRIVATE ${GTKMM_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)

add_custom_command(TARGET stapikcalendar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/resources
        $<TARGET_FILE_DIR:stapikcalendar>/resources)