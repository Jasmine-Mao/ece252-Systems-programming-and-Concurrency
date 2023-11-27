CC = gcc
CFLAGS_XML2 = $(shell xml2-config --cflags)
CFLAGS_CURL = $(shell curl-config --cflags)
CFLAGS = -Wall $(CFLAGS_XML2) $(CFLAGS_CURL) -g

LDLIBS_XML2 = $(shell xml2-config --libs)
LDLIBS_CURL = $(shell curl-config --libs)
LDLIBS = $(LDLIBS_XML2) $(LDLIBS_CURL)

SRC_DIR = png_fun

FINDPNG2_OBJS = $(SRC_DIR)/findpng3.o $(SRC_DIR)/frontier_stack.o $(SRC_DIR)/hashtable.o $(SRC_DIR)/is_png.o

all: findpng3

findpng3: $(FINDPNG2_OBJS)
	$(CC) $(CFLAGS) -o findpng3 $(FINDPNG2_OBJS) -lcurl -pthread -lxml2 -lz

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.out $(SRC_DIR)/*.o $(SRC_DIR)/*.out $(UTILS_DIR)/*.o $(UTILS_DIR)/*.out findpng3