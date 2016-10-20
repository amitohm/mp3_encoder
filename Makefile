# Makefile for jpeg decoder using libjpegdec.a
# deepak.nayak@analog.com

.IGNORE:

.KEEP_STATE:

.SUFFIXES: .o .c

.c.o: 
	$(CC) $(CCFLAGS) -c $< -o $@

CC = gcc

CCFLAGS += -g -Wall

BIN = mp3enc

SRCS = $(wildcard *.c) 
OBJS = $(SRCS:.c=.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CCFLAGS) -o $(BIN) $(OBJS) -I .

$(LIB_JPEG): 
	@cd $(SUBDIR_JPEG) ; make
	
clean:
	rm -rf $(OBJS) $(BIN) $(BIN).gdb


