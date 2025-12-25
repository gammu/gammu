#include <gammu-smsd.h>

int main(int rgc, char** argv)
{
    GSM_SMSDConfig *smsd;

    smsd = SMSD_NewConfig("c++smsd");

    SMSD_FreeConfig(smsd);

    return 0;
}
