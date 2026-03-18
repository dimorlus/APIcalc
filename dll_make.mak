# ──────────────────────────────────────────────────────────────
# Основные пути к toolchain из MSYS2 UCRT64
# ──────────────────────────────────────────────────────────────

CXX            = c:/MinGW64-gcc14/mingw64/bin/g++.exe
MAKE           = c:/MinGW64-gcc14/mingw64/bin/mingw32-make.exe   # если хочешь использовать из MSYS2

# Если хочешь оставить свой старый make — просто не меняй MAKE, оставь как есть

# ──────────────────────────────────────────────────────────────
# Флаги компиляции (добавляем пути include, усиливаем статику)
# ──────────────────────────────────────────────────────────────

#CXXFLAGS = -std=c++17 -Wall \
#           -DCALCLIB_EXPORTS -DWIN32 -D_WINDOWS -D_USRDLL \
#           -I../.. \
#           -O2 -mfpmath=387 -mlong-double-80 \
#           -D__USE_MINGW_ANSI_STDIO=1 \
#			-Ic:/MinGW64-gcc14/mingw64/include/ \
#            -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0 \
#            -Ic:/MinGW64-gcc14/mingw64/include/c++/14.2.0/x86_64-w64-mingw32 
			
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
# Версия gcc может быть другой — проверь `g++ --version` и подставь вместо 15.2.0

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
# Линковка — полностью статическая (самый надёжный вариант для MSYS2)
# ──────────────────────────────────────────────────────────────
# LDFLAGS = -shared -static -static-libgcc -static-libstdc++ -lquadmath

LDFLAGS = -shared -static-libgcc -static-libstdc++ \
          -Wl,--allow-multiple-definition \
          -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive \
          -Wl,-Bstatic -lquadmath -Wl,-Bdynamic

#LDFLAGS = -shared \
#          -static \
#          -static-libgcc \
#          -static-libstdc++ \
#          -Wl,-Bstatic \
#          -lwinpthread \
#          -Wl,-Bdynamic \
#          -L"c:/MinGW64-gcc14/mingw64/lib"

# Вариант 2 (ещё более жёсткий, часто решает проблемы с winpthread):
# LDFLAGS = -shared -static -static-libgcc -static-libstdc++ \
#           -Wl,--whole-archive -lwinpthread -Wl,--no-whole-archive

DEFFILE  = calclib.def

# ──────────────────────────────────────────────────────────────
# Остальное остаётся почти без изменений
# ──────────────────────────────────────────────────────────────

SRCS = scalc.cpp sfmts.cpp sfunc.cpp calclib.cpp
OBJS = $(patsubst %.cpp,$(OUTDIR)/%.o,$(notdir $(SRCS)))

VPATH = .

all: $(OUTDIR) $(TARGET)

$(OUTDIR):
	mkdir $(OUTDIR)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(DEFFILE) $(LDFLAGS) -o $@
#$(TARGET): $(OBJS)
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(DEFFILE)

$(OUTDIR)/%.o: %.cpp $(MAKEFILE_LIST) ver.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OUTDIR)\*.o $(OUTDIR)\*.dll 2>nul || rm -f $(OUTDIR)/*.o $(OUTDIR)/*.dll

# 80-bit long double build (32-bit) — можно оставить как есть или тоже перевести на новый CXX80
# ...