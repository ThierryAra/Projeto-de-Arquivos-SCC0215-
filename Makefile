
objects = src/main.c src/record.c src/useful.c src/header.c src/index.c src/list_stack.c src/index_B.c
headers = headers/record.h headers/useful.h headers/header.h headers/index.h headers/list_stack.h headers/index_B.h
executable = run
file = 17

all:
	gcc $(objects) -o $(executable) -g

run: all
	./$(executable)

valgrind: all
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

valgrind_copy: all
	cp -fR arquivos/antes/binario$(file).bin .
	cp -fR arquivos/antes/indice$(file).bin .
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

zip:
	zip trabalho.zip src headers Makefile 

git: 
	git add .
	git commit -m "Mexendo na remocao"
	git push

cmp_bin:
	xxd binario$(file).bin > b1.my
	xxd arquivos/depois/binario$(file).bin > b2.out
	code --diff b1.my b2.out

cmp_ind:
	xxd indice$(file).bin > i1.my
	xxd arquivos/depois/indice$(file).bin > i2.out
	code --diff i1.my i2.out

rm:
	rm binario$(file).bin
	rm indice$(file).bin