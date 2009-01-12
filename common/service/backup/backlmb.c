/* (c) 2001-2004 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include <gammu-config.h>

#include "../../phone/nokia/nfunc.h"
#include "../../misc/coding/coding.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "../../debug.h"
#include "backlmb.h"
#include "../../gsmphones.h"

#ifdef GSM_ENABLE_BACKUP

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;

static GSM_Error SaveLMBStartupEntry(FILE *file, GSM_Bitmap bitmap)
{
	size_t count = 13;
	GSM_Phone_Bitmap_Types 	Type;
	/* Welcome note and logo header block */
	char req[1000] = {
		'W','E','L',' ',    /*block identifier*/
		00,00,              /*block data size*/
		0x02,00,00,00,00,00,
		0x00};              /*number of blocks (like in 6110 frame)*/

	if (bitmap.Type == GSM_StartupLogo) {
		req[count++] = 0x01;
		req[count++] = bitmap.BitmapHeight;
		req[count++] = bitmap.BitmapWidth;
		Type = GSM_NokiaStartupLogo;
	        switch (bitmap.BitmapHeight) {
			case 65: Type = GSM_Nokia7110StartupLogo; break;
			case 60: Type = GSM_Nokia6210StartupLogo; break;
		}
		PHONE_EncodeBitmap(Type, req+count, &bitmap);
		count = count + PHONE_GetBitmapSize(Type, 0, 0);

		req[12]++;
	}
	if (bitmap.Type == GSM_WelcomeNote_Text) {
		req[count++]=0x02;
		req[count++]=UnicodeLength(bitmap.Text);
		memcpy(req+count,DecodeUnicodeString(bitmap.Text),UnicodeLength(bitmap.Text));
		count=count+UnicodeLength(bitmap.Text);

		req[12]++;
	}

	req[4]=(count-12)%256;
	req[5]=(count-12)/256;

	chk_fwrite(req, 1, count, file);
	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

