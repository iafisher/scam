CC = gcc
OBJS = collector.o scamval.o scam.tab.o lex.yy.o 
CFLAGS = -Wall -std=gnu99 -I../include -c
LFLAGS = -lfl -I../include

main: $(OBJS)
	$(CC) $(OBJS) -o main $(LFLAGS)

collector.o: ../src/collector.c
	$(CC) ../src/collector.c $(CFLAGS)

scamval.o: ../src/scamval.c
	$(CC) ../src/scamval.c $(CFLAGS)

scam.tab.o: scam.tab.c
	$(CC) -c scam.tab.c -I../include

lex.yy.o: lex.yy.c
	$(CC) -c lex.yy.c -I../include

scam.tab.c: scam.y
	bison -d scam.y

lex.yy.c: scam.l
	flex scam.l

clean:
	rm *.o main
