/**
 * Simple test case just showing sizes of some structures.
 */

#include "../libgammu/gsmstate.h" /* Needed for state machine internals */

#define SIZE(name) printf("Size of " #name " = %ld (%ld MiB)\n", (long) sizeof(name), (long) sizeof(name) / (1024 * 1024));

int main(int argc UNUSED, char **argv UNUSED)
{
    SIZE(GSM_StateMachine);
    SIZE(GSM_Phone);
    SIZE(GSM_Device);
    SIZE(GSM_Protocol);
    SIZE(GSM_User);
#ifdef GSM_ENABLE_MBUS2
    SIZE(GSM_Protocol_MBUS2Data);
#endif
#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_FBUS2IRDA) || defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_DKU5FBUS2) || defined(GSM_ENABLE_FBUS2PL2303) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2)
    SIZE(GSM_Protocol_FBUS2Data);
#endif
#if defined(GSM_ENABLE_PHONETBLUE) || defined(GSM_ENABLE_IRDAPHONET) || defined(GSM_ENABLE_BLUEPHONET) || defined(GSM_ENABLE_DKU2PHONET)
    SIZE(GSM_Protocol_PHONETData);
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_BLUEAT) || defined(GSM_ENABLE_IRDAAT) || defined(GSM_ENABLE_DKU2AT)
    SIZE(GSM_Protocol_ATData);
#endif
#ifdef GSM_ENABLE_ALCABUS
    SIZE(GSM_Protocol_ALCABUSData);
#endif
#if defined(GSM_ENABLE_IRDAOBEX) || defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_ATOBEX)
    SIZE(GSM_Protocol_OBEXData);
#endif
#if defined(GSM_ENABLE_BLUEGNAPBUS) || defined(GSM_ENABLE_IRDAGNAPBUS)
    SIZE(GSM_Protocol_GNAPBUSData);
#endif
#ifdef GSM_ENABLE_NOKIA3320
    SIZE(GSM_Phone_N3320Data);
#endif
#ifdef GSM_ENABLE_NOKIA3650
    SIZE(GSM_Phone_N3650Data);
#endif
#ifdef GSM_ENABLE_NOKIA650
    SIZE(GSM_Phone_N650Data);
#endif
#ifdef GSM_ENABLE_NOKIA6110
    SIZE(GSM_Phone_N6110Data);
#endif
#ifdef GSM_ENABLE_NOKIA6510
    SIZE(GSM_Phone_N6510Data);
#endif
#ifdef GSM_ENABLE_NOKIA7110
    SIZE(GSM_Phone_N7110Data);
#endif
#ifdef GSM_ENABLE_ATGEN
    SIZE(GSM_Phone_ATGENData);
#endif
#ifdef GSM_ENABLE_ALCATEL
    SIZE(GSM_Phone_ALCATELData);
#endif
#ifdef GSM_ENABLE_ATOBEX
    SIZE(GSM_Phone_ATOBEXData);
#endif
#ifdef GSM_ENABLE_OBEXGEN
    SIZE(GSM_Phone_OBEXGENData);
#endif
#ifdef GSM_ENABLE_GNAPGEN
    SIZE(GSM_Phone_GNAPGENData);
#endif
    SIZE(GSM_Phone_DUMMYData);
    return 0;
}
