QT += core widgets gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = PhoneBook

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    contact.cpp \
    contactvalidator.cpp

HEADERS += \
    mainwindow.h \
    contact.h \
    contactvalidator.h

# Для правильной работы регулярных выражений с юникодом
DEFINES += QT_USE_QSTRINGBUILDER


