#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "neptune.h"
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
    void* root = type_init_list();
    ((void**)root)[0] = malloc(sizeof(void*)*8);
    ((void***)root)[0][0] = load_handle;
    ((void***)root)[0][1] = BASIC_TYPE_FUNC_BIN;
    ((void***)root)[0][2] = str_let("load");
    ((void***)root)[0][3] = BASIC_TYPE_STRING;
    ((void***)root)[0][4] = str_let("load");
    ((void***)root)[0][5] = BASIC_TYPE_STRING;
    ((void***)root)[0][6] = 0;//This value is not necessary.
    ((void***)root)[0][7] = BASIC_TYPE_END;
    source = exec(root, source);
    str_free(source);
    dlclose(load_handle);
    return 0;
}
