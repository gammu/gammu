#define _GNU_SOURCE /* For strcasestr */
#include <string.h>

#include "../helper/locales.h"

#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <gammu.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "common.h"
#include "misc.h"
#include "message.h"

#include "../helper/formats.h"
#include "../helper/printing.h"
#include "../helper/string.h"

#ifdef GSM_ENABLE_NOKIA_DCT3
#  include "depend/nokia/dct3.h"
#  include "depend/nokia/dct3trac/wmx.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
#  include "depend/nokia/dct4.h"
#endif

void PrintNetworkInfo(GSM_NetworkInfo NetInfo)
{
	printf(LISTFORMAT, _("Network state"));
	switch (NetInfo.State) {
		case GSM_HomeNetwork		: printf("%s\n", _("home network")); 		 	break;
		case GSM_RoamingNetwork		: printf("%s\n", _("roaming network")); 	 	break;
		case GSM_RequestingNetwork	: printf("%s\n", _("requesting network")); 	 	break;
		case GSM_NoNetwork		: printf("%s\n", _("not logged into network")); 	break;
		case GSM_RegistrationDenied	: printf("%s\n", _("registration to network denied"));	break;
		case GSM_NetworkStatusUnknown	: printf("%s\n", _("unknown"));			break;
#ifndef CHECK_CASES
		default				: printf("%s\n", _("unknown"));
#endif
	}
	if (NetInfo.State == GSM_HomeNetwork || NetInfo.State == GSM_RoamingNetwork) {
		printf(LISTFORMAT, _("Network"));
		printf("%s (%s",
			NetInfo.NetworkCode,
			DecodeUnicodeConsole(GSM_GetNetworkName(NetInfo.NetworkCode)));
		printf(", %s)",
			DecodeUnicodeConsole(GSM_GetCountryName(NetInfo.NetworkCode)));
		printf(", LAC %s, CID %s\n",
			NetInfo.LAC,NetInfo.CID);
		if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
			printf(LISTFORMAT "\"%s\"\n",
				_("Name in phone"),
				DecodeUnicodeConsole(NetInfo.NetworkName));
		}
	}
}

GSM_Error GSM_PlayRingtone(GSM_Ringtone ringtone)
{
	int 		i;
	gboolean 		first=TRUE;
	GSM_Error 	error;

	signal(SIGINT, interrupt);
	printf("%s\n", _("Press Ctrl+C to break..."));

	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (gshutdown) break;
		if (ringtone.NoteTone.Commands[i].Type != RING_Note) continue;
		error=PHONE_RTTLPlayOneNote(gsm,ringtone.NoteTone.Commands[i].Note,first);
		if (error!=ERR_NONE) return error;
		first = FALSE;
	}

	/* Disables buzzer */
	return GSM_PlayTone(gsm,255*255,0,FALSE);
}

void PlayRingtone(int argc UNUSED, char *argv[])
{
	GSM_Ringtone ringtone,ringtone2;
	GSM_Error error;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);

	error=GSM_RingtoneConvert(&ringtone2,&ringtone,RING_NOTETONE);
	Print_Error(error);

	GSM_Init(TRUE);

	error=GSM_PlayRingtone(ringtone2);
	Print_Error(error);

	GSM_Terminate();
}

/**
 * Checks for firmware version available in Gammu database.
 */
void CheckFirmware(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_File PhoneDB;
	char model[GSM_MAX_MODEL_LENGTH];
	char current_version[GSM_MAX_VERSION_LENGTH];
	char url[70 + GSM_MAX_MODEL_LENGTH];
	char latest_version[GSM_MAX_VERSION_LENGTH];
	size_t pos = 0, oldpos = 0, i;
	GSM_Error error;

	GSM_Init(TRUE);

	/* Get model information */
	error = GSM_GetModel(gsm, model);
	Print_Error(error);

	/* Empty string */
	latest_version[0] = 0;

	/* Request information from phone db */
	sprintf(url, "http://www.gammu.org/support/phones/phonedbxml.php?model=%s", model);
	PhoneDB.Buffer = NULL;
	PhoneDB.Used = 0;
	if (!GSM_ReadHTTPFile(url, &PhoneDB))
		return;

	/* Parse reply */
	while (pos < PhoneDB.Used) {
		if (PhoneDB.Buffer[pos] != 10) {
			pos++;
			continue;
		}
		PhoneDB.Buffer[pos] = 0;
		if (strstr(PhoneDB.Buffer + oldpos, "<firmware>") == NULL) {
			pos++;
			oldpos = pos;
			continue;
		}
		sprintf(latest_version, "%s", strstr(PhoneDB.Buffer + oldpos, "<version>") + 9);
		for (i = 0; i < strlen(latest_version); i++) {
			if (latest_version[i] == '<') {
				latest_version[i] = 0;
				break;
			}
		}
		pos++;
		oldpos = pos;
	}
	free(PhoneDB.Buffer);

	/* Did we find something? */
	if (latest_version[0] == 0) {
		return;
	}

	/* Get phone firmware version */
	error = GSM_GetFirmware(gsm, current_version, NULL, NULL);
	Print_Error(error);

	/* Compare versions */
	if (!GSM_IsNewerVersion(latest_version, current_version))
		return;

	/* Print information to user */
	printf_info(_("Never version of firmware is available!\n"));
	printf_info(_("Latest version is %s and you run %s.\n"),
			latest_version, current_version);

	GSM_Terminate();
}



void Identify(int argc, char *argv[])
{
	char buffer[GSM_MAX_INFO_LENGTH];
	char date[GSM_MAX_VERSION_DATE_LENGTH];
	double num;
	GSM_Error error;

	GSM_Init(TRUE);

	error=GSM_GetManufacturer(gsm, buffer);
	Print_Error(error);
	printf(LISTFORMAT "%s\n", _("Manufacturer"), buffer);
	error=GSM_GetModel(gsm, buffer);
	Print_Error(error);
	printf(LISTFORMAT "%s (%s)\n", _("Model"),
			GSM_GetModelInfo(gsm)->model,
			buffer);

	error=GSM_GetFirmware(gsm, buffer, date, &num);
	Print_Error(error);
	printf(LISTFORMAT "%s", _("Firmware"), buffer);
	error=GSM_GetPPM(gsm, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(" %s", buffer);
	}
	if (date[0] != 0) printf(" (%s)", date);
	printf("\n");

	error=GSM_GetHardware(gsm, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Hardware"),buffer);
	}

	error=GSM_GetIMEI(gsm, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("IMEI"), buffer);

		error=GSM_GetOriginalIMEI(gsm, buffer);
		if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
			if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
			if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Original IMEI"), buffer);
		}
	}

	error=GSM_GetManufactureMonth(gsm, buffer);
	if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Manufactured"),buffer);
	}

	error=GSM_GetProductCode(gsm, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Product code"),buffer);
	}

	error=GSM_GetSIMIMSI(gsm, buffer);
	switch (error) {
		case ERR_SECURITYERROR:
		case ERR_NOTSUPPORTED:
		case ERR_NOTIMPLEMENTED:
			break;
		case ERR_NONE:
			printf(LISTFORMAT "%s\n", _("SIM IMSI"),buffer);
			break;
		default:
			Print_Error(error);
	}

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3Info();
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4Info();
#endif

	GSM_Terminate();
}

void NetworkInfo(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_NetworkInfo		NetInfo;

	GSM_Init(TRUE);

	if (GSM_GetNetworkInfo(gsm,&NetInfo)==ERR_NONE) {
		PrintNetworkInfo(NetInfo);
	}
	GSM_Terminate();
}

void IncomingCall(GSM_StateMachine *sm UNUSED, GSM_Call call, void *user_data)
{
	printf(LISTFORMAT, _("Call info"));
	if (call.CallIDAvailable) printf(_("ID %i, "),call.CallID);
	switch(call.Status) {
		case GSM_CALL_IncomingCall  	: printf(_("incoming call from \"%s\"\n"),DecodeUnicodeConsole(call.PhoneNumber));  	break;
		case GSM_CALL_OutgoingCall  	: printf(_("outgoing call to \"%s\"\n"),DecodeUnicodeConsole(call.PhoneNumber));    	break;
		case GSM_CALL_CallStart     	: printf("%s\n", _("call started")); 					  	  		break;
		case GSM_CALL_CallEnd	   	: printf("%s\n", _("end of call (unknown side)")); 					  	break;
		case GSM_CALL_CallLocalEnd  	: printf("%s\n", _("call end from our side"));						  	break;
		case GSM_CALL_CallRemoteEnd 	: printf(_("call end from remote side (code %i)\n"),call.StatusCode);			break;
		case GSM_CALL_CallEstablished   : printf("%s\n", _("call established. Waiting for answer"));				  	break;
		case GSM_CALL_CallHeld		: printf("%s\n", _("call held"));								break;
		case GSM_CALL_CallResumed	: printf("%s\n", _("call resumed"));								break;
		case GSM_CALL_CallSwitched	: printf("%s\n", _("call switched"));								break;
	}
}

#define PRINTUSED(name, used, free) \
{ \
	printf(LISTFORMAT, name);	\
	printf(_("%3d used"), used); \
	printf(", "); \
	printf(_("%3d free"), free); \
	printf("\n"); \
}

