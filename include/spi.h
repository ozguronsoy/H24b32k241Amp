#if !defined(INTTYPES_H)
#include <inttypes.h>
#define INTTYPES_H
#endif

typedef volatile struct
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SR;
    uint32_t DR;
    uint32_t CRCPR;
    uint32_t RXCRCR;
    uint32_t TXCRCR;
    uint32_t I2SCFGR;
    uint32_t I2SPR;
} SpiRegisters;