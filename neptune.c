

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "neptune.h"
String* read_file_as_string(char* path){
    FILE *fp = fopen(path, "r");
    int fileSize, k;
    for(fileSize=0;0<(k=fgetc(fp))&&k<128;fileSize++);
    fclose(fp);
    fp = fopen(path, "r");
    char* ans;
    ans = (char*)malloc(sizeof(char)*fileSize);
    int i=0;
    for(i=0; 0<(ans[i]=fgetc(fp))&&ans[i]<128; i++);
    ans[i]=0;
    return (String*)ans;
}
Unicode str_get(String* source, unsigned long where){
    return ((char*)source)[where];
}
Unicode unicode(char input){
    Unicode ans;
    ans = input;
    return ans;
}
unsigned long str_len(String* input){
    unsigned long ans = 0;
    char* ch_input = (char*)input;
    while(ch_input[ans]!=0)ans++;
    return ans;
}
// target will be freed
String* str_replace(String* target, unsigned long from, unsigned long to, String* with){
    unsigned long with_len = str_len(with);
    unsigned long target_len = str_len(target);
    char* ans = (char*)malloc(sizeof(char)*(target_len-(to-from)+with_len));
    unsigned long ans_cursor = 0;
    unsigned long free_cursor = 0;    // another cursor
    for(; free_cursor!=from; free_cursor++, ans_cursor++)
        ans[ans_cursor] = ((char*)target)[free_cursor];
    for(free_cursor=0; free_cursor!=with_len; free_cursor++, ans_cursor++)
        ans[ans_cursor] = ((char*)with)[free_cursor];
    for(free_cursor=to; free_cursor!=target_len; free_cursor++, ans_cursor++)
        ans[ans_cursor] = ((char*)target)[free_cursor];
    ans[ans_cursor] = 0;
    return (String*)ans;
}
String* str_copy(String* target, unsigned long from, unsigned long to){
    if (from==to) {  // This will copy all string.
        unsigned long target_len = str_len(target);
        char* ans = (char*)malloc(sizeof(char)*(target_len+1));
        unsigned long cursor = 0;
        for(; target[cursor]!=0; cursor++) {
            ans[cursor] = target[cursor];
        }
        ans[cursor]=0;
        return ans;
    }
    unsigned long target_len = str_len(target);
    char* ans = (char*)malloc(sizeof(char)*(target_len+1));
    unsigned long tar_cursor = from;
    unsigned long ans_cursor = 0;
    for(; tar_cursor!=to; tar_cursor++, ans_cursor++){
        ans[ans_cursor] = target[tar_cursor];
    }
    ans[ans_cursor] = 0;
    return ans;
}
void str_free(String* input){
    free(input);
    return;
}
int str_same(String* a, String* b){
    unsigned long a_len = 0;
    unsigned long b_len = 0;
    a_len = str_len(a);
    b_len = str_len(b);
    if (a_len!=b_len)return FALSE;
    unsigned long cursor=0;
    for(; cursor!=a_len; cursor++){
        if (((char*)a)[cursor]!=((char*)b)[cursor])return FALSE;
    }
    return TRUE;
}
void str_print(String* input){
    printf("%s", input);
    return;
}
String* str_let(char* input){
    unsigned long input_len = 0;
    for(; input[input_len]!=0; input_len++);
    String* ans = (String*)malloc(sizeof(char)*(input_len+1));
    unsigned long i=0;
    input_len=0;
    for(; input[input_len]!=0; input_len++, i++){
        ans[i] = input[i];
    }
    ans[i]=0;
    return ans;
}
// This function will free input, and alloc new space as return
char* str_2char_ptr(String* input){
    return (char*)input;
}
// function_name will be freed
Func _match_function(Var* root, String* function_name) {
    Var current_var = *root;
    while(current_var.next_node!=VAR_TYPE_END){
        if(current_var.type==VAR_TYPE_FUNCTION)
            if(str_same(current_var.name, function_name)==TRUE){
                return ((Func)current_var.ptr);
            }
        current_var = *current_var.next_node;
    }
}

int is_valid_open(String* source, unsigned long where){
    if (str_get(source, where)!=unicode('['))return FALSE;
    if (where==0)return TRUE;
    if (str_get(source,where)==unicode('\\'))return FALSE;
    return TRUE;
}
int is_valid_close(String* source, unsigned long where){
    if (str_get(source, where)!=unicode(']'))return FALSE;
    if (where==0)return TRUE;
    if (str_get(source, where)==unicode('\\'))return FALSE;
    return TRUE;
}
int is_valid_function(String* source, unsigned long where){
    if (is_valid_open(source, where)==FALSE)return FALSE;
    if (where==0)return TRUE;
    if (str_get(source, where-1)!=unicode('\''))return TRUE;
    return FALSE;
}
// return new head of chain/stack
Var* var_push(Var* target, Var* input){
    if (input->next_node!=VAR_TYPE_END){
        printf("var_push: ERROR, input already has some nodes attached\n");
        // TODO
    }
    input->next_node = target;
    target->last_node = input;
    return input;
}
// WARNING: make sure you have already got the head of stack
// before using this function, this will detach the head node
// with stack!!!
// Return new head of stack.
Var* var_pop(Var* target){
    target->next_node->last_node = VAR_TYPE_END;
    return target->next_node;
}
Var* var_init(String* name, void* ptr, Var* next_node, Var* last_node, int type){
    Var* ans = (Var*)malloc(sizeof(Var)*1);
    ans->name = name;
    ans->ptr = ptr;
    ans->next_node = next_node;
    ans->last_node = last_node;
    ans->type = type;
    return ans;
}
// ---------------------------------------------------------------- here the all function should have standard form ---- {
// This function will free source and alloc new space as return
String* exec(Var** root, String* source){
    unsigned long cursor = 0;
    // Here is going to unfold function
    for(; str_get(source, cursor)!=unicode('\0');){
        if (is_valid_function(source, cursor)==TRUE){
            // get function_start and function_end;
            unsigned long function_start, function_end;
            function_start = cursor;
            cursor++;
            int layer = 1;
            for(; layer!=0; cursor++){
                if (is_valid_open(source, cursor)==TRUE)layer++;
                if (is_valid_close(source, cursor)==TRUE)layer--;
            }
            function_end = cursor;
            source = str_replace(
                source,
                function_start,
                function_end,
                exec(root,
                    str_copy(source, function_start+1, function_end-1)
                )
            );
            cursor = function_start;
        }else if(is_valid_open(source, cursor)==TRUE){
            // here is going to skip '[]
            cursor++;
            int layer = 1;
            for(; layer!=0; cursor++){
                if(is_valid_open(source, cursor)==TRUE)
                    layer++;
                if(is_valid_close(source, cursor)==TRUE)
                    layer--;
            }
        }else{
            cursor++;
        }
    }
    // Here we get the pure function struct without recursion
    unsigned long function_name_start, function_name_end;
    cursor=0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    function_name_start = cursor;
    if(str_get(source, cursor)==unicode('\0'))return source;
    for(; str_get(source, cursor)!=unicode(' ')&& str_get(source,cursor)!= unicode('\0'); cursor++);
    function_name_end = cursor;
    unsigned long function_body_start, function_body_end;
    function_body_start = function_name_end;
    function_body_end = str_len(source);
    String* name_tmp = str_copy(source, function_name_start, function_name_end);
    Func function_ptr = _match_function(*root, name_tmp);
    String* ans = function_ptr(root, str_copy(source, function_body_start, function_body_end));
    str_free(source);  // source freed here.
    return ans;
}
//-------------------------------------------------------------- here the function should all have standard form ---- }