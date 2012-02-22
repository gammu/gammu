/**
 * \file gammu-statemachine.h
 * \author Michal Čihař
 *
 * State machine data.
 */
#ifndef __gammu_statemachine_h
#define __gammu_statemachine_h

/**
 * \defgroup StateMachine State machine
 * Generic state machine layer.
 */

#include <gammu-types.h>
#include <gammu-error.h>
#include <gammu-inifile.h>

/**
 * Callback function for logging.
 *
 * \param text Text to be printed, \n will be also sent (as a separate
 * message).
 */
typedef void (*GSM_Log_Function) (const char *text, void *data);

/**
 * Private structure holding information about phone connection. Should
 * be allocated by \ref GSM_AllocStateMachine and freed by
 * \ref GSM_FreeStateMachine.
 *
 * \ingroup StateMachine
 */
typedef struct _GSM_StateMachine GSM_StateMachine;

#include <gammu-info.h>

/**
 * Configuration of state machine.
 *
 * \ingroup StateMachine
 */
typedef struct {
	/**
	 * Model from config file
	 */
	char Model[50];
	/**
	 * Debug level
	 */
	char DebugLevel[50];
	/**
	 * Device name from config file
	 */
	char *Device;
	/**
	 * Connection type as string
	 */
	char *Connection;
	/**
	 * Synchronize time on startup?
	 */
	gboolean SyncTime;
	/**
	 * Lock device ? (Unix)
	 */
	gboolean LockDevice;
	/**
	 * Name of debug file
	 */
	char *DebugFile;
	/**
	 * Display something during start ?
	 */
	gboolean StartInfo;
	/**
	 * Should we use global debug file?
	 */
	gboolean UseGlobalDebugFile;
	/**
	 * Text for reminder calendar entry category in local language
	 */
	char TextReminder[32];
	/**
	 * Text for meeting calendar entry category in local language
	 */
	char TextMeeting[32];
	/**
	 * Text for call calendar entry category in local language
	 */
	char TextCall[32];
	/**
	 * Text for birthday calendar entry category in local language
	 */
	char TextBirthday[32];
	/**
	 * Text for memo calendar entry category in local language
	 */
	char TextMemo[32];
	/**
	 * Phone features override.
	 */
	GSM_Feature PhoneFeatures[GSM_MAX_PHONE_FEATURES + 1];
} GSM_Config;

/**
 * Connection types definitions.
 */
typedef enum {
	GCT_MBUS2 = 1,
	GCT_FBUS2,
	GCT_FBUS2DLR3,
	GCT_DKU2AT,
	GCT_DKU2PHONET,
	GCT_DKU5FBUS2,
	GCT_ARK3116FBUS2,
	GCT_FBUS2PL2303,
	GCT_FBUS2BLUE,
	GCT_FBUS2IRDA,
	GCT_PHONETBLUE,
	GCT_AT,
	GCT_BLUEGNAPBUS,
	GCT_IRDAOBEX,
	GCT_IRDAGNAPBUS,
	GCT_IRDAAT,
	GCT_IRDAPHONET,
	GCT_BLUEFBUS2,
	GCT_BLUEAT,
	GCT_BLUEPHONET,
	GCT_BLUEOBEX,
	GCT_FBUS2USB,
	GCT_BLUES60,
	GCT_NONE
} GSM_ConnectionType;

/**
 * Initiates connection with custom logging callback.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param ReplyNum Number of replies to await (usually 3).
 * \param log_function Logging function, see GSM_SetDebugFunction.
 * \param user_data User data for logging function, see GSM_SetDebugFunction.
 * \return Error code
 * \see GSM_SetDebugFunction
 */
GSM_Error GSM_InitConnection_Log(GSM_StateMachine * s, int ReplyNum,
				 GSM_Log_Function log_function,
				 void *user_data);

/**
 * Initiates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param ReplyNum Number of replies to await (usually 3).
 * \return Error code
 */
