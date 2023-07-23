#define NVIC_ISER0 (*(uint32_t*)0xE000E100u)
#define NVIC_ISER1 (*(uint32_t*)(0xE000E100u + 0x04))
#define NVIC_IPR4  (*(uint32_t*)(0xE000E400u + 0x04 * 4))
#define NVIC_IPR12 (*(uint32_t*)(0xE000E400u + 0x04 * 12))