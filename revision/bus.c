#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    char* ptr = (1<<20) + 59;
    int x = *(int*)((char*)ptr);

    return 0;
}
