//
// Created by 仇庚垚 on 2024/2/12.
//

// This file is all std functions will be loaded in environment.
#include "neptune.h"
#include <stdio.h>
#include <stdlib.h>

String* extern_print(void* root, String* source) {
    str_print(source);
    return source;
}
String* extern_file(void* root, String* source) {
    // TODO
    return source;
}
String* extern_comment(void* root, String* source) {
    // TODO
    return source;
}
String* extern_if(void* root, String* source) {
    // TODO
    return source;
}
String* extern_while(void* root, String* source) {
    // TODO
    return source;
}
