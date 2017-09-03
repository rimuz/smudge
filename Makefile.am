AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = smudge
smudge_CXXFLAGS = -O3 -std=c++11 -Wall -I./src $(COMPILE_ARGS)
smudge_CFLAGS =

if IS_WINDOWS
smudge_CXXFLAGS += -Wl,--export-all-symbols,--out-implib,libsmudge.a
else
smudge_CXXFLAGS += -Wl,--export-dynamic
smudge_LDFLAGS = -pthread
smudge_LDADD = -ldl
endif

smudge_SOURCES = \
			sm/compile/v1/compiler/Compiler.cpp \
			sm/compile/v1/compiler/globalScopeCompile.cpp \
			sm/compile/v1/compiler/localScopeCompile.cpp \
			sm/compile/v1/compiler/operatorsCompile.cpp \
			sm/compile/Statement.cpp \
			sm/error/error.cpp \
			sm/exec/interpreter/Interpreter.cpp \
			sm/lib/stdlib.cpp \
			sm/lib/cast.cpp \
			sm/lib/io.cpp \
			sm/lib/lang.cpp \
			sm/lib/math.cpp \
			sm/lib/system.cpp \
			sm/lib/thread.cpp \
			sm/parse/Tokenizer.cpp \
			sm/runtime/casts.cpp \
			sm/runtime/gc.cpp \
			sm/runtime/id.cpp \
			sm/utils/unicode/utf8.cpp \
			sm/main.cpp

man1_MANS = \
			mans/smudge.man
