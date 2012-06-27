vpath % .. ../tslib ../xfs ../linux
CFLAGS = -I ../ -g
CC = gcc
hello: hello.c read_super.o syscall.o rbtree.o read_file.o xfsd_dir2_sf.o xfsd_bmap_btree.o

dll: read_super.o syscall.o read_file.o xfsd_dir2_sf.o xfsd_bmap_btree.o
	$(CC) $(CFLAGS) -shared -fpic read_super.o syscall.o read_file.o xfsd_dir2_sf.o xfsd_bmap_btree.o -o xfsd.dll

clean:
	rm -f hello
	rm -f *.o
