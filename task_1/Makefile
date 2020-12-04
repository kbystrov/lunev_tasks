CFLAGS= -Wall -Wextra
DFLAGS= -g3
#DFLAGS+= -D DEBUG_PRINT_INFO

CFLAGS	+=	$(DFLAGS)

SOURCES=fifo_reader.c fifo_writer.c
OBJECTS=$(SOURCES:.c=.out)

all: $(OBJECTS)

$(OBJECTS): %.out: %.c
	$(CC) $(CFLAGS) -o $@ $< $(ADD_SOURCES)

clean: 
	rm $(OBJECTS)