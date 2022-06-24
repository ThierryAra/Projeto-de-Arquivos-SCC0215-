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
	cp -fR arquivos/antes/binario7.bin .
	cp -fR arquivos/antes/indice7.bin .
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all -s ./$(executable)

valgrind_lab: all
	cp -fR /mnt/c/Users/12681094/Documents/Projeto-de-Aquivos-SCC0215-/arquivos/antes/binario5.bin .
	cp -fR /mnt/c/Users/12681094/Documents/Projeto-de-Aquivos-SCC0215-/arquivos/antes/indice5.bin .
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all  ./$(executable)

zip:
	zip thierry.zip $(objects) $(headers) $(bin) Makefile 

git: 
	git add .
	git commit -m "Mexendo no indice"
	git push