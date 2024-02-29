

// This file is all std functions will be loaded in environment.
#include "sublisp.h"
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
String* load(Var** root, String* source){
    unsigned long cursor = 0;
    unsigned long handle_start = 0;
    unsigned long handle_end = 0;
    unsigned long symbol_start = 0;
    unsigned long symbol_end = 0;
    unsigned long var_name_start = 0;
    unsigned long var_name_end = 0;
    // Skip blanks
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    handle_start = cursor;
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    handle_end = cursor;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    symbol_start = cursor;
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    symbol_end = cursor;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    var_name_start = cursor;
    for(; str_get(source, cursor)!=unicode(' ')
        &&str_get(source, cursor)!=unicode('\0'); cursor++);
    var_name_end = cursor;
    // Here we have handle_start handle_end symbol_start symbol_end var_name_start var_name_end
    String* handle_name = str_copy(source, handle_start, handle_end);
    String* symbol_name = str_copy(source, symbol_start, symbol_end);
    String* var_name = str_copy(source, var_name_start, var_name_end);
    Var* current_var = *root;
    void* handle = NULL;
    while(current_var!=VAR_TYPE_END){
        if(current_var->type == VAR_TYPE_FUNCTION_HANDLE)
            if(str_same(current_var->name, handle_name)==TRUE)
                handle = current_var->ptr;
        current_var = current_var->next_node;
    }
    // TODO: need some operation to process undetected var name situation.
    char* symbol_name_char = str_2char_ptr(symbol_name);
    Func function_ptr = dlsym(handle, symbol_name_char);
    str_free(handle_name);
    Var* new_var = var_init(var_name,
                            function_ptr,
                            VAR_TYPE_END,
                            VAR_TYPE_END,
                            VAR_TYPE_FUNCTION);
    *root = var_push(*root, new_var);
    str_free(source);
    return str_let("");
}
// TODO: this function currently only hold ascii
// Now it supports some escape charaters.
String* extern_print(Var** root, String* source) {
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

String* extern_file(Var** root, String* source) {
    // TODO
    str_free(source);
    return str_let("");
}
String* extern_comment(Var** root, String* source) {
    String* ans = str_let("");
    str_free(source);
    return ans;
}
String* extern_if(Var** root, String* source) {
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
String* extern_while(Var** root, String* source) {
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
String* extern_add(Var** root, String* source) {
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
String* extern_sub(Var** root, String* source) {
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
String* extern_mul(Var** root, String* source) {
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
String* extern_div(Var** root, String* source) {
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
String* extern_and(Var** root, String* source){
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
String* extern_or(Var** root, String* source){
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
String* extern_not(Var** root, String* source){
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
String* extern_equal(Var** root, String* source){
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
String* extern_ge(Var** root, String* source){
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
String* extern_le(Var** root, String* source){
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
String* extern_lar(Var** root, String* source){
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
String* extern_les(Var** root, String* source){
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
String* extern_echo(Var** root, String* source){
    return source;
}
String* extern_input(Var** root, String* source){
    String* input_string = str_let("");

    str_free(source);
    return str_let("");
}
// This function will set and assign a new var(string) or assign an existed var(string).
String* extern_new(Var** root, String* source){
    unsigned long var_name_start = 0;
    unsigned long var_name_end = 0;
    unsigned long value_start = 0;
    unsigned long value_end = 0;
    unsigned long cursor = 0;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    var_name_start = cursor;
    for(; str_get(source, cursor)!=unicode(' '); cursor++);
    var_name_end = cursor;
    for(; str_get(source, cursor)==unicode(' '); cursor++);
    int layer = 1;
    cursor+=2;    // Skip '[
    value_start = cursor;
    while(layer!=0){
        if(is_valid_open(source, cursor)==TRUE)layer++;
        if(is_valid_close(source, cursor)==TRUE)layer--;
        cursor++;
    }
    value_end = cursor-1;
    String* var_name = str_copy(source, var_name_start, var_name_end);
    String* value = str_copy(source, value_start, value_end);
    Var* new_var = var_init(var_name,
                            value,
                            VAR_TYPE_END,
                            VAR_TYPE_END,
                            VAR_TYPE_STRING);
    str_free(source);
    // Here we have var_name and value
    // Check if there already have such value
    Var* current_var = *root;
    while(!(current_var==VAR_TYPE_END
            ||current_var->type==VAR_TYPE_LAYER_BOUNDARY
            || str_same(current_var->name, var_name)==TRUE)){
        current_var = current_var->next_node;
    }
    if(current_var==VAR_TYPE_END||current_var->type==VAR_TYPE_LAYER_BOUNDARY){
        // can't find var name -- push in a new function
        *root = var_push(*root, new_var);
    }else if(str_same(current_var->name, var_name)==TRUE){
        if(current_var->next_node!=VAR_TYPE_END){
            current_var->next_node->last_node = new_var;
            new_var->next_node = current_var->next_node;
        }
        if(current_var->last_node!=VAR_TYPE_END){
            current_var->last_node->next_node = new_var;
            new_var->last_node = current_var->last_node;
        }
        var_free(current_var);
    }
    return str_let("");
}
String* extern_let(Var** root, String* source){
    // TODO
    str_free(source);
    return str_let("");
}