#define CHECKMEMORYSTATUS(x, m, name) \
{ \
	x.MemoryType=m;							\
	if ( (error = GSM_GetMemoryStatus(gsm, &x)) == ERR_NONE)			\
		PRINTUSED(name, x.MemoryUsed, x.MemoryFree);  \
}

#define CHECK_EXIT \
{ \
	if (gshutdown) break; \
	if (error != ERR_NONE && error != ERR_UNKNOWN && error != ERR_NOTSUPPORTED && error != ERR_MEMORY && error != ERR_EMPTY && error != ERR_SOURCENOTAVAILABLE && error != ERR_NOTIMPLEMENTED) break; \
}

void Monitor(int argc, char *argv[])
{
	GSM_MemoryStatus	MemStatus;
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarStatus	CalendarStatus;
	GSM_NetworkInfo		NetInfo;
    	GSM_BatteryCharge   	BatteryCharge;
    	GSM_SignalQuality   	SignalQuality;
	GSM_Error		error;
	int 			count = -1;

	if (argc >= 3) {
		count = atoi(argv[2]);
		if (count <= 0) count = -1;
	}

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));
	printf("%s\n\n", _("Entering monitor mode..."));

	GSM_Init(TRUE);

	GSM_SetIncomingSMSCallback(gsm, IncomingSMS, NULL);
	GSM_SetIncomingCBCallback(gsm, IncomingCB, NULL);
	GSM_SetIncomingCallCallback(gsm, IncomingCall, NULL);
	GSM_SetIncomingUSSDCallback(gsm, IncomingUSSD, NULL);

	error=GSM_SetIncomingSMS  		(gsm,TRUE);
	printf("%-35s : %s\n", _("Enabling info about incoming SMS"), GSM_ErrorString(error));
	error=GSM_SetIncomingCB   		(gsm,TRUE);
	printf("%-35s : %s\n", _("Enabling info about incoming CB"), GSM_ErrorString(error));
	error=GSM_SetIncomingCall 		(gsm,TRUE);
	printf("%-35s : %s\n", _("Enabling info about calls"), GSM_ErrorString(error));
	error=GSM_SetIncomingUSSD 		(gsm,TRUE);
	printf("%-35s : %s\n", _("Enabling info about USSD"), GSM_ErrorString(error));

	while (!gshutdown && count != 0) {
		if (count > 0) count--;
		CHECKMEMORYSTATUS(MemStatus,MEM_SM,"SIM phonebook");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_DC,"Dialled numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_RC,"Received numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_MC,"Missed numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_ON,"Own numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_ME,"Phone phonebook");
		CHECK_EXIT;
		if ( (error = GSM_GetToDoStatus(gsm, &ToDoStatus)) == ERR_NONE) {
			PRINTUSED(_("Todos"), ToDoStatus.Used, ToDoStatus.Free);
		}
		CHECK_EXIT;
		if ( (error = GSM_GetCalendarStatus(gsm, &CalendarStatus)) == ERR_NONE) {
			PRINTUSED(_("Calendar"), CalendarStatus.Used, CalendarStatus.Free);
		}
		CHECK_EXIT;
		if ( (error = GSM_GetBatteryCharge(gsm,&BatteryCharge)) == ERR_NONE) {
            		if (BatteryCharge.BatteryPercent != -1) {
				printf(LISTFORMAT, _("Battery level"));
				printf(_("%i percent"), BatteryCharge.BatteryPercent);
				printf("\n");
			}
            		if (BatteryCharge.BatteryCapacity != -1) {
				printf(LISTFORMAT, _("Battery capacity"));
				printf(_("%i mAh"), BatteryCharge.BatteryCapacity);
				printf("\n");
			}
            		if (BatteryCharge.BatteryTemperature != -1) {
				printf(LISTFORMAT, _("Battery temperature"));
				/* l10n: This means degrees Celsius */
				printf(_("%i C"), BatteryCharge.BatteryTemperature);
				printf("\n");
			}
            		if (BatteryCharge.PhoneTemperature != -1) {
				printf(LISTFORMAT, _("Phone temperature"));
				printf(_("%i C"), BatteryCharge.PhoneTemperature);
				printf("\n");
			}
            		if (BatteryCharge.BatteryVoltage != -1) {
				printf(LISTFORMAT, _("Battery voltage"));
				printf(_("%i mV"), BatteryCharge.BatteryVoltage);
				printf("\n");
			}
            		if (BatteryCharge.ChargeVoltage != -1) {
				printf(LISTFORMAT, _("Charge voltage"));
				printf(_("%i mV"), BatteryCharge.ChargeVoltage);
				printf("\n");
			}
            		if (BatteryCharge.ChargeCurrent != -1) {
				printf(LISTFORMAT, _("Charge current"));
				printf(_("%i mA"), BatteryCharge.ChargeCurrent);
				printf("\n");
			}
            		if (BatteryCharge.PhoneCurrent != -1) {
				printf(LISTFORMAT, _("Phone current"));
				printf(_("%i mA"), BatteryCharge.PhoneCurrent);
				printf("\n");
			}
            		if (BatteryCharge.ChargeState != 0) {
                		printf(LISTFORMAT, _("Charge state"));
                		switch (BatteryCharge.ChargeState) {
                    			case GSM_BatteryPowered:
						printf(_("powered from battery"));
						break;
                    			case GSM_BatteryConnected:
						printf(_("battery connected, but not powered from battery"));
                        			break;
                    			case GSM_BatteryCharging:
						printf(_("battery connected and is being charged"));
                        			break;
                    			case GSM_BatteryFull:
						printf(_("battery connected and is fully charged"));
                        			break;
                    			case GSM_BatteryNotConnected:
                        			printf(_("battery not connected"));
                        			break;
                    			case GSM_PowerFault:
                        			printf(_("detected power failure"));
                        			break;
#ifndef CHECK_CASES
                    			default:
                        			printf(_("unknown"));
                       				break;
#endif
                		}
                		printf("\n");
            		}
            		if (BatteryCharge.BatteryType != 0) {
                		printf(LISTFORMAT, _("Battery type"));
                		switch (BatteryCharge.BatteryType) {
                    			case GSM_BatteryLiIon:
						printf(_("Lithium Ion"));
						break;
                    			case GSM_BatteryLiPol:
						printf(_("Lithium Polymer"));
						break;
                    			case GSM_BatteryNiMH:
						printf(_("NiMH"));
						break;
					case GSM_BatteryUnknown:
#ifndef CHECK_CASES
                    			default:
#endif
                        			printf(_("unknown"));
                       				break;
                		}
                		printf("\n");
            		}
        	}
		CHECK_EXIT;
		if ( (error = GSM_GetSignalQuality(gsm,&SignalQuality)) == ERR_NONE) {
            		if (SignalQuality.SignalStrength != -1) {
				printf(LISTFORMAT, _("Signal strength"));
				printf(_("%i dBm"), SignalQuality.SignalStrength);
                		printf("\n");
			}
            		if (SignalQuality.SignalPercent  != -1) {
				printf(LISTFORMAT, _("Network level"));
				printf(_("%i percent"), SignalQuality.SignalPercent);
                		printf("\n");
			}
            		if (SignalQuality.BitErrorRate   != -1) {
				printf(LISTFORMAT, _("Bit error rate"));
				printf(_("%i percent"), SignalQuality.BitErrorRate);
                		printf("\n");
			}
        	}
		CHECK_EXIT;
		if ( (error = GSM_GetSMSStatus(gsm,&SMSStatus)) == ERR_NONE) {
			if (SMSStatus.SIMSize > 0) {
				printf(LISTFORMAT, _("SIM SMS status"));
				printf(_("%i used"), SMSStatus.SIMUsed);
				printf(", ");
				printf(_("%i unread"), SMSStatus.SIMUnRead);
				printf(", ");
				printf(_("%i locations"), SMSStatus.SIMSize);
				printf("\n");
			}

			if (SMSStatus.PhoneSize > 0) {
				printf(LISTFORMAT, _("Phone SMS status"));
				printf(_("%i used"), SMSStatus.PhoneUsed);
				printf(", ");
				printf(_("%i unread"), SMSStatus.PhoneUnRead);
				printf(", ");
				printf(_("%i locations"), SMSStatus.PhoneSize);
				if (SMSStatus.TemplatesUsed != 0) {
					printf(", ");
					printf(_("%i templates"), SMSStatus.TemplatesUsed);
				}
				printf("\n");
			}
		}
		CHECK_EXIT;
		if ( (error = GSM_GetNetworkInfo(gsm,&NetInfo)) == ERR_NONE) {
			PrintNetworkInfo(NetInfo);
		}
		if (wasincomingsms) DisplayIncomingSMS();
		printf("\n");
	}

	printf("%s\n", _("Leaving monitor mode..."));

	/* Report if we failed because of an error */
	Print_Error(error);

	GSM_Terminate();
}

void GetRingtone(int argc, char *argv[])
{
	GSM_Ringtone 	ringtone;
	gboolean		PhoneRingtone = FALSE;
	GSM_Error error;

	if (strcasestr(argv[1], "getphoneringtone") != NULL) {
		PhoneRingtone = TRUE;
	}

	GetStartStop(&ringtone.Location, NULL, 2, argc, argv);

	GSM_Init(TRUE);

	ringtone.Format=0;

	error=GSM_GetRingtone(gsm,&ringtone,PhoneRingtone);
	Print_Error(error);

	switch (ringtone.Format) {
		case RING_NOTETONE	: printf(_("Smart Messaging"));	break;
		case RING_NOKIABINARY	: printf(_("Nokia binary"));	break;
		case RING_MIDI		: printf(_("MIDI"));		break;
		case RING_MMF		: printf(_("SMAF (MMF)"));	break;
	}
	printf(_(" format, ringtone \"%s\"\n"),DecodeUnicodeConsole(ringtone.Name));

	if (argc==4) {
		error=GSM_SaveRingtoneFile(argv[3], &ringtone);
		Print_Error(error);
	}

	GSM_Terminate();
}

