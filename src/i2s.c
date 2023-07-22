#if !defined(I2S_H)
#include "i2s.h"
#define I2S_H
#endif

#if !defined(RCC_H)
#include "rcc.h"
#define RCC_H
#endif

#if !defined(GPIO_H)
#include "gpio.h"
#define GPIO_H
#endif


#define I2S_SAMPLE_HO(sample) ((sample & 0x00FFFF00) >> 8) // for 24-bit data, 32-bit container
#define I2S_SAMPLE_LO(sample) ((sample & 0x000000FF) << 8)

#define I2S3_REGISTERS ((SpiRegisters*)0x40003C00u)
#define I2S3EXT_REGISTERS ((SpiRegisters*)0x40004000u)




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



void I2S_ConfigureRCC();
void I2S_ConfigureI2S3Registers();
void I2S_ConfigureGPIORegisters();
void I2S_ConfigurePLLI2S();

void InitializeI2S3()
{
    I2S_ConfigureRCC();
    I2S_ConfigureGPIORegisters();
    I2S_ConfigureI2S3Registers();
    I2S_ConfigurePLLI2S();

    // enable the I2S peripheral
    I2S3_REGISTERS->I2SCFGR |= 1 << 10;
    I2S3EXT_REGISTERS->I2SCFGR |= 1 << 10;
}

void I2S_ConfigureRCC()
{
    RCC_AHB1ENR |= 0b11; // enable the GPIO A & B clock
    RCC_APB1ENR |= 0b1 << 15; // enable the SPI3 clock
}

void I2S_ConfigureI2S3Registers()
{
    I2S3_REGISTERS->I2SCFGR |= 0b1010 << 8; // select the I2S mode as master-transmit
    I2S3EXT_REGISTERS->I2SCFGR |= 0b1011 << 8; // select the I2S mode as master-receive

    // steady state low, 24-bit data, 32 bit container, MSB (left) justified
    I2S3_REGISTERS->I2SCFGR |= 0b010011;
    I2S3EXT_REGISTERS->I2SCFGR |= 0b010011;

    // MCK enabled, odd and I2SDIV = 3 for 96kHz
    I2S3_REGISTERS->I2SPR |= 0b1100000011;
    I2S3EXT_REGISTERS->I2SPR |= 0b1100000011;
}

void I2S_ConfigureGPIORegisters()
{
    // set to alternate function mode
    GPIOA_REGISTERS->MODER |= 0b10 << 8;
    GPIOB_REGISTERS->MODER |= 0b101010 << 6;
    GPIOB_REGISTERS->MODER |= 0b10 << 20;

    // alternate function mapping
    GPIOA_REGISTERS->AFRL |= 0b0110 << 16;
    GPIOB_REGISTERS->AFRL |= 0b011001110110 << 12;
    GPIOB_REGISTERS->AFRH |= 0b0110 << 8;
}

void I2S_ConfigurePLLI2S()
{
    RCC_PLLI2SCFGR |= 16; // PLLI2SM, PLLM VCO = 1MHz
    RCC_PLLI2SCFGR &= ~(0b111111111 << 6); // clear the PLLI2SN
    RCC_PLLI2SCFGR |= 344 << 6; // PLLI2SN
    RCC_PLLI2SCFGR |= 2 << 28; // PLLI2SR
    RCC_CR |= 1 << 26; // Enable the PLLI2S
}