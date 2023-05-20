D_MK = .build

EF_BIN = $(D_MK)/bin
CF_SRC = $(wildcard src/*.c)
CF_TARGET = $(wildcard *.c)
CF_TEST = $(wildcard tests/*.c)

CC = clang
OPT = -O1
D = NDEBUG 
C_FLAGS = -Wall -Wextra -g $(OPT) -fPIE -D$(D) -Iinclude/

OF_TARGET = $(CF_TARGET:%.c=$(D_MK)/%.o)
OF_SRC = $(CF_SRC:%.c=$(D_MK)/%.o)
AF_SRC = $(CF_SRC:%.c=$(D_MK)/%.a)
EF_TEST = $(CF_TEST:%.c=$(D_MK)/%.out)


all: $(AF_SRC)

$(AF_SRC): $(OF_SRC)
	@mkdir -p $(@D)
	ar rcs $@ $^
	ranlib $@

$(EF_BIN): $(AF_SRC)
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -o $@ $(CF_TARGET) $^ 

$(D_MK)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(C_FLAGS) -MMD -o $@ $<

run: $(EF_BIN)
	@$< 

test: $(OF_SRC) $(EF_TEST)
	@for test in $(EF_TEST); do \
		./$$test; \
	done

$(EF_TEST): $(AF_SRC)
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -o $@ $(@:$(D_MK)/%.out=%.c) $^

clean:
	rm -rf $(D_MK)