static GSM_Error SaveLMBCallerEntry(FILE *file, GSM_Bitmap bitmap)
{
	size_t count = 12, textlen;
	char req[500] = {
		'C','G','R',' ',    /*block identifier*/
		00,00,              /*block data size*/
		02,00,
		00,                 /*group number=0,1,etc.*/
		00,00,00};

	req[count++] = bitmap.Location - 1;
	if (bitmap.DefaultName) {
		req[count++] = 0;
	} else {
		textlen = UnicodeLength(bitmap.Text);
		req[count++] = textlen;
		memcpy(req+count,DecodeUnicodeString(bitmap.Text),textlen);
		count += textlen;
	}
	if (bitmap.DefaultRingtone) {
		req[count++] = 0x16;
	} else {
		req[count++] = bitmap.RingtoneID;
	}
	if (bitmap.BitmapEnabled) req[count++] = 0x01; else req[count++] = 0x00;
	req[count++] = (PHONE_GetBitmapSize(GSM_NokiaCallerLogo,0,0) + 4) >> 8;
	req[count++] = (PHONE_GetBitmapSize(GSM_NokiaCallerLogo,0,0) + 4) % 0xff;
	if (bitmap.DefaultBitmap) {
		bitmap.BitmapWidth  = 72;
		bitmap.BitmapHeight = 14;
		GSM_ClearBitmap(&bitmap);
	}
	NOKIA_CopyBitmap(GSM_NokiaCallerLogo, &bitmap, req, &count);
	req[count++]=0;

	req[4]=(count-12)%256;
	req[5]=(count-12)/256;
	req[8]=bitmap.Location;

	chk_fwrite(req, 1, count, file);
	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

GSM_Error SaveLMBPBKEntry(FILE *file, GSM_MemoryEntry *entry)
{
	GSM_StateMachine	*fake_sm;
	size_t			count = 16, blocks;
	char 			req[500] = {
		'P','B','E','2', /*block identifier*/
		00,00,           /*block data size*/
		00,00,
		00,00,           /*position of phonebook entry*/
		03,              /*memory type. ME=02;SM=03*/
		00,
		00,00,           /*position of phonebook entry*/
		03,              /*memory type. ME=02;SM=03*/
		00};

	fake_sm = GSM_AllocStateMachine();
	fake_sm->di.use_global = true;
	fake_sm->Phone.Data.ModelInfo = GetModelData(NULL, "unknown", NULL, NULL);

	count=count+N71_65_EncodePhonebookFrame(fake_sm, req+16, entry, &blocks, true, true);

	req[4]=(count-12)%256;
	req[5]=(count-12)/256;
	req[8]=req[12] = entry->Location & 0xff;
	req[9]=req[13] = (entry->Location >> 8);
	if (entry->MemoryType==MEM_ME) req[10]=req[14]=2;

	chk_fwrite(req, 1, count, file);
	GSM_FreeStateMachine(fake_sm);
	return ERR_NONE;
fail:
	GSM_FreeStateMachine(fake_sm);
	return ERR_WRITING_FILE;
}

GSM_Error SaveLMB(char *FileName, GSM_Backup *backup)
{
	FILE 	*file;
	int 	i;
	char 	LMBHeader[] = {'L','M','B',' '}; /*file identifier*/
	char 	PBKHeader[] = {		      	 /*Phonebook header block */
			'P','B','K',' ', 	 /*block identifier*/
			0x08,00,         	 /*block data size*/
			0x02,00,
			03,              	 /*memory type. ME=02;SM=03*/
			00,00,00,
			00,00,           	 /*size of phonebook*/
			14,              	 /*max length of each position*/
			00,00,00,00,00};
	GSM_Error error;


	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	/* Write the header of the file. */
	chk_fwrite(LMBHeader, 1, sizeof(LMBHeader), file);

	if (backup->PhonePhonebook[0]!=NULL) {
		PBKHeader[8]	= 2;		/* memory type=MEM_ME */
		PBKHeader[12] 	= (unsigned char)(500 % 256);
		PBKHeader[13] 	= 500 / 256;
		chk_fwrite(PBKHeader, 1, sizeof(PBKHeader), file);
		i=0;
		while (backup->PhonePhonebook[i]!=NULL) {
			error = SaveLMBPBKEntry(file, backup->PhonePhonebook[i]);
			if (error != ERR_NONE) return error;
			i++;
		}
	}
	if (backup->SIMPhonebook[0]!=NULL) {
		PBKHeader[8]	= 3;		/* memory type=MEM_SM */
		PBKHeader[12] 	= (unsigned char)(250 % 256);
		PBKHeader[13] 	= 250 / 256;
		PBKHeader[14]	= 0x16;		/* max size of one entry */
		chk_fwrite(PBKHeader, 1, sizeof(PBKHeader), file);
		i=0;
		while (backup->SIMPhonebook[i]!=NULL) {
			error = SaveLMBPBKEntry(file, backup->SIMPhonebook[i]);
			if (error != ERR_NONE) return error;
			i++;
		}
	}
	i=0;
	while (backup->CallerLogos[i]!=NULL) {
		error = SaveLMBCallerEntry(file, *backup->CallerLogos[i]);
		if (error != ERR_NONE) return error;
		i++;
	}
	if (backup->StartupLogo!=NULL) {
		error = SaveLMBStartupEntry(file, *backup->StartupLogo);
		if (error != ERR_NONE) return error;
	}

	fclose(file);
	return ERR_NONE;
fail:
	fclose(file);
	return ERR_WRITING_FILE;
}

static GSM_Error LoadLMBCallerEntry(unsigned char *buffer UNUSED, unsigned char *buffer2, GSM_Backup *backup)
{
	GSM_Bitmap 	bitmap;
	int 		num;

#ifdef DEBUG
	dbgprintf(NULL, "Number %i, name \"", buffer2[0]+1);
	for (num=0;num<buffer2[1];num++) dbgprintf(NULL, "%c", buffer2[num+2]);
	dbgprintf(NULL, "\"\n");
	dbgprintf(NULL, "Ringtone ID=%i\n", buffer2[num+2]);
	if (buffer2[num+3]==1) {
		dbgprintf(NULL, "Logo enabled\n");
	} else {
		dbgprintf(NULL, "Logo disabled\n");
	}
#endif

  	bitmap.Location		= buffer2[0] + 1;
	bitmap.Type		= GSM_CallerGroupLogo;
	bitmap.DefaultRingtone 	= false;
	bitmap.RingtoneID		= buffer2[buffer2[1]+2];

	EncodeUnicode(bitmap.Text,buffer2+2,buffer2[1]);
	if (bitmap.Text[0] == 0x00 && bitmap.Text[1] == 0x00) {
		bitmap.DefaultName = true;
	} else {
		bitmap.DefaultName = false;
	}

	bitmap.BitmapEnabled = false;
	if (buffer2[buffer2[1]+3]==1) bitmap.BitmapEnabled=true;

	bitmap.DefaultBitmap = false;
	PHONE_DecodeBitmap(GSM_NokiaCallerLogo, buffer2+(buffer2[1]+10), &bitmap);

#ifdef DEBUG
	dbgprintf(NULL, "Caller logo\n");
	if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE)
		GSM_PrintBitmap(GSM_global_debug.df,&bitmap);
#endif

	num = 0;
	while (backup->CallerLogos[num] != NULL) num++;
	if (num < GSM_BACKUP_MAX_CALLER) {
		backup->CallerLogos[num] = malloc(sizeof(GSM_Bitmap));
	        if (backup->CallerLogos[num] == NULL) return ERR_MOREMEMORY;
		backup->CallerLogos[num + 1] = NULL;
	} else {
		dbgprintf(NULL, "Increase GSM_BACKUP_MAX_CALLER\n");
		return ERR_MOREMEMORY;
	}
	*backup->CallerLogos[num] = bitmap;

	return ERR_NONE;
}

