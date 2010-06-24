
/* (c) 2002-2006 by Marcin Wiacek */
/* ----------------- Splitting SMS into parts ------------------------------ */

unsigned char GSM_MakeSMSIDFromTime(void)
{
	GSM_DateTime 	Date;
	unsigned char	retval;

	GSM_GetCurrentDateTime (&Date);
	retval = Date.Second;
	switch (Date.Minute/10) {
		case 2: case 7: 	retval = retval +  60; break;
		case 4: case 8: 	retval = retval + 120; break;
		case 9: case 5: case 0: retval = retval + 180; break;
	}
	retval += Date.Minute/10;
	return retval;
}

void GSM_Find_Free_Used_SMS2(GSM_Coding_Type Coding,GSM_SMSMessage SMS, int *UsedText, int *FreeText, int *FreeBytes)
{
	int UsedBytes;

	switch (Coding) {
	case SMS_Coding_Default:
		FindDefaultAlphabetLen(SMS.Text,&UsedBytes,UsedText,500);
		UsedBytes = *UsedText * 7 / 8;
		if (UsedBytes * 8 / 7 != *UsedText) UsedBytes++;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length - UsedBytes;
		*FreeText = (GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length) * 8 / 7 - *UsedText;
		break;
	case SMS_Coding_Unicode:
		*UsedText = UnicodeLength(SMS.Text);
		UsedBytes = *UsedText * 2;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length - UsedBytes;
		*FreeText = *FreeBytes / 2;
		break;
	case SMS_Coding_8bit:
		*UsedText = UsedBytes = SMS.Length;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length - UsedBytes;
		*FreeText = *FreeBytes;
		break;
	}
	dbgprintf("UDH len %i, UsedBytes %i, FreeText %i, UsedText %i, FreeBytes %i\n",SMS.UDH.Length,UsedBytes,*FreeText,*UsedText,*FreeBytes);
}

GSM_Error GSM_AddSMS_Text_UDH(GSM_MultiSMSMessage 	*SMS,
		      		GSM_Coding_Type		Coding,
		      		char 			*Buffer,
		      		int			BufferLen,
		      		bool 			UDH,
		      		int 			*UsedText,
		      		int			*CopiedText,
		      		int			*CopiedSMSText)
{
	int FreeText,FreeBytes,Copy,i,j;

	dbgprintf("Checking used\n");
	GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);

	if (UDH) {
		dbgprintf("Adding UDH\n");
		if (FreeBytes - BufferLen <= 0) {
			dbgprintf("Going to the new SMS\n");
			SMS->Number++;
			GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);
		}
		if (SMS->SMS[SMS->Number].UDH.Length == 0) {
			SMS->SMS[SMS->Number].UDH.Length  = 1;
			SMS->SMS[SMS->Number].UDH.Text[0] = 0x00;
		}
		memcpy(SMS->SMS[SMS->Number].UDH.Text+SMS->SMS[SMS->Number].UDH.Length,Buffer,BufferLen);
		SMS->SMS[SMS->Number].UDH.Length  	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Text[0] 	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Type 		=  UDH_UserUDH;
		dbgprintf("UDH added %i\n",BufferLen);
	} else {
		dbgprintf("Adding text\n");
		if (FreeText == 0) {
			dbgprintf("Going to the new SMS\n");
			SMS->Number++;
			GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);
		}

		Copy = FreeText;
		dbgprintf("copy %i\n",Copy);
		if (BufferLen < Copy) Copy = BufferLen;
		dbgprintf("copy %i\n",Copy);

		switch (Coding) {
		case SMS_Coding_Default:
			FindDefaultAlphabetLen((unsigned char *)Buffer,&i,&j,FreeText);
			dbgprintf("def length %i %i\n",i,j);
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+i*2]   = 0;
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+i*2+1] = 0;
			memcpy(SMS->SMS[SMS->Number].Text+UnicodeLength(SMS->SMS[SMS->Number].Text)*2,Buffer,i*2);
			*CopiedText 	= i;
			*CopiedSMSText 	= j;
			SMS->SMS[SMS->Number].Length += i;
			break;
		case SMS_Coding_Unicode:
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+Copy*2]   = 0;
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+Copy*2+1] = 0;
			memcpy(SMS->SMS[SMS->Number].Text+UnicodeLength(SMS->SMS[SMS->Number].Text)*2,Buffer,Copy*2);
			*CopiedText = *CopiedSMSText = Copy;
			SMS->SMS[SMS->Number].Length += Copy;
			break;
		case SMS_Coding_8bit:
			memcpy(SMS->SMS[SMS->Number].Text+SMS->SMS[SMS->Number].Length,Buffer,Copy);
			SMS->SMS[SMS->Number].Length += Copy;
			*CopiedText = *CopiedSMSText = Copy;
			break;
		}
		dbgprintf("Text added\n");
	}

	dbgprintf("Checking on the end\n");
	GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);

	return ERR_NONE;
}

