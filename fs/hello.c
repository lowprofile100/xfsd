#include <stdio.h>
#include "tslib/read_super.h"
#include "ntddk.h"

int main()
{
	char magic[100];
	init();

	get_magic( magic);
	printf("%s\n", magic);
	return 0;
}

NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING
RegistryPath)
{
	main();
return STATUS_UNSUCCESSFUL;
}