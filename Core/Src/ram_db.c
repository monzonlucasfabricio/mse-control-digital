#include <stdio.h>
#include "ram_db.h"

__attribute__((section(".ramdb"))) uint8_t ram[RAM_SIZE];

/************************ TEST DE RAMDB ***************************/
void test_u8(void)
{
    RAM_WRITE_U8(RAM_1, 0x12);
    RAM_WRITE_U8(RAM_2, 0x34);
    RAM_WRITE_U8(RAM_3, 0x55);
    
    uint8_t val_1 = RAM_READ_U8(RAM_1);
    uint8_t val_2 = RAM_READ_U8(RAM_2);
    uint8_t val_3 = RAM_READ_U8(RAM_3);

    print_debug_msg("RAM_1 : %#X",val_1);
    print_debug_msg("RAM_2 : %#X",val_2);
    print_debug_msg("RAM_3 : %#X",val_3);
}

/************************ TEST DE RAMDB ***************************/
