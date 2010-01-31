/* (c) by Walek */

#include "../../../libgammu/gsmstate.h"

#ifdef GSM_ENABLE_ATGEN

#include <string.h>

#include "../../../libgammu/misc/coding/coding.h"
#include "../../../libgammu/gsmcomon.h"
#include "../../../libgammu/service/gsmnet.h"
#include "../../../libgammu/phone/at/atgen.h"
#include "../../gammu.h"
#include "dsiemens.h"
#include "chiffre.h"
#include "../../../helper/cmdline.h"

extern GSM_Error  ATGEN_GetSIMIMSI (GSM_StateMachine *sm, char *IMSI);
extern GSM_Error  ATGEN_GetMemoryStatus (GSM_StateMachine *sm, GSM_MemoryStatus *status);
extern GSM_Error  ATGEN_SetMemory (GSM_StateMachine *sm, GSM_MemoryEntry *pbk);
extern GSM_Reply_Function UserReplyFunctionsAtS[];

gboolean 	new_variable;
GSM_Error CheckSiemens(void)
{
	if (gsm->Phone.Data.Priv.ATGEN.Manufacturer != AT_Siemens) return ERR_NOTSUPPORTED;
	return ERR_NONE;
}

GSM_Error ATSIEMENS_Reply_GetSAT(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	GSM_Phone_ATGENData		*Priv = &(sm->Phone.Data.Priv.ATGEN);
	GSM_SAT_Measure_results		MeasureResult;
	unsigned char 			buf[256];
        int                             length,i,rep,ChNo=1,result=0,origARFCN=0;
	size_t j=0;
    	int				freq_tmp,frequency[24];
   	GSM_NetworkInfo			Network;

    	if (Priv->ReplyState!=AT_Reply_OK) return ERR_UNKNOWN;
    	if (sm->Protocol.Data.AT.EditMode) sm->Protocol.Data.AT.EditMode = FALSE;
	if (strstr(GetLineString(msg.Buffer,&Priv->Lines,2),"SSTK")) {
    	    length = strlen(GetLineString(msg.Buffer,&Priv->Lines,2))-7;
    	    DecodeHexBin(buf, GetLineString(msg.Buffer,&Priv->Lines,2)+7,length);
	    if (buf[0]==0x7f) {
		new_variable=TRUE;
		return ERR_NONE;
	    }
	    else return ERR_UNKNOWN;
	}
    	if (!strstr(GetLineString(msg.Buffer,&Priv->Lines,3),"SSTK")) return ERR_UNKNOWN;

    	length = strlen(GetLineString(msg.Buffer,&Priv->Lines,3))-7;
    	DecodeHexBin(buf, GetLineString(msg.Buffer,&Priv->Lines,3)+7,length);

    	if (buf[3]!=0x26) return ERR_UNKNOWN;

#ifdef DEBUG
    	smprintf(sm, "SAT command: Provide Local Information\nFunction: ");
    	switch (buf[4]) {
        	case 00: smprintf(sm, "Loc Info\n");			break;
		case 01: smprintf(sm, "IMEI\n");				break;
		case 02: smprintf(sm, "Network Measure\n");		break;
		case 03: smprintf(sm, "Date time and timezone\n");	break;
		case 04: smprintf(sm, "Language setting\n");		break;
		case 05: smprintf(sm, "Timing advance\n");		break;
    	}
#endif
    	/* Loc Info (MCC, MNC, LAC, Cell ID) */
    	if (buf[4]==00) {
        	DecodeBCD (Network.NetworkCode,buf+14,2);
        	Network.NetworkCode[3] = ' ';
        	DecodeBCD (Network.NetworkCode+4,buf+16,1);
        	EncodeHexBin (Network.LAC,buf+17,2);
        	EncodeHexBin (Network.CID,buf+19,2);

		printf("   Network code              : %s\n",Network.NetworkCode);
		printf("   Network name for Gammu    : %s\n",
			DecodeUnicodeString(GSM_GetNetworkName(Network.NetworkCode)));
		printf("   CID   : %s\n",Network.CID);
		printf("   LAC   : %s\n",Network.LAC);
    	}

    	/* Network Measure */
    	if (buf[4]==02) {

	    for (i=0;i<24;i++) frequency[i]=0;
	    if (!new_variable) {
	        GetBufferI(buf+32,&j,&result,7);
	        result &= 0x67;
	        if (result !=0x47) return ERR_NOTSUPPORTED;
	    }
#ifdef DEBUG
	    if (new_variable)	smprintf(sm, "New variable Bitmap format\n");
	    else	 	smprintf(sm, "Old variable Bitmap format\n");
#endif
            GetBufferI(buf+32,&j,&origARFCN,10);
	/* 10 bit origin ARFCN or first frequency (new variable format) */
#ifdef DEBUG
	    smprintf(sm, "Origin BCCH = %i\n",origARFCN);
#endif
            rep     = buf[31]*8;
	    if (!new_variable ){
    	        for (i=0;i<rep;i++){
	    	    result = 0;
                    GetBufferI(buf+32,&j,&result,1);
		    if (result) {
    	    		frequency[ChNo]=i+origARFCN+1;
			ChNo++;
		    }
	    	}
	    }
	    else {
	        frequency[ChNo++]=origARFCN;
		for (i=0; i<rep; i+=10){
		    result = 0;
		    GetBufferI(buf+32,&j,&result,10);
		    if (!result) break;
		    frequency[ChNo++]=result;
		}
		j=1;
		while (j) {
		    j=0;
		    for (i=0; i<ChNo-1; i++){
			if (frequency[i] > frequency[i+1]){
			    freq_tmp=frequency[i];
			    frequency[i]=frequency[i+1];
			    frequency[i+1]=freq_tmp;
			    j=1;
			}
		    }
		}
	    };
#ifdef DEBUG
    	smprintf(sm, "Neighbor BCCH list: ");
	for (i=1;i<ChNo;i++) smprintf(sm, "%d ",frequency[i]);
	smprintf(sm, "\n");
#endif
	j	= 0;
	result	= 0;
	GetBufferI(buf+14,&j,&result,1);
	if (result) 	MeasureResult.BA_used=TRUE;
	else		MeasureResult.BA_used=FALSE;

	result	= 0;
	GetBufferI(buf+14,&j,&result,1);
	if (result) 	MeasureResult.DTX_used=TRUE;
	else		MeasureResult.DTX_used=FALSE;

	result	= 0;
	GetBufferI(buf+14,&j,&result,6);
	MeasureResult.RXLEV_FullServicingCell=result-110;

	j++;	//skip spare bit
	result	= 0;
	GetBufferI(buf+14,&j,&result,1);
	if (result) 	MeasureResult.MeasValid=TRUE;
	else		MeasureResult.MeasValid=FALSE;

	result	= 0;
	GetBufferI(buf+14,&j,&result,6);
	MeasureResult.RXLEV_SubServicingCell=result-110;

	j++;	//skip spare bit
	result	= 0;
	GetBufferI(buf+14,&j,&result,3);
	MeasureResult.RXQUAL_FullServicingCell=result;

	result	= 0;
	GetBufferI(buf+14,&j,&result,3);
	MeasureResult.RXQUAL_SubServicingCell=result;

	printf ("RX Level FULL Servicing Cell = %i\n",MeasureResult.RXLEV_FullServicingCell);
	printf ("RX Level Sub Servicing Cell = %i\n",MeasureResult.RXLEV_FullServicingCell);

	printf ("RX Quality Full Servicing Cell = %i\n",MeasureResult.RXQUAL_FullServicingCell);
	printf ("RX Quality Sub Servicing Cell = %i\n",MeasureResult.RXQUAL_SubServicingCell);

	result	= 0;
	GetBufferI(buf+14,&j,&result,3);
	MeasureResult.NO_NCELL_M=result;

	rep=MeasureResult.NO_NCELL_M;

	for (i=0;i<MeasureResult.NO_NCELL_M;i++) {
	    result	= 0;
	    GetBufferI(buf+14,&j,&result,6);
	    MeasureResult.NeighbourCell[i].RxLev = result-110;

	    result	= 0;
	    GetBufferI(buf+14,&j,&result,5);
	    if (new_variable)
		   MeasureResult.NeighbourCell[i].ChFreq = frequency[result+1];
	    else   MeasureResult.NeighbourCell[i].ChFreq = frequency[result];

	    result	= 0;
	    GetBufferI(buf+14,&j,&result,3);
	    MeasureResult.NeighbourCell[i].NB = 10 * result;
	    result	= 0;
	    GetBufferI(buf+14,&j,&result,3);
	    MeasureResult.NeighbourCell[i].NB +=  result;

	    if (MeasureResult.NeighbourCell[i].ChFreq)
		printf("CH = %i,\t",MeasureResult.NeighbourCell[i].ChFreq);
	    else
		printf("CH = Unknown\t");
		printf("RX Lev = %i dBm\t",MeasureResult.NeighbourCell[i].RxLev);
		printf("BSIC CELL = %i\n",MeasureResult.NeighbourCell[i].NB);
	    }
    	}
#ifdef DEBUG
    	if  (buf[4]==05) {				//Timing Advance
		if (buf[11]) smprintf(sm, "Unknown Timing Advance\n");
		else smprintf(sm, "Timing Advance = %i\n",buf[14] & 0x3f);
    	}
#endif
    	return ERR_NONE;
}

