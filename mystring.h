#pragma once
#include <stdint.h>
#include <stdlib.h>

// String structure
struct _String {
	char* data;// DO NOT READ OR WRITE TO THIS DATA MANUALLY. USE THE FUNCTIONS.
	size_t length;// READ ONLY. DO NOT OVERWRITE.
	int flag_avoid_GC;// Okay to read/write, but ideally use str_set_avoid_GC and set_clear_avoid_GC.
};
typedef struct _String* string;

const uint32_t __float_nan_value = 0x7fc00000;
const uint32_t __float_inf_value = 0x7f800000;
const uint32_t __float_neg_inf_value = 0xff800000;
const uint64_t __double_nan_value = 0x7ff8000000000000;
const uint64_t __double_inf_value = 0x7ff0000000000000;
const uint64_t __double_neg_inf_value = 0xfff0000000000000;

#define EMPTY_STR (string){.data = calloc(1, sizeof(char)), .length = 0}
#define FLOAT_NAN *((float*)(&__float_nan_value))
#define FLOAT_INF *((float*)(&__float_inf_value))
#define FLOAT_NEG_INF *((float*)(&__float_neg_inf_value))
#define DOUBLE_NAN *((double*)(&__double_nan_value))
#define DOUBLE_INF *((double*)(&__double_inf_value))
#define DOUBLE_NEG_INF *((double*)(&__double_neg_inf_value))
#define SCI_NOT_LIM 10 // scientific notation character limit for when converting floating point numbers to strings

// String linked list used for garbage collection. Please do not use.
struct _String_LL_GC {
	struct _String_LL_GC* prev;
	struct _String* str;
	struct _String_LL_GC* next;
};

struct _String_LL_GC _String_GC_LL = { .prev = NULL, .str = NULL, .next = NULL };

// RESERVED. DO NOT USE
void __str_add_LLGC(struct _String* str) {

	if (!str) return;
	struct _String_LL_GC* curr = &_String_GC_LL;
	while (curr->next) curr = curr->next;

	if (!(curr->next = malloc(sizeof(struct _String_LL_GC)))) return;
	curr->next->prev = curr;
	curr->next->str = str;
	curr->next->next = NULL;
}

// RESERVED. DO NOT USE
void __str_remove_LLGC(struct _String* str) {

	if (!str) return;
	struct _String_LL_GC* curr = &_String_GC_LL;
	while (curr->next) {

		curr = curr->next;
		if (!curr->str) continue;
		if (curr->str == str && !curr->str->flag_avoid_GC) {

			curr->next->prev = curr->prev;
			curr->prev->next = curr->next;
			free(curr);
			break;
		}
	}
}

#pragma region MEMORY ANALYSIS FUNCTIONS
// -------------------------
// MEMORY ANALYSIS FUNCTIONS
// -------------------------

// copies lim bytes of memory from src to dst. returns dst.
void* mem_cpy(void* dst, void* src, size_t lim) {

	while (lim--) ((char*)dst)[lim] = ((char*)src)[lim];
	return dst;
}

// copies memory chunk_size bytes at a time, and after each chunk, passes that chunk's pointer to a function.
// if the custom functon returns a non-zero value, copying stops at that chunk.
// acts the same as mem_cpy if function never returns non-zero.
void* mem_cpy_cond(void* dst, void* src, size_t lim, int (*func)(void*), size_t chunk_size) {

	size_t i = 0;
	char* chunk_loc = dst;
	while (i < lim) {

		((char*)dst)[i] = ((char*)src)[i];
		if (i % chunk_size == 0) {
			chunk_loc += chunk_size;
			if ((*func)(chunk_loc)) break;
		}
	}

	return dst;
}

// compares two lim byte long areas of memory. returns 1 if both areas are equal, 0 if not.
int mem_comp(void* area1, void* area2, size_t lim) {

	while (--lim) {
		if (((char*)area1)[lim] != ((char*)area2)[lim]) return 0;
	}

	return 1;
}

// looks through lim bytes starting at src to try and find memory matching the first key_size bytes at key.
// returns the first found memory adress, null if none are found.
void* mem_find(void* src, size_t lim, void* key, size_t key_size) {

	for (size_t i = 0; i < lim; i++) {
		if (mem_comp(&(((char*)src)[i]), ((char*)key), key_size)) return &(((char*)src)[i]);
	}

	return NULL;
}

