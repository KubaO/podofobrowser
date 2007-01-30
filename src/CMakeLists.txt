QT4_WRAP_UI(podofobrowser_UIC_HEADERS
	podofoaboutdlg.ui
	podofobrowserbase.ui
	)

SET(podofobrowser_MOC_HEADERS
	podofobrowser.h
	)
QT4_WRAP_CPP(podofobrowser_MOC_SRCS ${podofobrowser_MOC_HEADERS})

# WTF does this not get done for us?
SET_SOURCE_FILES_PROPERTIES(
	podofobrowser.cpp
	PROPERTIES
	OBJECT_DEPENDS "${podofobrowser_UIC_HEADERS}"
	)

ADD_EXECUTABLE(podofobrowser
	podofobrowser.cpp
	pdflistviewitem.cpp
	main.cpp
	${podofobrowser_MOC_SRCS}
	)

IF(NOT WIN32)
	SET(extra_libs ${LIBFONTCONFIG_LIB})
ELSE(NOT WIN32)
	SET(extra_libs)
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
	)

INSTALL(TARGETS podofobrowser
	RUNTIME DESTINATION "bin"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	)
