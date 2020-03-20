  /* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */
/* Phones ID (c) partially by Walek */

#include <stdarg.h>
#define _GNU_SOURCE /* For strcasestr */
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include <gammu-call.h>
#include <gammu-settings.h>
#include <gammu-unicode.h>
#include <gammu-config.h>
#include <gammu-misc.h>

#include "debug.h"
#include "gsmcomon.h"
#include "gsmphones.h"
#include "gsmstate.h"
#include "misc/coding/coding.h"
#include "misc/misc.h"
#include "device/devfunc.h"

#include "../libgammu/misc/string.h"

#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <ctype.h>

#endif

#if defined(WIN32) || defined(DJGPP)
/* Needed for SHGFP_TYPE_CURRENT */
#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif
#include <shlobj.h>

#define FALLBACK_GAMMURC "gammurc"
#define GAMMURC_NAME "\\gammurc"
#else
#define FALLBACK_GAMMURC "/etc/gammurc"
#define GAMMURC_NAME "/.gammurc"
#endif
#define XDG_GAMMURC_NAME "/gammu/config"

/* Win32 compatibility */
#ifndef PATH_MAX
#define PATH_MAX (MAX_PATH)
#endif

/* Default settings */
#if defined(WIN32) || defined(DJGPP)
#  define DEFAULT_DEVICE "com2:"
#else
#  define DEFAULT_DEVICE "/dev/ttyUSB0"
#endif
#define DEFAULT_MODEL ""
#define DEFAULT_CONNECTION "at"
#define DEFAULT_SYNCHRONIZE_TIME FALSE
#define DEFAULT_DEBUG_FILE ""
#define DEFAULT_DEBUG_LEVEL ""
#define DEFAULT_LOCK_DEVICE FALSE
#define DEFAULT_START_INFO FALSE

/**
 * Returns current debuging descriptor. It honors use_global
 * flag.
 */
GSM_Debug_Info *GSM_GetDI(GSM_StateMachine *s)
{
	GSM_Debug_Info *curdi;

	curdi = &GSM_global_debug;
	if (s != NULL && s->di.use_global == FALSE) {
		curdi = &(s->di);
	}
	return curdi;
}

static void GSM_RegisterConnection(GSM_StateMachine *s, unsigned int connection,
		GSM_Device_Functions *device, GSM_Protocol_Functions *protocol)
{
	if ((unsigned int)s->ConnectionType == connection) {
		s->Device.Functions	= device;
		s->Protocol.Functions	= protocol;
	}
}

/**
 * Information about connection names and parameters.
 */
typedef struct {
	/**
	 * Name of the connection used in config file.
	 */
	const char *Name;
	/**
	 * Connection type.
	 */
	const GSM_ConnectionType Connection;
	/**
	 * Whether to disable DTR/RTS handling on this connection.
	 */
	gboolean SkipDtrRts;
} GSM_ConnectionInfo;

/**
 * Mapping of connection names to internal identifications.
 */
static const GSM_ConnectionInfo GSM_Connections[] = {
	{"at", GCT_AT, FALSE},

	/* cables */
	{"mbus", GCT_MBUS2, FALSE},

	{"fbus", GCT_FBUS2, FALSE},

	{"fbususb", GCT_FBUS2USB, FALSE},

	{"fbuspl2303", GCT_FBUS2PL2303, FALSE},

	{"dlr3", GCT_FBUS2DLR3, FALSE},
	{"fbusdlr3", GCT_FBUS2DLR3, FALSE},

	{"dku5", GCT_DKU5FBUS2, FALSE},
	{"fbusdku5", GCT_DKU5FBUS2, FALSE},
	{"dku5fbus", GCT_DKU5FBUS2, FALSE},

	{"ark3116fbus", GCT_DKU5FBUS2, TRUE},
#ifdef WIN32
	{"dku2", GCT_DKU2PHONET, FALSE},
	{"dku2phonet", GCT_DKU2PHONET, FALSE},
#else
	{"dku2", GCT_FBUS2USB, FALSE},
	{"dku2phonet", GCT_FBUS2USB, FALSE},
#endif
	{"dku2at", GCT_DKU2AT, FALSE},

        /* for serial ports assigned by bt stack */
	{"fbusblue", GCT_FBUS2BLUE, FALSE},
	{"phonetblue", GCT_PHONETBLUE, FALSE},

	/* bt */
	{"blueobex", GCT_BLUEOBEX, FALSE},
	{"bluephonet", GCT_BLUEPHONET, FALSE},
	{"blueat", GCT_BLUEAT, FALSE},
	{"bluerfobex", GCT_BLUEOBEX, FALSE},
	{"bluefbus", GCT_BLUEFBUS2, FALSE},
	{"bluerffbus", GCT_BLUEFBUS2, FALSE},
	{"bluerfphonet", GCT_BLUEPHONET, FALSE},
	{"bluerfat", GCT_BLUEAT, FALSE},
	{"bluerfgnapbus", GCT_BLUEGNAPBUS, FALSE},
	{"blues60", GCT_BLUES60, FALSE},
	{"bluerfs60", GCT_BLUES60, FALSE},

	/* proxy */
	{"proxyobex", GCT_PROXYOBEX, FALSE},
	{"proxyphonet", GCT_PROXYPHONET, FALSE},
	{"proxyat", GCT_PROXYAT, FALSE},
	{"proxygnapbus", GCT_PROXYGNAPBUS, FALSE},
	{"proxyfbus", GCT_PROXYFBUS2, FALSE},
	{"proxys60", GCT_PROXYS60, FALSE},

	/* old "serial" irda */
	{"infrared", GCT_FBUS2IRDA, FALSE},
	{"fbusirda", GCT_FBUS2IRDA, FALSE},

	/* socket irda */
	{"irda", GCT_IRDAPHONET, FALSE},
	{"irdaphonet", GCT_IRDAPHONET, FALSE},
	{"irdaat", GCT_IRDAAT, FALSE},
	{"irdaobex", GCT_IRDAOBEX, FALSE},
	{"irdagnapbus", GCT_IRDAGNAPBUS, FALSE},

	/* testing purposes */
	{"none", GCT_NONE, FALSE},
};

GSM_Device_Functions NoneDevice = {
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION
};

GSM_Protocol_Functions NoProtocol = {
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION
};

