
objects = src/main.c src/record.c src/useful.c src/header.c src/index.c src/list_stack.c src/index_B.c
headers = headers/record.h headers/useful.h headers/header.h headers/index.h headers/list_stack.h headers/index_B.h
executable = run
file = 8

all:
	gcc -g $(objects) -o $(executable) -g

run: all
	./$(executable)

valgrind: all
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

init:
	cp -fR arquivos/antes/binario$(file).bin .
	cp -fR arquivos/antes/indice$(file).bin .

valgrind_copy: all
	clear
	cp -fR arquivos/antes/binario$(file).bin .
	cp -fR arquivos/antes/indice$(file).bin .
	valgrind ./$(executable)