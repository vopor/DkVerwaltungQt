cmake_minimum_required(VERSION 3.10)

project(DkVerwaltungQt LANGUAGES CXX)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
# set(CMAKE_PREFIX_PATH C:\\Qt\\5.15.2\\msvc2019\\lib\\cmake)
list(APPEND CMAKE_PREFIX_PATH C:\\Qt\\5.15.2\\msvc2019\\lib\\cmake)
endif (WIN32)

if (UNIX)
# set(CMAKE_PREFIX_PATH  /opt/qt515/lib/cmake)
list(APPEND CMAKE_PREFIX_PATH  /opt/qt515/lib/cmake)
endif (UNIX)

if (APPLE)
# set(CMAKE_PREFIX_PATH  $ENV{HOME}/Qt/5.15.2/clang_64/lib/cmake)
list(APPEND CMAKE_PREFIX_PATH  $ENV{HOME}/Qt/5.15.2/clang_64/lib/cmake)
endif (APPLE)

find_package(Qt5 COMPONENTS Core Gui Widgets Network Sql PrintSupport WebEngine WebEngineWidgets REQUIRED)

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

file(GLOB RESOURCE_FILES
    Jahreskontoauszug.html
    Zinsbescheinigung.html
    F13TurleyGmbH2.gif
    sendDKJAKtos.py
    printCommandDescription.sh
    mail-content.txt
    importCsvIntoDkVerwaltungQt.py
)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES} ${RESOURCE_FILES})

foreach(RESOURCE_FILE ${RESOURCE_FILES})
  set_property(SOURCE ${RESOURCE_FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RES_PATH}")
endforeach(RESOURCE_FILE)

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Sql Qt5::PrintSupport Qt5::WebEngine Qt5::WebEngineWidgets)
