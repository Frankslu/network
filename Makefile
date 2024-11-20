NAME = http-server

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj-$(NAME)

BINARY = $(BUILD_DIR)/$(NAME)
SRCS = $(shell find src -name "*.c")
OBJS = $(addprefix $(OBJ_DIR)/,$(SRCS:%.c=%.o))

LIBS = ssl crypto pthread
CFLAGS += -O2 -flto -Wall -Werror
LDFLAGS += $(CFLAGS) $(addprefix -l,$(LIBS))

CC = gcc
LD = gcc

SCRIPTS = $(shell find . -name "*.sh")

$(OBJ_DIR)/%.o: %.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $< -c -o $@ -ggdb3 $(CFLAGS) -MMD 

$(BINARY): $(OBJS)
	@echo + LD $@
	@mkdir -p $(dir $@)
	@$(LD) $^ -o $@ $(LDFLAGS)

all: $(BINARY)

run: all
	$(BINARY) 2>&1 | tee log

clean:
	@rm build -rf

$(SCRIPTS:%.sh=%):
	./$@.sh

.PHONY: clean all run $(SCRIPTS:%.sh=%)
.DEFAULT_GOAL := all

-include $(OBJS:.o=.d)
