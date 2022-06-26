objects = src/main.c src/record.c src/useful.c src/header.c src/index.c src/list_stack.c
headers = headers/record.h headers/useful.h headers/header.h headers/index.h headers/list_stack.h
executable = run

all:
	gcc $(objects) -o $(executable) -g

run: all
	./$(executable)

valgrind: all
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

valgrind_copy: all
	cp -fR arquivos/antes/binario8.bin .
	cp -fR arquivos/antes/indice8.bin .
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

valgrind_lab: all
	cp -fR /mnt/c/Users/12681094/Documents/Projeto-de-Aquivos-SCC0215-/arquivos/antes/binario5.bin .
	cp -fR /mnt/c/Users/12681094/Documents/Projeto-de-Aquivos-SCC0215-/arquivos/antes/indice5.bin .
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all  ./$(executable)

zip:
	zip thierry.zip $(objects) $(headers) Makefile 

git: 
	git add .
	git commit -m "Mexendo na remocao"
	git push