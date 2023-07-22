#define GPIOA_REGISTERS ((GpioRegisters*)0x40020000u)
#define GPIOB_REGISTERS ((GpioRegisters*)0x40020400u)
#define GPIOC_REGISTERS ((GpioRegisters*)0x40020800u)

typedef volatile struct
{
    uint32_t MODER;
    uint32_t OTYPER;
    uint32_t OSPEEDR;
    uint32_t PUPDR;
    uint32_t IDR;
    uint32_t ODR;
    uint32_t BSRR;
    uint32_t LCKR;
    uint32_t AFRL;
    uint32_t AFRH;
} GpioRegisters;