GSM_Error GSM_InitConnection(GSM_StateMachine * s, int ReplyNum);

/**
 * Terminates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Error code
 */
GSM_Error GSM_TerminateConnection(GSM_StateMachine * s);

/**
 * Aborts current operation.
 *
 * This is thread safe call to abort any existing operations with the
 * phone.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Error code
 */
GSM_Error GSM_AbortOperation(GSM_StateMachine * s);

/**
 * Attempts to read data from phone. This can be used for getting
 * status of incoming events, which would not be found out without
 * polling device.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param waitforreply Whether to wait for some event
 * \return Number of read bytes
 */
int GSM_ReadDevice(GSM_StateMachine * s, gboolean waitforreply);

/**
 * Detects whether state machine is connected.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Whether phone is connected.
 */
gboolean GSM_IsConnected(GSM_StateMachine * s);

/**
 * Finds and reads gammu configuration file. The search order depends on
 * platform. On POSIX systems it looks for ~/.gammurc and then for
 * /etc/gammurc, on Windows for gammurc in Application data folder, then
 * in home and last fallback is in current driectory.
 *
 * \param result Ini file representation
 * \param force_config Forcing of custom path instead of autodetected
 * one (if NULL, autodetection is performed).
 *
 * \return Error code
 *
 * \ingroup StateMachine
 */
GSM_Error GSM_FindGammuRC(INI_Section ** result, const char *force_config);

/**
 * Processes gammu configuration.
 *
 * \param cfg_info Ini file representation.
 * \param cfg Where to store configuration.
 * \param num Number of section to read.
 * \return Whether we got valid configuration. Especially check for
 * ERR_USING_DEFAULTS.
 *
 * \ingroup StateMachine
 *
 * \see GSM_FallbackConfig
 */
GSM_Error GSM_ReadConfig(INI_Section * cfg_info, GSM_Config * cfg, int num);

/**
 * Gets gammu configuration from state machine. This actually returns
 * pointer to internal configuration storage, so you can use it also for
 * updating existing settings.
 *
 * \param s State machine data
 * \param num Number of section to read, -1 for currently used.
 * \return Pointer to configuration.
 *
 * \ingroup StateMachine
 */
GSM_Config *GSM_GetConfig(GSM_StateMachine * s, int num);

/**
 * Gets number of active gammu configurations.
 *
 * \param s State machine data
 * \return Number of sections.
 *
 * \ingroup StateMachine
 */
int GSM_GetConfigNum(const GSM_StateMachine * s);

/**
 * Gets number of active gammu configurations.
 *
 * \param s State machine data
 * \param sections Number of sections.
 *
 * \ingroup StateMachine
 */
void GSM_SetConfigNum(GSM_StateMachine * s, int sections);

/**
 * Allocates new clean state machine structure. You should free it then
 * by \ref GSM_FreeStateMachine.
 *
 * \return Pointer to state machine structure.
 *
 * \ingroup StateMachine
 */
GSM_StateMachine *GSM_AllocStateMachine(void);

/**
 * Frees state machine structure allocated by
 * \ref GSM_AllocStateMachine.
 *
 * \param s Pointer to state machine structure.
 *
 * \ingroup StateMachine
 */
void GSM_FreeStateMachine(GSM_StateMachine * s);

/**
 * Gets number of active gammu configurations.
 *
 * \param s State machine data
 * \return Connection type.
 *
 * \ingroup StateMachine
 */
GSM_ConnectionType GSM_GetUsedConnection(GSM_StateMachine * s);

/**
 * Installs applet required for configured connection to the phone.
 *
 * \param s State machine data.
 * \param ExtraPath Extra path where to search for installation data.
 * \param Minimal Whether to do minimal installation (eg. without support
 * libraries), useful for applet updates
 * \return Result of operation.
 *
 * \ingroup StateMachine
 */
GSM_Error GSM_Install(GSM_StateMachine *s, const char *ExtraPath, gboolean Minimal);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
