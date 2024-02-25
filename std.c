//
// Created by 仇庚垚 on 2024/2/12.
//

// This file is all std functions will be loaded in environment.
#include "neptune.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
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
// Now it supports some escape charaters.
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
            if(str_get(source, cursor)==unicode('['))printf("[");
            if(str_get(source, cursor)==unicode(']'))printf("]");
            if(str_get(source, cursor)==unicode('\\'))printf("\\");
        }else{
            printf("%c", str_get(source, cursor));
        }
    }
    str_free(source);
    return ans;
}
// current_layer must be 0 at first time using.
void _extern_ls_tree(void* root, unsigned long current_layer, unsigned long total_depth){
    unsigned long cursor = 0;
    for(; ((void**)root)[cursor*2+1]!=BASIC_TYPE_END; cursor++){
        for(unsigned int i=0; i!=current_layer; i++)printf("    ");
        if(((void**)root)[cursor*2+1]==BASIC_TYPE_LIST){
            printf("<LIST_AT 0x%lx>", ((unsigned long*)root)[cursor*2]);
            printf("\n");
            _extern_ls_tree(((void**)root)[cursor*2], current_layer+1, total_depth);
        }else
        if(((void**)root)[cursor*2+1]==BASIC_TYPE_STRING){
            printf("\"");
            str_print(((void**)root)[cursor*2]);
            printf("\"\n");
        }else
        if(((void**)root)[cursor*2+1]==BASIC_TYPE_FUNC_SRC){
            printf("<SRC_FUNC_AT 0x%lx>", ((unsigned long*)root)[cursor*2]);
            printf("\n");
        }else
        if(((void**)root)[cursor*2+1]==BASIC_TYPE_FUNC_BIN){
            printf("<BIN_FUNC_AT 0x%lx>", ((unsigned long*)root)[cursor*2]);
            printf("\n");
        }else
        if(((void**)root)[cursor*2+1]==BASIC_TYPE_NONE_ALLOC){
            printf("<NONE_ALLOC>");
            printf("\n");
        }else{
            // TODO
        }
    }
    return;
}
// This function can show the struct of root.
String* extern_ls(void* root, String* source) {
    _extern_ls_tree(root, 0, 100);
    str_free(source);
    return str_let("");
}
String* extern_file(void* root, String* source) {
    // TODO
    str_free(source);
    return str_let("");
}
String* extern_comment(void* root, String* source) {
    String* ans = str_let("");
    str_free(source);
    return ans;
}
String* extern_if(void* root, String* source) {
    unsigned long cursor = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    if(str_get(source, cursor)==unicode('T')){
        cursor++;
        // read and expect '[
        for(; str_get(source, cursor)==unicode(' '); cursor++);
        // skip '[
        // TODO: Unexpected input formation processing is required here.
        cursor+=2;
        unsigned long body_start = cursor;
        unsigned long body_end = 0;
        unsigned long layer = 1;
        for(; layer!=0; cursor++){
            if(is_valid_open(source, cursor)==TRUE)
                layer++;
            if(is_valid_close(source, cursor)==TRUE)
                layer--;
        }
        body_end = cursor-1;
        String* body = str_copy(source, body_start, body_end);
        str_free(source);
        return exec(root, body);
    }else{
        str_free(source);
        return str_let("");
    }
    // Unreachable
    return source;
}
String* extern_while(void* root, String* source) {
    unsigned long cursor = 0;
    unsigned long condition_start = 0;
    unsigned long condition_end = 0;
    unsigned long body_start = 0;
    unsigned long body_end = 0;
    {
        // Skip whitespace
        for(; str_get(source, cursor)==unicode(' '); cursor++);
        // TODO: inspection of expected '[
        cursor+=2;
        condition_start = cursor;
        int layer=1;
        while(layer!=0){
            if(is_valid_open(source, cursor)==TRUE)layer++;
            if(is_valid_close(source, cursor)==TRUE)layer--;
            cursor++;
        }
        condition_end = cursor-1;
        // Skip whitespace;
        for(; str_get(source, cursor)==unicode(' '); cursor++);
        // TODO: inspection of expected '[
        cursor+=2;
        body_start = cursor;
        layer=1;
        while(layer!=0){
            if(is_valid_open(source, cursor)==TRUE)layer++;
            if(is_valid_close(source, cursor)==TRUE)layer--;
            cursor++;
        }
        body_end = cursor-1;
    }
    // here we have condition and body.
    String* condition_ans = exec(root, str_copy(source, condition_start, condition_end));
    cursor=0;
    // Skip blank of condition_ans
    for(; str_get(condition_ans, cursor)==unicode(' '); cursor++);
    while(str_get(condition_ans, cursor)== unicode('T')){
        str_free(condition_ans);
        String* body_ans = exec(root, str_copy(source, body_start, body_end));
        str_free(body_ans);
        condition_ans = exec(root, str_copy(source, condition_start, condition_end));
        cursor=0;
        for(; str_get(condition_ans, cursor)== unicode(' '); cursor++);
    }
    str_free(condition_ans);
    str_free(source);
    return source;
}
String* extern_add(void* root, String* source) {
    // check input type.
    unsigned long cursor = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    short is_input_a_double=0,
        is_input_b_double=0;
    for(; str_get(source, cursor)!= unicode(' '); cursor++){
        if(str_get(source, cursor)==unicode('.'))is_input_a_double=1;
    }
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++){
        if(str_get(source, cursor)==unicode('.'))is_input_b_double=1;
    }
    // here we know the type of input_a and input_b.
    if (is_input_a_double==0&&is_input_b_double==0){
        long long int input_a, input_b;
        char* char_source = str_2char_ptr(source);
        sscanf(char_source, "%lld %lld", &input_a, &input_b);
        char* char_ans = (char*)malloc(sizeof(char)*50);
        sprintf(char_ans, "%ld", input_b+input_a);
        String* ans = str_let(char_ans);
        free(char_ans);
        free(char_source);
        return ans;
    }
    if (is_input_a_double==1&&is_input_b_double==0){
        double input_a;
        long long int input_b;
        char* char_source = str_2char_ptr(source);
        sscanf(char_source, "%f %lld", &input_a, &input_b);
        char* char_ans = (char*)malloc(sizeof(char)*50);
        sprintf(char_ans, "%f", input_a+input_b);
        String* ans = str_let(char_ans);
        free(char_ans);
        free(char_source);
        return ans;
    }
    if (is_input_a_double==0&&is_input_b_double==1){
        long long int input_a;
        double input_b;
        char* char_source = str_2char_ptr(source);
        sscanf(char_source, "%lld %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a+input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if (is_input_a_double==1&&is_input_b_double==1){
        double input_a, input_b;
        char* char_source = str_2char_ptr(source);
        sscanf(char_source, "%f %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a+input_b);
        String* ans = str_let((char_ans));
        free(char_source);
        return ans;
    }
    // Unreachable
    return str_let("");
}
String* extern_sub(void* root, String* source) {
    unsigned long cursor = 0;
    for(; str_get(source, cursor)== unicode(' '); cursor++);
    short is_input_a_double=0, is_input_b_double=0;
    for(; str_get(source,cursor)!=unicode(' '); cursor++)
        if (str_get(source, cursor)== unicode('.'))
            is_input_a_double =1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!= unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if (str_get(source, cursor)==unicode('.'))
            is_input_b_double=1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a=0, input_b=0;
        sscanf(char_source, "%lld %lld", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%lld", input_a-input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a;
        long long int input_b;
        sscanf(char_source, "%f %lld", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a-input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a;
        double input_b;
        sscanf(char_source, "%lld %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a-input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a;
        double input_b;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a-input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    // Unreachable
    free(char_source);
    return str_let("");
}
String* extern_mul(void* root, String* source) {
    unsigned long cursor=0;
    short is_input_a_double=0, is_input_b_double=0;
    for(; str_get(source, cursor)== unicode(' '); cursor++);
    for(; str_get(source, cursor)!= unicode(' '); cursor++)
        if (str_get(source, cursor)==unicode('.'))
            is_input_a_double=1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if (str_get(source, cursor)==unicode('.'))
            is_input_b_double=1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a;
        long long int input_b;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%ld", input_a*input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a;
        long long int input_b;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a*input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a;
        double input_b;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a*input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a;
        double input_b;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a*input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    // Unreachable
    free(char_source);
    return source;
}
String* extern_div(void* root, String* source) {
    unsigned long cursor = 0;
    short is_input_a_double=0, is_input_b_double=0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' '); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_a_double=1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!= unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if(str_get(source,cursor)==unicode('.'))
            is_input_b_double=1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a, input_b;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", ((double)input_a)/((double)input_b));
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a;
        double input_b;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a/input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a;
        long long int input_b;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a/input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a;
        double input_b;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        char char_ans[50];
        sprintf(char_ans, "%f", input_a/input_b);
        String* ans = str_let(char_ans);
        free(char_source);
        return ans;
    }
    // Unreachable
    free(char_source);
    return str_let("");
}
String* extern_and(void* root, String* source){
    unsigned long cursor = 0;
    short input_a=0, input_b=0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    if(str_get(source, cursor)==unicode('T'))
        input_a=1;
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    if(str_get(source, cursor)== unicode('T'))
        input_b=1;
    str_free(source);
    if((input_a==1)&&(input_b))return str_let("T");
    else return str_let("F");
    str_free(source);
    return str_let("");
}
String* extern_or(void* root, String* source){
    unsigned long cursor=0;
    short input_a=0, input_b=0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    if(str_get(source, cursor)==unicode('T'))
        input_a=1;
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    if(str_get(source, cursor)==unicode('T'))
        input_b=1;
    str_free(source);
    if((input_a==1)||(input_b==1))return str_let("T");
    else return str_let("F");
    str_free(source);
    return str_let("");
}
String* extern_not(void* root, String* source){
    unsigned long cursor=0;
    short input=0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    if(str_get(source, cursor)==unicode('T'))
        input=1;
    str_free(source);
    if(!(input==1))return str_let("T");
    else return str_let("F");
    str_free(source);
    return str_let("");
}
String* extern_equal(void* root, String* source){
    unsigned long cursor=0;
    short is_input_a_double=0, is_input_b_double=0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' '); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_a_double = 1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!= unicode('\0'); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_b_double = 1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a=0;
        long long int input_b=0;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        free(char_source);
        if(input_a==input_b){
            return str_let("T");
        }else{
            return str_let("F");
        }
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a=0;
        long long int input_b=0;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        free(char_source);
        if(input_a==input_b){
            return str_let("T");
        }else{
            return str_let("F");
        }
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a=0;
        double input_b=0;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        free(char_source);
        if(input_a==input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        free(char_source);
        if(input_a==input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    // unreachable
    str_free(source);
    return str_let("");
}
// This handle >= operator.
String* extern_ge(void* root, String* source){
    unsigned long cursor=0;
    short is_input_a_double = 0;
    short is_input_b_double = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' '); cursor++)
        if(str_get(source, cursor)=='.')
            is_input_a_double = 1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if(str_get(source, cursor)=='.')
            is_input_b_double = 1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        free(char_source);
        if(input_a>=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        free(char_source);
        if(input_a>=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        free(char_source);
        if(input_a>=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        free(char_source);
        if(input_a>=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    free(char_source);
    return str_let("");
}
String* extern_le(void* root, String* source){
    unsigned long cursor=0;
    short is_input_a_double = 0;
    short is_input_b_double = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' '); cursor++)
        if(str_get(source,cursor)==unicode('.'))
            is_input_a_double = 1;
    for(; str_get(source, cursor)== unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_b_double = 1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        free(char_source);
        if(input_a<=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        free(char_source);
        if(input_a<=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        free(char_source);
        if(input_a<=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a=0;
        double input_b=0;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        free(char_source);
        if(input_a<=input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    str_free(source);
    return str_let("");
}
String* extern_lar(void* root, String* source){
    unsigned long cursor=0;
    short is_input_a_double = 0;
    short is_input_b_double = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' '); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_a_double = 1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_b_double = 1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a=0;
        long long int input_b=0;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        free(char_source);
        if(input_a>input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        free(char_source);
        if(input_a>input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        free(char_source);
        if(input_a>input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        free(char_source);
        if(input_a>input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    str_free(source);
    return str_let("");
}
String* extern_les(void* root, String* source){
    unsigned long cursor=0;
    short is_input_a_double = 0;
    short is_input_b_double = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' '); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_a_double = 1;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    for(; str_get(source, cursor)!=unicode(' ')&&str_get(source, cursor)!=unicode('\0'); cursor++)
        if(str_get(source, cursor)==unicode('.'))
            is_input_b_double = 1;
    char* char_source = str_2char_ptr(source);
    if(is_input_a_double==0&&is_input_b_double==0){
        long long int input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%ld %ld", &input_a, &input_b);
        free(char_source);
        if(input_a<input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==0){
        double input_a = 0;
        long long int input_b = 0;
        sscanf(char_source, "%f %ld", &input_a, &input_b);
        free(char_source);
        if(input_a<input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==0&&is_input_b_double==1){
        long long int input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%ld %f", &input_a, &input_b);
        free(char_source);
        if(input_a<input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    if(is_input_a_double==1&&is_input_b_double==1){
        double input_a = 0;
        double input_b = 0;
        sscanf(char_source, "%f %f", &input_a, &input_b);
        free(char_source);
        if(input_a<input_b)
            return str_let("T");
        else
            return str_let("F");
    }
    str_free(source);
    return str_let("");
}
String* extern_echo(void* root, String* source){
    return source;
}
String* extern_input(void* root, String* source){
    // TODO
}
String* extern_var(void* root, String* source){
    // TODO
}