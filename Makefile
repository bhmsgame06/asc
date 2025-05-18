SRC_DIR := src
BUILD_DIR := bin
OBJS := $(SRC_DIR)/main.o \
		$(SRC_DIR)/ascii_gen.o
LIBS := -lpng
CC := gcc
CFLAGS := -O2
LD := gcc
LDFLAGS := $(LIBS)

all: asc

asc: $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(LD) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

install:
	install $(BUILD_DIR)/asc /usr/bin

clean:
	rm -f $(OBJS)