// fills lim bytes of memory starting at dst with value of parameter byte.
void mem_set(void* dst, unsigned char byte, size_t lim) {
	
	if (!dst) return;

	for (size_t i = 0; i < lim; i++) {
		((char*)dst)[i] = byte;
	}
}
#pragma endregion

#pragma region STRING MEMORY MANAGEMENT
// ------------------------
// STRING MEMORY MANAGEMENT
// ------------------------

// creates a new string struct from a char array/pointer.
// this will crash if pointer leads to non-null junk data or a string without a terminator.
// this copies the contents of text into a new string structure which must freed it with str_delete after use.
// if return value is not stored, string must be freed with str_delete_all.
string str_new(char* text) {

	int32_t length = 0;
	int32_t i = -1;
	string newstr = NULL;

	if (!text) return NULL;

	if (!(newstr = malloc(sizeof(struct _String)))) goto error;

	while (text[length++]); // cannot prevent segfault if memory is BS lmao
	length--;

	if (!(newstr->data = malloc(length + 1))) goto error2;
	newstr->length = length;

	mem_cpy(newstr->data, text, length + 1); // possible improvement: combine with other while loop

	__str_add_LLGC(newstr);
	newstr->flag_avoid_GC = 0;
	return newstr;

error2:
	free(newstr);
error:
	return NULL;
}

// copies the text on a pre-existing string struct.
void str_set_text(string str, char* text) {
	
	int32_t newlen = 0;

	if (!text) return;

	while (text[newlen]) newlen++;

	char* data = realloc(str->data, newlen + 1);
	if (!data) return;
	str->data = data;

	mem_cpy(str->data, text, newlen + 1);
	str->length = newlen;
}

// returns pointer to the text found in the string in a safe way.
// THE DATA POINTED TO SHOULD ONLY BE READ. To get a char array copy of the string, use str_to_arr.
// returns NULL if error.
char* str_get_text(string str) {

	if (str->length < 0) return NULL;
	if (!str->data) return calloc(1, sizeof(char));

	return str->data;
}

// free the memory used by the string structure, including the struct itself.
void str_delete(string str) {
	
	__str_remove_LLGC(str);
	free(str->data);
	free(str);
}

// copies the contents of a string to a new char buffer.
// this returns allocated memory, please free after use.
char* str_to_arr(string str) {

	if (str->length < 0) return NULL;

	char* ret = NULL;
	if (!(ret = malloc(str->length + 1))) return NULL;
	
	mem_cpy(ret, str->data, str->length + 1);
	return ret;
}

// copies the contents of the string to a specified char buffer
void str_to_buf(string str, char* buf) {

	if (!buf) return;

	mem_cpy(buf, str->data, str->length); //segfault here if buf is garbage data
}

// marks a flag to prevent this string from being cleared by str_delete_all. this means that this string MUST be freed with str_delete.
void str_set_avoid_GC(string str) {
	str->flag_avoid_GC = 1;
}

// clears flag so that string is freed when str_delete_all is called.
void str_clear_avoid_GC(string str) {
	str->flag_avoid_GC = 0;
}

// deletes all strings left that haven't been freed. This deletes all strings except those with the avoid_GC flag set, so be careful.
void str_delete_all(void) {

	struct _String_LL_GC* curr = &_String_GC_LL;
	while (curr->next) {

		curr = curr->next;
		if (!curr->str) continue;
		if (!curr->str->flag_avoid_GC) {

			free(curr->str->data);
			free(curr->str);
			curr->next->prev = curr->prev;
			curr->prev->next = curr->next;
			void* to_del = curr;
			curr = curr->prev;
			free(to_del);
			continue;
		}
	}
}
#pragma endregion

#pragma region STRING TO NUMBER CONVERSION
// ---------------------------
// STRING TO NUMBER CONVERSION
// ---------------------------

