CC = gcc
CFLAGS = -Wall -g

SRC_DIR = png_fun
UTILS_DIR = $(SRC_DIR)/png_utils

FINDPNG_OBJS = $(SRC_DIR)/find_png.o $(UTILS_DIR)/png_info.o
CATPNG_OBJS = $(SRC_DIR)/cat_png.o $(UTILS_DIR)/png_info.o $(UTILS_DIR)/crc.o $(UTILS_DIR)/zutil.o

all: findpng catpng

findpng: $(FINDPNG_OBJS)
	$(CC) $(CFLAGS) -o findpng $(FINDPNG_OBJS)

catpng: $(CATPNG_OBJS)
	$(CC) $(CFLAGS) -o catpng $(CATPNG_OBJS) -lz

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.out $(SRC_DIR)/*.o $(SRC_DIR)/*.out $(UTILS_DIR)/*.o $(UTILS_DIR)/*.out findpng catpng