#if !defined(INTTYPES_H)
#include <inttypes.h>
#define INTTYPES_H
#endif

#define DMA1_REGISTERS ((DmaRegisters*)0x40026000u)
#define DMA2_REGISTERS ((DmaRegisters*)0x40026400u)

typedef volatile struct 
{
    uint32_t CR;
    uint32_t NDTR;
    uint32_t PAR;
    uint32_t M0AR;
    uint32_t M1AR;
    uint32_t FCR;
} DmaStreamRegisters;

typedef volatile struct
{
    uint32_t LISR;
    uint32_t HISR;
    uint32_t LIFCR;
    uint32_t HIFCR;
    DmaStreamRegisters S0;
    DmaStreamRegisters S1;
    DmaStreamRegisters S2;
    DmaStreamRegisters S3;
    DmaStreamRegisters S4;
    DmaStreamRegisters S5;
    DmaStreamRegisters S6;
    DmaStreamRegisters S7;
} DmaRegisters;