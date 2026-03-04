cmake_minimum_required(VERSION 3.19)
project(PROJECT_1 LANGUAGES CXX)
add_library(Eigen INTERFACE)
target_include_directories(Eigen INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)


find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(PROJECT_1
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    engine/model/Node.h
    engine/model/member.h
    engine/engine.h engine/engine.cpp
    graphicsview.h graphicsview.cpp
    engine/model/Load.h
    ui/dialogs/loaddialog.h ui/dialogs/loaddialog.cpp ui/dialogs/loaddialog.ui
    engine/solver/frame_solver.h engine/solver/frame_solver.cpp
    engine/model/member.cpp

)

target_link_libraries(PROJECT_1
    PRIVATE
        Qt::Core
        Qt::Widgets
        Eigen
)

include(GNUInstallDirs)

install(TARGETS PROJECT_1
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET PROJECT_1
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