GSM_Error ATSIEMENS_Reply_GetNetmon(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
    	GSM_Phone_ATGENData	*Priv = &(sm->Phone.Data.Priv.ATGEN);
    	int 			i=2;

    	if (!strstr(GetLineString(msg.Buffer,&Priv->Lines,1),"AT^S^MI")) return ERR_UNKNOWN;
    	while (strlen(GetLineString(msg.Buffer,&Priv->Lines,i+1)))
		printf("%s\n",GetLineString(msg.Buffer,&Priv->Lines,i++));
    	printf("\n");
    	return ERR_NONE;
}

GSM_Error ATSIEMENS_GetSAT(GSM_StateMachine *sm)
{
    	GSM_Phone_ATGENData	*Priv = &(sm->Phone.Data.Priv.ATGEN);
    	GSM_Error		error;
    	const char		*reqSAT[]= {"D009810301260082028182",
				    	    "D009810301260282028182",
				    	    "D009810301260582028182"};
	char req[32];
    	int 			i,len;

    	if (Priv->Manufacturer!=AT_Siemens) return ERR_NOTSUPPORTED;

        strcpy(req, "AT^SSTK=?\r");
        error = GSM_WaitForAutoLen(sm, req, 0x00, 3, ID_User1);

    	for (i=0;i<3;i++){
		len				= strlen(reqSAT[i]);
		sm->Protocol.Data.AT.EditMode 	= TRUE;
        	sprintf(req, "AT^SSTK=%i,1\r",len/2);
        	error = GSM_WaitForAutoLen(sm, req, 0x00, 3, ID_User1);
		sm->Phone.Data.DispatchError	= ERR_TIMEOUT;
		sm->Phone.Data.RequestID		= ID_User1;
    		error = sm->Protocol.Functions->WriteMessage(sm, reqSAT[i], len, 0x00);
    		if (error!=ERR_NONE) return error;
    		error = sm->Protocol.Functions->WriteMessage(sm, "\x1A", 1, 0x00);
		if (error!=ERR_NONE) return error;
        	error = GSM_WaitForOnce (sm, NULL,0x00, 0x00, 4);
    		if (error!=ERR_NONE) return error;
    	}
    	return ERR_NONE;
}

