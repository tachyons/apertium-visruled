#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T12:25:25
#
#-------------------------------------------------

QT       += core gui xml widgets

TARGET = apertium-visruled
TEMPLATE = app


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/config.cpp \
    src/filesystem.cpp \
    src/node.cpp \
    src/filetab.cpp \
    src/sectiontab.cpp \
    src/newfiledialog.cpp \
    src/diagram.cpp \
    src/propertywidget.cpp \
    src/action.cpp \
    src/resources.cpp \
    src/sidebar.cpp \
    src/settingsdialog.cpp \
    src/tools.cpp \
    src/testdialog.cpp

HEADERS  += src/mainwindow.h \
    src/node.h \
    src/config.h \
    src/filesystem.h \
    src/filetab.h \
    src/sectiontab.h \
    src/newfiledialog.h \
    src/action.h \
    src/diagram.h \
    src/propertywidget.h \
    src/resources.h \
    src/sidebar.h \
    src/settingsdialog.h \
    src/tools.h \
    src/testdialog.h

FORMS    += src/mainwindow.ui \
    src/filetab.ui \
    src/sectiontab.ui \
    src/newfiledialog.ui \
    src/settingsdialog.ui \
    src/testdialog.ui

OTHER_FILES += \
    res/schema.xml \
    misc/tests/test.t2x \
    misc/tests/test.t1x \
    res/lists.xml \
    Changelog \
    INSTALL \
    COPYING

unix {
    INSTALLS += target data icon desktopfile

    target.files = $$TARGET
    target.path = /usr/local/bin
    data.files = res
    data.path = /usr/local/share/apertium-visruled
    icon.files = misc/apertium-visruled.png
    icon.path = /usr/local/share/icons
    desktopfile.files = misc/apertium-visruled.desktop
    desktopfile.path = /usr/local/share/applications
    QMAKE_CXXFLAGS += -DVISRULED_DATADIR=$$data.path
}

win32 {
    QMAKE_CXXFLAGS += -DVISRULED_DATADIR="../apertium-visruled"
}