static GSM_Error GSM_RegisterAllConnections(GSM_StateMachine *s, const char *connection)
{
	size_t i;
	char *buff, *nodtr_pos, *nopower_pos;

	/* Copy connection name, so that we can play with it */
	buff = strdup(connection);
	if (buff == NULL) {
		return ERR_MOREMEMORY;
	}

	/* We check here is used connection string type is correct for ANY
	 * OS. If not, we return with error, that string is incorrect at all
	 */
	s->ConnectionType = 0;
	s->SkipDtrRts = FALSE;
	s->NoPowerCable = FALSE;

	/* Are we asked for connection using stupid cable? */
	nodtr_pos = strcasestr(buff, "-nodtr");
	if (nodtr_pos != NULL) {
		*nodtr_pos = 0;
	}

	/* Are we asked for connection using cable which does not
	 * use DTR/RTS as power supply? */
	nopower_pos = strcasestr(buff, "-nopower");
	if (nopower_pos != NULL) {
		*nopower_pos = 0;
		s->NoPowerCable = TRUE;
	}

	/* Compare known connections to what we got */
	for (i = 0; i < sizeof(GSM_Connections) / sizeof(GSM_Connections[0]); i++) {
		/* Check connection name */
		if (strcasecmp(GSM_Connections[i].Name, buff) == 0) {
			s->ConnectionType = GSM_Connections[i].Connection;
			s->SkipDtrRts = GSM_Connections[i].SkipDtrRts;
			break;
		}
	}

	/* If we were forced, set this flag */
	if (nodtr_pos != NULL) {
		s->SkipDtrRts = TRUE;
	}

	/* Special case - at can contains speed */
	if (s->ConnectionType == 0 && strncasecmp("at", buff, 2) == 0) {
		s->Speed = FindSerialSpeed(buff + 2);
		if (s->Speed != 0) {
			s->ConnectionType = GCT_AT;
		}
	}

	/* Free allocated memory */
	free(buff);
	buff = NULL;

	if (s->ConnectionType == 0) {
		return ERR_UNKNOWNCONNECTIONTYPESTRING;
	}

	/* We check now if user gave connection type compiled & available
	 * for used OS (if not, we return, that source not available)
	 */
	s->Device.Functions	= NULL;
	s->Protocol.Functions	= NULL;
	GSM_RegisterConnection(s, GCT_NONE, 	  &NoneDevice,    &NoProtocol);
#ifdef GSM_ENABLE_MBUS2
	GSM_RegisterConnection(s, GCT_MBUS2, 	  &SerialDevice,  &MBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2
	GSM_RegisterConnection(s, GCT_FBUS2,	  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2DLR3
	GSM_RegisterConnection(s, GCT_FBUS2DLR3,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_DKU5FBUS2
	GSM_RegisterConnection(s, GCT_DKU5FBUS2,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2PL2303
	GSM_RegisterConnection(s, GCT_FBUS2PL2303,&SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2BLUE
	GSM_RegisterConnection(s, GCT_FBUS2BLUE,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2IRDA
	GSM_RegisterConnection(s, GCT_FBUS2IRDA,  &SerialDevice,  &FBUS2Protocol);
#endif
#if defined(GSM_ENABLE_DKU2PHONET) && defined(GSM_ENABLE_USBDEVICE)
	GSM_RegisterConnection(s, GCT_FBUS2USB,	  &FBUSUSBDevice,  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_DKU2PHONET
	GSM_RegisterConnection(s, GCT_DKU2PHONET, &SerialDevice,  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_DKU2AT
	GSM_RegisterConnection(s, GCT_DKU2AT,     &SerialDevice,  &ATProtocol);
#endif
#ifdef GSM_ENABLE_AT
	GSM_RegisterConnection(s, GCT_AT, 	  &SerialDevice,  &ATProtocol);
#endif
#ifdef GSM_ENABLE_PHONETBLUE
	GSM_RegisterConnection(s, GCT_PHONETBLUE, &SerialDevice,  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_IRDAGNAPBUS
	GSM_RegisterConnection(s, GCT_IRDAGNAPBUS,&IrdaDevice,    &GNAPBUSProtocol);
#endif
#ifdef GSM_ENABLE_IRDAPHONET
	GSM_RegisterConnection(s, GCT_IRDAPHONET, &IrdaDevice, 	  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_IRDAAT
	GSM_RegisterConnection(s, GCT_IRDAAT, 	  &IrdaDevice,    &ATProtocol);
#endif
#ifdef GSM_ENABLE_IRDAOBEX
	GSM_RegisterConnection(s, GCT_IRDAOBEX,   &IrdaDevice,    &OBEXProtocol);
#endif
#ifdef GSM_ENABLE_BLUEGNAPBUS
	GSM_RegisterConnection(s, GCT_BLUES60,	  &BlueToothDevice,&S60Protocol);
#endif
#ifdef GSM_ENABLE_BLUEGNAPBUS
	GSM_RegisterConnection(s, GCT_BLUEGNAPBUS,&BlueToothDevice,&GNAPBUSProtocol);
#endif
#ifdef GSM_ENABLE_BLUEFBUS2
	GSM_RegisterConnection(s, GCT_BLUEFBUS2,  &BlueToothDevice,&FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_BLUEPHONET
	GSM_RegisterConnection(s, GCT_BLUEPHONET, &BlueToothDevice,&PHONETProtocol);
#endif
#ifdef GSM_ENABLE_BLUEAT
	GSM_RegisterConnection(s, GCT_BLUEAT, 	  &BlueToothDevice,&ATProtocol);
#endif
#ifdef GSM_ENABLE_BLUEOBEX
	GSM_RegisterConnection(s, GCT_BLUEOBEX,   &BlueToothDevice,&OBEXProtocol);
#endif
#ifdef GSM_ENABLE_PROXY
#ifdef GSM_ENABLE_S60
	GSM_RegisterConnection(s, GCT_PROXYS60,	  &ProxyDevice,&S60Protocol);
#endif
#ifdef GSM_ENABLE_BLUEGNAPBUS
	GSM_RegisterConnection(s, GCT_PROXYGNAPBUS,&ProxyDevice,&GNAPBUSProtocol);
#endif
#ifdef GSM_ENABLE_FBUS2
	GSM_RegisterConnection(s, GCT_PROXYFBUS2,  &ProxyDevice,&FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_DKU2PHONET
	GSM_RegisterConnection(s, GCT_PROXYPHONET, &ProxyDevice,&PHONETProtocol);
#endif
#ifdef GSM_ENABLE_ATGEN
	GSM_RegisterConnection(s, GCT_PROXYAT, 	  &ProxyDevice,&ATProtocol);
#endif
#ifdef GSM_ENABLE_OBEXGEN
	GSM_RegisterConnection(s, GCT_PROXYOBEX,   &ProxyDevice,&OBEXProtocol);
#endif
#endif
	if (s->Device.Functions == NULL || s->Protocol.Functions == NULL) {
		smprintf(s, "Connection %s is know but was disabled on compile time\n", connection);
		return ERR_DISABLED;
	}

	return ERR_NONE;
}

static void GSM_RegisterModule(GSM_StateMachine *s,GSM_Phone_Functions *phone)
{
	/* Auto model */
	if (s->CurrentConfig->Model[0] == 0) {
		if (strstr(phone->models,GetModelData(s, NULL, s->Phone.Data.Model, NULL)->model) != NULL) {
			smprintf(s,"[Module           - \"%s\"]\n",phone->models);
			s->Phone.Functions = phone;
		}
	} else {
		if (strstr(phone->models,s->CurrentConfig->Model) != NULL) {
			smprintf(s,"[Module           - \"%s\"]\n",phone->models);
			s->Phone.Functions = phone;
		}
	}
}

/**
 * Tries to register all modules to find one matching current configuration.
 *
 * \param s State machine pointer.
 *
 * \return Error code, ERR_NONE on success.
 */
GSM_Error GSM_RegisterAllPhoneModules(GSM_StateMachine *s)
{
	GSM_PhoneModel *model;

	/* Auto model */
	if (s->CurrentConfig->Model[0] == 0) {
		model = GetModelData(s, NULL, s->Phone.Data.Model, NULL);
#ifdef GSM_ENABLE_ATGEN
		/* With ATgen and auto model we can work with unknown models too */
		if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_PROXYAT || s->ConnectionType==GCT_IRDAAT || s->ConnectionType==GCT_DKU2AT) {
#ifdef GSM_ENABLE_ALCATEL
			/* If phone provides Alcatel specific functions, enable them */
			if (model->model[0] != 0 && GSM_IsPhoneFeatureAvailable(model, F_ALCATEL)) {
				smprintf(s,"[Module           - \"%s\"]\n",ALCATELPhone.models);
				s->Phone.Functions = &ALCATELPhone;
				return ERR_NONE;
			}
#endif
#ifdef GSM_ENABLE_ATOBEX
			/* If phone provides Sony-Ericsson specific functions, enable them */
			if (model->model[0] != 0 && GSM_IsPhoneFeatureAvailable(model, F_OBEX)) {
				smprintf(s,"[Module           - \"%s\"]\n",ATOBEXPhone.models);
				s->Phone.Functions = &ATOBEXPhone;
				return ERR_NONE;
			}
#endif
			smprintf(s,"[Module           - \"%s\"]\n",ATGENPhone.models);
			s->Phone.Functions = &ATGENPhone;
			return ERR_NONE;
		}
#endif
		/* With OBEXgen and auto model we can work with unknown models too */
#ifdef GSM_ENABLE_OBEXGEN
		if (s->ConnectionType==GCT_BLUEOBEX || s->ConnectionType==GCT_PROXYOBEX || s->ConnectionType==GCT_IRDAOBEX) {
			smprintf(s,"[Module           - \"%s\"]\n",OBEXGENPhone.models);
			s->Phone.Functions = &OBEXGENPhone;
			return ERR_NONE;
		}
#endif

#ifdef GSM_ENABLE_BACKUP
		if (s->ConnectionType == GCT_NONE) {
			smprintf(s,"[Module           - \"%s\"]\n",DUMMYPhone.models);
			s->Phone.Functions = &DUMMYPhone;
			return ERR_NONE;
		}
#endif

		/* With GNAPgen and auto model we can work with unknown models too */
#ifdef GSM_ENABLE_GNAPGEN
		if (s->ConnectionType == GCT_BLUEGNAPBUS || s->ConnectionType == GCT_PROXYGNAPBUS || s->ConnectionType == GCT_IRDAGNAPBUS) {
			smprintf(s,"[Module           - \"%s\"]\n",GNAPGENPhone.models);
			s->Phone.Functions = &GNAPGENPhone;
			return ERR_NONE;
		}
#endif
#ifdef GSM_ENABLE_S60
		if (s->ConnectionType == GCT_BLUES60 || s->ConnectionType == GCT_PROXYS60) {
			smprintf(s,"[Module           - \"%s\"]\n",S60Phone.models);
			s->Phone.Functions = &S60Phone;
			return ERR_NONE;
		}
#endif

#ifdef GSM_ENABLE_NOKIA6510
		if ( s->ConnectionType ==  GCT_MBUS2 ||
				s->ConnectionType ==  GCT_FBUS2 ||
				s->ConnectionType ==  GCT_FBUS2USB ||
				s->ConnectionType ==  GCT_FBUS2DLR3 ||
				s->ConnectionType ==  GCT_FBUS2PL2303 ||
				s->ConnectionType ==  GCT_FBUS2BLUE ||
				s->ConnectionType ==  GCT_FBUS2IRDA ||
				s->ConnectionType ==  GCT_DKU5FBUS2 ||
				s->ConnectionType ==  GCT_DKU2PHONET ||
				s->ConnectionType ==  GCT_PHONETBLUE ||
				s->ConnectionType ==  GCT_IRDAPHONET ||
				s->ConnectionType ==  GCT_BLUEFBUS2 ||
				s->ConnectionType ==  GCT_PROXYFBUS2 ||
				s->ConnectionType ==  GCT_PROXYPHONET ||
				s->ConnectionType ==  GCT_BLUEPHONET) {
			/* Try to detect phone type */
			if (strcmp(model->model, "unknown") == 0 && model->features[0] == 0) {
				smprintf(s, "WARNING: phone not known, please report it to authors (see <https://wammu.eu/support/bugs/>). Thank you.\n");
				if (strncmp(s->Phone.Data.Model, "RM-", 3) == 0) {
					/* 167 is really a wild guess */
					if (atoi(s->Phone.Data.Model + 3) > 167) {
						smprintf(s, "WARNING: Guessed phone as S40/30 compatible (RM series)!\n");
						GSM_AddPhoneFeature(model, F_SERIES40_30);
						GSM_AddPhoneFeature(model, F_FILES2);
						GSM_AddPhoneFeature(model, F_TODO66);
						GSM_AddPhoneFeature(model, F_RADIO);
						GSM_AddPhoneFeature(model, F_NOTES);
						GSM_AddPhoneFeature(model, F_SMS_FILES);
						GSM_AddPhoneFeature(model, F_6230iCALLER);
					}
				}
				if (strncmp(s->Phone.Data.Model, "RH-", 3) == 0) {
					/* 63 is really a wild guess */
					if (atoi(s->Phone.Data.Model + 3) > 63) {
						smprintf(s, "WARNING: Guessed phone as S40/30 compatible (RH series)!\n");
						GSM_AddPhoneFeature(model, F_SERIES40_30);
						GSM_AddPhoneFeature(model, F_FILES2);
						GSM_AddPhoneFeature(model, F_TODO66);
						GSM_AddPhoneFeature(model, F_RADIO);
						GSM_AddPhoneFeature(model, F_NOTES);
						GSM_AddPhoneFeature(model, F_SMS_FILES);
					}
				}
			}

			/* If phone is S40, use 6510 */
			if (GSM_IsPhoneFeatureAvailable(model, F_SERIES40_30)) {
				smprintf(s,"[Module           - \"%s\"]\n", N6510Phone.models);
				s->Phone.Functions = &N6510Phone;
				return ERR_NONE;
			}
		}
#endif
		if (model->model[0] == 0) return ERR_UNKNOWNMODELSTRING;
	}
	s->Phone.Functions = NULL;
#ifdef GSM_ENABLE_ATGEN
	/* AT module can have the same models ID to "normal" Nokia modules */
	if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_PROXYAT || s->ConnectionType==GCT_BLUEAT || s->ConnectionType==GCT_IRDAAT || s->ConnectionType==GCT_DKU2AT) {
		GSM_RegisterModule(s,&ATGENPhone);
		if (s->Phone.Functions != NULL) return ERR_NONE;
	}
#endif
#ifdef GSM_ENABLE_BACKUP
	GSM_RegisterModule(s, &DUMMYPhone);
#endif
#ifdef GSM_ENABLE_OBEXGEN
	GSM_RegisterModule(s,&OBEXGENPhone);
#endif
#ifdef GSM_ENABLE_GNAPGEN
	GSM_RegisterModule(s,&GNAPGENPhone);
#endif
#ifdef GSM_ENABLE_S60
	GSM_RegisterModule(s,&S60Phone);
#endif
#ifdef GSM_ENABLE_NOKIA3320
	GSM_RegisterModule(s,&N3320Phone);
#endif
#ifdef GSM_ENABLE_NOKIA3650
	GSM_RegisterModule(s,&N3650Phone);
#endif
#ifdef GSM_ENABLE_NOKIA650
	GSM_RegisterModule(s,&N650Phone);
#endif
#ifdef GSM_ENABLE_NOKIA6110
	GSM_RegisterModule(s,&N6110Phone);
#endif
#ifdef GSM_ENABLE_NOKIA6510
	GSM_RegisterModule(s,&N6510Phone);
#endif
#ifdef GSM_ENABLE_NOKIA7110
	GSM_RegisterModule(s,&N7110Phone);
#endif
#ifdef GSM_ENABLE_NOKIA9210
	GSM_RegisterModule(s,&N9210Phone);
#endif
#ifdef GSM_ENABLE_ALCATEL
	GSM_RegisterModule(s,&ALCATELPhone);
#endif
#ifdef GSM_ENABLE_ATOBEX
	GSM_RegisterModule(s,&ATOBEXPhone);
#endif
	if (s->Phone.Functions == NULL) {
		return ERR_UNKNOWNMODELSTRING;
	}
	return ERR_NONE;
}


/**
 * Opens connection to device and initiates protocol layer.
 */
GSM_Error GSM_OpenConnection(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->CurrentConfig->LockDevice) {
		error = lock_device(s, s->CurrentConfig->Device, &(s->LockFile));
		if (error != ERR_NONE) return error;
	}

	/* Irda devices can set now model to some specific and
	 * we don't have to make auto detection later */
	error=s->Device.Functions->OpenDevice(s);
	if (error!=ERR_NONE) {
		if (s->LockFile != NULL)
			unlock_device(s, &(s->LockFile));
		return error;
	}

	s->opened = TRUE;

	error=s->Protocol.Functions->Initialise(s);
	if (error!=ERR_NONE) return error;

	return ERR_NONE;
}

/**
 * Internal function which just closes connection and cleans up structures.
 */
GSM_Error GSM_CloseConnection(GSM_StateMachine *s)
{
	GSM_Error error;

	smprintf(s, "[Closing]\n");

	/* Terminate protocol */
	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	/* Close the device */
	error = s->Device.Functions->CloseDevice(s);
	if (error != ERR_NONE) return error;

	/* Release lock if there was any */
	if (s->LockFile != NULL) {
		unlock_device(s, &(s->LockFile));
	}

	/* Null all structures in case we will be asked for new initialisation */
	s->Phone.Data.ModelInfo		  = NULL;
	s->Phone.Data.Manufacturer[0]	  = 0;
	s->Phone.Data.Model[0]		  = 0;
	s->Phone.Data.Version[0]	  = 0;
	s->Phone.Data.VerDate[0]	  = 0;
	s->Phone.Data.VerNum		  = 0;

	return ERR_NONE;
}


/**
 * Tries to read model using configured phone connection.
 */
GSM_Error GSM_TryGetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	error = GSM_OpenConnection(s);
	if (error != ERR_NONE) return error;

	/* If still auto model, try to get model by asking phone for it */
	if (s->Phone.Data.Model[0]==0) {
		smprintf(s,"[Module           - \"auto\"]\n");
		switch (s->ConnectionType) {
#ifdef GSM_ENABLE_BACKUP
			case GCT_NONE:
				s->Phone.Functions = &DUMMYPhone;
				break;
#endif
#ifdef GSM_ENABLE_ATGEN
			case GCT_AT:
			case GCT_BLUEAT:
			case GCT_PROXYAT:
			case GCT_IRDAAT:
			case GCT_DKU2AT:
				s->Phone.Functions = &ATGENPhone;
				break;
#endif
#ifdef GSM_ENABLE_OBEXGEN
			case GCT_IRDAOBEX:
			case GCT_PROXYOBEX:
			case GCT_BLUEOBEX:
				s->Phone.Functions = &OBEXGENPhone;
				break;
#endif
#ifdef GSM_ENABLE_GNAPGEN
			case GCT_BLUEGNAPBUS:
			case GCT_PROXYGNAPBUS:
			case GCT_IRDAGNAPBUS:
				s->Phone.Functions = &GNAPGENPhone;
				break;
#endif
#ifdef GSM_ENABLE_S60
			case GCT_BLUES60:
			case GCT_PROXYS60:
				s->Phone.Functions = &S60Phone;
				break;
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
			case GCT_MBUS2:
			case GCT_FBUS2:
			case GCT_FBUS2USB:
			case GCT_FBUS2DLR3:
			case GCT_FBUS2PL2303:
			case GCT_FBUS2BLUE:
			case GCT_FBUS2IRDA:
			case GCT_DKU5FBUS2:
			case GCT_DKU2PHONET:
			case GCT_PHONETBLUE:
			case GCT_IRDAPHONET:
			case GCT_BLUEFBUS2:
			case GCT_PROXYFBUS2:
			case GCT_BLUEPHONET:
			case GCT_PROXYPHONET:
				s->Phone.Functions = &NAUTOPhone;
				break;
#endif
			default:
				s->Phone.Functions = NULL;
		}
		/* Did we find matching phone driver? */
		if (s->Phone.Functions == NULL) {
			smprintf(s, "ERROR: Could not find proper module for autodetection!\n");
			return ERR_UNKNOWN;
		}

		/* Initialize the phone driver */
		error = s->Phone.Functions->Initialise(s);
		if (error != ERR_NONE) return error;

		/* Get model name from phone */
		error = s->Phone.Functions->GetModel(s);
		if (error != ERR_NONE) return error;

		/* And terminate it again */
		error = s->Phone.Functions->Terminate(s);
		if (error != ERR_NONE) return error;
	}
	return ERR_NONE;
}

GSM_Error GSM_InitConnection_Log(GSM_StateMachine *s, int ReplyNum, GSM_Log_Function log_function, void *user_data)
{
	GSM_Error	error;
	GSM_DateTime	current_time;
	int		i;

	for (i=0;i<s->ConfigNum;i++) {
		s->CurrentConfig		  = &s->Config[i];

		/* Skip non configured sections */
		if (s->CurrentConfig->Connection == NULL) {
			smprintf_level(s, D_ERROR, "[Empty section    - %d]\n", i);
			continue;
		}

		s->Speed			  = 0;
		s->ReplyNum			  = ReplyNum;
		s->Phone.Data.ModelInfo		  = GetModelData(s, "unknown", NULL, NULL);
		s->Phone.Data.Manufacturer[0]	  = 0;
		s->Phone.Data.Model[0]		  = 0;
		s->Phone.Data.Version[0]	  = 0;
		s->Phone.Data.VerDate[0]	  = 0;
		s->Phone.Data.VerNum		  = 0;
		s->Phone.Data.StartInfoCounter	  = 0;
		s->Phone.Data.SentMsg		  = NULL;

		s->Phone.Data.HardwareCache[0]	  = 0;
		s->Phone.Data.ProductCodeCache[0] = 0;
		s->Phone.Data.EnableIncomingCall  = FALSE;
		s->Phone.Data.EnableIncomingSMS	  = FALSE;
		s->Phone.Data.EnableIncomingCB	  = FALSE;
		s->Phone.Data.EnableIncomingUSSD  = FALSE;
		s->User.UserReplyFunctions	  = NULL;
		s->User.IncomingCall		  = NULL;
		s->User.IncomingSMS		  = NULL;
		s->User.IncomingCB		  = NULL;
		s->User.IncomingUSSD		  = NULL;
		s->User.SendSMSStatus		  = NULL;
		s->LockFile			  = NULL;
		s->opened			  = FALSE;
		s->Phone.Functions		  = NULL;

		s->di 				  = GSM_none_debug;
		s->di.use_global 		  = s->CurrentConfig->UseGlobalDebugFile;
		if (!s->di.use_global) {
			GSM_SetDebugFunction(log_function, user_data, &s->di);
			GSM_SetDebugLevel(s->CurrentConfig->DebugLevel, &s->di);
			error = GSM_SetDebugFile(s->CurrentConfig->DebugFile, &s->di);
			if (error != ERR_NONE) {
				GSM_LogError(s, "Init:GSM_SetDebugFile" , error);
				return error;
			}
		}

		smprintf_level(s, D_ERROR, "[Gammu            - %s]\n", GAMMU_VERSION);
		StripSpaces(s->CurrentConfig->Connection);
		StripSpaces(s->CurrentConfig->Model);
		StripSpaces(s->CurrentConfig->Device);
		smprintf_level(s, D_ERROR, "[Connection       - \"%s\"]\n",
				s->CurrentConfig->Connection);
		smprintf_level(s, D_ERROR, "[Connection index - %d]\n", i);
		smprintf_level(s, D_ERROR, "[Model type       - \"%s\"]\n",
				s->CurrentConfig->Model);
		smprintf_level(s, D_ERROR, "[Device           - \"%s\"]\n",
				s->CurrentConfig->Device);
		if (strlen(GetOS()) != 0) {
			smprintf_level(s, D_ERROR, "[Running on       - %s]\n",
					GetOS());
		}

		if (GSM_GetDI(s)->dl == DL_BINARY) {
			smprintf(s,"%c",((unsigned char)strlen(GAMMU_VERSION)));
			smprintf(s,"%s",GAMMU_VERSION);
		}

		error = GSM_RegisterAllConnections(s, s->CurrentConfig->Connection);
		if (error != ERR_NONE) {
			GSM_LogError(s, "Init:GSM_RegisterAllConnections" , error);
			return error;
		}

autodetect:
		/* Model auto */
		/* Try to guess correct driver based on model */
		if (s->CurrentConfig->Model[0] == 0 &&
				s->ConnectionType != GCT_NONE &&
				s->ConnectionType != GCT_IRDAOBEX &&
				s->ConnectionType != GCT_BLUEOBEX &&
				s->ConnectionType != GCT_PROXYOBEX &&
				s->ConnectionType != GCT_BLUEGNAPBUS &&
				s->ConnectionType != GCT_PROXYGNAPBUS &&
				s->ConnectionType != GCT_IRDAGNAPBUS &&
				s->ConnectionType != GCT_PROXYS60 &&
				s->ConnectionType != GCT_BLUES60) {
			error = GSM_TryGetModel(s);
			/* Fall back to other configuraitons if the device is not existing (or similar error) */
			if ((i != s->ConfigNum - 1) && (
				(error == ERR_DEVICEOPENERROR) ||
				(error == ERR_DEVICELOCKED) ||
				(error == ERR_DEVICENOTEXIST) ||
				(error == ERR_DEVICEBUSY) ||
				(error == ERR_DEVICENOPERMISSION) ||
				(error == ERR_DEVICENODRIVER) ||
				(error == ERR_DEVICENOTWORK))) {
				GSM_CloseConnection(s);
				continue;
			}
			if (error != ERR_NONE) {
				GSM_LogError(s, "Init:GSM_TryGetModel" , error);
				return error;
			}
		}

		/* Switching to "correct" module */
		error = GSM_RegisterAllPhoneModules(s);
		/* If user selected soemthing which is not supported, try autodetection */
		if (s->CurrentConfig->Model[0] != 0 && error == ERR_UNKNOWNMODELSTRING) {
			smprintf(s, "Configured model %s is not known, retrying with autodetection!\n",
					s->CurrentConfig->Model);
			s->CurrentConfig->Model[0] = 0;
			goto autodetect;
		}
		if (error != ERR_NONE) {
			GSM_LogError(s, "Init:GSM_RegisterAllPhoneModules" , error);
			return error;
		}

		/* We didn't open device earlier ? Make it now */
		if (!s->opened) {
			error = GSM_OpenConnection(s);
			if ((i != s->ConfigNum - 1) && (
				(error == ERR_DEVICEOPENERROR) ||
				(error == ERR_DEVICELOCKED) ||
				(error == ERR_DEVICENOTEXIST) ||
				(error == ERR_DEVICEBUSY) ||
				(error == ERR_DEVICENOPERMISSION) ||
				(error == ERR_DEVICENODRIVER) ||
				(error == ERR_DEVICENOTWORK))) {
				GSM_CloseConnection(s);
				continue;
			}
			if (error != ERR_NONE) {
				GSM_LogError(s, "Init:GSM_OpenConnection" , error);
				return error;
			}
		}

		/* Initialize phone layer */
		error=s->Phone.Functions->Initialise(s);
		if (error == ERR_TIMEOUT && i != s->ConfigNum - 1) {
			GSM_CloseConnection(s);
			continue;
		}
		if (error != ERR_NONE) {
			GSM_LogError(s, "Init:Phone->Initialise" , error);
			return error;
		}

		if (s->CurrentConfig->StartInfo) {
			s->Phone.Functions->ShowStartInfo(s,TRUE);
			s->Phone.Data.StartInfoCounter = 30;
		}

		if (s->CurrentConfig->SyncTime) {
			GSM_GetCurrentDateTime (&current_time);
			s->Phone.Functions->SetDateTime(s,&current_time);
		}

		/* For debug it's good to have firmware and real model version and manufacturer */
		error=s->Phone.Functions->GetManufacturer(s);
		if (error == ERR_TIMEOUT && i != s->ConfigNum - 1) {
			GSM_CloseConnection(s);
			continue;
		}
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			GSM_LogError(s, "Init:Phone->GetManufacturer" , error);
			return error;
		}

		error=s->Phone.Functions->GetModel(s);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			GSM_LogError(s, "Init:Phone->GetModel" , error);
			return error;
		}

		error=s->Phone.Functions->GetFirmware(s);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			GSM_LogError(s, "Init:Phone->GetFirmware" , error);
			return error;
		}

		error=s->Phone.Functions->SetPower(s, 1);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			GSM_LogError(s, "Init:Phone->SetPower" , error);
			return error;
		}

		error=s->Phone.Functions->PostConnect(s);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			GSM_LogError(s, "Init:Phone->PostConnect" , error);
			return error;
		}

		smprintf(s,"[Connected]\n");
		return ERR_NONE;
	}
	return ERR_UNCONFIGURED;
}

GSM_Error GSM_InitConnection(GSM_StateMachine *s, int ReplyNum)
{
	return GSM_InitConnection_Log(s, ReplyNum, GSM_none_debug.log_function, GSM_none_debug.user_data);
}

int GSM_ReadDevice (GSM_StateMachine *s, gboolean waitforreply)
{
	GSM_DateTime	Date;
	unsigned char	buff[65536]={'\0'};
	int		res=0,count=0,i=0;

	if (!GSM_IsConnected(s)) {
		return -1;
	}

	GSM_GetCurrentDateTime (&Date);
	i = Date.Second;
	while (i == Date.Second && !s->Abort) {
		res = s->Device.Functions->ReadDevice(s, buff, sizeof(buff));

		if (!waitforreply) {
			break;
		}
		if (res > 0) {
			break;
		}
		usleep(5000);
		GSM_GetCurrentDateTime(&Date);
	}
	for (count = 0; count < res; count++) {
		s->Protocol.Functions->StateMachine(s, buff[count]);
	}
	return res;
}

GSM_Error GSM_TerminateConnection(GSM_StateMachine *s)
{
	GSM_Error error;

	if (!s->opened) return ERR_NONE;

	smprintf(s,"[Terminating]\n");

	if (s->CurrentConfig->StartInfo) {
		if (s->Phone.Data.StartInfoCounter > 0) s->Phone.Functions->ShowStartInfo(s,FALSE);
	}

	if (s->Phone.Functions != NULL) {
		error=s->Phone.Functions->Terminate(s);
		if (error!=ERR_NONE) return error;
	}

	error = GSM_CloseConnection(s);
	if (error != ERR_NONE) return error;

	GSM_SetDebugFileDescriptor(NULL, FALSE, &(s->di));

	s->opened = FALSE;

	return ERR_NONE;
}

gboolean GSM_IsConnected(GSM_StateMachine *s) {
	return (s != NULL) && s->Phone.Functions != NULL && s->opened;
}

GSM_Error GSM_AbortOperation(GSM_StateMachine * s)
{
	s->Abort = TRUE;
	return ERR_NONE;
}

GSM_Error GSM_WaitForOnce(GSM_StateMachine *s, unsigned const char *buffer,
			  size_t length, int type, int timeout)
{
	GSM_Phone_Data *Phone = &s->Phone.Data;
	GSM_Protocol_Message sentmsg;
	int i = 0;

	do {
		if (length != 0) {
			sentmsg.Length 	= length;
			sentmsg.Type	= type;
			sentmsg.Buffer 	= (unsigned char *)malloc(length);
			memcpy(sentmsg.Buffer, buffer, length);
			Phone->SentMsg  = &sentmsg;
		}

		/* Some data received. Reset timer */
		if (GSM_ReadDevice(s, TRUE) > 0) {
			i = 0;
		} else {
			usleep(10000);
		}

		if (length != 0) {
			free(sentmsg.Buffer);
			sentmsg.Buffer = NULL;
			Phone->SentMsg = NULL;
		}

		if (s->Abort) {
			return ERR_ABORTED;
		}

		/* Request completed */
		if (Phone->RequestID == ID_None) {
			return Phone->DispatchError;
		}
		i++;
	} while (i < timeout);

	return ERR_TIMEOUT;
}

GSM_Error GSM_WaitFor (GSM_StateMachine *s, unsigned const char *buffer,
		       size_t length, int type, int timeout,
		       GSM_Phone_RequestID request)
{
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	GSM_Error		error;
	int			reply;

	if (s->CurrentConfig->StartInfo) {
		if (Phone->StartInfoCounter > 0) {
			Phone->StartInfoCounter--;
			if (Phone->StartInfoCounter == 0) {
				s->Phone.Functions->ShowStartInfo(s,FALSE);
			}
		}
	}

	Phone->RequestID	= request;
	Phone->DispatchError	= ERR_TIMEOUT;

	for (reply = 0; reply < s->ReplyNum; reply++) {
		if (reply != 0) {
			smprintf_level(s, D_ERROR, "[Retrying %i type 0x%02X]\n", reply, type);
		}
		error = s->Protocol.Functions->WriteMessage(s, buffer, length, type);
		if (error != ERR_NONE) {
			return error;
		}

		/* Special case when no reply is expected */
		if (request == ID_None) {
			return ERR_NONE;
		}

		error = GSM_WaitForOnce(s, buffer, length, type, timeout);
		if (error != ERR_TIMEOUT) {
			return error;
		}
  }

	if (request != ID_Reset && GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RESET_AFTER_TIMEOUT)) {
		smprintf_level(s, D_ERROR, "Performing device reset after timeout!\n");
		GSM_Reset(s, FALSE);
	}

	return ERR_TIMEOUT;
}

static GSM_Error CheckReplyFunctions(GSM_StateMachine *s, GSM_Reply_Function *Reply, int *reply)
{
	GSM_Phone_Data			*Data	  = &s->Phone.Data;
	GSM_Protocol_Message		*msg	  = s->Phone.Data.RequestMsg;
	gboolean				execute;
	gboolean				available = FALSE;
	int				i	  = 0;

	while (Reply[i].requestID != ID_None) {
		execute = FALSE;
		/* Long ID frames like S60 */
		if (Reply[i].msgtype[0] == 0 && Reply[i].subtypechar == 0) {
			if (Reply[i].subtype == msg->Type) {
				execute = TRUE;
			}
		/* Binary frames like in Nokia */
		} else if (strlen(Reply[i].msgtype) < 2) {
			if (Reply[i].msgtype[0]==msg->Type) {
				if (Reply[i].subtypechar!=0) {
					if (Reply[i].subtypechar<=msg->Length) {
						if (msg->Buffer[Reply[i].subtypechar]==Reply[i].subtype)
							execute=TRUE;
					}
				} else {
					execute=TRUE;
				}
			}
		} else {
			if (strlen(Reply[i].msgtype) < msg->Length) {
				if (strncmp(Reply[i].msgtype,msg->Buffer,strlen(Reply[i].msgtype))==0) {
					execute=TRUE;
				}
			}
		}

		if (execute) {
			*reply = i;
			if (Reply[i].requestID == ID_IncomingFrame ||
			    Reply[i].requestID == Data->RequestID ||
			    Data->RequestID    == ID_EachFrame) {
				return ERR_NONE;
			}
			available = TRUE;
		}
		i++;
	}

	if (available) {
		return ERR_FRAMENOTREQUESTED;
	} else {
		return ERR_UNKNOWNFRAME;
	}
}

GSM_Error EventQueue_Push(GSM_StateMachine *s, const EventBinding *binding)
{
  DeferredEventQueue *Queue = &s->Phone.Data.DeferredEvents;

  assert(binding != NULL);
  assert(Queue->head < MAX_DEFERRED_EVENTS);

  if(Queue->entries == MAX_DEFERRED_EVENTS)
    return ERR_FULL;

  Queue->event_bindings[Queue->head] = *binding;
  Queue->head = (Queue->head + 1) % MAX_DEFERRED_EVENTS;
  ++Queue->entries;

  assert(Queue->entries <= MAX_DEFERRED_EVENTS);

  return ERR_NONE;
}

GSM_Error EventQueue_Pop(GSM_StateMachine *s, EventBinding *binding)
{
  DeferredEventQueue *Queue = &s->Phone.Data.DeferredEvents;

  assert(binding != NULL);

  if(Queue->entries == 0)
    return ERR_EMPTY;

  *binding = Queue->event_bindings[Queue->tail];
  Queue->tail = (Queue->tail + 1) % MAX_DEFERRED_EVENTS;
  --Queue->entries;

  assert(Queue->entries >= 0);

  return ERR_NONE;
}

void GSM_CancelEventsOfType(GSM_StateMachine *s, unsigned event_types)
{
  DeferredEventQueue *q = &s->Phone.Data.DeferredEvents;
  int i = q->tail;

  while(i != q->head) {
    if(q->event_bindings[i].type & event_types)
      q->event_bindings[i].event_cancelled = TRUE;

    i = (i + 1) % MAX_DEFERRED_EVENTS;
  }
}

GSM_Error GSM_DeferIncomingCallEvent(GSM_StateMachine *s, GSM_Call *call, BeforeDeferredEvent before_event)
{
  GSM_Error error;
  EventBinding binding;

  if(s->Phone.Data.RequestID == ID_None) {
    s->User.IncomingCall(s, call, s->User.IncomingCallUserData);
    return ERR_NONE;
  }

  binding.type = GSM_EV_CALL;
  binding.handler = (EventHandler)s->User.IncomingCall;
  binding.before_event = before_event;
  binding.after_event = NULL;
  binding.event_cancelled = FALSE;
  binding.event_data.call = *call;
  binding.user_data = s->User.IncomingCallUserData;

  error = EventQueue_Push(s, &binding);

  if(error != ERR_NONE)
    smprintf_level(s, D_ERROR,
      "the incoming call handler could not be deferred.\n");

  return error;
}

GSM_Error ProcessDeferredEvent(GSM_StateMachine *s)
{
  EventBinding binding;
  GSM_Error error = EventQueue_Pop(s, &binding);

  if(error != ERR_NONE)
    return error;

  assert(s->Phone.Data.RequestID == ID_None);
  assert(binding.handler != NULL);
  assert(binding.type != GSM_EV_UNSET);

  if(binding.event_cancelled == FALSE) {
    if (binding.before_event)
      error = binding.before_event(s);

    if(error == ERR_NONE)
      binding.handler(s, &binding.event_data, binding.user_data);
  }

  if(binding.after_event)
    binding.after_event(s, &binding);

  return error;
}

GSM_Error GSM_DispatchMessage(GSM_StateMachine *s)
{
	GSM_Error		error	= ERR_UNKNOWNFRAME;
	GSM_Protocol_Message	*msg 	= s->Phone.Data.RequestMsg;
	GSM_Phone_Data 		*Phone	= &s->Phone.Data;
	gboolean			disp    = FALSE;
	GSM_Reply_Function	*Reply;
	int			reply;

	s->MessagesCount++;

	GSM_DumpMessageTextRecv(s, msg->Buffer, msg->Length, msg->Type);
	GSM_DumpMessageBinaryRecv(s, msg->Buffer, msg->Length, msg->Type);

	Reply = s->User.UserReplyFunctions;
	if (Reply != NULL) {
		error = CheckReplyFunctions(s, Reply, &reply);
	}

	if (error == ERR_UNKNOWNFRAME) {
		Reply = s->Phone.Functions->ReplyFunctions;
		error = CheckReplyFunctions(s, Reply, &reply);
	}

	if (error == ERR_NONE) {
		error = Reply[reply].Function(msg, s);
		if (Reply[reply].requestID == Phone->RequestID) {
			if (error == ERR_NEEDANOTHERANSWER) {
				error = ERR_NONE;
			} else {
				Phone->RequestID = ID_None;
        while(ProcessDeferredEvent(s) == ERR_NONE);
			}
		}
	}

	if (strcmp(s->Phone.Functions->models,"NAUTO")) {
		disp = TRUE;
		switch (error) {
		case ERR_UNKNOWNRESPONSE:
			smprintf_level(s, D_ERROR, "\nUNKNOWN response");
			break;
		case ERR_UNKNOWNFRAME:
			smprintf_level(s, D_ERROR, "\nUNKNOWN frame");
			break;
		case ERR_FRAMENOTREQUESTED:
			smprintf_level(s, D_ERROR, "\nFrame not request now");
			break;
		default:
			disp = FALSE;
		}

		if (error == ERR_UNKNOWNFRAME || error == ERR_FRAMENOTREQUESTED) {
			error = ERR_TIMEOUT;
		}
	}

	if (disp) {
		smprintf(s,". Please report the error, see <https://wammu.eu/support/bugs/>. Thank you\n");
		if (Phone->SentMsg != NULL) {
			smprintf(s,"LAST SENT frame ");
			smprintf(s, "type 0x%02X/length %ld", Phone->SentMsg->Type, (long)Phone->SentMsg->Length);
			DumpMessage(GSM_GetDI(s), Phone->SentMsg->Buffer, Phone->SentMsg->Length);
		}
		smprintf(s, "RECEIVED frame ");
		smprintf(s, "type 0x%02X/length 0x%lx/%ld", msg->Type, (long)msg->Length, (long)msg->Length);
		DumpMessage(GSM_GetDI(s), msg->Buffer, msg->Length);
		smprintf(s, "\n");
	}

	return error;
}

GSM_Error GSM_TryReadGammuRC (const char *path, INI_Section **result)
{
	dbgprintf(NULL, "Open config: \"%s\"\n", path);
	return  INI_ReadFile(path, FALSE, result);
}

GSM_Error GSM_FindGammuRC (INI_Section **result, const char *force_config)
{
	char configfile[PATH_MAX + 1];
	char *envpath;
	GSM_Error error;
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	struct passwd *pwent;
#endif

	*result = NULL;

	if (force_config != NULL) {
		return GSM_TryReadGammuRC(force_config, result);
	}

#ifdef WIN32
	/* Get Windows application data path */
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, configfile))) {
		strcat(configfile, GAMMURC_NAME);

		error = GSM_TryReadGammuRC(configfile, result);
		if (error == ERR_NONE) return ERR_NONE;
	}
