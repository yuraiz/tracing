# Usage:
# make        # compile all binary
# make run 	  # compile and run the bindary
# make clean  # remove ALL binaries and objects

# Recursive widlcard
# https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make/2483203#2483203
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

.PHONY = all clean run

# compiler to use
CC = clang
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wdouble-promotion	\
	-Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -Wno-flexible-array-extensions\
	-g -fsanitize=undefined -fsanitize-trap -fsanitize=address			

LINKFLAGS := -std=c11 -g -F /System/Library/PrivateFrameworks -lreadline -framework CoreSymbolication -fsanitize=address

PROGRAM_NAME := tracer

# All *.c files in src/ (including subdirectories)
SRC_FILES := $(call rwildcard, src, *.c)
OBJECT_FILES := $(SRC_FILES:src/%.c=target/%.o)

all: target/${PROGRAM_NAME}

target/%.o: src/%.c
	@mkdir -p ${dir $@}
	@echo "Compiling $<"
	@${CC} ${CFLAGS} -c $< -o $@

target/${PROGRAM_NAME}: ${OBJECT_FILES}
	${CC} ${LINKFLAGS} $^ -o $@

clean:
	rm -rf target/
