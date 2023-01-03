LIBS = -pthread -ggdb3 -lssl -lcrypto
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean all

all: download_manager

download_manager: $(OBJS)
	gcc $(OBJS) $(LIBS) -o download_manager

download_manager.o: $(SRCS)
	gcc -c $(CFLAGS) $(SRCS) $(LIBS)

clean:
	@rm -rf $(OBJS) download_manager