#endif

	/* XDG paths */
	envpath = getenv("XDG_CONFIG_HOME");
	if (envpath) {
		strcpy(configfile, envpath);
		strcat(configfile, XDG_GAMMURC_NAME);

		error = GSM_TryReadGammuRC(configfile, result);
		if (error == ERR_NONE) return ERR_NONE;
	} else {
		envpath  = getenv("HOME");
		if (envpath) {
			strcpy(configfile, envpath);
			strcat(configfile, "/.config");
			strcat(configfile, XDG_GAMMURC_NAME);

			error = GSM_TryReadGammuRC(configfile, result);
			if (error == ERR_NONE) return ERR_NONE;
		}
	}

	/* Try home from environment */
	envpath  = getenv("HOME");
	if (envpath) {
		strcpy(configfile, envpath);
		strcat(configfile, GAMMURC_NAME);

		error = GSM_TryReadGammuRC(configfile, result);
		if (error == ERR_NONE) return ERR_NONE;
	}

#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	/* Tru home from passwd */
	pwent = getpwuid(getuid());
	if (pwent != NULL) {
		strcpy(configfile, pwent->pw_dir);
		strcat(configfile, GAMMURC_NAME);

		error = GSM_TryReadGammuRC(configfile, result);
		if (error == ERR_NONE) return ERR_NONE;
	}

