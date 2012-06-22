vpath % .. ../tslib ../xfs ../linux
CFLAGS = -I ../ -g
hello: hello.c read_super.o syscall.o rbtree.o read_file.o xfsd_dir2_sf.o

read_file.o: read_file.c read_file.h

xfsd_dir2_sf.o: xfs_dir2_priv.h xfsd_dir2_sf.c

clean:
	rm -f hello
	rm -f *.o
