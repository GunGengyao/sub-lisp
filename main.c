#include <stdio.h>
#include "neptune.h"
int main() {
    String* source = read_file_as_string("test.np");
    str_print(source);
    str_free(source);
    return 0;
}