#endif

#if defined(WIN32)
	/* Reset as we're using strcat */
	configfile[0] = 0;

	/* This makes sense only on Windows */
	envpath = getenv("HOMEDRIVE");
	if (envpath) {
		strcat(configfile, envpath);
	}

	envpath = getenv("HOMEPATH");
	if (envpath) {
		strcat(configfile, envpath);
		strcat(configfile, GAMMURC_NAME);

		error = GSM_TryReadGammuRC(configfile, result);
		if (error == ERR_NONE) return ERR_NONE;
	}
#endif

	/* Try fallback config as last */
	return GSM_TryReadGammuRC(FALLBACK_GAMMURC, result);
}

GSM_Config *GSM_GetConfig(GSM_StateMachine *s, int num)
{
	if (num == -1) {
		return s->CurrentConfig;
	} else {
		if (num > MAX_CONFIG_NUM) return NULL;
		return &(s->Config[num]);
	}
}


int GSM_GetConfigNum(const GSM_StateMachine *s)
{
	return s->ConfigNum;
}

void GSM_SetConfigNum(GSM_StateMachine *s, int sections)
{
	if (sections > MAX_CONFIG_NUM) return;
	s->ConfigNum = sections;
}

/**
 * Expand path to user home.
 */
void GSM_ExpandUserPath(char **string)
{
	char *tmp = NULL, *home = NULL;

	/* Is there something to expand */
	if (*string[0] != '~') return;

	/* Grab home */
	home = getenv("HOME");
	if (home == NULL) return;

	/* Allocate memory */
	tmp = (char *)malloc(strlen(home) + strlen(*string) + 2);
	if (tmp == NULL) return;

	/* Create final path */
	strcpy(tmp, home);
	strcat(tmp, *string + 1);

	/* Free old storage and replace it */
	free(*string);
	*string = tmp;
}

