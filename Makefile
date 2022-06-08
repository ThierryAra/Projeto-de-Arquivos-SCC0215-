objects = main.c record.c useful.c header.c index.c
headers = record.h useful.h header.h index.h
executable = run

all:
	gcc $(objects) -o $(executable) -g

run: all
	./$(executable)

valgrind: all
	valgrind --leak-check=full ./$(executable)

zip:
	zip thierry.zip $(objects) $(headers) $(bin) Makefile 

git: 
	git add .
	git commit -m "Mexendo no indice"
	git push