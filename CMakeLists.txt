cmake_minimum_required(VERSION 4.2)
project(stapikcalendar VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(SIGCPP REQUIRED IMPORTED_TARGET sigc++-3.0)
find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(stapikcommon)

stapik_add_application(
        NAME stapikcalendar
        DISPLAY_NAME "Stapik Calendar"
        DESCRIPTION "A retro-styled desktop calendar application"
        APPLICATION_ID pl.stapik.calendar
        RESOURCES_DIRECTORY resources
        PACKAGING_DIRECTORY packaging
        MAINTAINER "Sebastian Smoliński <sebastian.smolinski6@email.com>"
        HOMEPAGE "https://github.com/Stapik-Group/stapik-calendar"
        DESKTOP_CATEGORIES Office Calendar
        SOURCES
                src/main.cpp
                src/core/command/AddEntryCommand.cpp
                src/core/command/EditEntryCommand.cpp
                src/core/command/DeleteEntryCommand.cpp
                src/core/command/EntryCommandBase.cpp
                src/core/command/IndexedEntryCommand.cpp
                src/core/command/MoveEntryCommand.cpp
                src/core/util/UrlTitleFetcher.cpp
                src/core/util/ClipboardUrlDetector.cpp
                src/core/util/DateUtils.cpp
                src/core/util/EntryColorUtils.cpp
                src/core/util/EntryDragPayload.cpp
                src/infrastructure/storage/CalendarStorage.cpp
                src/infrastructure/storage/CalendarSyncCoordinator.cpp
                src/ui/window/MainWindow.cpp
                src/ui/action/MenuActionHandler.cpp
                src/ui/dialog/CalendarEntryDialog.cpp
                src/ui/view/CalendarCell.cpp
                src/ui/view/CalendarGrid.cpp
                src/ui/view/CalendarHeader.cpp
                src/ui/view/CalendarView.cpp
                src/ui/view/CalendarNavBar.cpp
                src/ui/widget/CalendarEntryWidget.cpp
                src/ui/widget/EntryColorPickerWidget.cpp
        LINK_LIBRARIES PkgConfig::GTKMM CURL::libcurl nlohmann_json::nlohmann_json)
