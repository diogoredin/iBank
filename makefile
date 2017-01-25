CFLAGS = -g -Wall -pedantic
LDFLAGS= -pthread -lpthread

all: i-banco-terminal i-banco

i-banco-terminal: commandlinereader.o i-banco-terminal.o
	gcc $(CFLAGS) -o i-banco-terminal commandlinereader.o i-banco-terminal.o

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc $(CFLAGS) -c commandlinereader.c

i-banco-terminal.o: i-banco-terminal.c headers-terminal.h
	gcc $(CFLAGS) -c i-banco-terminal.c

i-banco: commandlinereader.o tarefas.o contas.o i-banco.o
	gcc $(CFLAGS) $(LDFLAGS) -o i-banco commandlinereader.o tarefas.o contas.o i-banco.o

contas.o: contas.c contas.h
	gcc $(CFLAGS) -c contas.c

tarefas.o: tarefas.c tarefas.h
	gcc $(CFLAGS) -c tarefas.c

i-banco.o: i-banco.c headers.h
	gcc $(CFLAGS) -c i-banco.c

clean:
	rm -f *.o i-banco
	rm -f *.txt i-banco
	[ -f "i-banco-pipe" ] || rm -f "i-banco-pipe"
	[ -f "/tmp/i-banco-pipe" ] || rm -f "/tmp/i-banco-pipe"
	rm -f "/tmp/ibanco_terminal_"*
	clear

zip:
	zip proj *