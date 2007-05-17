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
 * Private structure holding information about phone connection.
 *
 * \ingroup StateMachine
 */
typedef struct _GSM_StateMachine 	GSM_StateMachine;

/**
 * Configuration of state machine.
 *
 * \ingroup StateMachine
 */
typedef struct {
	/* Config file (or Registry or...) variables */
	char			Model[50];	   /**< Model from config file 		*/
	char			DebugLevel[50];    /**< Debug level			*/
	char 			*Device;	   /**< Device name from config file 	*/
	char			*Connection;	   /**< Connection type as string		*/
	char			*SyncTime;	   /**< Synchronize time on startup? 	*/
	char			*LockDevice;	   /**< Lock device ? (Unix)		*/
	char			*DebugFile;        /**< Name of debug file		*/
	char 			*Localize;	   /**< Name of localisation file		*/
	char			*StartInfo;	   /**< Display something during start ?  */
	bool			UseGlobalDebugFile;/**< Should we use global debug file?	*/
	bool			DefaultModel;
	bool			DefaultDebugLevel;
	bool			DefaultDevice;
	bool			DefaultConnection;
	bool			DefaultSyncTime;
	bool			DefaultLockDevice;
	bool			DefaultDebugFile;
	bool			DefaultLocalize;
	bool			DefaultStartInfo;
	char			TextReminder[32]; /**< Text for reminder calendar entry category in local language */
	char			TextMeeting[32]; /**< Text for meeting calendar entry category in local language */
	char			TextCall[32]; /**< Text for call calendar entry category in local language */
	char			TextBirthday[32]; /**< Text for birthday calendar entry category in local language */
	char			TextMemo[32]; /**< Text for memo calendar entry category in local language */
} GSM_Config;

/**
 * Initiates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param ReplyNum Number of replies to await (usually 3).
 * \return Error code
 */
GSM_Error GSM_InitConnection(GSM_StateMachine *s, int ReplyNum);

/**
 * Terminates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Error code
 */
GSM_Error GSM_TerminateConnection(GSM_StateMachine *s);

/**
 * Attempts to read data from phone. This can be used for getting 
 * status of incoming events, which would not be found out without 
 * polling device.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param wait Whether to wait for some event
 * \return Number of read bytes
 */
int GSM_ReadDevice(GSM_StateMachine *s, bool wait);

/**
 * Detects whether state machine is connected.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Whether phone is connected.
 */
bool GSM_IsConnected(GSM_StateMachine *s);

/**
 * Reads gammu configuration file.
 *
 * \param result Ini file representation
 * \return Error code
 *
 * \ingroup StateMachine
 */
GSM_Error GSM_FindGammuRC(INI_Section **result);


/**
 * Processes gammu configuration.
 *
 * \param cfg_info Ini file representation.
 * \param cfg Where to store configuration.
 * \param num Number of section to read.
 * \return Whether we got valid configuration.
 *
 * \ingroup StateMachine
 */
bool GSM_ReadConfig(INI_Section *cfg_info, GSM_Config *cfg, int num);


/**
 * Gets gammu configuration from state machine.
 *
 * \param s State machine data
 * \param num Number of section to read, -1 for currently used.
 * \return Pointer to configuration.
 *
 * \ingroup StateMachine
 */
GSM_Config *GSM_GetConfig(GSM_StateMachine *s, int num);

/**
 * Allocates new clean state machine structure by malloc.
 *
 * \return Pointer to state machien structure, can be freed using free.
 *
 * \ingroup StateMachine
 */
GSM_StateMachine *GAMMU_AllocStateMachine(void);
#endif
/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
