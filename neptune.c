//
// Created by 仇庚垚 on 2024/2/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
// ------------------------------------------------------ neptune.h -----------------------------{
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
    for(i=0; input[input_len]!=0; input_len++){
        ans[i] = input[i];
    }
    ans[i]=0;
    return ans;
}
// This function will free input, and alloc new space as return
char* str_2char_ptr(String* input){
    return (char*)input;
}
void* type_init_list(){
    void** ans = malloc(sizeof(void*)*2);
    ans[0] = BASIC_TYPE_END;
    ans[1] = BASIC_TYPE_END;
    return ans;
}
unsigned long type_list_len(void* input){
    unsigned long ans=0;
    void** new_input = (void**)input;
    for(; new_input[2*ans]!=BASIC_TYPE_END; ans++);
    return ans*2;
}
// deep usually be 0
void type_print(void* input, long recur_times, long deep){
    unsigned long cursor = 0;
    for(; ((void**)input)[2*cursor+1]!=BASIC_TYPE_END; cursor++){
        for(; deep!=0; deep--)printf("    ");
        switch((int)(((void**)input)[2*cursor+1])){
            case BASIC_TYPE_STRING:{
                str_print(((void**)input)[2*cursor]);
                printf("\n");
            }
            case BASIC_TYPE_LIST:{
                type_print(((void**)input)[2*cursor], recur_times-1, deep+1);
            }
            case BASIC_TYPE_FUNC:{
                printf("<FUNC_AT:0x%x>\n", ((void**)input)[2*cursor]);
            }
            case BASIC_TYPE_END:{
                return;
            }
            default:{
                // TODO: can't reach here.
            }
        }
    }
    return;
}
void type_free(void* input) {
    unsigned long cursor = 0;
    for(; ((void**)input)[cursor*2+1]!=BASIC_TYPE_END; cursor++) {
        void* current_ptr = ((void**)input)[cursor*2];
        switch((int)((void**)input)[cursor*2+1]) {
            case BASIC_TYPE_STRING: {
                str_free((String*)current_ptr);
            }
            case BASIC_TYPE_LIST: {
                type_free(current_ptr);
            }
            case BASIC_TYPE_FUNC: {
                return;
            }
            case BASIC_TYPE_END: {
                return;
            }
            default: {
                // TODO: can't reach here.
            }
        }
    }
}
// ----------------------------------------------------------- neptune.h ------------------------------------}

Func _match_function(void* root, String* function_name){
    Func ans;    // WATCH OUT!!!
    void** root_tmp = (void**)root;
    root = *root_tmp;
    void* function_list = ((void**)root)[0];
    unsigned long cur = 0;
    for(; ((void***)function_list)[cur][0]!=0; cur++){
        if (str_same(((void***)function_list)[cur][0], function_name)==TRUE){
            return ((Func**)function_list)[cur][1];
        }
    }
    return ans;
}

int is_vaild_open(String* source, unsigned long where){
    if (str_get(source, where)!=unicode('['))return FALSE;
    if (where==0)return TRUE;
    if (str_get(source,where)==unicode('\\'))return FALSE;
    return TRUE;
}
int is_vaild_close(String* source, unsigned long where){
    if (str_get(source, where)!=unicode(']'))return FALSE;
    if (where==0)return TRUE;
    if (str_get(source, where)==unicode('\\'))return FALSE;
    return TRUE;
}
int is_vaild_function(String* source, unsigned long where){
    if (is_vaild_open(source, where)==FALSE)return FALSE;
    if (where==0)return TRUE;
    if (str_get(source, where-1)!=unicode('\''))return TRUE;
    return FALSE;
}
// ---------------------------------------------------------------- here the all function should have standard form ---- {
// This function will free source and alloc new space as return
String* exec(void* root, String* source){
    unsigned long cursor = 0;
    for(; str_get(source, cursor)!=unicode('\0');){
        if (is_vaild_function(source, cursor)==TRUE){
            // get function_start and function_end;
            unsigned long function_start, function_end;
            function_start = cursor;
            cursor++;
            int layer = 1;
            for(; layer!=0; cursor++){
                if (is_vaild_open(source, cursor)==TRUE)layer++;
                if (is_vaild_close(source, cursor)==TRUE)layer--;
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
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    function_name_end = cursor;
    unsigned long function_body_start, function_body_end;
    function_body_start = function_name_end;
    function_body_end = str_len(source);
    String* name_tmp = str_copy(source, function_name_start, function_name_end);
    return _match_function(root, name_tmp)(root, str_copy(source, function_body_start, function_body_end));
}
String* load(void* root, String* source){
    String* ans = str_let("");
    unsigned long cursor = 0;
    for(;str_get(source,cursor)==unicode(' '); cursor++);
    // Read library path.
    unsigned long path_start = cursor;
    unsigned long path_end   = 0;
    for(;str_get(source, cursor)!=unicode(' '); cursor++);
    path_end = cursor;
    String* path = str_copy(source, path_start, path_end);
    unsigned long func_name_start = 0;
    unsigned long func_name_end   = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    func_name_start = cursor;
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    func_name_end = cursor;
    String* function_name = str_copy(source, func_name_start, func_name_end);
    unsigned long as_start = 0;
    unsigned long as_end   = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    as_start = cursor;
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++);
    as_end = cursor;
    String* as_name = str_copy(source, as_start, as_end);
    // Here we have path, function_name, as_name.
    // load function name
    char* path_char_ptr = str_2char_ptr(path);
    char* function_name_char_ptr = str_2char_ptr(function_name);
    void* function_handle = dlopen(path_char_ptr, RTLD_LAZY);
    void* function_ptr = dlsym(function_handle, function_name_char_ptr);
    // TODO
    free(function_name_char_ptr);
    str_free(as_name);
    free(path_char_ptr);
    str_free(source);
    dlclose(function_handle);
    return ans;
}
String* print(void* root, String* source){
    String* ans = str_let("");
    str_print(source);
    return ans;
}
//-------------------------------------------------------------- here the function should all have standard form ---- }