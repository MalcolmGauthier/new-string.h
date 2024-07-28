#include <stdio.h>
#include "mystring.h"

int main() {

	string num = str_from_int(0x80000000);
	string test = str_new("jwhdiuewhduiwhdi3yd73y7ydwyhdh23uyhduwhudhu7qdui");
	test->flag_avoid_GC = 1;
	printf("%s", str_get_text(num));

	str_delete_all();
	return 0;
}