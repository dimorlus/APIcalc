# ────────────────────────────────────────────────────────────────
# Makefile for full fcalc.exe with 128-bit long double support
# ────────────────────────────────────────────────────────────────

CXX            = c:/MinGW64-gcc14/mingw64/bin/g++.exe
MAKE           = c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe
WINDRES        = c:/MinGW64-gcc14/mingw64/bin/windres.exe

# ────────────────────────────────────────────────────────────────
# Compilation flags (NO UNICODE - use ANSI everywhere)
# ────────────────────────────────────────────────────────────────

CXXFLAGS = -std=c++17 -Wall -DWIN32 -D_WINDOWS -I../.. \
           -Os \
           -mlong-double-128 -fext-numeric-literals \
           -D__USE_MINGW_ANSI_STDIO=1 \
           -Ic:/MinGW64-gcc14/mingw64/include \
           -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0 \
           -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0/x86_64-w64-mingw32

CXXFLAGS += -Wno-unused-variable \
            -Wno-unused-but-set-variable \
            -Wno-switch \
            -Wno-class-memaccess \
            -Wno-sign-compare \
            -Wno-reorder

ifdef DEBUG
  CXXFLAGS += -g -O0 -D_DEBUG
  OUTDIR    = gcc_debug
else
  CXXFLAGS += -Os -DNDEBUG
  CXXFLAGS += -ffunction-sections -fdata-sections
  OUTDIR    = gcc_release
endif

TARGET = $(OUTDIR)/fcalc.exe

# ────────────────────────────────────────────────────────────────
# Linking flags (fully static, GUI application, NO UNICODE)
# ────────────────────────────────────────────────────────────────

LDFLAGS = -mwindows -static-libgcc -static-libstdc++ \
          -Wl,--allow-multiple-definition \
          -Wl,--enable-auto-image-base \
          -Wl,-Bstatic -lwinpthread -lquadmath -Wl,-Bdynamic

LDFLAGS += -s -flto -Wl,--gc-sections


# Windows libraries for GUI
LIBS = -lhtmlhelp -lcomctl32 -lgdi32 -lcomdlg32 -lole32 -lshell32 -ladvapi32

# ────────────────────────────────────────────────────────────────
# Source files
# ────────────────────────────────────────────────────────────────

# Calculator engine sources (NO calclib.cpp - that's for DLL)
ENGINE_SRCS = scalc.cpp sfmts.cpp sfunc.cpp bmp.cpp files.cpp \
              graphics.cpp matrix.cpp memmng.cpp output.cpp scanners.cpp \
              script.cpp solvers.cpp sym_vars.cpp

# GUI sources
GUI_SRCS = WinApiCalc.cpp

# All sources
SRCS = $(ENGINE_SRCS) $(GUI_SRCS)

# Object files
ENGINE_OBJS = $(patsubst %.cpp,$(OUTDIR)/%.o,$(notdir $(ENGINE_SRCS)))
GUI_OBJS    = $(patsubst %.cpp,$(OUTDIR)/%.o,$(notdir $(GUI_SRCS)))
OBJS        = $(ENGINE_OBJS) $(GUI_OBJS)

# Resource files
RCFILE  = WinApiCalc.rc
RCOBJ   = $(OUTDIR)/fcalc_res.o

# ────────────────────────────────────────────────────────────────
# Build rules
# ────────────────────────────────────────────────────────────────

VPATH = .

all: $(OUTDIR) $(TARGET)

$(OUTDIR):
	mkdir $(OUTDIR)

$(TARGET): $(OBJS) $(RCOBJ)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@
	@echo ──────────────────────────────────────
	@echo Build complete: $(TARGET)
	@echo ──────────────────────────────────────

# Compile sources
$(OUTDIR)/%.o: %.cpp $(MAKEFILE_LIST)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile resource file
$(RCOBJ): $(RCFILE) resource.h
	$(WINDRES) -I. -I../.. --input-format=rc --output-format=coff $(RCFILE) -o $@

# ────────────────────────────────────────────────────────────────
# Utility targets
# ────────────────────────────────────────────────────────────────

clean:
	del /Q $(OUTDIR)\*.o $(OUTDIR)\*.exe 2>nul || rm -f $(OUTDIR)/*.o $(OUTDIR)/*.exe

run: $(TARGET)
	$(TARGET)

debug:
	$(MAKE) DEBUG=1

rebuild: clean all

.PHONY: all clean run debug rebuild
