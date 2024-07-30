# MyString

Personal project i did over the course of 3 days where i made my own version of the string.h standard library function set.
Sloppy code and little documentation because I don't think anyone other than me should ever use this.

Everything is located in a singular uncompiled .h file so that it's easy to copy and paste into new projects.
The string struct contains a regular C byte-string that should only be read or set using the provided functions and a length variable that is readily available for all strings that once again should never be overwritten.
The strings are dynamically located and need to be freed manually, however if a string is created without being assigned a name, it can still be freed once the function to delete all currently allocated strings is called.
However, if you want to keep some strings even when the memory is fully freed up, each string has a member variable that when set, makes the delete all function skip over that string, and keeps it allocated.

I added a few functions of my own I thought would be useful. Example: a more advanced memory copy function that lets you decide when to stop copying with a function pointer parameter.

A string can be converted to a char array, int, uint, long, ulong or even a float or double.
Furthermore, the reverse is true, as a string can be created from not just a char array, but also a char multipled n times, an int/uint/long/ulong, but also a float or double.

The only thing I think is left to work on is performance and scientific notation for float -> string conversion, as right now it returns up to a 50 or 340 character string. (to show off its precision, i guess)

P.S.: the test.c file is just a quick way to test the string functions.