GSM_Error ATSIEMENS_GetNetmon(GSM_StateMachine *sm,int test_no)
{
	GSM_Phone_ATGENData	*Priv = &(sm->Phone.Data.Priv.ATGEN);
	unsigned char		req[32];

	if (Priv->Manufacturer!=AT_Siemens) return ERR_NOTSUPPORTED;
	sprintf(req, "AT^S^MI=%d\r",test_no);
	printf ("Siemens NetMonitor test #%i\n",test_no);
	return GSM_WaitForAutoLen(sm, req, 0x00, 3, ID_User2);
}

GSM_Error ATSIEMENS_ActivateNetmon (GSM_StateMachine *sm,int netmon_type)
{
	GSM_Phone_ATGENData	*Priv = &(sm->Phone.Data.Priv.ATGEN);
	char		req[32];

	if (Priv->Manufacturer!=AT_Siemens) return ERR_NOTSUPPORTED;

	strcpy(req, "AT\r");
	printf ("Activate Siemens NetMonitor\n");
	siemens_code (req,req,2);

	return GSM_WaitForAutoLen(sm, req, 0x00, 3, ID_User2);
}

void ATSIEMENSActivateNetmon(int argc, char *argv[])
{
	GSM_MemoryStatus status;
	GSM_MemoryEntry	 pbk;
	GSM_Error error;
	int 		 netmon_type, pbk_maxlocation;
	char 		 imsi[15], NetMonCode[32];

	GSM_Init(TRUE);
	if (CheckSiemens()==ERR_NOTSUPPORTED) Print_Error(ERR_NOTSUPPORTED);
	gsm->User.UserReplyFunctions=UserReplyFunctionsAtS;

	printf ("Activate NetMonitor...\n");
	netmon_type = GetInt(argv[2]);

	if ((netmon_type==1) || (netmon_type==2)) {
	    error   = ATGEN_GetSIMIMSI (gsm,imsi);
	    Print_Error(error);
	    siemens_code(imsi,NetMonCode,netmon_type);

	    status.MemoryType = MEM_SM;
	    error = ATGEN_GetMemoryStatus (gsm,&status);
	    Print_Error(error);

	    pbk_maxlocation = status.MemoryUsed+status.MemoryFree;
	    pbk.MemoryType = MEM_SM;
	    pbk.Location = pbk_maxlocation;
	    pbk.EntriesNum = 2;
	    pbk.Entries[0].EntryType = PBK_Number_General;
	    EncodeUnicode (pbk.Entries[0].Text,NetMonCode,strlen(NetMonCode));
	    pbk.Entries[1].EntryType = PBK_Text_Name;
	    strcpy(NetMonCode, "Net Monitor");
	    EncodeUnicode (pbk.Entries[1].Text,NetMonCode,strlen(NetMonCode));
	    error = ATGEN_SetMemory (gsm, &pbk);
	    Print_Error(error);
	}
	else printf ("NetMonitor type should be:\n1 - full Netmon\n2 - simple NetMon\n");

	GSM_Terminate();
}

