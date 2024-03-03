

#ifndef NEPTUNE_H
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
typedef char String;
typedef long Unicode;

// Variable stack
typedef struct Var{
    String* name;
    void* ptr;
    struct Var* next_node;
    struct Var* last_node;
    int type;
} Var;
#define VAR_TYPE_END               0
#define VAR_TYPE_STRING            1
#define VAR_TYPE_VAR_STACK         2
#define VAR_TYPE_FUNCTION          3
#define VAR_TYPE_FUNCTION_HANDLE   4
#define VAR_TYPE_LAYER_BOUNDARY    5
#define VAR_TYPE_REFERENCE         6
Var* var_push(Var* target, Var* input);
Var* var_pop(Var* target);
// This function will allocate a new space.
Var* var_init(String* name, void* ptr, Var* next_node, Var* last_node, int type);
void var_free(Var* input);
// This function is used to replace the node in stack by with and
// return replace.
Var* var_replace_with(Var* replaced, Var* with);


typedef String* (*Func)(Var**, String*);    // Standard function pointer.

String* read_file_as_string(char* path);
Unicode str_get(String* source, unsigned long where);
Unicode unicode(char input);
unsigned long str_len(String* input);
String* str_replace(String* target, unsigned long from, unsigned long to, String* with);
String* str_copy(String* target, unsigned long from, unsigned long to);
void str_free(String* input);
int str_same(String* a, String* b);
void str_print(String* input);
// This function will not free input.
String* str_let(char* input);
char* str_2char_ptr(String* input);
String* str_append_char(String* target, char* with);

int is_valid_open(String* source, unsigned long where);
int is_valid_close(String* source, unsigned long where);
int is_valid_function(String* source, unsigned long where);



// functions with standard form
/*
 * standard functions MUST free source and allocate new
 * space as return.
 */
String* exec(Var** root, String* source);
String* load(Var** root, String* source);


#define NEPTUNE_H
#endif
