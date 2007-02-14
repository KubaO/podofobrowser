QT4_WRAP_UI(podofobrowser_UIC_HEADERS
	podofoaboutdlg.ui
	podofobrowserbase.ui
	)

SET(podofobrowser_MOC_HEADERS
	backgroundloader.h
	podofobrowser.h
	pdfobjectmodel.h
	)
QT4_WRAP_CPP(podofobrowser_MOC_SRCS ${podofobrowser_MOC_HEADERS})

# Qt resources such as icons to be embedded in the executable
SET(podofobrowser_RSRC_FILES
	podofobrowserrsrc.qrc
	)
QT4_ADD_RESOURCES(podofobrowser_QRC_SRCS ${podofobrowser_RSRC_FILES})

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
	main.cpp
	${podofobrowser_QRC_SRCS}
	${podofobrowser_MOC_SRCS}
	)

IF(NOT WIN32)
	SET(extra_libs ${LIBFONTCONFIG_LIB})
ELSE(NOT WIN32)
	SET(extra_libs kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid WS2_32)
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES(podofobrowser
	${QT_LIBRARIES}
	${LIBPODOFO_LIB}
	${LIBFREETYPE_LIB}
	${extra_libs}
	${LIBJPEG_LIB}
	${LIBZ_LIB}
	)

ADD_DEFINITIONS(
	${LIBPODOFO_CFLAGS}
	${QT_DEFINITIONS}
	-DQT_QT3SUPPORT_LIB
	-DQT3_SUPPORT
	-DQT_CORE_LIB
	-DQT_GUI_LIB
	-DQT_THREAD_SUPPORT
	)

INCLUDE_DIRECTORIES(
	${QT_INCLUDES}
	${PODOFO_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
	)

INSTALL(TARGETS podofobrowser
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	)
