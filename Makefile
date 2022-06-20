objects = main.c record.c useful.c header.c index.c list_stack.c
headers = record.h useful.h header.h index.h list_stack.h
executable = run

all:
	gcc $(objects) -o $(executable) -g

run: all
	./$(executable)

valgrind: all
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

valgrind_copy: all
	cp -fR arquivos/antes/binario5.bin /
	cp -fR arquivos/antes/indice5.bin /
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./$(executable)

zip:
	zip thierry.zip $(objects) $(headers) $(bin) Makefile 

git: 
	git add .
	git commit -m "Mexendo no indice"
	git push