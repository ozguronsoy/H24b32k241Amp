#define RCC_CR (*(uint32_t*)(0x40023800u))
#define RCC_AHB1ENR (*(uint32_t*)(0x40023800u + 0x30u))
#define RCC_APB1ENR (*(uint32_t*)(0x40023800u + 0x40u))
#define RCC_APB2ENR (*(uint32_t*)(0x40023800u + 0x44u))
#define RCC_PLLI2SCFGR (*(uint32_t*)(0x40023800u + 0x84u))

void RCC_ConfigureSystemClock();