QT5_WRAP_UI(podofobrowser_UIC_HEADERS
	podofoaboutdlg.ui
	podofobrowserbase.ui
	podofofinddlg.ui
	podofoinfodlg.ui
	podofogotodlg.ui
	podofogotopagedlg.ui
	podoforeplacedlg.ui
	)

SET(podofobrowser_MOC_HEADERS
	backgroundloader.h
	podofobrowser.h
	pdfobjectmodel.h
	hexwidget/QHexView.h
	)
QT5_WRAP_CPP(podofobrowser_MOC_SRCS ${podofobrowser_MOC_HEADERS})

SET(podofobrowser_PLAIN_HEADERS
    podofoinfodlg.h
    podofoutil.h
    )

# Qt resources such as icons to be embedded in the executable
SET(podofobrowser_RSRC_FILES
	podofobrowserrsrc.qrc
	)
QT5_ADD_RESOURCES(podofobrowser_QRC_SRCS ${podofobrowser_RSRC_FILES})

# WTF does this not get done for us?
SET_SOURCE_FILES_PROPERTIES(
	podofobrowser.cpp
	PROPERTIES
	OBJECT_DEPENDS "${podofobrowser_UIC_HEADERS}"
	)

ADD_EXECUTABLE(podofobrowser
	backgroundloader.cpp
	podofoutil.cpp
	pdfobjectmodel.cpp
	podofobrowser.cpp
	podofoinfodlg.cpp
	main.cpp
	hexwidget/QHexView.cpp
	${podofobrowser_QRC_SRCS}
	${podofobrowser_MOC_SRCS}
    ${podofobrowser_MOC_HEADERS}
    ${podofobrowser_PLAIN_HEADERS}
	)

IF(NOT WIN32)
	SET(extra_libs ${LIBFONTCONFIG_LIB})
ELSE(NOT WIN32)
	SET(extra_libs kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid WS2_32)
	# Suppress the console window for win32 release builds
	SET_TARGET_PROPERTIES(podofobrowser PROPERTIES
		LINK_FLAGS_RELEASE "-mwindows"
		LINK_FLAGS_MINSIZEREL "-mwindows"
		LINK_FLAGS_RELWITHDEBINFO "-mwindows"
		)
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES(podofobrowser
    Qt5::Widgets
	${LIBPODOFO_LIB}
	)

ADD_DEFINITIONS(
	${LIBPODOFO_CFLAGS}
	)

# For an explanation of QT_NO_CAST_FROM_ASCII and QT_NO_CAST_TO_ASCII see:
# http://doc.trolltech.com/qq/qq05-achtung.html
# though they've been renamed from QT_NO_ASCII_CAST and QT_NO_CAST_ASCII
# respectively.

INCLUDE_DIRECTORIES(
	${PODOFO_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
	)

INSTALL(TARGETS podofobrowser
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	)
