#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

int exec_main(int argc, char** argv) {


}

int main(int argc, char** argv) {
    printf("|%s|\n", string_int(0));
    int ret = exec_main(argc, argv);
    return ret;
}
