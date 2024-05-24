#ifndef RAM_DB_H
#define RAM_DB_H

#include <stdint.h>
#include "app_main.h"

#define RAM_SIZE 2048  // Tamaño total de la base de datos en bytes

extern uint8_t ram[RAM_SIZE];  // Declaración del espacio en RAM

typedef enum {
    RAM_1  = 0x2002F800,
    RAM_2  = 0x2002F801,
    RAM_3  = 0x2002F802,
    RAM_KP = 0x2002F803,
    RAM_KI = 0x2002F805,
    RAM_KD = 0x2002F807
} RAM_DB;

// Macros para escribir en la base de datos
#define RAM_WRITE_U8(addr, value) *((uint8_t *)(addr)) = (value)
#define RAM_WRITE_U16(addr, value) *((uint16_t *)(addr)) = (value)
#define RAM_WRITE_U32(addr, value) *((uint32_t *)(addr)) = (value)

// Macros para leer de la base de datos
#define RAM_READ_U8(addr) *((uint8_t *)(addr))
#define RAM_READ_U16(addr) *((uint16_t *)(addr))
#define RAM_READ_U32(addr) *((uint32_t *)(addr))

/* Test functions */

void test_u8(void);

#endif // RAM_DB_H