static GSM_Error LoadLMBStartupEntry(unsigned char *buffer UNUSED, unsigned char *buffer2, GSM_Backup *backup)
{
	int 			i,j;
#ifdef DEBUG
	int 			z;
#endif
	GSM_Phone_Bitmap_Types 	Type;

	j=1;
	for (i=0;i<buffer2[0];i++) {
		switch (buffer2[j++]) {
			case 1:
				dbgprintf(NULL, "Block 1 - startup logo\n");
				backup->StartupLogo = malloc(sizeof(GSM_Bitmap));
			        if (backup->StartupLogo == NULL) return ERR_MOREMEMORY;
				backup->StartupLogo->Location	= 1;
				backup->StartupLogo->BitmapHeight	= buffer2[j++];
				backup->StartupLogo->BitmapWidth	= buffer2[j++];
				Type = GSM_NokiaStartupLogo;
			        switch (backup->StartupLogo->BitmapHeight) {
					case 65: Type = GSM_Nokia7110StartupLogo; break;
					case 60: Type = GSM_Nokia6210StartupLogo; break;
				}
				PHONE_DecodeBitmap(Type, buffer2+j, backup->StartupLogo);
#ifdef DEBUG
				if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE)
					GSM_PrintBitmap(GSM_global_debug.df,backup->StartupLogo);
#endif
				j = j + PHONE_GetBitmapSize(Type,0,0);
				break;
			case 2:
#ifdef DEBUG
				dbgprintf(NULL, "Block 2 - welcome note \"");
				for (z=0;z<buffer2[j];z++) dbgprintf(NULL, "%c",buffer2[j+z+1]);
				dbgprintf(NULL, "\"\n");
#endif
				if (backup->StartupLogo == NULL) {
					backup->StartupLogo = malloc(sizeof(GSM_Bitmap));
				        if (backup->StartupLogo == NULL) return ERR_MOREMEMORY;
					backup->StartupLogo->Type = GSM_WelcomeNote_Text;
					EncodeUnicode(backup->StartupLogo->Text,buffer2+j,buffer2[j]);
				}
				j = j + buffer2[j];
			        break;
			default:
			        dbgprintf(NULL, "Unknown block %02x\n",buffer2[j]);
				break;
		}
	}
	return ERR_NONE;
}

