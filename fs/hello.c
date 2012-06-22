#include <stdio.h>
#include "tslib/read_super.h"
#include "tslib/read_file.h"

int main()
{
	char magic[100] = { 0};
	init();

	get_sb_magic( magic);
	printf("sb_magic \t\t\t%s\n", magic);
	printf("disk sb_magic int \t\t%u\n", get_dsb_magic_int());
	printf("sbs count \t\t\t%d\n", get_sbs_count());
	printf("disk sb size \t\t\t%d\n", get_dsb_size());
	printf("sb size \t\t\t%d\n", get_sb_size());
	printf("sb features2 \t\t\t0x%x\n", get_sb_features2());
	printf("sb sectsize \t\t\t%d\n", get_sb_sectsize());
	printf("sb inode free \t\t\t%d\n", get_sb_ifree());

	get_agf_magic( magic);
	printf("agf magic \t\t\t%s\n", magic);
	printf("agf flcount \t\t\t%d\n", get_agf_flcount());
	printf("agf version num \t\t%d\n", get_agf_versionnum());
	printf("agf free block 1 \t\t%d\n", get_agf_free_block( 0));
	printf("agf free block 2 \t\t%d\n", get_agf_free_block( 1));
	printf("agf free block 3 \t\t%d\n", get_agf_free_block( 2));
	printf("agf free block 4 \t\t%d\n", get_agf_free_block( 3));

	print("Begin to read disk\n");
	init_read_file_from_disk();
	char tmp[100];
	int ret = read_file_from_disk( "/xfsd_types.h", tmp, 100);
	printf("return %d\n", ret);
	ret = read_file_from_disk( "/xfsd/xfsd.h", tmp, 100);
	printf("return %d\n", ret);
	return 0;
}

#ifdef WIN32
#include "ntddk.h"
NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING
RegistryPath)
{
	main();
	return STATUS_UNSUCCESSFUL;
}
#endif
