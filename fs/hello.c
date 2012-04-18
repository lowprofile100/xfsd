#include <stdio.h>
#include "tslib/read_super.h"

int main()
{
	char magic[100];
	init();

	get_sb_magic( magic);
	printf("%s\n", magic);
	printf("%u\n", get_dsb_magic_int());
	printf("%d\n", get_sbs_count());
	printf("%d\n", get_dsb_size());
	printf("%d\n", get_sb_size());
	printf("0x%x\n", get_sb_features2());
	printf("%d\n", get_sb_sectsize());

	get_agf_magic( magic);
	printf("%s\n", magic);
	printf("%d\n", get_sb_ifree());
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
