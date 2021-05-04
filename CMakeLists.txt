cmake_minimum_required(VERSION 3.14)

project(DkVerwaltungQt LANGUAGES CXX)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Network REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)

file(GLOB SOURCE_FILES
    ansparrechner.cpp ansparrechner.h
    buchungform.cpp buchungform.h
    dbfkts.cpp dbfkts.h
    html2pdfconverter.cpp html2pdfconverter.h
    mainform.cpp mainform.h
    mainwindow.cpp mainwindow.h
    personform.cpp personform.h
    webenginepdfviewer.cpp webenginepdfviewer.h
    main.cpp
)

# "${SOURCE_ROOT}/"
file(GLOB RESOURCE_FILES
    Jahreskontoauszug.html
    Zinsbescheinigung.html
    F13TurleyGmbH2.gif
    sendDKJAKtos.py
    printCommandDescription.sh
    mail-content.txt
)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES} ${RESOURCE_FILES})

# add_custom_target("deploy")
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                        ${CMAKE_CURRENT_SOURCE_DIR}/Jahreskontoauszug.html $<TARGET_FILE_DIR:${PROJECT_NAME}>
# )

foreach(RESOURCE_FILE ${RESOURCE_FILES})
  #Set it's location inside the app package (under Resources)
  set_property(SOURCE ${RESOURCE_FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RES_PATH}")
endforeach(RESOURCE_FILE)

qt5_use_modules(DkVerwaltungQt Network Widgets Sql PrintSupport WebEngine WebEngineWidgets)
