
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

unsigned int maxpow2 (int value)
{
	int pow2=0x80;

	while ((pow2=pow2>>1) > value);
	return pow2;
}

unsigned int pow2iter (int value)
{
	int pow2=0x80, result=7;

	while ((pow2=pow2>>1) > value) result--;
	return result-1;
}

int cellfreq (int index, unsigned int w[], unsigned int Orig, int range)
{
/* base: (range):  	0 - 1024
 			1 - 512
 			2 - 256
 			3 - 128 */
	int j,n,reminder;

	j 	 = maxpow2(index);
	n 	 = w[index];
	reminder = 1024/range;

	while (index > 1) {
		if (2*index < 3*j) {
			index-=j/2;
			n=(n+w[index]-reminder/j-1) % (2*reminder/j-1)+1;
		} else {
			index-=j;
			n=(n+w[index]-1) % (2*reminder/j-1)+1;
		}
		j=j/2;
	}
	return Orig+n;
}

GSM_Error CheckSiemens()
{
	if (s.Phone.Data.Priv.ATGEN.Manufacturer != AT_Siemens) return GE_NOTSUPPORTED;
	return GE_NONE;
}

GSM_Error ATSIEMENS_Reply_GetSAT(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData		*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SAT_Measure_results		MeasureResult;
	unsigned char 			buf[256];
	int				length,i,j,result,rep,origARFCN,
    					bit_len=10,format_ident=10,ChNo=1;
    	unsigned int			frequency[24],freq_tmp[24];
   	GSM_NetworkInfo			Network;

    	if (Priv->ReplyState!=AT_Reply_OK) return GE_UNKNOWN;
    	if (s->Protocol.Data.AT.EditMode) s->Protocol.Data.AT.EditMode = false;
    	if (!strstr(GetLineString(msg.Buffer,Priv->Lines,3),"SSTK")) return GE_UNKNOWN;
        
    	length = strlen(GetLineString(msg.Buffer,Priv->Lines,3))-7;
    	DecodeHexBin(buf, GetLineString(msg.Buffer,Priv->Lines,3)+7,length);

    	if (buf[3]!=0x26) return GE_UNKNOWN;
    
#ifdef DEBUG
    	dbgprintf ("SAT command: Provide Local Information\nFunction: ");
    	switch (buf[4]) {
        	case 00: dbgprintf ("Loc Info\n");			break;
		case 01: dbgprintf ("IMEI\n");				break;
		case 02: dbgprintf ("Network Measure\n");		break;
		case 03: dbgprintf ("Date time and timezone\n");	break;
		case 04: dbgprintf ("Language setting\n");		break;
		case 05: dbgprintf ("Timing advance\n");		break;
    	}
#endif

    	/* Loc Info (MCC, MNC, LAC, Cell ID) */
    	if (buf[4]==00) {
        	DecodeBCD (Network.NetworkCode,buf+14,2);
        	Network.NetworkCode[3] = ' ';
        	DecodeBCD (Network.NetworkCode+4,buf+16,1);
        	EncodeHexBin (Network.LAC,buf+17,2);
        	EncodeHexBin (Network.CellID,buf+19,2);

		printf("   Network code              : %s\n",Network.NetworkCode);
		printf("   Network name for Gammu    : %s\n",
			DecodeUnicodeString(GSM_GetNetworkName(Network.NetworkCode)));
		printf("   CellID: %s\n",Network.CellID);
		printf("   LAC   : %s\n",Network.LAC);
    	}

    	/* Network Measure */
    	if (buf[4]==02) {
		j	= 0;
		rep	= buf[31]*8;
		result	= 0;
		j	= BitUnPackI(buf+32,j,&result,7);
		result &= 0x67;
#ifdef DEBUG
		switch (result) {
	    	case 0x44: 	format_ident=2; // 010xx100B - 512 range
				bit_len=9;
				dbgprintf ("512 range\n");
				break;
	    	case 0x45: 	format_ident=4; // 010xx101B - 256 range
				bit_len=8;
				dbgprintf ("256 range\n");	    
				break;
	    	case 0x46: 	format_ident=8; // 010xx110B - 128 range
				bit_len=7;
				dbgprintf ("128 range\n");	    
				break;
	    	case 0x47: 	format_ident=10; // 010xx111B - variable bit map
				dbgprintf ("Bitmap\n");	    
				break;
	    	default:	if (!(result & 4)) {
			    		format_ident=1; //0100xxxB - 1024 range
			    		bit_len=10;
			    		dbgprintf ("1024 range \n");
				} else format_ident=0;
		}
#endif    
		if (!format_ident) return GE_NOTSUPPORTED;
	
		origARFCN = 0;
		if (format_ident==1) j--;
		j = BitUnPackI(buf+32,j,&origARFCN,10);

		/* 10 bit origin ARFCN or first frequency (1024 range) */
#ifdef DEBUG
		dbgprintf("Origin BCCH = %i\n",origARFCN);
#endif
		if (format_ident!=10){
	    		i=0;
	    		if (format_ident==1){
				freq_tmp[ChNo]=origARFCN;
				frequency[ChNo]=cellfreq(ChNo, freq_tmp, 0, format_ident) % 1024;
				printf ("freq(%i) = %i\n",ChNo,frequency[ChNo]);
				ChNo++;
	    		}
    	    		while (i < rep){
				result = 0; 
				j = BitUnPackI(buf+32,j,&result,bit_len-pow2iter(ChNo));
				if (!result) break;
				i+=j;
				freq_tmp[ChNo]=result;
				if (format_ident==1) 
		   			frequency[ChNo]=cellfreq(ChNo, freq_tmp, 0, format_ident) % 1024;
				else   
		    			frequency[ChNo]=cellfreq(ChNo, freq_tmp, origARFCN, format_ident) % 1024;
				ChNo++;
	    		}
		}    

		if (format_ident==10){
	    		for (i=0;i<rep;i++){
				result = 0; 
				j      = BitUnPackI(buf+32,j,&result,1);
				if (result) {
		    		frequency[ChNo]=i+origARFCN+1;
		    		ChNo++;
			}
	    	}    
	}
#ifdef DEBUG
    	dbgprintf("Neighbor BCCH list: ");
	for (i=1;i<ChNo;i++) dbgprintf ("%d ",frequency[i]);
	dbgprintf ("\n");
#endif
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
	
	printf ("RX Level FULL Servicing Cell = %i\n",MeasureResult.RXLEV_FullServicingCell);
	printf ("RX Level Sub Servicing Cell = %i\n",MeasureResult.RXLEV_FullServicingCell);

	printf ("RX Quality Full Servicing Cell = %i\n",MeasureResult.RXQUAL_FullServicingCell);
	printf ("RX Quality Sub Servicing Cell = %i\n",MeasureResult.RXQUAL_SubServicingCell);

	result	= 0;
	j	= BitUnPackI(buf+14,j,&result,3);
	MeasureResult.NO_NCELL_M=result;

	rep=MeasureResult.NO_NCELL_M;
	
	for (i=0;i<MeasureResult.NO_NCELL_M;i++) {
		result	= 0;
	    	j	= BitUnPackI(buf+14,j,&result,6);
	    	MeasureResult.NeighbourCell[i].RxLev = result-110;

	    	result	= 0;
	    	j	= BitUnPackI(buf+14,j,&result,5);
	    	MeasureResult.NeighbourCell[i].ChFreq = frequency[result];

	    	result	= 0;
	    	j	= BitUnPackI(buf+14,j,&result,3);
	    	MeasureResult.NeighbourCell[i].NB = 10 * result;
	    	result	= 0;
	    	j	= BitUnPackI(buf+14,j,&result,3);
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
		if (buf[11]) dbgprintf ("Unknown Timing Advance\n");
		else dbgprintf ("Timing Advance = %i\n",buf[14] & 0x3f);
    	}
#endif
    	return GE_NONE;
}

GSM_Error ATSIEMENS_Reply_GetNetmon(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
    	int 			i=2;
    
    	if (!strstr(GetLineString(msg.Buffer,Priv->Lines,1),"AT^S^MI")) return GE_UNKNOWN;
    	while (strlen(GetLineString(msg.Buffer,Priv->Lines,i+1)))   
		printf("%s\n",GetLineString(msg.Buffer,Priv->Lines,i++));
    	printf("\n");
    	return GE_NONE;
}

GSM_Error ATSIEMENS_GetSAT(GSM_StateMachine *s)
{
    	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
    	GSM_Error		error;
    	unsigned char		*reqSAT[]= {"D009810301260082028182",
				    	"D009810301260282028182",
				    	"D009810301260582028182"},req[32];
    	int 			i,len;
        
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

GSM_Error ATSIEMENS_GetNetmon(GSM_StateMachine *s,int test_no)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char		req[32];
        
	if (Priv->Manufacturer!=AT_Siemens) return GE_NOTSUPPORTED;
	sprintf(req, "AT^S^MI=%d\r",test_no);
	printf ("Siemens NetMonitor test #%i\n",test_no); 
	return GSM_WaitFor(s, req, strlen(req), 0x00, 3, ID_User2);
}

void ATSIEMENSSATNetmon(int argc, char *argv[])
{
	GSM_Init(true);
	if (CheckSiemens()==GE_NOTSUPPORTED) Print_Error(GE_NOTSUPPORTED);
	s.User.UserReplyFunctions=UserReplyFunctionsAtS;

	printf ("Getting Siemens Sim Aplication Toolkit NetMonitor...\n");

	error=ATSIEMENS_GetSAT(&s);    
	Print_Error(error);    
	GSM_Terminate();
}

void ATSIEMENSNetmonitor(int argc, char *argv[])
{
	int test_no;

	GSM_Init(true);
	if (CheckSiemens()==GE_NOTSUPPORTED) Print_Error(GE_NOTSUPPORTED);
	s.User.UserReplyFunctions=UserReplyFunctionsAtS;

	printf ("Getting Siemens NetMonitor...\n");

	test_no = atoi(argv[2]);
	error   = ATSIEMENS_GetNetmon (&s,test_no+1);
	Print_Error(error);
    
	GSM_Terminate();
}

static GSM_Reply_Function UserReplyFunctionsAtS[] = {
	{ATSIEMENS_Reply_GetSAT,	"AT^SSTK",	0x00,0x00,ID_User1	},
	{ATSIEMENS_Reply_GetNetmon,	"AT^S^MI",	0x00,0x00,ID_User2	},
	{NULL,				"\x00",		0x00,0x00,ID_None	}
};
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