void GetRingtonesList(int argc UNUSED, char *argv[] UNUSED)
{
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	GSM_Error error;
	int			i;

	GSM_Init(TRUE);

	error=GSM_GetRingtonesInfo(gsm,&Info);
 	if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
	Print_Error(error);

	GSM_Terminate();

	for (i=0;i<Info.Number;i++) printf("%i. \"%s\"\n",i+1,DecodeUnicodeConsole(Info.Ringtone[i].Name));

 	if (Info.Ringtone) free(Info.Ringtone);
}

void DialVoice(int argc, char *argv[])
{
	GSM_CallShowNumber ShowNumber = GSM_CALL_DefaultNumberPresence;
	GSM_Error error;

	if (argc > 3) {
		if (strcasecmp(argv[3],"show") == 0) {		ShowNumber = GSM_CALL_ShowNumber;
		} else if (strcasecmp(argv[3],"hide") == 0) {	ShowNumber = GSM_CALL_HideNumber;
		} else {
			printf(_("Unknown parameter (\"%s\")\n"),argv[3]);
			Terminate(3);
		}
	}

	GSM_Init(TRUE);

	error=GSM_DialVoice(gsm, argv[2], ShowNumber);
	Print_Error(error);

	GSM_Terminate();
}

int TerminateID = -1;

void IncomingCall0(GSM_StateMachine *sm UNUSED, GSM_Call call, void * user_data)
{
	if (call.CallIDAvailable) TerminateID = call.CallID;
}

void MakeTerminatedCall(int argc, char *argv[])
{
	GSM_CallShowNumber 	ShowNumber = GSM_CALL_DefaultNumberPresence;
	GSM_Error error;

	if (argc > 4) {
		if (strcasecmp(argv[4],"show") == 0) {		ShowNumber = GSM_CALL_ShowNumber;
		} else if (strcasecmp(argv[4],"hide") == 0) {	ShowNumber = GSM_CALL_HideNumber;
		} else {
			printf(_("Unknown parameter (\"%s\")\n"),argv[4]);
			Terminate(3);
		}
	}

	GSM_Init(TRUE);

	TerminateID = -1;
	GSM_SetIncomingCallCallback(gsm, IncomingCall0, NULL);

	error=GSM_SetIncomingCall(gsm,TRUE);
	Print_Error(error);

	error=GSM_DialVoice(gsm, argv[2], ShowNumber);
	Print_Error(error);

	sleep(atoi(argv[3]));
	GSM_ReadDevice(gsm,TRUE);

	if (TerminateID != -1) {
		error=GSM_CancelCall(gsm,TerminateID,FALSE);
		Print_Error(error);
	}

	GSM_Terminate();
}


void CancelCall(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	if (argc>2) {
		error=GSM_CancelCall(gsm,atoi(argv[2]),FALSE);
	} else {
		error=GSM_CancelCall(gsm,0,TRUE);
	}
	Print_Error(error);

	GSM_Terminate();
}

void AnswerCall(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	if (argc>2) {
		error=GSM_AnswerCall(gsm,atoi(argv[2]),FALSE);
	} else {
		error=GSM_AnswerCall(gsm,0,TRUE);
	}
	Print_Error(error);

	GSM_Terminate();
}

