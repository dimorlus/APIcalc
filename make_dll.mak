# Makefile for calclib.dll — GCC/MinGW
# Usage:
#   make          — build x64 release
#   make DEBUG=1  — build x64 debug
#   make ARCH=32  — build x86 (32-bit)
#   make 80bit    — build x86 32-bit with true 80-bit long double

CXX80    = c:/Qt/Tools/mingw810_32/bin/g++
#	CXX      = c:/Qt/Tools/mingw810_64/bin/g++
CXX      = c:/Qt/Tools/mingw1310_64/bin/g++
#	CXX      = c:/Qt/Tools/mingw1120_64/bin/g++
#	CXX      = g++
CXXFLAGS = -std=c++17 -Wall -DCALCLIB_EXPORTS -DWIN32 -D_WINDOWS -D_USRDLL -I../..

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

LDFLAGS  = -shared -static-libgcc -static-libstdc++
DEFFILE  = calclib.def

SRCS = scalc.cpp \
       sfmts.cpp \
       sfunc.cpp \
       calclib.cpp

OBJS = $(patsubst %.cpp,$(OUTDIR)/%.o,$(notdir $(SRCS)))

# Vpath so make finds sources by filename
VPATH = . 

all: $(OUTDIR) $(TARGET)

$(OUTDIR):
	mkdir $(OUTDIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(DEFFILE)

$(OUTDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OUTDIR)\*.o $(OUTDIR)\*.dll 2>nul || rm -f $(OUTDIR)/*.o $(OUTDIR)/*.dll

# 80-bit long double build (32-bit x87)
OUTDIR80   = gcc_release80
TARGET80   = $(OUTDIR80)/calclib.dll

CXX80      = c:/Qt/Tools/mingw810_32/bin/g++
CXXFLAGS80 = -std=c++17 -Wall -DCALCLIB_EXPORTS -DWIN32 -D_WINDOWS -D_USRDLL -D_CALCLIB_  \
             -mlong-double-80 -mfpmath=387 -O2 -DNDEBUG
OBJS80     = $(patsubst %.cpp,$(OUTDIR80)/%.o,$(notdir $(SRCS)))

80bit: $(OUTDIR80) $(TARGET80)

$(OUTDIR80):
	mkdir $(OUTDIR80)

$(TARGET80): $(OBJS80)
	$(CXX80) $(CXXFLAGS80) $(LDFLAGS) -o $@ $^ $(DEFFILE)

$(OUTDIR80)/%.o: %.cpp
	$(CXX80) $(CXXFLAGS80) -c $< -o $@
.PHONY: all clean 80bit
