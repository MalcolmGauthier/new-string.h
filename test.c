#include <stdio.h>
#include "mystring.h"

int main() {

	uint32_t f = 0x7f800001;
	//printf("%s", str_get_text(str_from_flt(*(float*)(&f))));
	printf("%s", str_get_text(str_from_dbl(0.00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001)));
	int _ = getchar();
	str_delete_all();
	return 0;
}