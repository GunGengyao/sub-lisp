#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "sublisp.h"
int main() {
    String* source = read_file_as_string("test.np");
    // pretreatment
    String* tmp = str_let(" ");
    for(unsigned long cursor = 0; str_get(source, cursor)!=unicode('\0'); cursor++) {
        if(str_get(source, cursor)==unicode('\n'))
            source = str_replace(source, cursor, cursor+1, tmp);
    }
    str_free(tmp);
    // init root.
    void* load_handle = dlopen("libstd.dylib", RTLD_LAZY);
    Var* root = var_init(str_let("std_functions_handle"),
                        load_handle,
                        VAR_TYPE_END,
                        VAR_TYPE_END,
                        VAR_TYPE_FUNCTION_HANDLE);
    Var* load_func = var_init(str_let("load"),
                             dlsym(load_handle, "load"),
                             VAR_TYPE_END,
                             VAR_TYPE_END,
                             VAR_TYPE_FUNCTION);
    root = var_push(root, load_func);
    source = exec(&root, source);
    str_free(source);
    var_free(root);
    return 0;
}
