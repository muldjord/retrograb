TEMPLATE = app
TARGET = Mouzoom
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += release
QT += widgets

include(./VERSION)
DEFINES+=VERSION=\\\"$$VERSION\\\"

# Input
HEADERS += src/window.h \
           src/slider.h

SOURCES += src/main.cpp \
           src/window.cpp \
           src/slider.cpp
