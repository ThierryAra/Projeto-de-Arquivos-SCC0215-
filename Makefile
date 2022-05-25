objects = main.c record.c write_read_file.c header.c
headers = record.h write_read_file.h header.h
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
	git commit -m "Trabalho 1 pronto!"
	git push