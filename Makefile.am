AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = smudge
smudge_CXXFLAGS = -O3 -std=c++11 -Wall -I./src $(COMPILE_ARGS)
smudge_CFLAGS =
smudge_SOURCES = src/compile/compiler/Compiler.cpp \
			src/compile/compiler/globalScopeCompile.cpp \
			src/compile/compiler/localScopeCompile.cpp \
			src/compile/compiler/operatorsCompile.cpp \
			src/compile/Statement.cpp \
			src/error/error.cpp \
			src/exec/interpreter/Interpreter.cpp \
			src/lib/io.cpp \
			src/lib/lang.cpp \
			src/parse/Tokenizer.cpp \
			src/runtime/casts.cpp \
			src/runtime/gc.cpp \
			src/runtime/id.cpp \
			src/utils/unicode/utf8.cpp \
			src/main.cpp
