cmake_minimum_required(VERSION 3.16)

project(HostsParameters VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(APPLICATION_DIR_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/hostsapp.rc")

set(PROJECT_SOURCES
    app.rc
    resources.qrc
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/HostModel.h
    src/newhostdialog.h
    src/newhostdialog.cpp
    src/newhostdialog.ui
    src/helper.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HostsParameters
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET HostsParameters APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(HostsParameters SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(HostsParameters
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(HostsParameters PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.HostsParameters)
endif()
set_target_properties(HostsParameters PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS HostsParameters
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HostsParameters)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    if(WIN32)
        find_program(QT_DEPLOY_QT NAMES windeployqt)
        add_custom_target(Script-DeployRelease
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${QT_DEPLOY_QT} --qmldir=${CMAKE_CURRENT_LIST_DIR} --no-translations --compiler-runtime ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}.exe
            COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}.exe" "${CMAKE_SOURCE_DIR}/dist/Hosts File Editor.exe"
            COMMENT "Windows Deploying Qt Dependencies After Build........."
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        # MSIX
        find_program(MAKEAPPX NAMES makeappx)
        find_program(SIGNTOOL NAMES signtool)

        add_custom_target(Script-PackageMSIX
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/AppxManifest.xml ${CMAKE_SOURCE_DIR}/dist/AppxManifest.xml
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/priconfig.xml ${CMAKE_SOURCE_DIR}/dist/priconfig.xml
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/resources.pri ${CMAKE_SOURCE_DIR}/dist/resources.pri
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/resources.scale-125.pri ${CMAKE_SOURCE_DIR}/dist/resources.scale-125.pri
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/resources.scale-150.pri ${CMAKE_SOURCE_DIR}/dist/resources.scale-150.pri
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/resources.scale-200.pri ${CMAKE_SOURCE_DIR}/dist/resources.scale-200.pri
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/package/resources.scale-400.pri ${CMAKE_SOURCE_DIR}/dist/resources.scale-400.pri
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/package/assets ${CMAKE_SOURCE_DIR}/dist/assets
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/dist/vc_redist.x64.exe
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/dist/HostsFileEditor.msix
            COMMAND ${MAKEAPPX} pack -d ${CMAKE_SOURCE_DIR}/dist -p ${CMAKE_SOURCE_DIR}/dist/HostsFileEditor.msix
            COMMAND ${SIGNTOOL} sign /f ${CMAKE_SOURCE_DIR}/selfsignedcert.pfx /p password /fd SHA256 /td SHA256 /tr http://timestamp.comodoca.com ${CMAKE_SOURCE_DIR}/dist/HostsFileEditor.msix
            COMMENT "Creating MSIX package........."
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

    endif()
endif()
