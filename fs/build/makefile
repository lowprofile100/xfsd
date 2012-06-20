vpath % .. ../tslib ../xfs ../linux
CFLAGS = -I ../
hello: hello.c read_super.o syscall.o rbtree.o read_file.o

clean:
	rm -f hello
	rm -f *.o
