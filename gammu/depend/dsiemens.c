
#include "../../common/gsmstate.h"

#ifdef GSM_ENABLE_ATGEN

#include <string.h>

#include "../../common/misc/coding/coding.h"
#include "../../common/gsmcomon.h"
#include "../../common/service/gsmnet.h"
#include "../../common/phone/at/atgen.h"
#include "../gammu.h"
#include "dsiemens.h"

extern GSM_Reply_Function UserReplyFunctionsAtS[];

GSM_Error CheckSiemens()
{
	if (s.Phone.Data.Priv.ATGEN.Manufacturer != AT_Siemens) return GE_NOTSUPPORTED;
	return GE_NONE;
}

GSM_Error ATSIEMENS_Reply_GetSAT(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    GSM_Phone_ATGENData		*Priv = &s->Phone.Data.Priv.ATGEN;
    GSM_SAT_Measure_results	MeasureResult;
    unsigned char 		buf[256];
    int				length,i,j,result,rep,origARFCN,ChNo=1;
    unsigned int		frequency[12];
    GSM_NetworkInfo		Network;

    if (Priv->ReplyState!=AT_Reply_OK) return GE_UNKNOWN;
    if (s->Protocol.Data.AT.EditMode) s->Protocol.Data.AT.EditMode = false;
    if (!strstr(GetLineString(msg.Buffer,Priv->Lines,3),"SSTK")) return GE_UNKNOWN;
        
    length = strlen(GetLineString(msg.Buffer,Priv->Lines,3))-7;
    DecodeHexBin(buf, GetLineString(msg.Buffer,Priv->Lines,3)+7,length);

    if (buf[3]!=0x26) return GE_UNKNOWN;
    
#ifdef DEBUG
    printf ("SAT command: Provide Local Information\nFunction: ");
    switch (buf[4]) {
        case 00: dprintf ("Loc Info\n");		break;
	case 01: dprintf ("IMEI\n");			break;
	case 02: dprintf ("Network Measure\n");		break;
	case 03: dprintf ("Date time and timezone\n");	break;
	case 04: dprintf ("Language setting\n");	break;
	case 05: dprintf ("Timing advance\n");		break;
    }
#endif

    /* Loc Info (MCC, MNC, LAC, Cell ID) */
    if (buf[4]==00) {
        DecodeBCD (Network.NetworkCode,buf+14,2);
        Network.NetworkCode[3] = ' ';
        DecodeBCD (Network.NetworkCode+4,buf+16,1);
        EncodeHexBin (Network.LAC,buf+17,2);
        EncodeHexBin (Network.CellID,buf+19,2);

	smprintf(s, "   Network code              : %s\n",Network.NetworkCode);
	smprintf(s, "   Network name for Gammu    : %s\n",
	    DecodeUnicodeString(GSM_GetNetworkName(Network.NetworkCode)));
	smprintf(s, "   CellID: %s\n",Network.CellID);
	smprintf(s, "   LAC   : %s\n",Network.LAC);
    }

    /* Network Measure */
    if (buf[4]==02) {
	j	= 0;
	rep	= buf[31]*8;
	result	= 0;
	j	= BitUnPackI(buf+32,j,&result,7);
	result &= 0x67;

	/* 010xx111B - only variable bit map format supported */
	if (result !=0x47) return GE_NOTSUPPORTED;
	
	origARFCN = 0;
	j	  = BitUnPackI(buf+32,j,&origARFCN,10);
	/* 10 bit origin ARFCN */
    	dprintf("Origin BCCH = %i\n",origARFCN);
	
	for (i=0;i<rep;i++){
	    result = 0; 
	    j	   = BitUnPackI(buf+32,j,&result,1);
	    if (result) {
		frequency[ChNo]=i+origARFCN+1;
		ChNo++;
	    }
	}    
	
	j	= 0;
	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,1);
	if (result) 	MeasureResult.BA_used=true;
	else		MeasureResult.BA_used=false;

	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,1);
	if (result) 	MeasureResult.DTX_used=true;
	else		MeasureResult.DTX_used=false;
	
	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,6);
	MeasureResult.RXLEV_FullServicingCell=result-110;
	
	j++;	//skip spare bit
	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,1);
	if (result) 	MeasureResult.MeasValid=true;
	else		MeasureResult.MeasValid=false;
	
	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,6);
	MeasureResult.RXLEV_SubServicingCell=result-110;

	j++;	//skip spare bit
	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,3);
	MeasureResult.RXQUAL_FullServicingCell=result;
	
	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,3);
	MeasureResult.RXQUAL_SubServicingCell=result;
	
	dprintf ("RX Level FULL Servicing Cell = %i\n",MeasureResult.RXLEV_FullServicingCell);
	dprintf ("RX Level Sub Servicing Cell = %i\n",MeasureResult.RXLEV_FullServicingCell);

	dprintf ("RX Quality Full Servicing Cell = %i\n",MeasureResult.RXQUAL_FullServicingCell);
	dprintf ("RX Quality Sub Servicing Cell = %i\n",MeasureResult.RXQUAL_SubServicingCell);

	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,3);
	MeasureResult.NO_NCELL_M=result;

	rep=MeasureResult.NO_NCELL_M;
	
	for (i=0;i<MeasureResult.NO_NCELL_M;i++) {
	    result	= 0;
	    j		= BitUnPackI(buf+14,j,&result,6);
	    MeasureResult.NeighbourCell[i].RxLev = result-110;

	    result	= 0;
	    j		= BitUnPackI(buf+14,j,&result,5);
	    MeasureResult.NeighbourCell[i].ChFreq = frequency[result];

	    result	= 0;
	    j		= BitUnPackI(buf+14,j,&result,3);
	    MeasureResult.NeighbourCell[i].NB = 10 * result;
	    result	= 0;
	    j		= BitUnPackI(buf+14,j,&result,3);
	    MeasureResult.NeighbourCell[i].NB +=  result;
#ifdef DEBUG
	    if (MeasureResult.NeighbourCell[i].ChFreq)
		dprintf("CH = %i,\t",MeasureResult.NeighbourCell[i].ChFreq);
	    else
		dprintf("CH = Unknown\t",MeasureResult.NeighbourCell[i].ChFreq);
	    dprintf("RX Lev = %i dBm\t",MeasureResult.NeighbourCell[i].RxLev);
	    dprintf("BSIC CELL = %i\n",MeasureResult.NeighbourCell[i].NB);
#endif
	}
    }
