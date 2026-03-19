# ──────────────────────────────────────────────────────────────
# toolchain 
# ──────────────────────────────────────────────────────────────

CXX            = c:/MinGW64-gcc14/mingw64/bin/g++.exe
MAKE           = c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe   

# ──────────────────────────────────────────────────────────────
# Compilation flags (adding include paths, enhancing statics)
# ──────────────────────────────────────────────────────────────
	
CXXFLAGS = -std=c++17 -Wall -DCALCLIB_EXPORTS -DWIN32 -D_WINDOWS -D_USRDLL -I../.. \
           -O2 \
           -mlong-double-128 -fext-numeric-literals\
           -D__USE_MINGW_ANSI_STDIO=1 \
           -Ic:/MinGW64-gcc14/mingw64/include \
           -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0 \
           -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0/x86_64-w64-mingw32		

CXXFLAGS += -Wno-unused-variable \
            -Wno-unused-but-set-variable \
            -Wno-switch \
            -Wno-class-memaccess		   

ifdef DEBUG
  CXXFLAGS += -g -O0 -D_DEBUG
  OUTDIR    = gcc_debug
else
  CXXFLAGS += -O2 -DNDEBUG
  OUTDIR    = gcc_release
endif

ifdef ARCH
  ifeq ($(ARCH),32)
    CXXFLAGS += -m32
    TARGET    = $(OUTDIR)/calclib32.dll
  else
    TARGET    = $(OUTDIR)/calclib.dll
  endif
else
  TARGET = $(OUTDIR)/calclib.dll
endif

# ──────────────────────────────────────────────────────────────
# Linking is completely static
# ──────────────────────────────────────────────────────────────
LDFLAGS = -shared -static-libgcc -static-libstdc++ \
          -Wl,--allow-multiple-definition \
          -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive \
          -Wl,-Bstatic -lquadmath -Wl,-Bdynamic

DEFFILE  = calclib.def

SRCS = scalc.cpp sfmts.cpp sfunc.cpp calclib.cpp
OBJS = $(patsubst %.cpp,$(OUTDIR)/%.o,$(notdir $(SRCS)))

VPATH = .

all: $(OUTDIR) $(TARGET)

$(OUTDIR):
	mkdir $(OUTDIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(DEFFILE) $(LDFLAGS) -o $@

$(OUTDIR)/%.o: %.cpp $(MAKEFILE_LIST) ver.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OUTDIR)\*.o $(OUTDIR)\*.dll 2>nul || rm -f $(OUTDIR)/*.o $(OUTDIR)/*.dll
