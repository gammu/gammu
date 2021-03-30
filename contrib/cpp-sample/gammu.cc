#include <gammu.h>

int main(int rgc, char** argv)
{
    GSM_Debug_Info *di;

    di = GSM_GetGlobalDebug();
    GSM_SetDebugLevel("none", di);

    return 0;
}
