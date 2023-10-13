CC = gcc
CFLAGS = -Wall -g

SRC_DIR = png_fun
#UTILS_DIR = $(SRC_DIR)/png_utils

PASTER_OBJS = $(SRC_DIR)/paster.o # $(UTILS_DIR)/png_info.o $(UTILS_DIR)/crc.o $(UTILS_DIR)/zutil.o

all: paster

paster: $(PASTER_OBJS)
	$(CC) $(CFLAGS) -o paster $(PASTER_OBJS) -lcurl -pthread

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.out $(SRC_DIR)/*.o $(SRC_DIR)/*.out paster