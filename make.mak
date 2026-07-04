# ──────────────────────────────────────────────────────────────
# Combined makefile — builds CLI, GUI, DLL from one engine compile
# ──────────────────────────────────────────────────────────────

CXX     = c:/MinGW64-gcc14/mingw64/bin/g++.exe
MAKE    = c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe
WINDRES = c:/MinGW64-gcc14/mingw64/bin/windres.exe
OUTDIR  = gcc_release

# ── Compilation flags ──

CXXFLAGS = -std=c++17 -Wall -DWIN32 -D_WINDOWS -I../.. \
           -Os -mlong-double-128 -fext-numeric-literals \
           -D__USE_MINGW_ANSI_STDIO=1 \
           -Ic:/MinGW64-gcc14/mingw64/include \
           -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0 \
           -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0/x86_64-w64-mingw32

CXXFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable \
            -Wno-switch -Wno-class-memaccess \
            -Wno-sign-compare -Wno-reorder

CXXFLAGS += -Os -DNDEBUG -ffunction-sections -fdata-sections

VPATH = . ccalc

# ── Sources ──

ENGINE_SRCS = scalc.cpp sfmts.cpp sfunc.cpp bmp.cpp files.cpp \
              graphics.cpp matrix.cpp memmng.cpp output.cpp scanners.cpp \
              script.cpp solvers.cpp sym_vars.cpp

ENGINE_OBJS = $(addprefix $(OUTDIR)/, $(ENGINE_SRCS:.cpp=.o))
CLI_OBJS    = $(OUTDIR)/ccalc.o $(OUTDIR)/help.o
GUI_OBJS    = $(OUTDIR)/WinApiCalc.o
DLL_OBJS    = $(OUTDIR)/calclib.o
CLI_RCOBJ   = $(OUTDIR)/ccalc_res.o
GUI_RCOBJ   = $(OUTDIR)/fcalc_res.o
DLL_RCOBJ   = $(OUTDIR)/calclib_res.o

CLI_TARGET = $(OUTDIR)/ccalc.exe
GUI_TARGET = $(OUTDIR)/fcalc.exe
DLL_TARGET = $(OUTDIR)/calclib.dll

# ── Link flags ──

LDFLAGS = -static-libgcc -static-libstdc++ \
          -Wl,--allow-multiple-definition \
          -Wl,--enable-auto-image-base \
          -Wl,-Bstatic -lwinpthread -lquadmath -Wl,-Bdynamic

LDFLAGS += -s -flto -Wl,--gc-sections

GUI_LIBS = -lhtmlhelp -lcomctl32 -lgdi32 -lcomdlg32 \
           -lole32 -lshell32 -ladvapi32

# ── Rules ──

all: $(OUTDIR) $(CLI_TARGET) $(GUI_TARGET) $(DLL_TARGET)

$(OUTDIR):
	mkdir $(OUTDIR)

# Engine objects (one compile for all three targets)
$(OUTDIR)/%.o: %.cpp make.mak
	$(CXX) $(CXXFLAGS) -c $< -o $@

# calclib.cpp needs DLL export defines
$(OUTDIR)/calclib.o: calclib.cpp make.mak
	$(CXX) $(CXXFLAGS) -DCALCLIB_EXPORTS -D_USRDLL -c $< -o $@

# Resources
$(CLI_RCOBJ): ccalc/ccalc.rc resource.h
	$(WINDRES) -I. -I../.. --input-format=rc --output-format=coff $< -o $@

$(GUI_RCOBJ): WinApiCalc.rc
	$(WINDRES) -I. -I../.. --input-format=rc --output-format=coff $< -o $@

$(DLL_RCOBJ): calclib.rc ver.h
	$(WINDRES) -I../.. $< -o $@

# Link targets
$(CLI_TARGET): $(ENGINE_OBJS) $(CLI_OBJS) $(CLI_RCOBJ)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(GUI_LIBS) -o $@
	@echo --- CLI: $@

$(GUI_TARGET): $(ENGINE_OBJS) $(GUI_OBJS) $(GUI_RCOBJ)
	$(CXX) $(CXXFLAGS) $^ -mwindows $(LDFLAGS) $(GUI_LIBS) -o $@
	@echo --- GUI: $@

$(DLL_TARGET): $(ENGINE_OBJS) $(DLL_OBJS) $(DLL_RCOBJ)
	$(CXX) $(CXXFLAGS) $^ -shared $(LDFLAGS) -o $@
	@echo --- DLL: $@

# ── Utilities ──

clean:
	-del /Q $(OUTDIR)\*.o $(OUTDIR)\*.exe $(OUTDIR)\*.dll 2>nul

rebuild: clean all

.PHONY: all clean rebuild
