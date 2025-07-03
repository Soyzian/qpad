QT       += core gui quick quickcontrols2
QTPLUGIN += qtextformats  # incluye el plugin de formatos de texto

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -lole32 -luuid -lcomdlg32 -lrpcrt4

SOURCES += \
    exportworker.cpp \
    main.cpp \
    qpad.cpp \
    qpad_config.cpp \
    qsettings.cpp \
    winblur.cpp

HEADERS += \
    exportworker.h \
    qpad.h \
    qpad_config.h \
    qsettings.h \
    winblur.h

FORMS += \
    qpad.ui \
    qpad_config.ui \
    qsettings.ui

TRANSLATIONS += \
    QPad_es_AR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    styles.qml \
    styles.qss

RESOURCES += \
    res.qrc
