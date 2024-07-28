#include <stdio.h>
#include "mystring.h"

int main() {

	string num = str_from_int(0x80000000);
	printf("%s", str_get_text(num));
	getchar();
	return 0;
}