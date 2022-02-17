CC = g++
CFLAGS = -Wall -O
CLIBS = -lstdc++
INC = -I src/

all: server client

server: server.o tands.o common.o
	${CC} ${CFLAGS} -o server server.o tands.o common.o ${CLIBS}

server.o: server.h server.cpp
	${CC} ${CFLAGS} ${INC} -c server.h server.cpp

client: client.o tands.o common.o
	${CC} ${CFLAGS} -o client client.o tands.o common.o ${CLIBS}

client.o: client.h client.cpp
	${CC} ${CFLAGS} ${INC} -c client.h client.cpp

tands.o: tands.c tands.h
	${CC} ${CFLAGS} ${INC} -c tands.h tands.c

common.o: common.cpp common.h
	${CC} ${CFLAGS} ${INC} -c common.h common.cpp

man: man-page/client.man man-page/server.man
	groff -Tpdf -man man-page/client.man > man-page/client.pdf
	groff -Tpdf -man man-page/server.man > man-page/server.pdf

clean:
	rm *.o
	rm *.gch
	rm server
	rm client
	rm -rf man-page/client.pdf
	rm -rf man-page/server.pdf