CC = gcc
CFLAGS = -Wall -g

SRC_DIR = png_fun

FINDPNG2_OBJS = $(SRC_DIR)/findpng2.o $(SRC_DIR)/frontier_stack.o  

all: findpng2

findpng2: $(FIND_OBJS)
	$(CC) $(CFLAGS) -o findpng2 $(FINDPNG2_OBJS) -lcurl -lz -pthread

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.out $(SRC_DIR)/*.o $(SRC_DIR)/*.out $(UTILS_DIR)/*.o $(UTILS_DIR)/*.out findpng2