// converts a string to a signed 32bit integer. returns 0 on error.
int32_t str_to_int(string str) {

	//max: -2147483648
	int32_t i = 0;
	int32_t ret = 0;
	int flip = 0;
	const int32_t LIM_ADD = INT32_MAX - (INT32_MAX % 10);
	const int32_t LIM_MUL = LIM_ADD / 10;
	const LIM_LEN = 11;
	const LIM_DIGIT = 7;

	if (!str) return 0;

	if (!str->data) return 0;

	if (str->length > LIM_LEN || str->length <= 0) return 0;

	if (*(str->data) == '-' && str->length > 1) {

		flip = 1;
		i++;
	}

	while (i < str->length) {

		if (str->data[i] < '0' || str->data[i] > '9') return 0;

		if (ret > LIM_MUL) return 0;
		ret *= 10;

		if (ret == LIM_ADD && str->data[i] - '0' > LIM_DIGIT) {
			if (*(str->data) != '-' || str->data[i] - '0' != LIM_DIGIT + 1) return 0; //-2147483648 is allowed, but not +2147483648
			flip = 0;
			ret *= -1;
		}
		ret += str->data[i] - '0';

		i++;
	}

	if (flip) ret *= -1;
	return ret;
}
// converts a string to an unsigned 32bit integer. returns 0 on error.
uint32_t str_to_uint(string str) {

	//max: 4294967295
	int32_t i = 0;
	uint32_t ret = 0;
	const uint32_t LIM_ADD = UINT32_MAX - (UINT32_MAX % 10u);
	const uint32_t LIM_MUL = LIM_ADD / 10;
	const LIM_LEN = 10;
	const LIM_DIGIT = 5;

	if (!str) return 0;

	if (!str->data) return 0;

	if (str->length > LIM_LEN || str->length <= 0) return 0;

	while (i < str->length) {

		if (str->data[i] < '0' || str->data[i] > '9') return 0;

		if (ret > LIM_MUL) return 0;
		ret *= 10;

		if (ret == LIM_ADD && str->data[i] - '0' > LIM_DIGIT) return 0;
		ret += str->data[i] - '0';

		i++;
	}

	return ret;
}
// converts a string to a signed 64bit integer. returns 0 on error.
int64_t str_to_lng(string str) {

	//max: -9223372036854775808
	int32_t i = 0;
	int64_t ret = 0;
	int flip = 0;
	const int64_t LIM_ADD = INT64_MAX - (INT64_MAX % 10);
	const int64_t LIM_MUL = LIM_ADD / 10;
	const LIM_LEN = 20;
	const LIM_DIGIT = 7;

	if (!str) return 0;

	if (!str->data) return 0;

	if (str->length > LIM_LEN || str->length <= 0) return 0;

	if (*(str->data) == '-' && str->length > 1) {

		flip = 1;
		i++;
	}

	while (i < str->length) {

		if (str->data[i] < '0' || str->data[i] > '9') return 0;

		if (ret > LIM_MUL) return 0;
		ret *= 10;

		if (ret == LIM_ADD && str->data[i] - '0' > LIM_DIGIT) {
			if (*(str->data) != '-' || str->data[i] - '0' != LIM_DIGIT + 1) return 0; //-9223372036854775808 is allowed, but not +9223372036854775808
			flip = 0;
			ret *= -1;
		}
		ret += str->data[i] - '0';

		i++;
	}

	if (flip) ret *= -1;
	return ret;
}
// converts a string to an unsigned 64bit integer. returns 0 on error.
uint64_t str_to_ulng(string str) {

	//max: 18446744073709551615
	int32_t i = 0;
	uint64_t ret = 0;
	const uint64_t LIM_ADD = UINT64_MAX - (UINT64_MAX % 10u);
	const uint64_t LIM_MUL = LIM_ADD / 10;
	const LIM_LEN = 20;
	const LIM_DIGIT = 5;

	if (!str) return 0;

	if (!str->data) return 0;

	if (str->length > LIM_LEN || str->length <= 0) return 0;

	while (i < str->length) {

		if (str->data[i] < '0' || str->data[i] > '9') return 0;

		if (ret > LIM_MUL) return 0;
		ret *= 10;

		if (ret == LIM_ADD && str->data[i] - '0' > LIM_DIGIT) return 0;
		ret += str->data[i] - '0';

		i++;
	}

	return ret;
}
// converts a string to a single precision floating point number.
// rounds to nearest valid value if it isn't one, returns NAN on error.
float str_to_flt(string str) {

	int32_t i = 0;
	float ret = 0;
	int has_fraction = 0;
	float power = 1;

	if (str->length <= 0 || !str->data) return FLOAT_NAN;

	while (i < str->length) {

		if (str->data[i] < '0' && str->data[i] > '9') {

			if ((str->data[i] != '.' && str->data[i] != ',') || has_fraction) return FLOAT_NAN;

			has_fraction = 1;
			power = 0;
			continue;
		}

		if (!has_fraction) {

			ret *= 10;
			ret += str->data[i] - '0';
		}
		else {

			power /= 10;

			if (power == 0) break;

			ret += (str->data[i] - '0') * power;
		}

		i++;
	}

	return ret;
}
// converts a string to a double precision floating point number.
// rounds to nearest valid value if it isn't one, returns NAN on error.
double str_to_dbl(string str) {

	int32_t i = 0;
	double ret = 0;
	int has_fraction = 0;
	double power = 1;

	if (str->length <= 0 || !str->data) return FLOAT_NAN;

	if (*str->data == '-') i++;

	while (i < str->length) {

		if (str->data[i] < '0' || str->data[i] > '9') {

			if ((str->data[i] != '.' && str->data[i] != ',') || has_fraction) return DOUBLE_NAN;

			has_fraction = 1;
			i++;
			continue;
		}

		if (!has_fraction) {

			ret *= 10;
			ret += str->data[i] - '0';
		}
		else {

			power /= 10;

			if (power == 0) break;

			ret += (str->data[i] - '0') * power;
		}

		i++;
	}

	if (*str->data == '-') {
		ret *= -1;
	}

	return ret;
}
#pragma endregion