void UnholdCall(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	error=GSM_UnholdCall(gsm,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

void HoldCall(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	error=GSM_HoldCall(gsm,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

void ConferenceCall(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	error=GSM_ConferenceCall(gsm,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

void SplitCall(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	error=GSM_SplitCall(gsm,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

void SwitchCall(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	if (argc > 2) {
		error=GSM_SwitchCall(gsm,atoi(argv[2]),FALSE);
	} else {
		error=GSM_SwitchCall(gsm,0,TRUE);
	}
	Print_Error(error);

	GSM_Terminate();
}

void TransferCall(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	if (argc > 2) {
		error=GSM_TransferCall(gsm,atoi(argv[2]),FALSE);
	} else {
		error=GSM_TransferCall(gsm,0,TRUE);
	}
	Print_Error(error);

	GSM_Terminate();
}

void Reset(int argc UNUSED, char *argv[])
{
	gboolean hard;
	GSM_Error error;

	if (strcasecmp(argv[2],"SOFT") == 0) {		hard=FALSE;
	} else if (strcasecmp(argv[2],"HARD") == 0) {	hard=TRUE;
	} else {
		printf(_("What type of reset do you want (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	GSM_Init(TRUE);

	error=GSM_Reset(gsm, hard);
	Print_Error(error);

	GSM_Terminate();
}

void GetWAPBookmark(int argc, char *argv[])
{
	GSM_WAPBookmark		bookmark;
	int			start,stop, i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		bookmark.Location=i;
		error=GSM_GetWAPBookmark(gsm,&bookmark);
		Print_Error(error);
		printf(LISTFORMAT "\"%s\"\n", _("Name"),DecodeUnicodeConsole(bookmark.Title));
		printf(LISTFORMAT "\"%s\"\n", _("Address"),DecodeUnicodeConsole(bookmark.Address));
	}

	GSM_Terminate();
}

void DeleteWAPBookmark(int argc, char *argv[])
{
	GSM_WAPBookmark	bookmark;
	int		start, stop, i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		bookmark.Location=i;
		error=GSM_DeleteWAPBookmark(gsm, &bookmark);
		Print_Error(error);
	}

	GSM_Terminate();
}

void GetGPRSPoint(int argc, char *argv[])
{
	GSM_GPRSAccessPoint	point;
	int			start,stop, i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		point.Location=i;
		error=GSM_GetGPRSAccessPoint(gsm,&point);
		if (error != ERR_EMPTY) {
			Print_Error(error);
			printf("%i. \"%s\"",point.Location,DecodeUnicodeConsole(point.Name));
		} else {
			printf(_("%i. Access point %i"),point.Location,point.Location);
		}
		if (point.Active) printf(_(" (active)"));
		printf("\n");
		if (error != ERR_EMPTY) {
			printf(LISTFORMAT "\"%s\"\n\n", _("Address"),DecodeUnicodeConsole(point.URL));
		} else {
			printf("\n");
		}
	}

	GSM_Terminate();
}

void GetBitmap(int argc, char *argv[])
{
	GSM_File		File;
	GSM_MultiBitmap 	MultiBitmap;
	int			location=0, Handle, Size;
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	unsigned char		buffer[10];
	GSM_Error error;

	if (strcasecmp(argv[2],"STARTUP") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
	} else if (strcasecmp(argv[2],"CALLER") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_CallerGroupLogo;
		GetStartStop(&location, NULL, 3, argc, argv);
		if (location>5) {
			printf("%s\n", _("Maximal location for caller logo can be 5"));
			exit (-1);
		}
	} else if (strcasecmp(argv[2],"PICTURE") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_PictureImage;
		GetStartStop(&location, NULL, 3, argc, argv);
	} else if (strcasecmp(argv[2],"TEXT") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_WelcomeNote_Text;
	} else if (strcasecmp(argv[2],"DEALER") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_DealerNote_Text;
	} else if (strcasecmp(argv[2],"OPERATOR") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_OperatorLogo;
	} else {
		printf(_("What type of logo do you want to get (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}
	MultiBitmap.Bitmap[0].Location=location;

	GSM_Init(TRUE);

	error=GSM_GetBitmap(gsm,&MultiBitmap.Bitmap[0]);
	Print_Error(error);

	MultiBitmap.Number = 1;

	error=ERR_NONE;
	switch (MultiBitmap.Bitmap[0].Type) {
	case GSM_CallerGroupLogo:
		if (!MultiBitmap.Bitmap[0].DefaultBitmap) GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printf(LISTFORMAT "\"%s\"", _("Group name"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		if (MultiBitmap.Bitmap[0].DefaultName) printf(" (%s)", _("default"));
		printf("\n");
		if (MultiBitmap.Bitmap[0].DefaultRingtone) {
			printf(LISTFORMAT "%s\n", _("Ringtone"), _("default"));
		} else if (GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(gsm), F_6230iCALLER)) {
			printf(LISTFORMAT "%i\n", _("Ringtone"),MultiBitmap.Bitmap[0].RingtoneID);
		} else if (MultiBitmap.Bitmap[0].FileSystemRingtone) {
			sprintf(buffer,"%i",MultiBitmap.Bitmap[0].RingtoneID);
			EncodeUnicode(File.ID_FullName,buffer,strlen(buffer));

			File.Buffer 	= NULL;
			File.Used 	= 0;

			error = GSM_GetFilePart(gsm,&File,&Handle,&Size);

		    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
			error = ERR_NONE;

			printf(LISTFORMAT "\"%s\" ", _("Ringtone"), DecodeUnicodeString(File.Name));
			printf(_("(file with ID %i)\n"), MultiBitmap.Bitmap[0].RingtoneID);
		} else {
			error = GSM_GetRingtonesInfo(gsm,&Info);
			if (error != ERR_NONE) Info.Number = 0;
			error = ERR_NONE;

			printf(LISTFORMAT, _("Ringtone"));
			if (UnicodeLength(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].RingtoneID))!=0) {
				/* l10n: Ringtone name and ID format */
				printf(_("\"%s\" (ID %i)\n"),
					DecodeUnicodeConsole(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].RingtoneID)),
					MultiBitmap.Bitmap[0].RingtoneID);
			} else {
				/* l10n: Ringtone ID format */
				printf(_("ID %i\n"),MultiBitmap.Bitmap[0].RingtoneID);
			}

			if (Info.Ringtone) free(Info.Ringtone);
		}
		if (MultiBitmap.Bitmap[0].BitmapEnabled) {
			printf(LISTFORMAT "%s\n", _("Bitmap"), _("enabled"));
		} else {
			printf(LISTFORMAT "%s\n", _("Bitmap"), _("disabled"));
		}
		if (MultiBitmap.Bitmap[0].FileSystemPicture) {
			printf(LISTFORMAT "%i\n", _("Bitmap ID"),MultiBitmap.Bitmap[0].PictureID);
		}
		if (argc>4 && !MultiBitmap.Bitmap[0].DefaultBitmap) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_StartupLogo:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		if (argc>3) error=GSM_SaveBitmapFile(argv[3],&MultiBitmap);
		break;
	case GSM_OperatorLogo:
		if (strcmp(MultiBitmap.Bitmap[0].NetworkCode,"000 00")!=0) {
			GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
			if (argc>3) error=GSM_SaveBitmapFile(argv[3],&MultiBitmap);
		} else {
			printf("%s\n", _("No operator logo in phone"));
		}
		break;
	case GSM_PictureImage:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printf(LISTFORMAT "\"%s\"\n", _("Text"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		printf(LISTFORMAT "\"%s\"\n", _("Sender"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Sender));
		if (MultiBitmap.Bitmap[0].Name)
			printf(LISTFORMAT "\"%s\"\n", _("Name"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Name));
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_WelcomeNote_Text:
		printf(_("Welcome note text is \"%s\"\n"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		break;
	case GSM_DealerNote_Text:
		printf(_("Dealer note text is \"%s\"\n"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		break;
	default:
		break;
	}
	Print_Error(error);

	GSM_Terminate();
}

void SetBitmap(int argc, char *argv[])
{
	GSM_Bitmap		Bitmap, NewBitmap;
	GSM_MultiBitmap		MultiBitmap;
	GSM_NetworkInfo		NetInfo;
	gboolean			init = TRUE;
	int			i;
	GSM_Error error;

	if (strcasecmp(argv[2],"STARTUP") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			Terminate(3);
		}
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
		MultiBitmap.Bitmap[0].Location=1;
		if (!strcmp(argv[3],"1")) MultiBitmap.Bitmap[0].Location = 2;
		if (!strcmp(argv[3],"2")) MultiBitmap.Bitmap[0].Location = 3;
		if (!strcmp(argv[3],"3")) MultiBitmap.Bitmap[0].Location = 4;
		if (MultiBitmap.Bitmap[0].Location == 1) {
			error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
			Print_Error(error);
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
	} else if (strcasecmp(argv[2],"TEXT") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			Terminate(3);
		}
		Bitmap.Type=GSM_WelcomeNote_Text;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (strcasecmp(argv[2],"DEALER") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			Terminate(3);
		}
		Bitmap.Type=GSM_DealerNote_Text;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (strcasecmp(argv[2],"CALLER") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			Terminate(3);
		}
		GetStartStop(&i, NULL, 3, argc, argv);
		if (i>5 && i!=255) {
			printf("%s\n", _("Maximal location for caller logo can be 5"));
			exit (-1);
		}
		MultiBitmap.Bitmap[0].Type 	= GSM_CallerGroupLogo;
		MultiBitmap.Bitmap[0].Location	= i;
		if (argc>4) {
			error=GSM_ReadBitmapFile(argv[4],&MultiBitmap);
			Print_Error(error);
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
		if (i!=255) {
			GSM_Init(TRUE);
			init = FALSE;
			NewBitmap.Type 	   = GSM_CallerGroupLogo;
			NewBitmap.Location = i;
			error=GSM_GetBitmap(gsm,&NewBitmap);
			Print_Error(error);
			Bitmap.RingtoneID	  = NewBitmap.RingtoneID;
			Bitmap.DefaultRingtone 	  = NewBitmap.DefaultRingtone;
			Bitmap.FileSystemRingtone = FALSE;
			CopyUnicodeString(Bitmap.Text, NewBitmap.Text);
			Bitmap.DefaultName	  = NewBitmap.DefaultName;
		}
	} else if (strcasecmp(argv[2],"PICTURE") == 0) {
		if (argc<5) {
			printf("%s\n", _("More parameters required!"));
			Terminate(3);
		}
		MultiBitmap.Bitmap[0].Type		= GSM_PictureImage;
		MultiBitmap.Bitmap[0].Location		= atoi(argv[4]);
		error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
		Print_Error(error);
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
		Bitmap.Text[0]=0;
		Bitmap.Text[1]=0;
		if (argc == 6) EncodeUnicode(Bitmap.Text,argv[5],strlen(argv[5]));
		Bitmap.Sender[0]=0;
		Bitmap.Sender[1]=0;
	} else if (strcasecmp(argv[2],"COLOUROPERATOR") == 0) {
		Bitmap.Type = GSM_ColourOperatorLogo_ID;
		strcpy(Bitmap.NetworkCode,"000 00");
		if (argc > 3) {
			Bitmap.ID = atoi(argv[3]);
			if (argc>4) {
				strncpy(Bitmap.NetworkCode,argv[4],6);
			} else {
				GSM_Init(TRUE);
				init = FALSE;
				error=GSM_GetNetworkInfo(gsm,&NetInfo);
				Print_Error(error);
				strcpy(Bitmap.NetworkCode,NetInfo.NetworkCode);
			}
		}
	} else if (strcasecmp(argv[2],"COLOURSTARTUP") == 0) {
		Bitmap.Type 	= GSM_ColourStartupLogo_ID;
		Bitmap.Location = 0;
		if (argc > 3) {
			Bitmap.Location = 1;
			Bitmap.ID 	= atoi(argv[3]);
		}
	} else if (strcasecmp(argv[2],"WALLPAPER") == 0) {
		Bitmap.Type 		= GSM_ColourWallPaper_ID;
		Bitmap.ID		= 0;
		if (argc > 3) Bitmap.ID = atoi(argv[3]);
	} else if (strcasecmp(argv[2],"OPERATOR") == 0) {
		MultiBitmap.Bitmap[0].Type	= GSM_OperatorLogo;
		MultiBitmap.Bitmap[0].Location 	= 1;
		strcpy(MultiBitmap.Bitmap[0].NetworkCode,"000 00");
		if (argc>3) {
			error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
			Print_Error(error);
			if (argc>4) {
				strncpy(MultiBitmap.Bitmap[0].NetworkCode,argv[4],6);
			} else {
				GSM_Init(TRUE);
				init = FALSE;
				error=GSM_GetNetworkInfo(gsm,&NetInfo);
				Print_Error(error);
				strcpy(MultiBitmap.Bitmap[0].NetworkCode,NetInfo.NetworkCode);
			}
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
	} else {
		printf(_("What type of logo do you want to set (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	if (init) GSM_Init(TRUE);

	error=GSM_SetBitmap(gsm,&Bitmap);
	Print_Error(error);

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif

	GSM_Terminate();
}

void SetRingtone(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Ringtone 	ringtone;
	int		i,nextlong=0;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);
	ringtone.Location = 255;
	for (i=3;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (strcasecmp(argv[i],"-scale") == 0) {
				ringtone.NoteTone.AllNotesScale = TRUE;
				break;
			}
			if (strcasecmp(argv[i],"-location") == 0) {
				nextlong = 1;
				break;
			}
			if (strcasecmp(argv[i],"-name") == 0) {
				nextlong = 2;
				break;
			}
			printf(_("Unknown parameter (\"%s\")"),argv[i]);
			Terminate(3);
		case 1:
			ringtone.Location=atoi(argv[i]);
			nextlong = 0;
			break;
		case 2:
			EncodeUnicode(ringtone.Name,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		}
	}
	if (nextlong!=0) {
		printf_err("%s\n", _("Parameter missing!"));
		Terminate(3);
	}
	if (ringtone.Location==0) {
		printf_err("%s\n", _("Please enumerate locations from 1"));
		exit (-1);
	}

	GSM_Init(TRUE);
	error=GSM_SetRingtone(gsm, &ringtone, &i);
	Print_Error(error);
#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

void ClearMemory(GSM_MemoryType type, const char *question)
{
	GSM_Error error;
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	gboolean			DoClear;
	int			i;

	DoClear = FALSE;
	MemStatus.MemoryType = type;
	error = GSM_GetMemoryStatus(gsm, &MemStatus);
	if (error == ERR_NONE && MemStatus.MemoryUsed !=0) {
		if (answer_yes("%s", question)) DoClear = TRUE;
	}
	if (DoClear) {
		error = GSM_DeleteAllMemory(gsm, type);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			for (i = 0; i < MemStatus.MemoryUsed + MemStatus.MemoryFree; i++) {
				Pbk.MemoryType 	= type;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=GSM_DeleteMemory(gsm, &Pbk);
				Print_Error(error);
				fprintf(stderr, "\r");
				fprintf(stderr, "%s ", _("Deleting:"));
				fprintf(stderr, _("%i percent"),
					(i + 1) * 100 / (MemStatus.MemoryUsed + MemStatus.MemoryFree));
				if (gshutdown) {
					GSM_Terminate();
					Terminate(0);
				}
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}
	}
}


void ClearAll(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarEntry	Calendar;
	GSM_ToDoEntry		ToDo;
	GSM_NoteEntry		Note;
	GSM_WAPBookmark		Bookmark;
	GSM_FMStation 		Station;
	gboolean			DoClear;
	GSM_Error error;

	GSM_Init(TRUE);

	ClearMemory(MEM_ME, _("Delete phone phonebook?"));
	ClearMemory(MEM_SM, _("Delete SIM phonebook?"));
	ClearMemory(MEM_MC, _("Delete missed calls?"));
	ClearMemory(MEM_DC, _("Delete dialled calls?"));
	ClearMemory(MEM_RC, _("Delete received calls?"));

	DoClear = FALSE;
	error = GSM_GetNextCalendar(gsm,&Calendar,TRUE);
	if (error == ERR_NONE) {
 		if (answer_yes(_("Delete phone calendar notes?"))) DoClear = TRUE;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=GSM_DeleteAllCalendar(gsm);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = GSM_GetNextCalendar(gsm,&Calendar,TRUE);
				if (error != ERR_NONE) break;
				error = GSM_DeleteCalendar(gsm,&Calendar);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
			fflush(stderr);
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}
	}

	DoClear = FALSE;
	error = GSM_GetToDoStatus(gsm,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes(_("Delete phone todos?"))) DoClear = TRUE;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=GSM_DeleteAllToDo(gsm);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = GSM_GetNextToDo(gsm,&ToDo,TRUE);
				if (error != ERR_NONE) break;
				error = GSM_DeleteToDo(gsm,&ToDo);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
			fflush(stderr);
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}
	}

	DoClear = FALSE;
	error = GSM_GetNotesStatus(gsm,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes(_("Delete phone notes?"))) DoClear = TRUE;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		while (1) {
			error = GSM_GetNextNote(gsm,&Note,TRUE);
			if (error != ERR_NONE) break;
			error = GSM_DeleteNote(gsm,&Note);
			Print_Error(error);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
		fflush(stderr);
	}

	Bookmark.Location = 1;
	error = GSM_GetWAPBookmark(gsm,&Bookmark);
	if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
		if (answer_yes(_("Delete phone WAP bookmarks?"))) {
			fprintf(stderr, LISTFORMAT, _("Deleting"));
			/* One thing to explain: DCT4 phones seems to have bug here.
			 * When delete for example first bookmark, phone change
			 * numeration for getting frame, not for deleting. So, we try to
			 * get 1'st bookmark. Inside frame is "correct" location. We use
			 * it later
			 */
			while (error==ERR_NONE) {
				error = GSM_DeleteWAPBookmark(gsm,&Bookmark);
				Bookmark.Location = 1;
				error = GSM_GetWAPBookmark(gsm,&Bookmark);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
			fflush(stderr);
		}
	}
	if (answer_yes(_("Delete all phone user ringtones?"))) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=GSM_DeleteUserRingtones(gsm);
		Print_Error(error);
		fprintf(stderr, "%s\n", _("Done"));
	}
	Station.Location = 1;
	error=GSM_GetFMStation(gsm,&Station);
	if (error == ERR_NONE || error == ERR_EMPTY) {
	 	if (answer_yes(_("Delete all phone FM radio stations?"))) {
 			error=GSM_ClearFMStations(gsm);
 			Print_Error(error);
		}
 	}

	GSM_Terminate();
}

void DisplayConnectionSettings(GSM_MultiWAPSettings *settings,int j)
{
	if (settings->Settings[j].IsContinuous) {
		printf(LISTFORMAT "%s\n", _("Connection type"), _("Continuous"));
	} else {
		printf(LISTFORMAT "%s\n", _("Connection type"), _("Temporary"));
	}
	if (settings->Settings[j].IsSecurity) {
		printf(LISTFORMAT "%s\n", _("Connection security"), _("On"));
	} else {
		printf(LISTFORMAT "%s\n", _("Connection security"), _("Off"));
	}
	printf(LISTFORMAT, _("Proxy"));
	printf(_("address \"%s\", port %i"), DecodeUnicodeConsole(settings->Proxy), settings->ProxyPort);
	printf("\n");
	printf(LISTFORMAT, _("Second proxy"));
	printf(_("address \"%s\", port %i"), DecodeUnicodeConsole(settings->Proxy2), settings->Proxy2Port);
	printf("\n");
	switch (settings->Settings[j].Bearer) {
	case WAPSETTINGS_BEARER_SMS:
		printf(LISTFORMAT "%s", _("Bearer"), _("SMS"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_SMS) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Server number"),DecodeUnicodeConsole(settings->Settings[j].Server));
		printf(LISTFORMAT "\"%s\"\n", _("Service number"),DecodeUnicodeConsole(settings->Settings[j].Service));
		break;
	case WAPSETTINGS_BEARER_DATA:
		printf(LISTFORMAT "%s", _("Bearer"), _("Data (CSD)"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_DATA) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Dial-up number"),DecodeUnicodeConsole(settings->Settings[j].DialUp));
		printf(LISTFORMAT "\"%s\"\n", _("IP address"),DecodeUnicodeConsole(settings->Settings[j].IPAddress));
		if (settings->Settings[j].ManualLogin) {
			printf(LISTFORMAT "%s\n", _("Login type"), _("Manual"));
		} else {
			printf(LISTFORMAT "%s\n", _("Login type"), _("Automatic"));
		}
		if (settings->Settings[j].IsNormalAuthentication) {
			printf(LISTFORMAT "%s\n", _("Authentication type"), _("Normal"));
		} else {
			printf(LISTFORMAT "%s\n", _("Authentication type"), _("Secure"));
		}
		if (settings->Settings[j].IsISDNCall) {
			printf(LISTFORMAT "%s\n", _("Data call type"), _("ISDN"));
              	} else {
			printf(LISTFORMAT "%s\n", _("Data call type"), _("Analogue"));
		}
		switch (settings->Settings[j].Speed) {
			case WAPSETTINGS_SPEED_9600  :
				printf(LISTFORMAT "%s\n", _("Data call speed"), "9600");
				break;
			case WAPSETTINGS_SPEED_14400 :
				printf(LISTFORMAT "%s\n", _("Data call speed"), "14400");
				break;
			case WAPSETTINGS_SPEED_AUTO  :
				printf(LISTFORMAT "%s\n", _("Data call speed"), _("Auto"));
				break;
		}
		printf(LISTFORMAT "\"%s\"\n", _("User name"),DecodeUnicodeConsole(settings->Settings[j].User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings->Settings[j].Password));
		break;
	case WAPSETTINGS_BEARER_USSD:
		printf(LISTFORMAT "%s", _("Bearer"), _("USSD"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_USSD) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Service code"),DecodeUnicodeConsole(settings->Settings[j].Code));
		if (settings->Settings[j].IsIP) {
			printf(LISTFORMAT "%s\n", _("Address type"), _("IP address"));
			printf(LISTFORMAT "%s\n", _("IP address"), DecodeUnicodeConsole(settings->Settings[j].Service));
		} else {
			printf(LISTFORMAT "%s\n", _("Address type"), _("Service number"));
			printf(LISTFORMAT "%s\n", _("Service number"), DecodeUnicodeConsole(settings->Settings[j].Service));
		}
		break;
	case WAPSETTINGS_BEARER_GPRS:
		printf(LISTFORMAT "%s", _("Bearer"), _("GPRS"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_GPRS) printf(_(" (active)"));
		printf("\n");
		if (settings->Settings[j].ManualLogin) {
			printf(LISTFORMAT "%s\n", _("Login type"), _("Manual"));
		} else {
			printf(LISTFORMAT "%s\n", _("Login type"), _("Automatic"));
		}
		if (settings->Settings[j].IsNormalAuthentication) {
			printf(LISTFORMAT "%s\n", _("Authentication type"), _("Normal"));
		} else {
			printf(LISTFORMAT "%s\n", _("Authentication type"), _("Secure"));
		}
		printf(LISTFORMAT "\"%s\"\n", _("Access point"),DecodeUnicodeConsole(settings->Settings[j].DialUp));
		printf(LISTFORMAT "\"%s\"\n", _("IP address"),DecodeUnicodeConsole(settings->Settings[j].IPAddress));
		printf(LISTFORMAT "\"%s\"\n", _("User name"),DecodeUnicodeConsole(settings->Settings[j].User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings->Settings[j].Password));
	}
}

void GetSyncMLSettings(int argc, char *argv[])
{
	GSM_SyncMLSettings	settings;
	int			start,stop,j,i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		error=GSM_GetSyncMLSettings(gsm,&settings);
		Print_Error(error);
		printf("%i. ", i);
		if (settings.Name[0]==0 && settings.Name[1]==0) {
			printf(_("Set %i"),i);
		} else {
			printf("%s", DecodeUnicodeConsole(settings.Name));
		}
		if (settings.Active) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("User"),DecodeUnicodeConsole(settings.User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings.Password));
		printf(LISTFORMAT "\"%s\"\n", _("Phonebook database"),DecodeUnicodeConsole(settings.PhonebookDataBase));
		printf(LISTFORMAT "\"%s\"\n", _("Calendar database"),DecodeUnicodeConsole(settings.CalendarDataBase));
		printf(LISTFORMAT "\"%s\"\n", _("Server"),DecodeUnicodeConsole(settings.Server));
		printf(LISTFORMAT, _("Sync. phonebook"));
		if (settings.SyncPhonebook) printf("%s\n", _("enabled"));
		if (!settings.SyncPhonebook) printf("%s\n", _("disabled"));
		printf(LISTFORMAT, _("Sync. calendar"));
		if (settings.SyncCalendar) printf("%s\n", _("enabled"));
		if (!settings.SyncCalendar) printf("%s\n", _("disabled"));
		printf("\n");
		for (j=0;j<settings.Connection.Number;j++) {
			printf(LISTFORMAT, _("Connection set name"));
			if (settings.Connection.Settings[j].Title[0]==0 && settings.Connection.Settings[j].Title[1]==0) {
				printf(_("Set %i"),i);
			} else {
				printf("%s",DecodeUnicodeConsole(settings.Connection.Settings[j].Title));
			}
			printf("\n");
			DisplayConnectionSettings(&settings.Connection,j);
			printf("\n");
		}
	}
	GSM_Terminate();
}

void GetChatSettings(int argc, char *argv[])
{
	GSM_ChatSettings	settings;
	int			start,stop,j,i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		error=GSM_GetChatSettings(gsm,&settings);
		Print_Error(error);
		printf("%i. ",i);
		if (settings.Name[0]==0 && settings.Name[1]==0) {
			printf(_("Set %i"),i);
		} else {
			printf("%s", DecodeUnicodeConsole(settings.Name));
		}
		if (settings.Active) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Homepage"),DecodeUnicodeConsole(settings.HomePage));
		printf(LISTFORMAT "\"%s\"\n", _("User"),DecodeUnicodeConsole(settings.User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings.Password));
		printf("\n");
		for (j=0;j<settings.Connection.Number;j++) {
			printf(LISTFORMAT, _("Connection set name"));
			if (settings.Connection.Settings[j].Title[0]==0 && settings.Connection.Settings[j].Title[1]==0) {
				printf(_("Set %i"),i);
			} else {
				printf("%s",DecodeUnicodeConsole(settings.Connection.Settings[j].Title));
			}
			DisplayConnectionSettings(&settings.Connection,j);
			printf("\n");
		}
	}
	GSM_Terminate();
}

void GetWAPMMSSettings(int argc, char *argv[])
{
	GSM_MultiWAPSettings	settings;
	int			start,stop,j, i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		if (strcasestr(argv[1], "getwapsettings") != NULL) {
			error=GSM_GetWAPSettings(gsm,&settings);
		} else {
			error=GSM_GetMMSSettings(gsm,&settings);
		}
		Print_Error(error);
		for (j=0;j<settings.Number;j++) {
			printf("%i. ", i);
			if (settings.Settings[j].Title[0]==0 && settings.Settings[j].Title[1]==0) {
				printf(_("Set %i"),i);
			} else {
				printf("%s", DecodeUnicodeConsole(settings.Settings[j].Title));
			}
			if (settings.Active) {
				printf(" (%s)", _("active"));
			}
			printf("\n");
			if (settings.ReadOnly) {
				printf(LISTFORMAT "%s\n", _("Read only"), _("yes"));
			}
			printf(LISTFORMAT "\"%s\"\n", _("Homepage"),
				DecodeUnicodeConsole(settings.Settings[j].HomePage));
			DisplayConnectionSettings(&settings,j);
			printf("\n");
		}
	}
	GSM_Terminate();
}

void CopyBitmap(int argc, char *argv[])
{
	GSM_MultiBitmap Bitmap;
	int		i;
	GSM_Error error;

	Bitmap.Bitmap[0].Type = GSM_None;

	error=GSM_ReadBitmapFile(argv[2],&Bitmap);
	Print_Error(error);

	if (argc==3) {
		for (i=0;i<Bitmap.Number;i++) {
			switch (Bitmap.Bitmap[i].Type) {
				case GSM_StartupLogo :	printf(_("Startup logo"));		break;
				case GSM_OperatorLogo:	printf(_("Operator logo")); 	break;
				case GSM_PictureImage:	printf(_("Picture")); 	break;
				case GSM_CallerGroupLogo  :	printf(_("Caller group logo")); 	break;
				default		     : 					break;
			}
			printf(_(", width %i, height %i\n"),
				(int)Bitmap.Bitmap[i].BitmapWidth,
				(int)Bitmap.Bitmap[i].BitmapHeight);
			GSM_PrintBitmap(stdout,&Bitmap.Bitmap[i]);
		}
	} else {
		if (argc == 5) {
			for (i=0;i<Bitmap.Number;i++) {
				if (strcasecmp(argv[4],"PICTURE") == 0) {
					Bitmap.Bitmap[i].Type = GSM_PictureImage;
				} else if (strcasecmp(argv[4],"STARTUP") == 0) {
					Bitmap.Bitmap[i].Type = GSM_StartupLogo;
				} else if (strcasecmp(argv[4],"CALLER") == 0) {
					Bitmap.Bitmap[i].Type = GSM_CallerGroupLogo;
				} else if (strcasecmp(argv[4],"OPERATOR") == 0) {
					Bitmap.Bitmap[i].Type = GSM_OperatorLogo;
				} else {
					printf(_("What format of output file logo (\"%s\") ?\n"),argv[4]);
					Terminate(3);
				}
			}
		}
		error=GSM_SaveBitmapFile(argv[3],&Bitmap);
		Print_Error(error);
	}
}

void CopyRingtone(int argc, char *argv[])
{
	GSM_Ringtone 		ringtone, ringtone2;
	GSM_RingtoneFormat	Format;
	GSM_Error error;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);

	Format = ringtone.Format;
	if (argc == 5) {
		if (strcasecmp(argv[4],"RTTL") == 0) {		Format = RING_NOTETONE;
		} else if (strcasecmp(argv[4],"BINARY") == 0) {	Format = RING_NOKIABINARY;
		} else {
			printf(_("What format of output ringtone file (\"%s\") ?\n"),argv[4]);
			Terminate(3);
		}
	}

	error=GSM_RingtoneConvert(&ringtone2,&ringtone,Format);
	Print_Error(error);

	error=GSM_SaveRingtoneFile(argv[3],&ringtone2);
	Print_Error(error);
}

void PressKeySequence(int argc UNUSED, char *argv[])
{
	size_t i,Length;
	GSM_KeyCode	KeyCode[500];
	GSM_Error error;

	error = MakeKeySequence(argv[2], KeyCode, &Length);
	if (error == ERR_NOTSUPPORTED) {
		printf(_("Unknown key/function name: \"%c\"\n"),argv[2][Length]);
		Terminate(3);
	}

	GSM_Init(TRUE);

	for (i=0;i<Length;i++) {
		error=GSM_PressKey(gsm, KeyCode[i], TRUE);
		Print_Error(error);
		error=GSM_PressKey(gsm, KeyCode[i], FALSE);
		Print_Error(error);
	}

	GSM_Terminate();
}

void GetAllCategories(int argc UNUSED, char *argv[])
{
	GSM_Category		Category;
	GSM_CategoryStatus	Status;
	int			j, count;
	GSM_Error error;

	if (strcasecmp(argv[2],"TODO") == 0) {
        	Category.Type 	= Category_ToDo;
        	Status.Type 	= Category_ToDo;
	} else if (strcasecmp(argv[2],"PHONEBOOK") == 0) {
	        Category.Type 	= Category_Phonebook;
        	Status.Type 	= Category_Phonebook;
	} else {
		printf(_("What type of categories do you want to get (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	GSM_Init(TRUE);

	error=GSM_GetCategoryStatus(gsm, &Status);
	Print_Error(error);

	for (count=0,j=1;count<Status.Used;j++)
	{
    		Category.Location=j;
		error=GSM_GetCategory(gsm, &Category);

		if (error != ERR_EMPTY) {
			printf(LISTFORMAT "%i\n", _("Location"),j);

			Print_Error(error);

			printf(LISTFORMAT "\"%s\"\n\n", _("Name"),DecodeUnicodeConsole(Category.Name));
			count++;
    		}
	}

	GSM_Terminate();
}

void GetCategory(int argc, char *argv[])
{
	GSM_Category	Category;
	int		start,stop,j;
	GSM_Error error;

	if (strcasecmp(argv[2],"TODO") == 0) {
    		Category.Type = Category_ToDo;
	} else if (strcasecmp(argv[2],"PHONEBOOK") == 0) {
    		Category.Type = Category_Phonebook;
	} else {
		printf(_("What type of categories do you want to get (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	GetStartStop(&start, &stop, 2, argc - 1, argv + 1);

	GSM_Init(TRUE);

	for (j=start;j<=stop;j++)
	{
    		printf(LISTFORMAT "%i\n", _("Location"),j);

		Category.Location=j;

		error=GSM_GetCategory(gsm, &Category);
		if (error != ERR_EMPTY) Print_Error(error);

		if (error == ERR_EMPTY) {
			printf("%s\n", _("Entry is empty"));
		} else {
        		printf(LISTFORMAT "\"%s\"\n\n", _("Name"),DecodeUnicodeConsole(Category.Name));
    		}
	}

	GSM_Terminate();
}

void AddCategory(int argc UNUSED, char *argv[])
{
	GSM_Category	Category;
	int		Length;
	GSM_Error error;

	if (strcasecmp(argv[2],"TODO") == 0) {
    		Category.Type = Category_ToDo;
	} else if (strcasecmp(argv[2],"PHONEBOOK") == 0) {
    		Category.Type = Category_Phonebook;
	} else {
		printf(_("What type of category do you want to add (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	GSM_Init(TRUE);

	Length = strlen(argv[3]);
	if (Length > GSM_MAX_CATEGORY_NAME_LENGTH) {
		printf(_("Text too long, truncating to %d chars!\n"), GSM_MAX_CATEGORY_NAME_LENGTH);
		Length = GSM_MAX_CATEGORY_NAME_LENGTH;
	}
	EncodeUnicode(Category.Name, argv[3], Length);

	Category.Location = 0;

	error = GSM_AddCategory(gsm, &Category);

	Print_Error(error);

	GSM_Terminate();
}

void GetSecurityStatus(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Init(TRUE);

	PrintSecurityStatus();

	GSM_Terminate();
}

void EnterSecurityCode(int argc UNUSED, char *argv[])
{
	GSM_SecurityCode Code;
	GSM_Error error;
	char message[200];
#ifdef HAVE_GETPASS
	char *pass;
#endif

	if (strcasecmp(argv[2],"PIN") == 0) {		Code.Type = SEC_Pin;
	} else if (strcasecmp(argv[2],"PUK") == 0) {	Code.Type = SEC_Puk;
	} else if (strcasecmp(argv[2],"PIN2") == 0) {	Code.Type = SEC_Pin2;
	} else if (strcasecmp(argv[2],"PUK2") == 0) {	Code.Type = SEC_Puk2;
	} else if (strcasecmp(argv[2],"PHONE") == 0) {	Code.Type = SEC_Phone;
	} else if (strcasecmp(argv[2],"NETWORK") == 0) {	Code.Type = SEC_Network;
	} else {
		printf_err("%s: %s\n", _("Invalid security code type"), argv[2]);
		Terminate(3);
	}

	if (strcmp(argv[3], "-") == 0) {
		sprintf(message, _("Enter %s code: "), argv[2]);
#ifdef HAVE_GETPASS
		pass = getpass(message);
		strcpy(Code.Code, pass);
#else
		/* Read code from stdin */
#ifdef HAVE_UNISTD_H
		if (isatty(fileno(stdin))) {
			printf("%s", message);
		}
#endif
		if (fscanf(stdin, "%15s", Code.Code) != 1) {
			printf_err("%s\n", _("No PIN code entered!"));
			Terminate(3);
		}
#endif
	} else {
		strcpy(Code.Code,argv[3]);
	}

	GSM_Init(TRUE);

	error=GSM_EnterSecurityCode(gsm,Code);
	Print_Error(error);

	GSM_Terminate();
}

void GetProfile(int argc, char *argv[])
{
	GSM_Profile 		Profile;
	int			start,stop,i,j,k;
	GSM_Bitmap		caller[5];
	gboolean			callerinit[5],special;
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	for (i=0;i<5;i++) callerinit[i] = FALSE;

	GSM_Init(TRUE);

	error=GSM_GetRingtonesInfo(gsm,&Info);
	if (error != ERR_NONE) Info.Number = 0;

	for (i=start;i<=stop;i++) {
		Profile.Location=i;
		error=GSM_GetProfile(gsm,&Profile);
		if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
		Print_Error(error);

		printf("%i. \"%s\"",i,DecodeUnicodeConsole(Profile.Name));
		if (Profile.Active)		printf(_(" (active)"));
		if (Profile.DefaultName) 	printf(_(" (default name)"));
		if (Profile.HeadSetProfile) 	printf(_(" (Head set profile)"));
		if (Profile.CarKitProfile) 	printf(_(" (Car kit profile)"));
		printf("\n");
		for (j=0;j<Profile.FeaturesNumber;j++) {
			special = FALSE;
			switch (Profile.FeatureID[j]) {
			case Profile_MessageToneID:
			case Profile_RingtoneID:
				special = TRUE;
				if (Profile.FeatureID[j] == Profile_RingtoneID) {
					printf(LISTFORMAT, _("Ringtone ID"));
				} else {
					printf(LISTFORMAT, _("Message alert tone ID"));
				}
				if (UnicodeLength(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j]))!=0) {
					printf("\"%s\"\n",DecodeUnicodeConsole(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])));
				} else {
					printf("%i\n",Profile.FeatureValue[j]);
				}
				break;
			case Profile_CallerGroups:
				special = TRUE;
				printf(LISTFORMAT, _("Call alert for"));
				for (k=0;k<5;k++) {
					if (Profile.CallerGroups[k]) {
						if (!callerinit[k]) {
							caller[k].Type	 	= GSM_CallerGroupLogo;
							caller[k].Location 	= k + 1;
							error=GSM_GetBitmap(gsm,&caller[k]);
							if (error == ERR_SECURITYERROR) {
								NOKIA_GetDefaultCallerGroupName(&caller[k]);
							} else {
								Print_Error(error);
							}
							callerinit[k]	= TRUE;
						}
						printf(" \"%s\"",DecodeUnicodeConsole(caller[k].Text));
					}
				}
				printf("\n");
				break;
			case Profile_ScreenSaverNumber:
				special = TRUE;
				printf(LISTFORMAT, _("Screen saver number"));
				printf("%i\n",Profile.FeatureValue[j]);
				break;
			case Profile_CallAlert  	: printf(LISTFORMAT, _("Incoming call alert")); break;
			case Profile_RingtoneVolume 	: printf(LISTFORMAT, _("Ringtone volume")); break;
			case Profile_Vibration		: printf(LISTFORMAT, _("Vibrating alert")); break;
			case Profile_MessageTone	: printf(LISTFORMAT, _("Message alert tone")); break;
			case Profile_KeypadTone		: printf(LISTFORMAT, _("Keypad tones")); break;
			case Profile_WarningTone	: printf(LISTFORMAT, _("Warning (games) tones")); break;
			case Profile_ScreenSaver	: printf(LISTFORMAT, _("Screen saver")); break;
			case Profile_ScreenSaverTime	: printf(LISTFORMAT, _("Screen saver timeout")); break;
			case Profile_AutoAnswer		: printf(LISTFORMAT, _("Automatic answer")); break;
			case Profile_Lights		: printf(LISTFORMAT, _("Lights")); break;
			default:
				printf("%s\n", _("Unknown"));
				special = TRUE;
			}
			if (!special) {
				switch (Profile.FeatureValue[j]) {
				case PROFILE_VOLUME_LEVEL1 		:
				case PROFILE_KEYPAD_LEVEL1 		: printf("%s\n", _("Level 1")); 		break;
				case PROFILE_VOLUME_LEVEL2 		:
				case PROFILE_KEYPAD_LEVEL2 		: printf("%s\n", _("Level 2"));		break;
				case PROFILE_VOLUME_LEVEL3 		:
				case PROFILE_KEYPAD_LEVEL3 		: printf("%s\n", _("Level 3")); 		break;
				case PROFILE_VOLUME_LEVEL4 		: printf("%s\n", _("Level 4")); 		break;
				case PROFILE_VOLUME_LEVEL5 		: printf("%s\n", _("Level 5")); 		break;
				case PROFILE_MESSAGE_NOTONE 		:
				case PROFILE_AUTOANSWER_OFF		:
				case PROFILE_LIGHTS_OFF  		:
				case PROFILE_SAVER_OFF			:
				case PROFILE_WARNING_OFF		:
				case PROFILE_CALLALERT_OFF	 	:
				case PROFILE_VIBRATION_OFF 		:
				case PROFILE_KEYPAD_OFF	   		: printf("%s\n", _("Off"));	  	break;
				case PROFILE_CALLALERT_RINGING   	: printf("%s\n", _("Ringing"));		break;
				case PROFILE_CALLALERT_BEEPONCE  	:
				case PROFILE_MESSAGE_BEEPONCE 		: printf("%s\n", _("Beep once")); 	break;
				case PROFILE_CALLALERT_RINGONCE  	: printf("%s\n", _("Ring once"));	break;
				case PROFILE_CALLALERT_ASCENDING 	: printf("%s\n", _("Ascending"));        break;
				case PROFILE_CALLALERT_CALLERGROUPS	: printf("%s\n", _("Caller groups"));	break;
				case PROFILE_MESSAGE_STANDARD 		: printf("%s\n", _("Standard"));  	break;
				case PROFILE_MESSAGE_SPECIAL 		: printf("%s\n", _("Special"));	 	break;
				case PROFILE_MESSAGE_ASCENDING		: printf("%s\n", _("Ascending")); 	break;
				case PROFILE_MESSAGE_PERSONAL		: printf("%s\n", _("Personal"));		break;
				case PROFILE_AUTOANSWER_ON		:
				case PROFILE_WARNING_ON			:
				case PROFILE_SAVER_ON			:
				case PROFILE_VIBRATION_ON 		: printf("%s\n", _("On"));  		break;
				case PROFILE_VIBRATION_FIRST 		: printf("%s\n", _("Vibrate first"));	break;
				case PROFILE_LIGHTS_AUTO 		: printf("%s\n", _("Auto")); 		break;
				case PROFILE_SAVER_TIMEOUT_5SEC	 	: PRINTSECONDS(5); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_20SEC 	: PRINTSECONDS(20); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_1MIN	 	: PRINTMINUTES(1); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_2MIN	 	: PRINTMINUTES(2); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_5MIN	 	: PRINTMINUTES(5); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_10MIN 	: PRINTMINUTES(10); printf("\n"); break;
				default					: printf("%s\n", _("Unknown"));
				}
			}
		}
		printf("\n");
	}

	GSM_Terminate();

	if (Info.Ringtone) free(Info.Ringtone);
}

void GetSpeedDial(int argc, char *argv[])
{
	GSM_SpeedDial		SpeedDial;
	GSM_MemoryEntry		Phonebook;
	int			start,stop,Name,Number,Group,i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		SpeedDial.Location=i;
		error=GSM_GetSpeedDial(gsm,&SpeedDial);
		printf(LISTFORMAT "%i\n", _("Location"), i);
		switch (error) {
		case ERR_EMPTY:
			printf("%s\n", _(" speed dial not assigned"));
			break;
		default:
			Print_Error(error);

			Phonebook.Location	= SpeedDial.MemoryLocation;
			Phonebook.MemoryType 	= SpeedDial.MemoryType;
			error=GSM_GetMemory(gsm,&Phonebook);

			GSM_PhonebookFindDefaultNameNumberGroup(&Phonebook, &Name, &Number, &Group);

			if (Name != -1) printf(LISTFORMAT "\"%s\"\n", _("Name"), DecodeUnicodeConsole(Phonebook.Entries[Name].Text));
			printf(LISTFORMAT "\"%s\"\"", _("Number"), DecodeUnicodeConsole(Phonebook.Entries[SpeedDial.MemoryNumberID-1].Text));
		}
		printf("\n");
	}

	GSM_Terminate();
}

void ResetPhoneSettings(int argc UNUSED, char *argv[])
{
	GSM_ResetSettingsType Type;
	GSM_Error error;

	if (strcasecmp(argv[2],"PHONE") == 0) {		 Type = GSM_RESET_PHONESETTINGS;
	} else if (strcasecmp(argv[2],"UIF") == 0) {	 Type = GSM_RESET_USERINTERFACE;
	} else if (strcasecmp(argv[2],"ALL") == 0) {	 Type = GSM_RESET_USERINTERFACE_PHONESETTINGS;
	} else if (strcasecmp(argv[2],"DEV") == 0) {	 Type = GSM_RESET_DEVICE;
	} else if (strcasecmp(argv[2],"FACTORY") == 0) { Type = GSM_RESET_FULLFACTORY;
	} else {
		printf(_("What type of reset phone settings (\"%s\") ?\n"),argv[2]);
		Terminate(3);
	}

	GSM_Init(TRUE);

	error=GSM_ResetPhoneSettings(gsm,Type);
	Print_Error(error);

 	GSM_Terminate();
}

void SendDTMF(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	error=GSM_SendDTMF(gsm,argv[2]);
	Print_Error(error);

 	GSM_Terminate();
}

void GetDisplayStatus(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_DisplayFeatures 	Features;
	int			i;

	GSM_Init(TRUE);

	error=GSM_GetDisplayStatus(gsm,&Features);
	Print_Error(error);

	printf(LISTFORMAT "\n", _("Currently shown on the display"));

	for (i=0;i<Features.Number;i++) {
		switch(Features.Feature[i]) {
		case GSM_CallActive	: printf("%s\n", _("Call active"));	break;
		case GSM_UnreadSMS	: printf("%s\n", _("Unread SMS"));	break;
		case GSM_VoiceCall	: printf("%s\n", _("Voice call"));	break;
		case GSM_FaxCall	: printf("%s\n", _("Fax call"));	break;
		case GSM_DataCall	: printf("%s\n", _("Data call"));	break;
		case GSM_KeypadLocked	: printf("%s\n", _("Keypad locked"));	break;
		case GSM_SMSMemoryFull	: printf("%s\n", _("SMS memory full"));
		}
	}

 	GSM_Terminate();
}

void SetAutoNetworkLogin(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_Init(TRUE);

	error=GSM_SetAutoNetworkLogin(gsm);
	Print_Error(error);

 	GSM_Terminate();
}

void GetFMStation(int argc, char *argv[])
{
	GSM_FMStation 	Station;
	int		start,stop,i;
	GSM_Error error;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		Station.Location=i;
		error=GSM_GetFMStation(gsm,&Station);
		printf(LISTFORMAT "%i\n", _("Location"), i);
		switch (error) {
		case ERR_EMPTY:
			printf("%s\n", _("Entry is empty"));
		    	break;
		case ERR_NONE:
			printf(LISTFORMAT "\"%s\"\n", _("Station name"), DecodeUnicodeConsole(Station.StationName));
			printf(LISTFORMAT "%.1f MHz\n", _("Frequency"), Station.Frequency);
			break;
		default:
			Print_Error(error);
		}
	}
	GSM_Terminate();
}

void CallDivert(int argc, char *argv[])
{
	GSM_Error error;
	GSM_MultiCallDivert cd;
	int i;

	     if (strcasecmp("get",	argv[2]) == 0) {}
	else if (strcasecmp("set",	argv[2]) == 0) {}
	else {
		printf(_("Unknown divert action (\"%s\")\n"),argv[2]);
		Terminate(3);
	}

	     if (strcasecmp("all"       , argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_AllTypes  ;}
	else if (strcasecmp("busy"      , argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_Busy      ;}
	else if (strcasecmp("noans"     , argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_NoAnswer  ;}
	else if (strcasecmp("outofreach", argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_OutOfReach;}
	else {
		printf(_("Unknown divert type (\"%s\")\n"),argv[3]);
		Terminate(3);
	}

	     if (strcasecmp("all"  , argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_AllCalls  ;}
	else if (strcasecmp("voice", argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_VoiceCalls;}
	else if (strcasecmp("fax"  , argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_FaxCalls  ;}
	else if (strcasecmp("data" , argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_DataCalls ;}
	else {
		printf(_("Unknown call type (\"%s\")\n"),argv[4]);
		Terminate(3);
	}

	GSM_Init(TRUE);

	if (strcasecmp("get", argv[2]) == 0) {
		error = GSM_GetCallDivert(gsm,&cd);
	    	Print_Error(error);
		printf(_("Query:"));
		printf("\n");
		printf("  " LISTFORMAT, _("Divert type"));
	} else {
		cd.Request.Number[0] = 0;
		cd.Request.Number[1] = 0;
		if (argc > 5) EncodeUnicode(cd.Request.Number,argv[5],strlen(argv[5]));

		cd.Request.Timeout = 0;
		if (argc > 6) cd.Request.Timeout = atoi(argv[6]);

		error = GSM_SetCallDivert(gsm,&cd);
	    	Print_Error(error);
		printf(_("Changed:"));
		printf("\n");
		printf("  " LISTFORMAT, _("Divert type"));
	}

      	switch (cd.Request.DivertType) {
   		case GSM_DIVERT_Busy      : printf(_("when busy"));				break;
	        case GSM_DIVERT_NoAnswer  : printf(_("when not answered"));			break;
      	        case GSM_DIVERT_OutOfReach: printf(_("when phone off or no coverage"));	break;
                case GSM_DIVERT_AllTypes  : printf(_("all types of diverts"));			break;
                default		          : printf(_("unknown %i"),cd.Request.DivertType);			break;
        }
	printf("\n");
	printf("  " LISTFORMAT, _("Call type"));
	switch (cd.Request.CallType) {
	 	case GSM_DIVERT_VoiceCalls: printf(_("voice"));				break;
                case GSM_DIVERT_FaxCalls  : printf(_("fax"));				break;
                case GSM_DIVERT_DataCalls : printf(_("data"));		 		break;
		case GSM_DIVERT_AllCalls  : printf(_("data & fax & voice"));		break;
                default                   : printf(_("unknown %i"),cd.Request.CallType);   		break;
        }
	printf("\n");
	printf(_("Response:"));
	printf("\n");

	for (i=0;i<cd.Response.EntriesNum;i++) {
		printf("\n");
		printf("  " LISTFORMAT, _("Call type"));
        	switch (cd.Response.Entries[i].CallType) {
                	case GSM_DIVERT_VoiceCalls: printf(_("voice"));		 	break;
                	case GSM_DIVERT_FaxCalls  : printf(_("fax"));		 	break;
                	case GSM_DIVERT_DataCalls : printf(_("data"));		 	break;
                	default                   : printf(_("unknown %i"),cd.Response.Entries[i].CallType);break;
              	}
		printf("\n");
		printf("  " LISTFORMAT, _("Timeout"));
		PRINTSECONDS(cd.Response.Entries[i].Timeout);
		printf("\n");
                printf("  "LISTFORMAT "%s\n", _("Number"),DecodeUnicodeString(cd.Response.Entries[i].Number));
        }
	printf("\n");

	GSM_Terminate();
}

void CancelAllDiverts(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_Init(TRUE);

	error = GSM_CancelAllDiverts(gsm);
    	Print_Error(error);

	GSM_Terminate();
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

