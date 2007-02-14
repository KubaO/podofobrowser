TEMPLATE = app

CONFIG += debug qt windows exceptions stl
QT += qt3support
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
