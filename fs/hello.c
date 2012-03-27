#include <stdio.h>
#include "read_super.h"

int main()
{
	init();
	char magic[100];
	get_magic( magic);
	printf("%s\n", magic);
	return 0;
}
