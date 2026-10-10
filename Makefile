# For debug builds, use "debug=1" in command line. For instance,
# to build DLLs in debug mode: nmake dlls debug=1

CC=cl
LINK=link
RC=rc

# Define the output directory
OUTDIR=output

!if "$(debug)"=="1"
EXTRA_CFLAGS=/DDEBUG
!else
EXTRA_CFLAGS=/DMYDLL_RELEASE_BUILD
!endif

LPZLIB=soft\zlib123-dll\lib
ZLIBDLL=soft\zlib123-dll\zlib1.dll

# Added /Fo$(OUTDIR)^\ to CFLAGS to force compiler to output .obj files into OUTDIR
CFLAGS=/nologo /Od /EHsc /wd4731 /Fo$(OUTDIR)^\ $(EXTRA_CFLAGS)
LFLAGS=/NOLOGO
LIBS=user32.lib gdi32.lib comctl32.lib version.lib
LIBSDLL=$(OUTDIR)\pngdib.obj libpng.a zdll.lib $(LIBS)

LUAINC=/I soft\LuaJIT-2.0.4\src
LUALIBPATH=soft\LuaJIT-2.0.4\src
LUALIB=lua51.lib

MAINC=/I soft\miniaudio

# Ensure the output directory exists before compiling anything
all: $(OUTDIR) $(OUTDIR)\sider.exe $(OUTDIR)\sider.dll

# Rule to create the output directory if it doesn't exist
$(OUTDIR):
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(OUTDIR)\sider.res: sider.rc
	$(RC) -r -fo $(OUTDIR)\sider.res sider.rc

$(OUTDIR)\sider_main.res: sider_main.rc sider.ico
	$(RC) -r -fo $(OUTDIR)\sider_main.res sider_main.rc

# Track object dependencies inside the output folder
$(OUTDIR)\audio.obj: audio.cpp audio.h sider.h common.h
$(OUTDIR)\common.obj: common.cpp common.h
$(OUTDIR)\trampoline.obj: trampoline.cpp trampoline.h
$(OUTDIR)\gameplay.obj: gameplay.cpp gameplay.h patterns.h common.h sider.h imageutil.h
$(OUTDIR)\gfx.obj: gfx.cpp gfx.h patterns.h common.h sider.h imageutil.h
$(OUTDIR)\imageutil.obj: imageutil.cpp imageutil.h
$(OUTDIR)\version.obj: version.cpp
$(OUTDIR)\sider.obj: sider.cpp sider.h patterns.h common.h imageutil.h audio.h trampoline.h
$(OUTDIR)\main.obj: main.cpp sider.h
$(OUTDIR)\zlibtool.obj: zlibtool.cpp

$(LUALIBPATH)\$(LUALIB):
	cd $(LUALIBPATH) && msvcbuild.bat

# Updated linker targets to use $(OUTDIR) paths
$(OUTDIR)\sider.dll: $(OUTDIR)\sider.obj $(OUTDIR)\imageutil.obj $(OUTDIR)\version.obj $(OUTDIR)\common.obj $(OUTDIR)\trampoline.obj $(OUTDIR)\gameplay.obj $(OUTDIR)\gfx.obj $(OUTDIR)\audio.obj $(OUTDIR)\sider.res $(LUALIBPATH)\$(LUALIB)
	$(LINK) $(LFLAGS) /out:$(OUTDIR)\sider.dll /DLL $(OUTDIR)\sider.obj $(OUTDIR)\imageutil.obj $(OUTDIR)\version.obj $(OUTDIR)\common.obj $(OUTDIR)\trampoline.obj $(OUTDIR)\gameplay.obj $(OUTDIR)\gfx.obj $(OUTDIR)\audio.obj $(OUTDIR)\sider.res /LIBPATH:$(LUALIBPATH) $(LIBS) $(LUALIB) /LIBPATH:"$(LIB)"

$(OUTDIR)\sider.exe: $(OUTDIR)\main.obj $(OUTDIR)\sider.dll $(OUTDIR)\sider_main.res
	$(LINK) $(LFLAGS) /out:$(OUTDIR)\sider.exe $(OUTDIR)\main.obj $(OUTDIR)\sider_main.res $(LIBS) $(OUTDIR)\sider.lib /LIBPATH:"$(LIB)"

$(OUTDIR)\zlibtool.exe: $(OUTDIR)\zlibtool.obj
	$(LINK) $(LFLAGS) /out:$(OUTDIR)\zlibtool.exe $(OUTDIR)\zlibtool.obj /LIBPATH:$(LPZLIB) $(LIBS) zdll.lib

# Custom inference rule to compile .cpp source file into $(OUTDIR)\filename.obj
{}.cpp{$(OUTDIR)}.obj:
	$(CC) $(CFLAGS) -c $(INC) $(LUAINC) $(MAINC) $<

clean:
	if exist $(OUTDIR) rmdir /s /q $(OUTDIR)

clean-all: clean
	cd $(LUALIBPATH) && del /Q lua51.exp lua51.lib lua51.dll luajit.exe
