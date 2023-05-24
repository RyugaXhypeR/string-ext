# The build directory which will store intermediate 
# forms of the files in the same format as main.
D_MK = .build

EF_BIN = $(D_MK)/bin
CF_SRC = $(wildcard src/*.c)
CF_TEST = $(wildcard tests/*.c)

CC = clang
OPT = -O1
# A debug flag, when `D` is set to `DEBUG`, certain methods from the library
# will print debug information.
# For example: When string has to allocate extra memory, 
# it will print information about the allocation.
#
# Example: `make D=DEBUG`
D = NDEBUG 
C_FLAGS = -Wall -Wextra -g $(OPT) -fPIE -D$(D) -Iinclude/

OF_SRC = $(CF_SRC:%.c=$(D_MK)/%.o)
AF_SRC = $(CF_SRC:%.c=$(D_MK)/%.a)
EF_TEST = $(CF_TEST:%.c=$(D_MK)/%.out)
DF_SRC = $(OF_SRC:%.o=%.d)

.PHONY: all clean

all: $(AF_SRC)

# Compile the library into a static library.
$(AF_SRC): $(OF_SRC)
	@mkdir -p $(@D)
	ar rcs $@ $^
	ranlib $@

$(D_MK)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(C_FLAGS) -MMD -o $@ $<

test: $(EF_TEST)
	@for test in $^; do \
		./$$test; \
	done

$(D_MK)/%.out: %.c $(AF_SRC)
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -o $@ $^

-include $(DF_SRC)

clean:
	$(RM) -rf $(D_MK)
