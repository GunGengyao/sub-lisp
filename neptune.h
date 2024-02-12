//
// Created by 仇庚垚 on 2024/2/12.
//

#ifndef NEPTUNE_H
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
typedef char String;
typedef long Unicode;
typedef String* (*Func)(void*, String*);
String* read_file_as_string(char* path);
Unicode str_get(String* source, unsigned long where);
Unicode unicode(char input);
unsigned long str_len(String* input);
String* str_replace(String* target, unsigned long from, unsigned long to, String* with);
String* str_copy(String* target, unsigned long from, unsigned long to);
void str_free(String* input);
int str_same(String* a, String* b);
void str_print(String* input);
String* str_let(char* input);
char* str_2char_ptr(String* input);
// functions with standard form
/*
 * standard functions MUST free source and allocate new
 * space as return.
 */
String* exec(void* root, String* source);
String* load(void* root, String* source);

// Basic neptune type labels
#define BASIC_TYPE_LIST 1
#define BASIC_TYPE_STRING 2
#define BASIC_TYPE_FUNC   3
#define BASIC_TYPE_END    0
void* type_init_list();
// input must be a list pointer
unsigned long type_list_len(void* input);
void type_print(void* input, long recur_times, long deep);
void type_free(void* input);
#define NEPTUNE_H
#endif
