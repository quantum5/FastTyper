SRCDIR=src
INCDIR=include

CXX=cl /nologo
LD=link /nologo
CXXFLAGS=/c /I$(INCDIR) /W4 /Zi /DWIN32_LEAN_AND_MEAN /DWINVER=0x0501 /D_WIN32_WINNT=0x0501 /wd4100
LDFLAGS=/subsystem:windows /debug /manifest /incremental:no /opt:REF
LDFLAGS=$(LDFLAGS) "/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'"
RC=rc /nologo
RCFLAGS=/i$(INCDIR)
LIBS=

!IFDEF UNICODE
CFLAGS = $(CFLAGS) /DUNICODE /D_UNICODE
!ENDIF

!IFDEF DEBUG
BUILD=Debug
CFLAGS=$(CFLAGS) /DDEBUG
LDFLAGS=$(LDFLAGS)
!ELSE
BUILD=Release
LDFLAGS=$(LDFLAGS)
!ENDIF

OUTDIR=build\$(BUILD)
DISTDIR=dist\$(BUILD)
FILES=$(OUTDIR)\FastTyper.obj \
      $(OUTDIR)\MainWindow.obj \
      $(OUTDIR)\Window.obj \
      $(OUTDIR)\DragAndDrop.obj \
      $(OUTDIR)\FastTyper.res

all: initdir $(DISTDIR)\FastTyper.exe

initdir:
	if not exist build md build
	if not exist $(OUTDIR) md $(OUTDIR)
	if not exist build md dist
	if not exist $(DISTDIR) md $(DISTDIR)

$(INCDIR)\MainWindow.hpp: $(INCDIR)\Window.hpp

$(SRCDIR)\MainWindow.cpp: $(INCDIR)\MainWindow.hpp $(INCDIR)\DragAndDrop.h
$(SRCDIR)\DragAndDrop.c: $(INCDIR)\DragAndDrop.h
$(SRCDIR)\FastTyper.cpp: $(INCDIR)\MainWindow.hpp
$(SRCDIR)\Window.cpp: $(INCDIR)\Window.hpp

$(OUTDIR)\FastTyper.res: FastTyper.rc
	$(RC) $(RCFLAGS) /fo$@ $**

{$(SRCDIR)}.cpp{$(OUTDIR)}.obj::
	$(CXX) $(CXXFLAGS) /Fo$(OUTDIR)\ /Fd$(OUTDIR)\ $<

{$(SRCDIR)}.c{$(OUTDIR)}.obj::
	$(CXX) $(CXXFLAGS) /Fo$(OUTDIR)\ /Fd$(OUTDIR)\ $<

$(DISTDIR)\FastTyper.exe: $(FILES)
	$(LD) /out:$@ $(LDFLAGS) $** $(LIBS)
	mt.exe -nologo -manifest $@.manifest -outputresource:$@;1 && del $@.manifest || set ERRORLEVEL=0