#pragma region NUMBER TO STRING CONVERSION
// ---------------------------
// NUMBER TO STRING CONVERSION
// ---------------------------

// creates and returns a new string from a given 32bit integer.
// returns null on error.
string str_from_int(int32_t num) {

	int len = 0;
	int i = 0;
	int abs_min_bugfix = 0;
	int32_t len_temp = num;
	string ret = str_new("-2147483648");
	if (ret->length == 0) return NULL;

	if (num < 0) {
		i++;
		ret->data[0] = '-';
		if (num == 0x80000000) {//if num = 0x80000000, *= -1 is an overflow. (only for 32 bit systems)
			num++;
			abs_min_bugfix = 1;
		}
		num *= -1;
	}

	while (len_temp) {
		len_temp /= 10;
		len++;
	}

	ret->length = i + len;

	while (len--) {
		ret->data[i + len] = (num % 10) + '0';
		num /= 10;
	}

	ret->data[ret->length] = '\0';

	if (abs_min_bugfix) ret->data[ret->length - 1]++;

	return ret;
}
// creates and returns a new string from a given unsinged 32bit integer.
// returns null on error.
string str_from_uint(uint32_t num) {

	int len = 0;
	uint32_t len_temp = num;
	string ret = str_new("4147483648");
	if (ret->length == 0) return NULL;

	while (len_temp) {
		len_temp /= 10;
		len++;
	}

	ret->length = len;

	while (len--) {
		ret->data[len] = (num % 10) + '0';
		num /= 10;
	}

	ret->data[ret->length] = '\0';

	return ret;
}
// creates and returns a new string from a given 64bit integer.
// returns null on error.
string str_from_lng(int64_t num) {

	int len = 0;
	int i = 0;
	int abs_min_bugfix = 0;
	int64_t len_temp = num;
	string ret = str_new("-9223372036854775808");
	if (ret->length == 0) return NULL;

	if (num < 0) {
		i++;
		ret->data[0] = '-';
		if (num == 0x8000000000000000) {//if num = 0x8000000000000000, *= -1 is an overflow.
			num++;
			abs_min_bugfix = 1;
		}
		num *= -1;
	}

	while (len_temp) {
		len_temp /= 10;
		len++;
	}

	ret->length = i + len;

	while (len--) {
		ret->data[i + len] = (num % 10) + '0';
		num /= 10;
	}

	ret->data[ret->length] = '\0';

	if (abs_min_bugfix) ret->data[ret->length - 1]++;

	return ret;
}
// creates and returns a new string from a given unsigned 64bit integer.
// returns null on error.
string str_from_ulng(uint64_t num) {

	int len = 0;
	uint64_t len_temp = num;
	string ret = str_new("18223372036854775807");
	if (ret->length == 0) return NULL;


	while (len_temp) {
		len_temp /= 10;
		len++;
	}

	ret->length = len;

	while (len--) {
		ret->data[len] = (num % 10) + '0';
		num /= 10;
	}

	ret->data[ret->length] = '\0';

	return ret;
}
// creates and returns a new string from a given unsigned 64bit integer.
// set sci_not to 1 to use scientific notation on strings longer than
// returns null on error.
string str_from_flt(float num, int sci_not) {

	union {
		float f;
		uint32_t i;
	} number;
	number.f = num;
	string ret;

	// exceptions to the float calculation formula
	if (number.i & 0x7fffffff > __float_inf_value) return str_new("NaN");
	if (number.i == __float_inf_value) return str_new("Infinity");
	if (number.i == __float_neg_inf_value) return str_new("-Infinity");
	if (number.i == 0) return str_new("0.0");
	if (number.i == 0x80000000) return str_new("-0.0");

	//??
}
// creates and returns a new string from a given unsigned 64bit integer.
// returns null on error.
string str_from_dbl(double num) {


}
#pragma endregion

