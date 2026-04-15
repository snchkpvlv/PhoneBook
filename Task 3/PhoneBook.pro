TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle

# Пути для libpq (Homebrew)
LIBS += -L/opt/homebrew/opt/libpq/lib -lpq
INCLUDEPATH += /opt/homebrew/opt/libpq/include

SOURCES += \
    main.cpp \
    Contact.cpp \
    ContactValidator.cpp \
    FileRepository.cpp \
    PostgresRepository.cpp \
    RepositoryFactory.cpp

HEADERS += \
    Contact.h \
    ContactValidator.h \
    IRepository.h \
    FileRepository.h \
    PostgresRepository.h \
    RepositoryFactory.h

DEFINES += QT_USE_QSTRINGBUILDER