#include "main.h"

int main()
{
    Initialize();
    while(1)
    {
        Update();
    }
    return 0;
}

void Initialize()
{
    INIT_MONITOR_HANDLES;
    INIT_LED;

    InitializeI2S3();
    InitializeAudioControls();
}

void Update()
{
}