/**
 * Parses an ASCII string of comma delimited single digits into a set of integer
 * parameters, any whitespace is ignored.
 *
 * @param out_params array to receive the parsed parameters
 * @param num_params number of parameters out_params can hold
 * @param args string containing list of parameters to parse
 * @return ERR_NONE on success, otherwise ERR_INVALIDDATA
 */
GSM_Error GSM_ReadParams(int *out_params, const int num_params, const char *args)
{
  int *params_ptr = out_params;
  const int *params_end = out_params + num_params * sizeof(int);
  const char *args_ptr = args;
  int whitespace_count = 0;
  gboolean expect_comma = 0;

  if (!args)
    return ERR_NONE;

  while (params_ptr < params_end) {
    while (isspace((unsigned char)*args_ptr)) {
      ++whitespace_count;
      ++args_ptr;
    }

    if(!*args_ptr)
      return ERR_NONE;

    switch (*args_ptr) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        if (expect_comma) {
          printf("expected comma but got %c for parameter %d\n", *args_ptr, (int)(++params_ptr - out_params));
          return ERR_INVALIDDATA;
        }
        *params_ptr = *args_ptr - '0';
        expect_comma = TRUE;
        break;

      case ',':
        ++params_ptr;
        expect_comma = FALSE;
        break;

      default: {
        printf("error parsing parameters, unrecognized token '%c' in position %d\n",
               *args_ptr, (int)(++params_ptr - --out_params + ++whitespace_count));
        return ERR_INVALIDDATA; }
    }

    ++args_ptr;
  }

  return ERR_NONE;
}

