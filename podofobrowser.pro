# Build Notes:
# OS X requires macports and a local build of the svn trunk of the podofo port.
# The podofo that comes with macports is broken and crashes.
# If you get "openssl development package not found" error from qmake, it's because
# qmake can't find /opt/local/bin/pkgconfig. Add /opt/local/bin to the build PATH
# within Qt Creator's project setup.

# To build a local version of PODOFO:
# cmake -G Ninja ~/wc/podofo -DFREETYPE_INCLUDE_DIR=/opt/local/include/freetype2 -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_INSTALL_PREFIX=~/wc/podofo-install
# And pass WITH_PODOFO = ... to qmake

TEMPLATE = app

CONFIG += c++11
QT += widgets

DEPENDPATH += . src
INCLUDEPATH += . src

exists($$WITH_PODOFO) {
    LIBS += -L$$WITH_PODOFO/lib
    INCLUDEPATH += $$WITH_PODOFO/include
}

macx {
    exists(/opt/local/bin/port) {
        LIBS += -L/opt/local/lib
        INCLUDEPATH += /opt/local/include
    }
    LIBS += -lpodofo -ljpeg
    CONFIG += link_pkgconfig
    PKGCONFIG += openssl freetype2 libidn fontconfig zlib
    lessThan(QT_MINOR_VERSION, 8) {
        QMAKE_MAC_SDK = macosx10.12
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12
    }
}

FORMS = \
	src/podofoaboutdlg.ui \
        src/podofobrowserbase.ui \
        src/podofofinddlg.ui \
        src/podofogotodlg.ui \
        src/podofogotopagedlg.ui \
        src/podofoinfodlg.ui \
        src/podoforeplacedlg.ui
HEADERS += \
        src/backgroundloader.h \
        src/pdfobjectmodel.h \
        src/podofobrowser.h \
        src/podofoinfodlg.h \
        src/podofoutil.h \
        src/hexwidget/QHexView.h
SOURCES += \
	src/main.cpp \
        src/backgroundloader.cpp \
        src/pdfobjectmodel.cpp \
        src/podofobrowser.cpp \
        src/podofoinfodlg.cpp \
        src/podofoutil.cpp \
        src/hexwidget/QHexView.cpp
RESOURCES += \
	src/podofobrowserrsrc.qrc
OTHER_FILES += \
        README
