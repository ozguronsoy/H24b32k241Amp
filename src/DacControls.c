#include "DacControls.h"
#include "spi.h"

// Configure the DAC unit via SPI
uint32_t ConfigureDAC()
{
    DEBUG_PRINT("DAC CONTROLS: Configuring the I2S DAC unit...\n");

    

    DEBUG_PRINT("DAC CONTROLS: Configured successfully\n");

    return RESULT_SUCCESS;
}