# The build directory which will store intermediate
# forms of the files in the same format as main.
D_MK = .build
D_INC = include
D_SRC = src
D_TEST = tests

EF_BIN = $(D_MK)/bin
CF_SRC = $(wildcard $(D_SRC)/*.c)
CF_TEST = $(wildcard $(D_TEST)/*.c)
HF_INC = $(wildcard $(D_INC)/*.h)
CF_SRC_NO_DIR = $(notdir $(CF_SRC))
HF_INC_NO_DIR = $(notdir $(HF_INC))

# Library installation locations
D_INC_INSTALL = /usr/local/include
D_LIB_INSTALL = /usr/local/lib

OPT = -O1
# A debug flag, when `D` is set to `DEBUG`, certain methods from the library
# will print debug information.
# For example: When string has to allocate extra memory,
# it will print information about the allocation.
#
# Example: `make D=DEBUG`
D = NDEBUG
C_FLAGS = -Wall -Wextra -g $(OPT) -fPIE -D$(D) -I$(D_SRC) -I$(D_INC)

OF_SRC = $(CF_SRC:%.c=$(D_MK)/%.o)
AF_SRC = $(CF_SRC:%.c=$(D_MK)/%.a)
EF_TEST = $(CF_TEST:%.c=$(D_MK)/%.out)
DF_SRC = $(OF_SRC:%.o=%.d)
HF_INST = $(HF_INC_NO_DIR:%.h=$(D_INC_INSTALL)/%.h)
AF_INST = $(CF_SRC_NO_DIR:%.c=$(D_LIB_INSTALL)/%.a)

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

install: $(AF_SRC)
	mkdir -p $(D_INC_INSTALL) $(D_LIB_INSTALL)
	cp $(D_INC)/* $(D_INC_INSTALL)
	cp $^ $(D_LIB_INSTALL)

uninstall:
	$(RM) $(HF_INST) $(AF_INST)

clean:
	$(RM) -rf $(D_MK)
