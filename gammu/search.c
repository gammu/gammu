#include "../common/misc/locales.h"

#include <gammu.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_PTHREAD
#  include <pthread.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  define THREAD_RETURN void *
#  define THREAD_RETURN_VAL NULL
#endif
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define THREAD_RETURN DWORD
#  define THREAD_RETURN_VAL 0
#endif

#include "search.h"
#include "common.h"

#include "../helper/formats.h"

#if defined(WIN32) || defined(HAVE_PTHREAD)
/**
 * Structure to hold information about connection for searching.
 */
typedef struct {
	/**
	 * Connection name.
	 */
	unsigned char Connection[50];
} OneConnectionInfo;

/**
 * Structure to hold device information for phone searching.
 */
typedef struct {
	/**
	 * Device name
	 */
	unsigned char Device[50];
	/**
	 * List of connections to try
	 */
	OneConnectionInfo Connections[5];
} OneDeviceInfo;

bool SearchOutput;

/**
 * Like printf, but only when output from searching is enabled.
 */
PRINTF_STYLE(1, 2)
int SearchPrintf(const char *format, ...)
{
	va_list ap;
	int ret;

	if (!SearchOutput)
		return 0;

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

void SearchPrintPhoneInfo(GSM_StateMachine * sm)
{
	GSM_Error error;
	char buffer[GSM_MAX_INFO_LENGTH];

	/* Try to get phone manufacturer */
	error = GSM_GetManufacturer(sm, buffer);

	/* Bail out if we failed */
	if (error != ERR_NONE) {
		SearchPrintf("\t%s\n", GSM_ErrorString(error));
		return;
	}

	/* Print basic information */
	printf("\t" LISTFORMAT "%s\n", _("Manufacturer"), buffer);

	/* Try to get phone model */
	error = GSM_GetModel(sm, buffer);

	/* Bail out if we failed */
	if (error != ERR_NONE) {
		SearchPrintf("\t%s\n", GSM_ErrorString(error));
		return;
	}

	/* Print model information */
	printf("\t" LISTFORMAT "%s (%s)\n", _("Model"),
	       GSM_GetModelInfo(sm)->model, buffer);
}

THREAD_RETURN SearchPhoneThread(void * arg)
{
	int j;
	OneDeviceInfo *Info = arg;
	GSM_Error error;
	GSM_StateMachine *search_gsm;
	GSM_Config *cfg;
	GSM_Config *globalcfg;

	/* Iterate over all connections */
	for (j = 0; strlen(Info->Connections[j].Connection) != 0; j++) {

		/* Allocate state machine */
		search_gsm = GSM_AllocStateMachine();
		if (search_gsm == NULL)
			return THREAD_RETURN_VAL;

		/* Get configuration pointers */
		cfg = GSM_GetConfig(search_gsm, 0);
		globalcfg = GSM_GetConfig(gsm, 0);

		/* We share some configuration with global one */
		cfg->UseGlobalDebugFile = globalcfg->UseGlobalDebugFile;
		cfg->DebugFile = strdup(globalcfg->DebugFile);
		strcpy(cfg->DebugLevel, globalcfg->DebugLevel);
		if (globalcfg->Localize == NULL) {
			cfg->Localize = NULL;
		} else {
			cfg->Localize = strdup(globalcfg->Localize);
		}

		/* Configure the tested state machine */
		cfg->Device = strdup(Info->Device);
		cfg->Connection = strdup(Info->Connections[j].Connection);
		cfg->SyncTime = strdup("no");
		cfg->Model[0] = 0;
		cfg->LockDevice = strdup("no");
		cfg->StartInfo = strdup("no");

		/* We have only one configured connection */
		GSM_SetConfigNum(search_gsm, 1);

		/* Let's connect */
		error = GSM_InitConnection(search_gsm, 1);

		printf(_("Connection \"%s\" on device \"%s\"\n"),
		       Info->Connections[j].Connection, Info->Device);

		/* Did we succeed? Show info */
		if (error == ERR_NONE) {
			SearchPrintPhoneInfo(search_gsm);
		} else {
			SearchPrintf("\t%s\n", GSM_ErrorString(error));
		}

		if (error != ERR_DEVICEOPENERROR) {
			GSM_TerminateConnection(search_gsm);
		}

		if (error == ERR_DEVICEOPENERROR)
			break;

		/* Free allocated buffer */
		GSM_FreeStateMachine(search_gsm);
	}
	return THREAD_RETURN_VAL;
}

#ifdef HAVE_PTHREAD
pthread_t Threads[100];
#else
HANDLE Threads[100];
#endif

OneDeviceInfo SearchDevices[60];

void MakeSearchThread(int i)
{
#ifdef HAVE_PTHREAD
	if (pthread_create
	    (&Threads[i], NULL, SearchPhoneThread,
	     &SearchDevices[i]) != 0) {
		fprintf(stderr, _("Error creating thread\n"));
	}
#else
	Threads[i] = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0,
			 (LPTHREAD_START_ROUTINE) SearchPhoneThread,
			 &SearchDevices[i], 0, NULL);
	if (Threads[i] 	== NULL) {
		fprintf(stderr, _("Error creating thread\n"));
	}
#endif
}

