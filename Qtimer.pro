QT       += core gui sql

TARGET = qtimer
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    data_provider.cpp \
    content_types.cpp \
    utils.cpp \
    myWidgets.cpp

HEADERS  += mainwindow.h \
    data_provider.h \
    content_types.h \
    utils.h \
    myWidgets.h

TRANSLATIONS += lang_ru.ts