GSM_Error GSM_ReadCNMIParams(int out_params[5], const char *args)
{
  return GSM_ReadParams(out_params, 5, args);
}

GSM_Error GSM_ReadConfig(INI_Section *cfg_info, GSM_Config *cfg, int num)
{
	INI_Section 	*h;
	unsigned char 	section[50]={0};
	gboolean	found = FALSE;
	char		*Temp = NULL;
	const int cnmi_default[5] = {-1,-1,-1,-1,-1};

	GSM_Error error = ERR_UNKNOWN;
	cfg->UseGlobalDebugFile	 = TRUE;
	memcpy(cfg->CNMIParams, &cnmi_default, sizeof(cfg->CNMIParams));

	/* If we don't have valid config, bail out */
	if (cfg_info == NULL) {
		error = ERR_UNCONFIGURED;
		goto fail;
	}

	/* Which section should we read? */
	if (num == 0) {
		snprintf(section, sizeof(section) - 1, "gammu");
	} else {
		snprintf(section, sizeof(section) - 1, "gammu%i", num);
	}

	/* Scan for section */
        for (h = cfg_info; h != NULL; h = h->Next) {
                if (strcasecmp(section, h->SectionName) == 0) {
			found = TRUE;
			break;
		}
        }
	if (!found) {
		error = ERR_NONE_SECTION;
		goto fail;
	}

	/* Set device name */
	free(cfg->Device);
	cfg->Device 	 = INI_GetValue(cfg_info, section, "device", 		FALSE);
	if (!cfg->Device) {
		cfg->Device 	 = INI_GetValue(cfg_info, section, "port", 		FALSE);
		if (!cfg->Device) {
			cfg->Device		 	 = strdup(DEFAULT_DEVICE);
		} else {
			cfg->Device			 = strdup(cfg->Device);
		}
	} else {
		cfg->Device			 = strdup(cfg->Device);
	}

	/* Set connection type */
	free(cfg->Connection);
	cfg->Connection  = INI_GetValue(cfg_info, section, "connection", 	FALSE);
	if (cfg->Connection == NULL) {
		cfg->Connection	 		 = strdup(DEFAULT_CONNECTION);
	} else {
		cfg->Connection			 = strdup(cfg->Connection);
	}

	/* Set time sync */
	cfg->SyncTime = INI_GetBool(cfg_info, section, "synchronizetime", DEFAULT_SYNCHRONIZE_TIME);

	/* Set debug file */
	free(cfg->DebugFile);
	cfg->DebugFile   = INI_GetValue(cfg_info, section, "logfile", 		FALSE);
	if (cfg->DebugFile == NULL) {
		cfg->DebugFile		 	 = strdup(DEFAULT_DEBUG_FILE);
	} else {
		cfg->DebugFile			 = strdup(cfg->DebugFile);
		GSM_ExpandUserPath(&cfg->DebugFile);
	}

	/* Set file locking */
	cfg->LockDevice  = INI_GetBool(cfg_info, section, "use_locking", DEFAULT_LOCK_DEVICE);

	/* Set model */
	Temp		 = INI_GetValue(cfg_info, section, "model", 		FALSE);
	if (Temp == NULL || strcmp(Temp, "auto") == 0) {
		strcpy(cfg->Model,DEFAULT_MODEL);
	} else {
		if (strlen(Temp) >= sizeof(cfg->Model))
			Temp[sizeof(cfg->Model) - 1] = '\0';
		strcpy(cfg->Model,Temp);
	}

	/* Set Log format */
	Temp		 = INI_GetValue(cfg_info, section, "logformat", 	FALSE);

	if (Temp == NULL) {
		strcpy(cfg->DebugLevel,DEFAULT_DEBUG_LEVEL);
	} else {
		if (strlen(Temp) >= sizeof(cfg->DebugLevel))
			Temp[sizeof(cfg->DebugLevel) - 1] = '\0';
		strcpy(cfg->DebugLevel,Temp);
	}

	/* Set startup info */
	cfg->StartInfo = INI_GetBool(cfg_info, section, "startinfo", DEFAULT_START_INFO);

	/* Read localised strings for some phones */

	Temp		 = INI_GetValue(cfg_info, section, "reminder", 		FALSE);

	if (Temp == NULL) {
		strcpy(cfg->TextReminder,"Reminder");
	} else {
		if (strlen(Temp) >= sizeof(cfg->TextReminder))
			Temp[sizeof(cfg->TextReminder) - 1] = '\0';
		strcpy(cfg->TextReminder,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "meeting", 		FALSE);

	if (Temp == NULL) {
		strcpy(cfg->TextMeeting,"Meeting");
	} else {
		if (strlen(Temp) >= sizeof(cfg->TextMeeting))
			Temp[sizeof(cfg->TextMeeting) - 1] = '\0';
		strcpy(cfg->TextMeeting,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "call", 		FALSE);

	if (Temp == NULL) {
		strcpy(cfg->TextCall,"Call");
	} else {
		if (strlen(Temp) >= sizeof(cfg->TextCall))
			Temp[sizeof(cfg->TextCall) - 1] = '\0';
		strcpy(cfg->TextCall,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "birthday", 		FALSE);

	if (Temp == NULL) {
		strcpy(cfg->TextBirthday,"Birthday");
	} else {
		if (strlen(Temp) >= sizeof(cfg->TextBirthday))
			Temp[sizeof(cfg->TextBirthday) - 1] = '\0';
		strcpy(cfg->TextBirthday,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "memo", 		FALSE);

	if (Temp == NULL) {
		strcpy(cfg->TextMemo,"Memo");
	} else {
		if (strlen(Temp) >= sizeof(cfg->TextMemo))
			Temp[sizeof(cfg->TextMemo) - 1] = '\0';
		strcpy(cfg->TextMemo,Temp);
	}

	/* Phone features */
	Temp		 = INI_GetValue(cfg_info, section, "features", 		FALSE);

	if (Temp == NULL) {
		cfg->PhoneFeatures[0] = 0;
	} else {
		error = GSM_SetFeatureString(cfg->PhoneFeatures, Temp);

		if (error != ERR_NONE) {
			goto fail;
		}
	}

	Temp = INI_GetValue(cfg_info, section, "atgen_setcnmi", FALSE);
	if (Temp) {
    error = GSM_ReadCNMIParams(cfg->CNMIParams, Temp);
    if (error != ERR_NONE)
      goto fail;
  }

  return ERR_NONE;

fail:
	/* Special case, this config needs to be somehow valid */
	if (num == 0) {
		cfg->Device		 	 = strdup(DEFAULT_DEVICE);
		cfg->Connection	 		 = strdup(DEFAULT_CONNECTION);
		cfg->SyncTime		 	 = DEFAULT_SYNCHRONIZE_TIME;
		cfg->DebugFile		 	 = strdup(DEFAULT_DEBUG_FILE);
		cfg->LockDevice	 		 = DEFAULT_LOCK_DEVICE;
		strcpy(cfg->Model,DEFAULT_MODEL);
		strcpy(cfg->DebugLevel,DEFAULT_DEBUG_LEVEL);
		cfg->StartInfo	 		 = DEFAULT_START_INFO;
		strcpy(cfg->TextReminder,"Reminder");
		strcpy(cfg->TextMeeting,"Meeting");
		strcpy(cfg->TextCall,"Call");
		strcpy(cfg->TextBirthday,"Birthday");
		strcpy(cfg->TextMemo,"Memo");
		cfg->PhoneFeatures[0] = 0;
		/* Indicate that we used defaults */
		return ERR_USING_DEFAULTS;
	}
	return error;
}

void GSM_DumpMessageText_Custom(GSM_StateMachine *s, unsigned const char *message, size_t messagesize, int type, const char *text)
{
	GSM_Debug_Info *curdi;

	curdi = GSM_GetDI(s);

	if (curdi->dl == DL_TEXT ||
			curdi->dl == DL_TEXTALL ||
			curdi->dl == DL_TEXTDATE ||
			curdi->dl == DL_TEXTALLDATE) {
		smprintf(s, "%s ", text);
		smprintf(s, "type 0x%02X/length 0x%02lX/%ld",
				type, (long)messagesize, (long)messagesize);
		DumpMessage(curdi, message, messagesize);
	}
}

void GSM_DumpMessageText(GSM_StateMachine *s, unsigned const char *message, size_t messagesize, int type)
{
	GSM_DumpMessageText_Custom(s, message, messagesize, type, "SENDING frame");
}

void GSM_DumpMessageTextRecv(GSM_StateMachine *s, unsigned const char *message, size_t messagesize, int type)
{
	GSM_DumpMessageText_Custom(s, message, messagesize, type, "RECEIVED frame");
}

void GSM_DumpMessageBinary_Custom(GSM_StateMachine *s, unsigned const char *message, size_t messagesize, int type, int direction)
{
	size_t i=0;
	GSM_Debug_Info *curdi;

	curdi = GSM_GetDI(s);

	if (curdi->dl == DL_BINARY) {
		smprintf(s,"%c", direction);
		smprintf(s,"%c",type);
		smprintf(s,"%c",(int)(messagesize/256));
		smprintf(s,"%c",(int)(messagesize%256));

		for (i=0;i<messagesize;i++) {
			smprintf(s,"%c",message[i]);
		}
	}
}
void GSM_DumpMessageBinary(GSM_StateMachine *s, unsigned const char *message, size_t messagesize, int type)
{
	GSM_DumpMessageBinary_Custom(s, message, messagesize, type, 0x01);
}

void GSM_DumpMessageBinaryRecv(GSM_StateMachine *s, unsigned const char *message, size_t messagesize, int type)
{
	GSM_DumpMessageBinary_Custom(s, message, messagesize, type, 0x02);
}

void GSM_OSErrorInfo(GSM_StateMachine *s, const char *description)
{
#ifdef WIN32
	int 		i=0;
	unsigned char 	*lpMsgBuf = NULL;
#endif
	GSM_Debug_Info *curdi;

	curdi = GSM_GetDI(s);

#ifdef WIN32
	/* We don't use errno in win32 - GetLastError gives better info */
	if (GetLastError() != 0) {
		if (curdi->dl == DL_TEXTERROR ||
				curdi->dl == DL_TEXT ||
				curdi->dl == DL_TEXTALL ||
				curdi->dl == DL_TEXTERRORDATE ||
				curdi->dl == DL_TEXTDATE ||
				curdi->dl == DL_TEXTALLDATE) {
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
				(LPTSTR) &lpMsgBuf,
				0,
				NULL
			);
			for (i=0;i<(int)strlen(lpMsgBuf);i++) {
				if (lpMsgBuf[i] == 13 || lpMsgBuf[i] == 10) {
					lpMsgBuf[i] = ' ';
				}
			}
			smprintf(s,"[System error     - %s, %i, \"%s\"]\n", description, (int)GetLastError(), (LPCTSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
	}
#else

	if (errno!=-1) {
		if (curdi->dl == DL_TEXTERROR ||
				curdi->dl == DL_TEXT ||
				curdi->dl == DL_TEXTALL ||
				curdi->dl == DL_TEXTERRORDATE ||
				curdi->dl == DL_TEXTDATE ||
				curdi->dl == DL_TEXTALLDATE) {
			smprintf(s,"[System error     - %s, %i, \"%s\"]\n",description,errno,strerror(errno));
		}
	}
#endif
}

void GSM_SetIncomingCallCallback(GSM_StateMachine *s, IncomingCallCallback callback, void *user_data)
{
	s->User.IncomingCall = callback;
	s->User.IncomingCallUserData = user_data;
}

void GSM_SetIncomingSMSCallback(GSM_StateMachine *s, IncomingSMSCallback callback, void *user_data)
{
	s->User.IncomingSMS = callback;
	s->User.IncomingSMSUserData = user_data;
}

void GSM_SetIncomingCBCallback(GSM_StateMachine *s, IncomingCBCallback callback, void *user_data)
{
	s->User.IncomingCB = callback;
	s->User.IncomingCBUserData = user_data;
}

void GSM_SetIncomingUSSDCallback(GSM_StateMachine *s, IncomingUSSDCallback callback, void *user_data)
{
	s->User.IncomingUSSD = callback;
	s->User.IncomingUSSDUserData = user_data;
}

void GSM_SetSendSMSStatusCallback(GSM_StateMachine *s, SendSMSStatusCallback callback, void *user_data)
{
	s->User.SendSMSStatus = callback;
	s->User.SendSMSStatusUserData = user_data;
}

GSM_StateMachine *GSM_AllocStateMachine(void)
{
	GSM_StateMachine *ret;
	ret = (GSM_StateMachine *)calloc(1, sizeof(GSM_StateMachine));
	if (ret == NULL) {
		return ret;
	}
	ret->CurrentConfig = &(ret->Config[0]);
	ret->Abort = FALSE;
	return ret;
}

void GSM_FreeStateMachine(GSM_StateMachine *s)
{
	int i=0;

	if (s == NULL) return;

	/* Free allocated memory */
	for (i = 0; i <= MAX_CONFIG_NUM; i++) {
		free(s->Config[i].Device);
		s->Config[i].Device = NULL;
		free(s->Config[i].Connection);
		s->Config[i].Connection = NULL;
		free(s->Config[i].DebugFile);
		s->Config[i].DebugFile = NULL;
	}
	free(s);
	s = NULL;
}


GSM_ConnectionType GSM_GetUsedConnection(GSM_StateMachine *s)
{
	return s->ConnectionType;
}

GSM_PhoneModel *GSM_GetModelInfo(GSM_StateMachine *s)
{
	return s->Phone.Data.ModelInfo;
}

GSM_Debug_Info *GSM_GetDebug(GSM_StateMachine *s)
{
	return s == NULL ? NULL : &(s->di);
}


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
