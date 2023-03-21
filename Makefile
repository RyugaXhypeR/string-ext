D_MK = .build

EF_BIN = $(D_MK)/bin
CF_SRC = $(wildcard src/*.c)
CF_TARGET = $(wildcard *.c)
CF_TEST = $(wildcard tests/*.c)
CF_ALL = $(CF_SRC) $(CF_TARGET)

CC = gcc
OPT = -O3
C_FLAGS = -Wall -Wextra -g $(OPT) -fPIE

AF_TARGET = $(CF_TARGET:%.c=$(D_MK)/%.asm)
SF_TARGET = $(CF_TARGET:%.c=$(D_MK)/%.so)
OF_TARGET = $(CF_TARGET:%.c=$(D_MK)/%.o)
OF_SRC = $(CF_SRC:%.c=$(D_MK)/%.o)
OF_ALL = $(CF_ALL:%.c=$(D_MK)/%.o)
EF_TEST = $(CF_TEST:%.c=$(D_MK)/%.out)


all: $(EF_BIN)


$(EF_BIN): $(OF_ALL)
	$(CC) -o $@ $^

-include $(DF_ALL)

$(D_MK)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(C_FLAGS) -MMD -o $@ $<

$(EF_TEST): $(CF_TEST)

$(D_MK)/%.out: %.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -o $@ $< $(CF_SRC)

asm: $(CF_TARGET)
	@mkdir -p $(D_MK)
	$(CC) $(C_FLAGS) -S $< -o $(AF_TARGET)

run: $(EF_BIN)
	@$<

test: $(OF_SRC) $(EF_TEST)
	@for test in $(EF_TEST); do ./$$test; done

so: $(OF_ALL)
	$(CC) $(C_FLAGS) -o $(SF_TARGET) $(CF_TARGET)

clean:
	rm -rf $(D_MK)
