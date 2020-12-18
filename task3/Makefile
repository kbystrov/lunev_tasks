CFLAGS= -Wall -Wextra
DFLAGS= -g3
#DFLAGS+= -D DEBUG_PRINT_INFO

CFLAGS	+=	$(DFLAGS)

SOURCES=reader.c writer.c
OBJECTS=$(SOURCES:.c=.out)
ADD_SOURCES=common.c

all: $(OBJECTS)

$(OBJECTS): %.out: %.c
	$(CC) $(CFLAGS) -o $@ $< $(ADD_SOURCES) 

clean: 
	rm $(OBJECTS)