void GSM_MakeMultiPartSMS(GSM_MultiSMSMessage	*SMS,
			  unsigned char		*MessageBuffer,
			  int			MessageLength,
			  GSM_UDH		UDHType,
			  GSM_Coding_Type	Coding,
			  int			Class,
			  unsigned char		ReplaceMessage)
{
	int 		j,Len,UsedText,CopiedText,CopiedSMSText;
	unsigned char 	UDHID;
	GSM_DateTime 	Date;

	Len = 0;
	while(1) {
		GSM_SetDefaultSMSData(&SMS->SMS[SMS->Number]);
		SMS->SMS[SMS->Number].Class    = Class;
		SMS->SMS[SMS->Number].Coding   = Coding;

		SMS->SMS[SMS->Number].UDH.Type = UDHType;
		GSM_EncodeUDHHeader(&SMS->SMS[SMS->Number].UDH);

		if (Coding == SMS_Coding_8bit) {
			GSM_AddSMS_Text_UDH(SMS,Coding,(char *)MessageBuffer+Len,MessageLength - Len,false,&UsedText,&CopiedText,&CopiedSMSText);
		} else {
			GSM_AddSMS_Text_UDH(SMS,Coding,(char *)MessageBuffer+Len*2,MessageLength - Len,false,&UsedText,&CopiedText,&CopiedSMSText);
		}
		Len += CopiedText;
		dbgprintf("%i %i\n",Len,MessageLength);
		if (Len == MessageLength) break;
		if (SMS->Number == MAX_MULTI_SMS) break;
		SMS->Number++;
	}

	SMS->Number++;

	UDHID = GSM_MakeSMSIDFromTime();
	GSM_GetCurrentDateTime (&Date);	
	for (j=0;j<SMS->Number;j++) {
		SMS->SMS[j].UDH.Type 		= UDHType;
		SMS->SMS[j].UDH.ID8bit 		= UDHID;
		SMS->SMS[j].UDH.ID16bit		= UDHID + 256 * Date.Hour;
		SMS->SMS[j].UDH.PartNumber 	= j+1;
		SMS->SMS[j].UDH.AllParts 	= SMS->Number;
		GSM_EncodeUDHHeader(&SMS->SMS[j].UDH);
	}
	if (SMS->Number == 1) SMS->SMS[0].ReplaceMessage = ReplaceMessage;
}

/* Calculates number of SMS and number of left chars in SMS */
void GSM_SMSCounter(int 		MessageLength,
		    unsigned char 	*MessageBuffer,
		    GSM_UDH	 	UDHType,
		    GSM_Coding_Type 	Coding,
		    int 		*SMSNum,
		    int 		*CharsLeft)
{
	int			UsedText,FreeBytes;
	GSM_MultiSMSMessage 	MultiSMS;

	MultiSMS.Number = 0;
	GSM_MakeMultiPartSMS(&MultiSMS,MessageBuffer,MessageLength,UDHType,Coding,-1,false);
	GSM_Find_Free_Used_SMS2(Coding,MultiSMS.SMS[MultiSMS.Number-1], &UsedText, CharsLeft, &FreeBytes);
	*SMSNum = MultiSMS.Number;
}


