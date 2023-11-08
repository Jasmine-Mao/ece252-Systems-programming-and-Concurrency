CC = gcc
CFLAGS = -Wall -g

SRC_DIR = png_fun
UTILS_DIR = $(SRC_DIR)/png_utils

PASTER_OBJS = $(SRC_DIR)/paster2.o $(SRC_DIR)/ring_buffer.o $(UTILS_DIR)/cat_png.o $(UTILS_DIR)/crc.o $(UTILS_DIR)/zutil.o

all: paster2

paster2: $(PASTER_OBJS)
<<<<<<< HEAD
	$(CC) $(CFLAGS) -o paster2 $(PASTER_OBJS) -lcurl -lz -lpthread -lrt
=======
	$(CC) $(CFLAGS) -o paster2 $(PASTER_OBJS) -lcurl -lz -pthread
>>>>>>> b1d53b30372fc43d6ab87d21bc3b5fa45c9e8172

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.out $(SRC_DIR)/*.o $(SRC_DIR)/*.out $(UTILS_DIR)/*.o $(UTILS_DIR)/*.out paster2