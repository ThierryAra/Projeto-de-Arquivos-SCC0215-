objects = main.c file_t1.c file_t2.c write_read_file.c
headers = file_t1.h file_t2.h write_read_file.h
executable = run

all:
	gcc $(objects) -o $(executable) -g

run: all
	./$(executable)

valgrind: all
	valgrind --leak-check=full ./$(executable)

zip:
	zip thierry.zip $(objects) $(headers) $(bin) Makefile 