GSM_Error GSM_LinkSMS(GSM_MultiSMSMessage **INPUT, GSM_MultiSMSMessage **OUTPUT, bool ems)
{
	bool			*INPUTSorted, copyit;
	int			i,OUTPUTNum,j,z,w;

	i = 0;
	while (INPUT[i] != NULL) i++;

	INPUTSorted = (bool*) calloc(i, sizeof(bool));
	if (INPUTSorted == NULL) return ERR_MOREMEMORY;

	OUTPUTNum = 0;
	OUTPUT[0] = NULL;

	if (ems) {
		i=0;
		while (INPUT[i] != NULL) {
			if (INPUT[i]->SMS[0].UDH.Type == UDH_UserUDH) {
				w=1;
				while (w < INPUT[i]->SMS[0].UDH.Length) {
					switch(INPUT[i]->SMS[0].UDH.Text[w]) {
					case 0x00:
						dbgprintf("Adding ID to user UDH - linked SMS with 8 bit ID\n");
						INPUT[i]->SMS[0].UDH.ID8bit	= INPUT[i]->SMS[0].UDH.Text[w+2];
						INPUT[i]->SMS[0].UDH.AllParts	= INPUT[i]->SMS[0].UDH.Text[w+3];
						INPUT[i]->SMS[0].UDH.PartNumber	= INPUT[i]->SMS[0].UDH.Text[w+4];
						break;
					case 0x08:
						dbgprintf("Adding ID to user UDH - linked SMS with 16 bit ID\n");
						INPUT[i]->SMS[0].UDH.ID16bit	= INPUT[i]->SMS[0].UDH.Text[w+2]*256+INPUT[i]->SMS[0].UDH.Text[w+3];
						INPUT[i]->SMS[0].UDH.AllParts	= INPUT[i]->SMS[0].UDH.Text[w+4];
						INPUT[i]->SMS[0].UDH.PartNumber	= INPUT[i]->SMS[0].UDH.Text[w+5];
						break;
					default:
						dbgprintf("Block %02x\n",INPUT[i]->SMS[0].UDH.Text[w]);
					}
					dbgprintf("%i %i %i %i\n",
						INPUT[i]->SMS[0].UDH.ID8bit,
						INPUT[i]->SMS[0].UDH.ID16bit,
						INPUT[i]->SMS[0].UDH.PartNumber,
						INPUT[i]->SMS[0].UDH.AllParts);
					w=w+INPUT[i]->SMS[0].UDH.Text[w+1]+2;
				}
			}
			i++;
		}
	}

	i=0;
	while (INPUT[i]!=NULL) {
		/* If this one SMS was sorted earlier, do not touch */
		if (INPUTSorted[i]) {
			i++;
			continue;
		}
		copyit = false;
		/* If we have:
		 * - linked sms returned by phone driver
		 * - sms without linking
		 * we copy it to OUTPUT
		 */
		if (INPUT[i]->Number 			!= 1 	       ||
		    INPUT[i]->SMS[0].UDH.Type 		== UDH_NoUDH   ||
                    INPUT[i]->SMS[0].UDH.PartNumber 	== -1) {
			copyit = true;
		}
		/* If we have unknown UDH, we copy it to OUTPUT */
		if (INPUT[i]->SMS[0].UDH.Type == UDH_UserUDH) {
			if (!ems) copyit = true;
			if (ems && INPUT[i]->SMS[0].UDH.PartNumber == -1) copyit = true;
		}
		if (copyit) {
			OUTPUT[OUTPUTNum] =(GSM_MultiSMSMessage *) malloc(sizeof(GSM_MultiSMSMessage));
			if (OUTPUT[OUTPUTNum] == NULL) {
				free(INPUTSorted);
				return ERR_MOREMEMORY;
			}
			OUTPUT[OUTPUTNum+1] = NULL;

			memcpy(OUTPUT[OUTPUTNum],INPUT[i],sizeof(GSM_MultiSMSMessage));
			INPUTSorted[i]=true;
			OUTPUTNum++;
			i = 0;
			continue;
		}
		/* We have 1'st part of linked sms. It's single.
		 * We will try to find other parts
		 */
		if (INPUT[i]->SMS[0].UDH.PartNumber == 1) {
			OUTPUT[OUTPUTNum] =(GSM_MultiSMSMessage *) malloc(sizeof(GSM_MultiSMSMessage));
			if (OUTPUT[OUTPUTNum] == NULL) {
				free(INPUTSorted);
				return ERR_MOREMEMORY;
			}
			OUTPUT[OUTPUTNum+1] = NULL;

			memcpy(&OUTPUT[OUTPUTNum]->SMS[0],&INPUT[i]->SMS[0],sizeof(GSM_SMSMessage));
			OUTPUT[OUTPUTNum]->Number = 1;
			INPUTSorted[i]	= true;
			j		= 1;
			/* We're searching for other parts in sequence */
			while (j!=INPUT[i]->SMS[0].UDH.AllParts) {
				z=0;
				while(INPUT[z]!=NULL) {
					/* This was sorted earlier or is not single */
					if (INPUTSorted[z] || INPUT[z]->Number != 1) {
						z++;
						continue;
					}
					if (ems && INPUT[i]->SMS[0].UDH.Type != UDH_ConcatenatedMessages &&
					    INPUT[i]->SMS[0].UDH.Type != UDH_ConcatenatedMessages16bit   &&
					    INPUT[i]->SMS[0].UDH.Type != UDH_UserUDH 			 &&
					    INPUT[z]->SMS[0].UDH.Type != UDH_ConcatenatedMessages 	 &&
					    INPUT[z]->SMS[0].UDH.Type != UDH_ConcatenatedMessages16bit   &&
					    INPUT[z]->SMS[0].UDH.Type != UDH_UserUDH) {
						if (INPUT[z]->SMS[0].UDH.Type != INPUT[i]->SMS[0].UDH.Type) {
							z++;
							continue;
						}
					}
					if (!ems && INPUT[z]->SMS[0].UDH.Type != INPUT[i]->SMS[0].UDH.Type) {
						z++;
						continue;
					}
					dbgprintf("compare %i         %i %i %i %i",
						j+1,
						INPUT[i]->SMS[0].UDH.ID8bit,
						INPUT[i]->SMS[0].UDH.ID16bit,
						INPUT[i]->SMS[0].UDH.PartNumber,
						INPUT[i]->SMS[0].UDH.AllParts);
					dbgprintf("         %i %i %i %i\n",
						INPUT[z]->SMS[0].UDH.ID8bit,
						INPUT[z]->SMS[0].UDH.ID16bit,
						INPUT[z]->SMS[0].UDH.PartNumber,
						INPUT[z]->SMS[0].UDH.AllParts);
					if (INPUT[z]->SMS[0].UDH.ID8bit      != INPUT[i]->SMS[0].UDH.ID8bit	||
					    INPUT[z]->SMS[0].UDH.ID16bit     != INPUT[i]->SMS[0].UDH.ID16bit	||
					    INPUT[z]->SMS[0].UDH.AllParts    != INPUT[i]->SMS[0].UDH.AllParts 	||
					    INPUT[z]->SMS[0].UDH.PartNumber  != j+1) {
						z++;
						continue;
					}
					/* For SMS_Deliver compare also SMSC and Sender number */
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver &&
					    (strcmp((char *)DecodeUnicodeString(INPUT[z]->SMS[0].SMSC.Number),(char *)DecodeUnicodeString(INPUT[i]->SMS[0].SMSC.Number)) ||
					     strcmp((char *)DecodeUnicodeString(INPUT[z]->SMS[0].Number),(char *)DecodeUnicodeString(INPUT[i]->SMS[0].Number)))) {
						z++;
						continue;
					}
					/* DCT4 Outbox: SMS Deliver. Empty number and SMSC. We compare dates */
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver 		&&
					    UnicodeLength(INPUT[z]->SMS[0].SMSC.Number)==0 	&&
					    UnicodeLength(INPUT[z]->SMS[0].Number)==0 		&&
					    (INPUT[z]->SMS[0].DateTime.Day    != INPUT[i]->SMS[0].DateTime.Day 	  ||
	   				     INPUT[z]->SMS[0].DateTime.Month  != INPUT[i]->SMS[0].DateTime.Month  ||
					     INPUT[z]->SMS[0].DateTime.Year   != INPUT[i]->SMS[0].DateTime.Year   ||
					     INPUT[z]->SMS[0].DateTime.Hour   != INPUT[i]->SMS[0].DateTime.Hour   ||
					     INPUT[z]->SMS[0].DateTime.Minute != INPUT[i]->SMS[0].DateTime.Minute ||
					     INPUT[z]->SMS[0].DateTime.Second != INPUT[i]->SMS[0].DateTime.Second)) {
						z++;
						continue;
					}
					/* We found correct sms. Copy it */
					memcpy(&OUTPUT[OUTPUTNum]->SMS[j],&INPUT[z]->SMS[0],sizeof(GSM_SMSMessage));
					OUTPUT[OUTPUTNum]->Number++;
					INPUTSorted[z]=true;
					break;
				}
				/* Incomplete sequence */
				if (OUTPUT[OUTPUTNum]->Number==j) {
					dbgprintf("Incomplete sequence\n");
					break;
				}
				j++;
			}
			OUTPUTNum++;
			i = 0;
			continue;
		}
		/* We have some next linked sms from sequence */
		if (INPUT[i]->SMS[0].UDH.PartNumber > 1) {
			j = 0;
			while (INPUT[j]!=NULL) {
				if (INPUTSorted[j]) {
					j++;
					continue;
				}
				/* We have some not unassigned first sms from sequence.
				 * We can't touch other sms from sequences
				 */
				if (INPUT[j]->SMS[0].UDH.PartNumber == 1) break;
				j++;
			}
			if (INPUT[j]==NULL) {
				OUTPUT[OUTPUTNum] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
				if (OUTPUT[OUTPUTNum] == NULL) {
					free(INPUTSorted);
					return ERR_MOREMEMORY;
				}
				OUTPUT[OUTPUTNum+1] = NULL;

				memcpy(OUTPUT[OUTPUTNum],INPUT[i],sizeof(GSM_MultiSMSMessage));
				INPUTSorted[i]=true;
				OUTPUTNum++;
				i = 0;
				continue;
			} else i++;
		}
	}
	free(INPUTSorted);
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
