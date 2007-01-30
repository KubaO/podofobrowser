# Freetype puts its headers in stupid places and expects you to run a
# non-portable shell script to find them.  Find ft2build.h and try to determine
# where freetype.h is relative to it.
# TODO: use pkg-config and/or freetype-config to find freetype when possible
FIND_PATH(LIBFREETYPE_FT2BUILD_H NAMES ft2build.h)
# We don't look for freetype.h since a file of the same name appears in
# FreeType 1 and confuses the detection code. Instead, locate
# config/ftheader.h since this does NOT appear in freetype 1.
# This file may appear in:
#	freetype2/freetype/config
#	freetype/freetype/config
#	freetype2/config
# depending on the freetype2 packaging and version.
FIND_PATH(LIBFREETYPE_FTHEADER_H
    NAMES config/ftheader.h freetype/config/ftheader.h
    PATHS
    ${LIBFREETYPE_FT2BUILD_H}/freetype2
    ${LIBFREETYPE_FT2BUILD_H}/freetype
    )
# At least the library will be somewhere sensible
FIND_LIBRARY(LIBFREETYPE_LIB NAMES freetype libfreetype)
MESSAGE("freetype lib: ${LIBFREETYPE_LIB}")

IF(LIBFREETYPE_FT2BUILD_H AND LIBFREETYPE_FTHEADER_H AND LIBFREETYPE_LIB)
	SET(LIBFREETYPE_FOUND TRUE CACHE BOOLEAN "Was libfreetype found")
ELSE(LIBFREETYPE_FT2BUILD_H AND LIBFREETYPE_FTHEADER_H AND LIBFREETYPE_LIB)
	SET(LIBFREETYPE_FOUND FALSE CACHE BOOLEAN "Was libfreetype found")
ENDIF(LIBFREETYPE_FT2BUILD_H AND LIBFREETYPE_FTHEADER_H AND LIBFREETYPE_LIB)