void SearchPhone(int argc, char *argv[])
{
	int i, dev = 0;
#ifdef HAVE_PTHREAD
	int fd;
	void *ret;
#endif


	SearchOutput = false;
	if (argc == 3 && strcasecmp(argv[2], "-debug") == 0)
		SearchOutput = true;

#ifdef WIN32
	SearchDevices[dev].Device[0] = 0;
	sprintf(SearchDevices[dev].Connections[0].Connection, "irdaphonet");
	sprintf(SearchDevices[dev].Connections[1].Connection, "irdaat");
	SearchDevices[dev].Connections[2].Connection[0] = 0;
	dev++;
	for (i = 0; i < 20; i++) {
		sprintf(SearchDevices[dev].Device, "com%i:", i + 1);
		sprintf(SearchDevices[dev].Connections[0].Connection,
			"fbusdlr3");
		sprintf(SearchDevices[dev].Connections[1].Connection, "fbus");
		sprintf(SearchDevices[dev].Connections[2].Connection,
			"at19200");
		sprintf(SearchDevices[dev].Connections[3].Connection, "mbus");
		SearchDevices[dev].Connections[4].Connection[0] = 0;
		dev++;
	}
#else
	for (i = 0; i < 6; i++) {
		sprintf(SearchDevices[dev].Device, "/dev/ircomm%i", i);
		fd = open(SearchDevices[dev].Device, O_RDWR);
		if (fd < 0) continue;
		close(fd);
		sprintf(SearchDevices[dev].Connections[0].Connection,
			"irdaphonet");
		sprintf(SearchDevices[dev].Connections[1].Connection,
			"at19200");
		SearchDevices[dev].Connections[2].Connection[0] = 0;
		dev++;
	}
	for (i = 0; i < 10; i++) {
		sprintf(SearchDevices[dev].Device, "/dev/ttyS%i", i);
		fd = open(SearchDevices[dev].Device, O_RDWR);
		if (fd < 0) continue;
		close(fd);
		sprintf(SearchDevices[dev].Connections[0].Connection,
			"fbusdlr3");
		sprintf(SearchDevices[dev].Connections[1].Connection, "fbus");
		sprintf(SearchDevices[dev].Connections[2].Connection,
			"at19200");
		sprintf(SearchDevices[dev].Connections[3].Connection, "mbus");
		SearchDevices[dev].Connections[4].Connection[0] = 0;
		dev++;
	}
	for (i = 0; i < 8; i++) {
		sprintf(SearchDevices[dev].Device, "/dev/ttyD00%i", i);
		fd = open(SearchDevices[dev].Device, O_RDWR);
		if (fd < 0) continue;
		close(fd);
		sprintf(SearchDevices[dev].Connections[0].Connection,
			"fbusdlr3");
		sprintf(SearchDevices[dev].Connections[1].Connection, "fbus");
		sprintf(SearchDevices[dev].Connections[2].Connection,
			"at19200");
		sprintf(SearchDevices[dev].Connections[3].Connection, "mbus");
		SearchDevices[dev].Connections[4].Connection[0] = 0;
		dev++;
	}
	for (i = 0; i < 4; i++) {
		sprintf(SearchDevices[dev].Device, "/dev/usb/tts/%i", i);
		fd = open(SearchDevices[dev].Device, O_RDWR);
		if (fd < 0) continue;
		close(fd);
		sprintf(SearchDevices[dev].Connections[0].Connection,
			"fbusdlr3");
		sprintf(SearchDevices[dev].Connections[1].Connection, "fbus");
		sprintf(SearchDevices[dev].Connections[2].Connection,
			"at19200");
		sprintf(SearchDevices[dev].Connections[3].Connection, "mbus");
		SearchDevices[dev].Connections[4].Connection[0] = 0;
		dev++;
	}
#endif
	for (i = 0; i < dev; i++)
		MakeSearchThread(i);
#ifdef WIN32
	WaitForMultipleObjects(dev, Threads, TRUE, INFINITE);
#else
	for (i = 0; i < dev; i++)
		pthread_join(Threads[i], &ret);
#endif
}
#endif				/*Support for threads */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
