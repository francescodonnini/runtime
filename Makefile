CC      ?= gcc
BUILD   := build
TARGET  := $(BUILD)/libruntime.so

SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:%.c=$(BUILD)/%.o)

CFLAGS  += -std=gnu17 -g -O2 -fPIC -MMD -MP \
           -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
           -Wstrict-prototypes -Wmissing-prototypes -Wvla \
           -Werror=implicit-function-declaration
CFLAGS  += -D_GNU_SOURCE
LDFLAGS += -shared
LDLIBS  += -ldl -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

compdb: clean
	bear -- $(MAKE)

format:
	clang-format -i $(SRCS) $(wildcard *.h)

format-check:
	clang-format --dry-run --Werror $(SRCS) $(wildcard *.h)

lint:
	clang-tidy -p . $(SRCS)
	cppcheck --enable=warning,performance,portability --quiet --suppress=missingIncludeSystem $(SRCS)

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)

.PHONY: all compdb format format-check lint clean