void ATSIEMENSSATNetmon(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);
	if (CheckSiemens()==ERR_NOTSUPPORTED) Print_Error(ERR_NOTSUPPORTED);
	gsm->User.UserReplyFunctions=UserReplyFunctionsAtS;

	printf ("Getting Siemens Sim Application Toolkit NetMonitor...\n");

	error=ATSIEMENS_GetSAT(gsm);
	Print_Error(error);
	GSM_Terminate();
}

void ATSIEMENSNetmonitor(int argc, char *argv[])
{
	int test_no;
	GSM_Error error;

	GSM_Init(TRUE);
	if (CheckSiemens()==ERR_NOTSUPPORTED) Print_Error(ERR_NOTSUPPORTED);
	gsm->User.UserReplyFunctions=UserReplyFunctionsAtS;

	printf ("Getting Siemens NetMonitor...\n");
	test_no = GetInt(argv[2]);
	error   = ATSIEMENS_GetNetmon (gsm,test_no+1);
	Print_Error(error);
	GSM_Terminate();
}

GSM_Reply_Function UserReplyFunctionsAtS[] = {
	{ATSIEMENS_Reply_GetSAT,	"AT^SSTK",	0x00,0x00,ID_User1	},
	{ATSIEMENS_Reply_GetNetmon,	"AT^S^MI",	0x00,0x00,ID_User2	},
	{NULL,				"\x00",		0x00,0x00,ID_None	}
};
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
