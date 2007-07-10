del CMakeCache.txt
@rem Uncomment these lines to force stronger debugging flags
@rem set CXXFLAGS=-g3 -O0 -fno-inline
@rem set CFLAGS=-g3 -O0 -fno-inline
@rem Cmake build type. debug or release .
set BT=debug
@rem Qt install directory
set QTDIR=c:\developer\qt\4.2.1
@rem GnuWin32 install directory
set GW32=c:\developer\gnuwin32
@rem Are we using a shared library build of PoDoFo? (0: no, 1: yes)
set PD_SHR=0
set PD_DIR="C:\Program Files\PoDoFo"
@rem Make options. -j2 is often useful for dual core machines.
set MAKEOPTS=
set PATH=%QTDIR%\bin;%PD_DIR%\bin;%PATH%
c:\developer\cmake\bin\cmake -G "MinGW Makefiles" -DCMAKE_INCLUDE_PATH=%GW32%\include -DCMAKE_LIBRARY_PATH=%GW32%\lib -DLIBPODOFO_SHARED=%PD_SHR% -DLIBPODOFO_DIR=%PD_DIR% -DCMAKE_BUILD_TYPE=%BT%
@echo Starting build with mingw32-make
mingw32-make %MAKEOPTS%
@pause
