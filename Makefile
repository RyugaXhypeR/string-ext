D_MK = .build

EF_BIN = $(D_MK)/bin
CF_SRC = $(wildcard src/*.c)
CF_TEST = $(wildcard tests/*.c)

CC = clang
OPT = -O1
D = NDEBUG 
C_FLAGS = -Wall -Wextra -g $(OPT) -fPIE -D$(D) -Iinclude/

OF_SRC = $(CF_SRC:%.c=$(D_MK)/%.o)
AF_SRC = $(CF_SRC:%.c=$(D_MK)/%.a)
EF_TEST = $(CF_TEST:%.c=$(D_MK)/%.out)
DF_SRC = $(OF_SRC:%.o=%.d)

.PHONY: all clean

all: $(AF_SRC)

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
	$(CC) $(C_FLAGS) -o $@ $< $(AF_SRC)

-include $(DF_SRC)

clean:
	$(RM) -rf $(D_MK)
