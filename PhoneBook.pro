TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle

SOURCES += \
    main.cpp \
    Contact.cpp \
    ContactValidator.cpp \
    FileStorage.cpp

HEADERS += \
    Contact.h \
    ContactValidator.h \
    FileStorage.h

DEFINES += QT_USE_QSTRINGBUILDER
