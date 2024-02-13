//
// Created by 仇庚垚 on 2024/2/12.
//

// This file is all std functions will be loaded in environment.
#include "neptune.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
// new_size must larger than current_size
// this function ONLY used in load
void* _my_realloc(void* target, unsigned long current_size, unsigned long new_size){
    void** ans = malloc(sizeof(void*)*new_size);
    unsigned long cursor=0;
    for(; cursor<current_size; cursor++)
        ans[cursor] = ((void**)target)[cursor];
    for(; cursor<new_size; cursor++)
        ans[cursor] = 0;
    free(target);
    return  ans;
}
/*  structure of function list
 *  [function handle pointer:FUNC_BIN||FUNC_SRC]01
 *  [function true name:string]23
 *  [function current name:string]45
 *  ...
 */
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
    char* path_char_ptr = str_2char_ptr(path);
    void* function_handle = dlopen(path_char_ptr, RTLD_LAZY);
    // Append new function at function list end
    unsigned long function_list_len=0;
    for(; ((void***)root)[0][function_list_len*2+1]!=BASIC_TYPE_END; function_list_len++);
    ((void***)root)[0] = _my_realloc(((void**)root)[0],function_list_len*2+1+1 ,function_list_len*2+1+1+6);
    ((void***)root)[0][function_list_len*2+1+6] = BASIC_TYPE_END;
    ((void***)root)[0][function_list_len*2+0] = function_handle;
    ((void***)root)[0][function_list_len*2+1] = BASIC_TYPE_FUNC_BIN;
    ((void***)root)[0][function_list_len*2+2] = function_name;
    ((void***)root)[0][function_list_len*2+3] = BASIC_TYPE_STRING;
    ((void***)root)[0][function_list_len*2+4] = as_name;
    ((void***)root)[0][function_list_len*2+5] = BASIC_TYPE_STRING;
    free(path_char_ptr);
    str_free(source);
    return ans;
}
// TODO: this function currently only hold ascii
String* extern_print(void* root, String* source) {
    String* ans = str_let("");
    // delete the first block in source;
    unsigned long cursor = 0;
    // Expect here the # after blank
    for(; str_get(source, cursor)== unicode(' '); cursor++);
    if(str_get(source,cursor)!=unicode('#')){
        printf("print: Error, unexpected input");
        str_free(source);
        return ans;
    }
    cursor++;
    for(; str_get(source, cursor)!= unicode('\0'); cursor++){
        if(str_get(source, cursor)== unicode('\\')){
            cursor++;
            if(str_get(source, cursor)== unicode('n'))printf("\n");
            if(str_get(source, cursor)==unicode('t'))printf("\t");
        }else{
            printf("%c", str_get(source, cursor));
        }
    }
    str_free(source);
    return ans;
}
String* extern_file(void* root, String* source) {
    // TODO
    return source;
}
String* extern_comment(void* root, String* source) {
    String* ans = str_let("");
    str_free(source);
    return ans;
}
String* extern_if(void* root, String* source) {
    // TODO
    return source;
}
String* extern_while(void* root, String* source) {
    // TODO
    return source;
}
String* extern_add(void* root, String* source) {
    // TODO
    return source;
}
String* extern_sub(void* root, String* source) {
    // TODO
    return source;
}
String* extern_mul(void* root, String* source) {
    // TODO
    return source;
}
String* extern_div(void* root, String* source) {
    // TODO
    return source;
}