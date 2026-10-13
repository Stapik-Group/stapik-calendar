cmake_minimum_required(VERSION 4.2)
project(stapikcalendar VERSION 1.3.0 LANGUAGES CXX)

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
        GIT_TAG v1.5.4
        # Lets an installed stapik-common be used instead of a download; the Flatpak build has no network.
        FIND_PACKAGE_ARGS 1.5.4
)
FetchContent_MakeAvailable(stapikcommon)

# Keeps the familiar package name: stapikcalendar_<version>_<arch>.deb
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

stapik_add_application(
        NAME stapikcalendar
        DISPLAY_NAME "Stapik Calendar"
        DESCRIPTION "A retro-styled desktop calendar application"
        APPLICATION_ID pl.stapik.calendar
        RESOURCES_DIRECTORY resources
        PACKAGING_DIRECTORY packaging
        MAINTAINER "Sebastian Smoliński <sebastian.smolinski6@gmail.com>"
        HOMEPAGE "https://github.com/Stapik-Group/stapik-calendar"
        LICENSE GPL-3.0-only
        LONG_DESCRIPTION "A desktop calendar with a monthly view. Every day can hold any number of entries with a name, an optional web link and a color. Entries are moved and copied by dragging, changes can be undone and the calendar is backed up automatically. It can optionally be kept in sync between your devices with a self-hosted Stapik Cloud server."
        LOCALIZED_NAMES "pl=Stapik Kalendarz" "de=Stapik Kalender"
        LOCALIZED_DESCRIPTIONS "pl=Kalendarz na komputer w stylu retro" "de=Ein Desktop-Kalender im Retro-Stil"
        KEYWORDS calendar planner agenda
        LOCALIZED_KEYWORDS "pl=kalendarz,planer,terminarz" "de=Kalender,Planer,Terminkalender"
        DESKTOP_CATEGORIES Office Calendar
        SOURCES
                src/main.cpp
                src/application/CalendarCloudHooks.cpp
                src/application/CalendarController.cpp
                src/core/model/CalendarDocument.cpp
                src/core/model/CalendarYears.cpp
                src/core/command/AddEntryCommand.cpp
                src/core/command/EditEntryCommand.cpp
                src/core/command/DeleteEntryCommand.cpp
                src/core/command/EntryCommandBase.cpp
                src/core/command/IndexedEntryCommand.cpp
                src/core/command/MoveEntryCommand.cpp
                src/core/util/UrlOpener.cpp
                src/core/util/UrlTitleFetcher.cpp
                src/core/util/ClipboardUrlDetector.cpp
                src/core/util/DateUtils.cpp
                src/core/util/EntryDragPayload.cpp
                src/infrastructure/storage/CalendarDocumentStore.cpp
                src/ui/SelfTest.cpp
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
