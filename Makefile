SRC_DIR := src
BUILD_DIR := bin
OBJS := $(SRC_DIR)/main.o $(SRC_DIR)/ascii_gen.o
LIBS := -lpng
CC := gcc
CFLAGS := -O2
LD := gcc
LDFLAGS := $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

asc: $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(LD) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

clean:
	rm $(OBJS)