#ifdef DEBUG
    if  (buf[4]==05) {					//Timing Advance
	if (buf[11]) dprintf ("Unknown Timing Advance\n");
	else dprintf ("Timing Advance = %i\n",buf[14] & 0x3f);
    }
#endif
    return GE_NONE;
}

GSM_Error ATSIEMENS_GetSAT(GSM_StateMachine *s)
{
    GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
    GSM_Error		error;
    unsigned char	*reqSAT[]= {"D009810301260082028182",
				    "D009810301260282028182",
				    "D009810301260582028182"},req[32];
    int 		i,len;
        
    if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
    
    for (i=0;i<3;i++){
	len				= strlen(reqSAT[i]);
	s->Protocol.Data.AT.EditMode 	= true;
        sprintf(req, "AT^SSTK=%i,1\r",len/2);
        error = GSM_WaitFor (s, req, strlen(req), 0x00, 3, ID_User1); 
	s->Phone.Data.DispatchError	= GE_TIMEOUT;
	s->Phone.Data.RequestID		= ID_User1;
    	error = s->Protocol.Functions->WriteMessage(s, reqSAT[i], len, 0x00);
    	if (error!=GE_NONE) return error;
    	error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
	if (error!=GE_NONE) return error;
        error = GSM_WaitForOnce (s, NULL,0x00, 0x00, 4);
    	if (error!=GE_NONE) return error;
    }
    return GE_NONE;
}

void ATSIEMENSNetmonitor(int argc, char *argv[])
{
    GSM_Init(true);
    
    if (CheckSiemens()==GE_NOTSUPPORTED) Print_Error(GE_NOTSUPPORTED);
    s.User.UserReplyFunctions=UserReplyFunctionsAtS;
    error=ATSIEMENS_GetSAT(&s);
    Print_Error(error);
    GSM_Terminate();
}

static GSM_Reply_Function UserReplyFunctionsAtS[] = {
	{ATSIEMENS_Reply_GetSAT,	"AT^SSTK",	0x00,0x00,ID_User1	},
	{NULL,				"\x00",		0x00,0x00,ID_None	}
};
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