static GSM_Error LoadLMBPbkEntry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{
	GSM_MemoryEntry 	pbk;
	int			num;
	GSM_StateMachine	*fake_sm;

	fake_sm = GSM_AllocStateMachine();
	fake_sm->di.use_global = true;
	fake_sm->Phone.Data.ModelInfo = GetModelData(NULL, "unknown", NULL, NULL);

#ifdef DEBUG
	dbgprintf(NULL, "Memory : ");
	switch(buffer[10]) {
		case 2 : dbgprintf(NULL, "(internal)\n"); break;
		case 3 : dbgprintf(NULL, "(sim)\n");	  break;
		default: dbgprintf(NULL, "(unknown)\n");  break;
	}
	dbgprintf(NULL, "Location : %i\n",buffer2[0]+buffer2[1]*256);
#endif

	pbk.MemoryType = 0; /* FIXME: I have no idea what should be set here, but needs to be set, see next function */
	N71_65_DecodePhonebook(fake_sm, &pbk, NULL,NULL,buffer2+4,(buffer[4]+buffer[5]*256)-4,false);

	pbk.MemoryType=MEM_SM;
	if (buffer[10]==2) pbk.MemoryType=MEM_ME;

	pbk.Location=buffer2[0]+256*buffer2[1];

	num = 0;
	if (buffer[10]==2) {
		while (backup->PhonePhonebook[num] != NULL) num++;
		if (num < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
			backup->PhonePhonebook[num] = malloc(sizeof(GSM_MemoryEntry));
		        if (backup->PhonePhonebook[num] == NULL) {
				GSM_FreeStateMachine(fake_sm);
				return ERR_MOREMEMORY;
			}
			backup->PhonePhonebook[num + 1] = NULL;
		} else {
			GSM_FreeStateMachine(fake_sm);
			dbgprintf(NULL, "Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
			return ERR_MOREMEMORY;
		}
		*backup->PhonePhonebook[num] = pbk;
	} else {
		while (backup->SIMPhonebook[num] != NULL) num++;
		if (num < GSM_BACKUP_MAX_SIMPHONEBOOK) {
			backup->SIMPhonebook[num] = malloc(sizeof(GSM_MemoryEntry));
		        if (backup->SIMPhonebook[num] == NULL) {
				GSM_FreeStateMachine(fake_sm);
				return ERR_MOREMEMORY;
			}
			backup->SIMPhonebook[num + 1] = NULL;
		} else {
			dbgprintf(NULL, "Increase GSM_BACKUP_MAX_SIMPHONEBOOK\n");
			GSM_FreeStateMachine(fake_sm);
			return ERR_MOREMEMORY;
		}
		*backup->SIMPhonebook[num] = pbk;
	}
	GSM_FreeStateMachine(fake_sm);
	return ERR_NONE;
}

GSM_Error LoadLMB(char *FileName, GSM_Backup *backup)
{
#ifdef DEBUG
	int 		i;
#endif
	unsigned char 	buffer[12], buffer2[1000];
	FILE		*file;
	GSM_Error	error;
	size_t blocksize;

	file = fopen(FileName, "rb");
	if (file == NULL) return(ERR_CANTOPENFILE);

	/* Read the header of the file. */
	if (fread(buffer, 1, 4, file) != 4) return ERR_FILENOTSUPPORTED;

	/* while we have something to read */
	while (fread(buffer, 1, 12, file) == 12) {
#ifdef DEBUG
		/* Info about block in the file */
		dbgprintf(NULL, "Block \"");
		for (i=0;i<4;i++) {dbgprintf(NULL, "%c",buffer[i]);}
		dbgprintf(NULL, "\" (");
		if (memcmp(buffer, "PBK ",4)==0) {	  dbgprintf(NULL, "Phonebook");
		} else if (memcmp(buffer, "PBE2",4)==0) { dbgprintf(NULL, "Phonebook entry");
		} else if (memcmp(buffer, "CGR ",4)==0) { dbgprintf(NULL, "Caller group");
		} else if (memcmp(buffer, "SPD ",4)==0) { dbgprintf(NULL, "Speed dial");
		} else if (memcmp(buffer, "OLG ",4)==0) { dbgprintf(NULL, "Operator logo");
		} else if (memcmp(buffer, "WEL ",4)==0) { dbgprintf(NULL, "Startup logo and welcome text");
		} else {				  dbgprintf(NULL, "unknown - ignored");
		}
		dbgprintf(NULL, ") - length %i\n", buffer[4]+buffer[5]*256);
#endif
      		/* reading block data */
		blocksize = buffer[4] + buffer[5] * 256;
		if (fread(buffer2, 1, blocksize, file) != blocksize) return ERR_FILENOTSUPPORTED;

#ifdef DEBUG
		if (memcmp(buffer, "PBK ",4)==0) {
			dbgprintf(NULL, "Size of phonebook %i, type %i ",(buffer2[0]+buffer2[1]*256),buffer[8]);
			switch(buffer[8]) {
				case 2 : dbgprintf(NULL, "(internal)");break;
				case 3 : dbgprintf(NULL, "(sim)")     ;break;
				default: dbgprintf(NULL, "(unknown)") ;break;
			}
			dbgprintf(NULL, ", length of each position - %i\n",buffer2[2]);
		}
#endif
		if (memcmp(buffer, "PBE2",4)==0) {
			error = LoadLMBPbkEntry(buffer,buffer2,backup);
			if (error != ERR_NONE) {
				fclose(file);
				return error;
			}
		}
		if (memcmp(buffer, "CGR ",4)==0) {
			error = LoadLMBCallerEntry(buffer, buffer2, backup);
			if (error != ERR_NONE) {
				fclose(file);
				return error;
			}
		}
		if (memcmp(buffer, "WEL ",4)==0) {
			error = LoadLMBStartupEntry(buffer, buffer2, backup);
			if (error != ERR_NONE) {
				fclose(file);
				return error;
			}
		}
        }

	fclose(file);

	return ERR_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
