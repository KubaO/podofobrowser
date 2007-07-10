#
# This project file is an example that can probably be adapted to build
# PoDoFoBrowser. You will need to, at minimum, change the paths
# to the required headers and libraries. As this file is not generally
# tested or maintained, more changes may be required.
#
# Unless you have a really pressing need, use CMake to build PoDoFoBrowser.
#
# To build using this project file you will need to delete the following
# line:

	READ_THE_COMMENTS_AT_THE_START_OF_THIS_FILE_OR_JUST_USE_CMAKE

TEMPLATE = app

CONFIG += debug qt windows exceptions stl
TARGET = src/podofobrowser
DEFINES += USING_SHARED_PODOFO
DEPENDPATH += .
INCLUDEPATH += . c:\PROGRA~1\podofo\include c:\developer\gnuwin32\include c:\developer\gnuwin32\include\freetype2

unix:LIBS += -lpodofo
win32:LIBS += C:/PROGRA~1/podofo/lib/libpodofo.dll.a

FORMS = \
	src/podofoaboutdlg.ui \
	src/podofobrowserbase.ui
HEADERS += \
	src/backgroundloader.h \
	src/pdflistviewitem.h \
	src/podofobrowser.h \
	src/podofoutil.h
SOURCES += \
	src/main.cpp \
	src/podofobrowser.cpp \
	src/pdfobjectmodel.cpp \
	src/backgroundloader.cpp \
	src/podofoutil.cpp
RESOURCES += \
	src/podofobrowserrsrc.qrc
