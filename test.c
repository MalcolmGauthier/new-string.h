#include <stdio.h>
#include "mystring.h"

int main() {

	string str = str_new("yo");
	string str2 = str_copy(str);
	str_set_text(str2, " wassup!");
	str_append(str, str2);
	str_append_chr(str, '!');
	printf("%i\n", (int)str->length);
	str_append_sec(str, str, 0, 1);
	printf("%i\n", str_comp(str, str2));
	str2 = str_copy_sec(str, 0, 1);
	str_crop(str, 2, 6);
	str_find_chr(str, 'w');

	printf("%s\n", str_to_arr(str));

	while (1)
	{
		string q = str_new("1");
		string w = str_new("1");
		string e = str_new("1");
		string r = str_new("1");
		string t = str_new("1");
		string y = str_new("1");
		string u = str_new("1");
		str_delete(w);
		str_delete(u);
		str_delete(y);
		str_delete(r);
		str_delete_all();
	}

	int _ = getchar();
	str_delete(str);
	str_delete_all();
	return 0;
}