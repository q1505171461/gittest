#include <stdio.h>
#include "ppp-b2b.h"

struct BitFieldStruct {
    unsigned short bit3 : 6;
    unsigned short bit1 : 1;
    unsigned char bit2 : 8;
};

int main() {
    struct BitFieldStruct bf;
    
    bf.bit1 = 1;
    bf.bit2 = 3;
    bf.bit3 = 7;
    
    printf("Size of struct: %lu\n", sizeof(bf));
    printf("bit1: %u\n", bf.bit1);
    printf("bit2: %u\n", bf.bit2);
    printf("bit3: %u\n", bf.bit3);
    return 0;
}
