QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
	DEFINES += HAVE_QT5
}

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

