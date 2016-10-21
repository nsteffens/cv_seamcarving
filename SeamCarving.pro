#-------------------------------------------------
#
# Project created by QtCreator 2016-10-16T14:52:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SeamCarving
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
        ImageReader.cpp \
        QtOpencvCore.cpp

HEADERS  += MainWindow.hpp \
        ImageReader.hpp \
        QtOpencvCore.hpp

FORMS    +=

macx {

    # MAC Compiler Flags
}

win32 {
    # Windows Compiler Flags
}

unix {

    QMAKE_CXXFLAGS += -std=c++11 -Wall -pedantic -Wno-unknown-pragmas

    INCLUDEPATH += /usr/include

    LIBS += -L/usr/local/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc

    QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-variable -Wno-reorder
}