#pragma region CHARACTER FUNCTIONS
// -------------------
// CHARACTER FUNCTIONS
// -------------------

// returns 1 if given character is alphanumeric, 0 if not.
int chr_is_alphanum(char c) {

	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
// returns 1 if given character is alphabetic, 0 if not.
int chr_is_alpha(char c) {

	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
// returns 1 if given character is numeric, 0 if not.
int chr_is_num(char c) {

	return c >= '0' && c <= '9';
}
// returns 1 if given character is a lower case letter, 0 if not.
int chr_is_lower(char c) {

	return c >= 'a' && c <= 'z';
}
// returns 1 if given character is an upper case letter, 0 if not.
int chr_is_upper(char c) {

	return c >= 'A' && c <= 'Z';
}
// returns 1 if given character is a hexadecimal number, 0 if not.
int chr_is_hex(char c) {

	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
// returns 1 if given character is visible, 0 if not.
int chr_is_visible(char c) {

	return c >= '!' && c <= '~';
}
// returns 1 if given character is visible or a space, 0 if not.
int chr_is_printable(char c) {

	return c >= ' ' && c <= '~';
}
// returns 1 if given character is a control character, 0 if not.
int chr_is_control(char c) {

	return c <= '\x1F' || c == '\x7F';
}

// converts a given character to an uppercase letter.
// returns the inputed character if not a lowercase letter.
char chr_to_upper(char c) {

	if (!chr_is_lower(c)) return c;
	return (char)(c + ('A' - 'a'));
}

// converts a given character to an lowercase letter.
// returns the inputed character if not an uppercase letter.
char chr_to_lower(char c) {

	if (!chr_is_upper(c)) return c;
	return (char)(c + ('a' - 'A'));
}

// creates a new string that consists of factor times char c.
// returns the new string, empty string on error.
string chr_mul(char c, int32_t factor) {

	string ret;
	char* str;

	if (factor <= 0) {
		ret = str_new("");
		return ret;
	}

	str = malloc(sizeof(char) * (factor + 1));

	for (int i = 0; i < factor; i++) str[i] = c;
	str[factor] = '\0';

	str_set_text(ret, str);
	free(str);

	return ret;
}
#pragma endregion

#pragma region STRING MANIPULATION
// -------------------
// STRING MANIPULATION
// ------------------- 

// crops the given string down to a new string comprising of the characters from start to end, which are indexes into the string.
// returns modified string, returns the original string on error.
string str_crop(string str, size_t start, size_t end) {
	
	if (!str) return NULL;

	if (start > end || end > str->length) return str;

	mem_cpy(str->data, str->data[start], end - start);
	str->length = end - start;
	str->data[str->length] = '\0';

	return str;
}

// appends str_src onto the end of str_dst.
// returns str_dst upon success, null upon failure.
string str_append(string str_dst, string str_src) {

	if (!str_dst) str_dst = str_new("");
	if (!str_src) return str_dst;

	if (str_src->length == 0) return str_dst;

	if (str_dst->length == 0) {
		str_set_text(str_dst, str_src->data);
		return str_dst;
	}

	char* data = realloc(str_dst->data, str_dst->length + str_src->length + 1);
	if (!data) return NULL;

	str_dst->data = data;
	mem_cpy(str_dst->data + str_dst->length, str_src->data, str_src->length + 1);

	str_dst->length += str_src->length;
	return str_dst;
}

// appends a given section of str_src onto the end of str_dst.
// returns str_dst upon success, null upon failure.
string str_append_sec(string str_dst, string str_src, size_t start, size_t end) {

	if (!str_dst) str_dst = str_new("");
	if (!str_src) return str_dst;

	string cpy = str_copy(str_src);
	str_crop(cpy, start, end);
	str_append(str_dst, cpy);
	str_delete(cpy);
	return str_dst;
}

// returns a new string struct with the same values as the given string struct.
// returns null on error.
string str_copy(string str) {

	if (!str) return NULL;

	string newstr = str_new(str->data);
	return newstr;
}

// returns a new string struct with the contents matching the given portion within and including indexes start and end.
// returns null on error.
string str_copy_sec(string str, size_t start, size_t end) {

	if (!str) return NULL;

	string ret = str_new(str->data);
	str_crop(ret, start, end);
	return ret;
}

// returns 1 if both given strings contain the same data, 0 if not.
int str_comp(string str1, string str2) {

	if (!str1 || !str2) return str1 == str2;

	if (str1->length != str2->length) return 0;

	mem_comp(str1->data, str2->data, str1->length);

	return 1;
}

// trims invisible characters located before the first visible character and after the last visible character.
// this of course does not trim the null terminator.
// returns str on success, the original string on failure.
string str_trim(string str) {

	int32_t start = 0;
	int32_t end = str->length - 1;

	if (!str) return NULL;

	if (str->length <= 0) return str;

	while (!chr_is_visible(str->data[start])) start++;
	if (start == str->length) return str;

	while (!chr_is_visible(str->data[end])) end--;

	return str_crop(str, start, end);
}

// converts all lowercase characters in a string to uppercase.
string str_to_upper(string str) {

	if (!str) return NULL;
	for (int32_t i = 0; i < str->length; i++) chr_to_upper(str->data[i]);
	return str;
}

// converts all uppercase characters in a string to lowercase.
string str_to_lower(string str) {

	if (!str) return NULL;
	for (int32_t i = 0; i < str->length; i++) chr_to_lower(str->data[i]);
	return str;
}

// replaces str with the contents of str repeated factor times.
// returns str, null on error.
string str_mul(string str, int32_t factor) {

	if (!str) return NULL;

	if (factor <= 0) {
		str_set_text(str, ""); return;
	}

	for (int32_t i = 0; i < factor - 1; i++) {
		str_append(str, str);
	}

	return str;
}
#pragma endregion

#pragma region STRING SEARCH
// -------------
// STRING SEARCH
// -------------

// find the first occurence of a character in a string.
// returns the index of the character in the string, and the length of the string if the character was not found.
size_t str_find_chr(string str, char chr) {

	if (!str) return 0;

	for (size_t i = 0; i < str->length; i++) {

		if (str->data[i] == chr) return i;
	}
	
	return str->length;
}

// find the last occurence of a character in a string.
// returns the index of the character in the string, and the length of the string if the character was not found.
size_t str_find_last_chr(string str, char chr) {

	if (!str) return 0;

	for (size_t i = str->length - 1; i >= 0; i--) {

		if (str->data[i] == chr) return i;
	}

	return str->length;
}

// finds the first longest sequence of characters in str that is only comprised of characters found in chrs_to_find.
// setting the not parameter to a non-zero value does the opposite, and looks for the longest sequence that doesn't contain the given characters.
// returns the index of the beginning of the sequence found, and the length of the string if none were found.
size_t str_find_chrs(string str, string chrs_to_find, int not) {

	size_t count = 0;
	size_t record = 0;
	size_t record_loc = 0;
	int chr_found = 0;

	if (!str) return 0;
	if (!chrs_to_find) return str->length;

	for (size_t i = 0; i < str->length; i++) {

		for (size_t j = 0; j < chrs_to_find->length; j++) {

			if (not) {
				if (str->data[i] == chrs_to_find->data[j]) continue;
			}
			else {
				if (str->data[i] != chrs_to_find->data[j]) continue;
			}
			
			count++;
			chr_found = 1;

			break;
		}

		if (!chr_found) {

			if (count > record) {

				record = count;
				record_loc = i - record;
			}

			count = 0;
		}

		chr_found = 0;
	}

	if (record > 0) return record;

	return str->length;
}

// finds the first character in str that is also contained within chrs_to_find.
// returns the index of the character found, and the length of the string if none were found.
size_t str_find_str_chr(string str, string chrs_to_find, int not) {

	if (!str) return 0;
	if (!chrs_to_find) return str->length;

	for (size_t i = 0; i < str->length; i++) {

		for (size_t j = 0; j < chrs_to_find->length; j++) {

			if (not) {
				if (str->data[i] == chrs_to_find->data[j]) continue;
			}
			else {
				if (str->data[i] != chrs_to_find->data[j]) continue;
			}

			return i;
		}
	}
	
	return str->length;
}

// finds the first occurence of sub_str within str.
// returns the index of the start of the substring, the length of str if not found.
size_t str_find_str(string str, string sub_str) {

	size_t count = 0;
	int str_found = 0;

	if (!str) return 0;
	if (!sub_str) return str->length;

	for (size_t i = 0; i < str->length; i++) {

		if (str->data[i] == sub_str->data[count]) count++;
		else count = 0;

		if (count == sub_str->length) return i - sub_str->length;
	}

	return str->length;
}

#pragma endregion