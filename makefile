CFLAGS = -g

objects = main.o queue.o pqueue.o read_config.o

test : $(objects)
	cc -o test $(objects)

main.o : queue.h pqueue.h read_config.h

.PHONY : clean
clean :
	rm test $(objects)
