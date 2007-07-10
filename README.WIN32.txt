PoDoFoBrowser runs fine on win32. In fact, about half of the testing &
development done on PoDoFoBrowser (at least by Craig) is done on Windows.

BUILDING ON WIN32
=================

Windows users can build PoDoFo and PoDoFoBrowser using the CMake build tool.
MinGW ("gcc for windows") will be required unless you are a commercial Qt
licensee, since there's no Visual Studio support in the Qt Open Source Edition.
Qt will install this for you.

See the INSTALL file for detailed instructons.

PRODUCING A REDISTRIBUTABLE BINARY
==================================

There isn't a PoDoFoBrowser installer or installer creator yet. It's easy
enough to package a PoDoFoBrowser release by hand, however. In general,
all you should need to include is the podofobrowser executable, the COPYING
file and all DLL dependencies not provided by the core OS.

You can determine what DLLs your build of PoDoFoBrowser depends on using a tool
like Dependency Walker (depends.exe) which you can get as a free download from
http://www.dependencywalker.com/ .

If you've built PoDoFoBrowser against a release version of Qt (dynamic),
dynamic versions of freetype6 and zlib from the GnuWin32 project,
and a static copy of PoDoFo, you'll need the following DLLs:

	qtcore4.dll
	qtgui4.dll
	zlib1.dll
	freetype6.dll
	mingwm10.dll

(while PoDoFoBrowser won't generally depend on mingwm10.dll, Qt does).

Add podofo.dll to that list if you've used a dynamically linked version
of PoDoFo (but remember that this is NOT RECOMMENDED for win32 users).

To test your binary distribution of PoDoFo, open a console window (cmd.exe)
and cd into the PoDoFoBrowser distribution directory you created earlier. Run:

    SET PATH=.
    podofobrowser

If PoDoFoBrowser executes without error and works correctly, you should
be fine to redistribute it by simply zipping up that directory. Don't forget
to include the COPYING file and preferably the README.

BUILDING A STATICALLY LINKED SINGLE EXE REDISTRIBUTABLE BINARY
==============================================================

It is almost certainly possible to create a statically linked version of
PoDoFoBrowser with (almost) no external dependencies. See below for how to get
around that "almost". Source changes are required at least for Qt plugin
import, but not much else should be needed.

The author has not verified this process to be correct but does not anticipate any problems.

To create a fully static build you will need to build a static version of Qt
including the QtCore and QtGui libraries, as well as at least the JPEG plugin.
You must import the JPEG plugin with:
	Q_IMPORT_PLUGIN(qjpeg)
For more information about static plugins see:
	http://doc.trolltech.com/4.2/plugins-howto.html#static-plugins

You will also require static library versions of zlib and freetype.

Since the linker may prefer a dynamic library to a static library if they are
in the same part of the search path, you may need to set -DCMAKE_LIBRARY_PATH
to include only the static libraries, and may have to set QTDIR to point to
your static copy of Qt.

When the resulting executable is linked, it will still depend on mingw10.dll .
This is required for thread-safe exception handling (-mthreads -fexceptions)
under MinGW. It is possible to avoid this requirement by building a version of
Qt without exception support. While PoDoFo and PoDoFoBrowser use exceptions and
require them to be available, PoDoFoBrowser tries to avoid throwing through
calls that go through external libraries like Qt. The author's understanding is
that so long as PoDoFo and PoDoFoBrowser are built with exception support you
should be OK, even if Qt is built without exceptions.

If anybody verifies this and creates a fully static build please mail
podofo-users at sf.net to let us know.
