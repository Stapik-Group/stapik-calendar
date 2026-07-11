cmake_minimum_required(VERSION 4.2)
project(stapikcalendar VERSION 1.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME_DISPLAY "Stapik Calendar")
set(PROJECT_AUTHOR "Sebastian Smoliński")
set(PROJECT_GITHUB_URL "https://github.com/Stapik-Group/stapik-calendar")

configure_file(
        ${CMAKE_SOURCE_DIR}/src/ui/dialog/Version.hpp.in
        ${CMAKE_BINARY_DIR}/generated/Version.hpp
        @ONLY
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)

find_package(CURL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        stapikcommon
        GIT_REPOSITORY https://github.com/Stapik-Group/stapik-common.git
        GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(stapikcommon)

add_executable(stapikcalendar
        src/main.cpp
        src/core/command/AddEntryCommand.cpp
        src/core/command/EditEntryCommand.cpp
        src/core/command/DeleteEntryCommand.cpp
        src/core/command/CalendarCommandHistory.cpp
        src/core/command/CalendarCommand.cpp
        src/core/command/EntryCommandBase.cpp
        src/core/command/IndexedEntryCommand.cpp
        src/core/util/UrlTitleFetcher.cpp
        src/core/util/ClipboardUrlDetector.cpp
        src/core/util/DateUtils.cpp
        src/infrastructure/storage/CalendarStorage.cpp
        src/infrastructure/storage/CalendarSyncCoordinator.cpp
        src/ui/window/MainWindow.cpp
        src/ui/action/MenuActionHandler.cpp
        src/ui/dialog/AboutDialog.cpp
        src/ui/dialog/CalendarEntryDialog.cpp
        src/ui/view/CalendarCell.cpp
        src/ui/view/CalendarGrid.cpp
        src/ui/view/CalendarHeader.cpp
        src/ui/view/CalendarView.cpp
        src/ui/view/CalendarNavBar.cpp
        src/ui/widget/MainMenu.cpp
        src/ui/widget/CalendarEntryWidget.cpp)

target_include_directories(stapikcalendar PRIVATE ${GTKMM_INCLUDE_DIRS})
target_include_directories(stapikcalendar PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_compile_options(stapikcalendar PRIVATE ${GTKMM_CFLAGS_OTHER})
target_link_libraries(stapikcalendar PRIVATE ${GTKMM_LIBRARIES} stapikcommon CURL::libcurl nlohmann_json::nlohmann_json)

add_custom_command(TARGET stapikcalendar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:stapikcalendar>/resources)

install(TARGETS stapikcalendar
    RUNTIME DESTINATION lib/stapikcalendar
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
    DESTINATION lib/stapikcalendar/resources
)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/packaging/stapikcalendar-launcher.sh
    DESTINATION bin
    RENAME stapikcalendar
)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/stapikcalendar.desktop
    DESTINATION share/applications
)

install(FILES ${CMAKE_SOURCE_DIR}/packaging/icon.png
    DESTINATION share/icons/hicolor/256x256/apps
    RENAME stapikcalendar.png
)

set(CPACK_PACKAGE_NAME "stapikcalendar")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A retro-styled desktop calendar application")
set(CPACK_PACKAGE_CONTACT "sebastian.smolinski6@email.com")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${PROJECT_AUTHOR} <sebastian.smolinski6@email.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)