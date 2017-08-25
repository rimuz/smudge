AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = smudge
smudge_CXXFLAGS = -O3 -std=c++11 -Wall -I./src $(COMPILE_ARGS)
smudge_CFLAGS =

if IS_WINDOWS
smudge_CXXFLAGS += -Wl,--export-all-symbols,--out-implib,libsmudge.a
else
smudge_CXXFLAGS += -Wl,--export-dynamic
smudge_LDADD = -ldl
endif

smudge_SOURCES = \
			src/compile/v1/compiler/Compiler.cpp \
			src/compile/v1/compiler/globalScopeCompile.cpp \
			src/compile/v1/compiler/localScopeCompile.cpp \
			src/compile/v1/compiler/operatorsCompile.cpp \
			src/compile/Statement.cpp \
			src/error/error.cpp \
			src/exec/interpreter/Interpreter.cpp \
			src/lib/stdlib.cpp \
			src/lib/io.cpp \
			src/lib/lang.cpp \
			src/lib/system.cpp \
			src/lib/cast.cpp \
			src/lib/math.cpp \
			src/parse/Tokenizer.cpp \
			src/runtime/casts.cpp \
			src/runtime/gc.cpp \
			src/runtime/id.cpp \
			src/utils/unicode/utf8.cpp \
			src/main.cpp

man1_MANS = \
			mans/smudge.man
