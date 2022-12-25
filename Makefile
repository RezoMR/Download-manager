LIBS = -pthread -ggdb3
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean all

all: download_manager

download_manager: $(OBJS)
	gcc $(LIBS) $(OBJS) -o download_manager

download_manager.o: $(SRCS)
	gcc -c $(CFLAGS) $(LIBS) $(SRCS)

clean:
	@rm -rf $(OBJS) download_manager
