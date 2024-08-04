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
PROGRAM_NAME := tracer

# All *.c files in src/ (including subdirectories)
SRC_FILES := $(call rwildcard, src, *.c)
OBJECT_FILES := $(SRC_FILES:src/%.c=target/%.o)

all: target/${PROGRAM_NAME}

target/%.o: src/%.c
	@mkdir -p ${dir $@}
	${CC} -c $< -o $@

target/${PROGRAM_NAME}: ${OBJECT_FILES}
	${CC} $^ -o $@

clean:
	rm -rf target/
