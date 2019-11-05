#include <stdio.h>
#include <stdlib.h>

int main()
{
    float a, b, c;
    int x, y, z;
    a = 1; 
    b = 2;
    x = 4;
    y = 5;
    c = y/x; 
    z = y/x;

    printf("c %e, z %e\n", c, z);
    printf("c %d, z %d\n", c, z);

    return 0;
}

