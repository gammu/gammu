/**
 * SIM packet disassembly functions
 * The Monty <monty@technojunkie.gr>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <gammu-misc.h>

#include "wmx-util.h"
#include "wmx-sim.h"

/* Disassemble SIM Command packet (0x25)
 * -- GSM 11.11 v6.2.0 (1999)
 */
/* vars:
 * ins    = sim cmd instruction
 * type   = 0x25 debug subtype
 * buffer = sim cmd's parameter 1 (P1) + P2 + P3 + additional data if exist
 *          (ie select file, additional data is file id)
 */
void simCommand_data(unsigned char ins, unsigned char type, unsigned char *buffer, size_t length)
{
	size_t 	x;
	int 	v=1; 	// v = verbose (to use or not to use ?)

	printf("[a0 %02x ", ins);
	for(x=0;x<length;x++) printf("%02x ", buffer[x]);
	printf("]\nSim ");

	/* This switches type. The below types are known/valid. Since I don't
	 * know all the types, i've created a switch based on instruction.
	 */
/*	switch(type) {
	case 0x06:
		printf("Request Status");
		break;
	case 0x07:
		printf("Select Command");
		break;
	case 0x10:
		printf("GetResponse Command\n");
		break;
	default:
		printf("Unknown Command\n");
		break;
	}
*/

	/* A switch case based on sim command's instruction */
	switch(ins) {
	case 0xa4:
		printf("Select Command");
		break;
	case 0xf2:
		printf("Request Status");
		break;
	case 0xb0:
		printf("Read Binary");
		break;
	case 0xd6:
		printf("Update Binary");
		break;
	case 0xb2:
		printf("Read Record");
		break;
	case 0xdc:
		printf("Update Record");
		break;
	case 0xa2:
		printf("Seek cmd");
		break;
	case 0x32:
		printf("Increase cmd");
		break;
	case 0x20:
		printf("Verify CHV");
		break;
	case 0x24:
		printf("Change CHV");
		break;
	case 0x26:
		printf("Disable CHV");
		break;
	case 0x28:
		printf("Enable CHV");
		break;
	case 0x2c:
		printf("Unblock CHV");
		break;
	case 0x04:
		printf("Invalidate");
		break;
	case 0x44:
		printf("Rehabilitate");
		break;
	case 0x88:
		printf("Run GSM Algorithm");
		break;
	case 0xfa:
		printf("Sleep");
		break;
	case 0xc0:
		printf("GetResponse Command");
		break;
	case 0x10:
		printf("Terminal Profile");
		break;
	case 0xc2:
		printf("Envelope");
		break;
	case 0x12:
		printf("Fetch");
		break;
	case 0x14:
		printf("Terminal Response");
		break;
	default:
		printf("Unknown SIM command ");
		printf(" : INS=%02x P1=%02x P2=%02x P3=%02x ", buffer[0], buffer[1], buffer[2], buffer[3]);
		printf("\nCommand=");
		for(x=0; x<length; x++) printf("%02x ",buffer[x]&0xFF);
		printf("\n");
		break;
	}

	if(v) {
		printf(" : INS=%02x P1=%02x P2=%02x P3=%02x ", ins, buffer[0], buffer[1], buffer[2]);
		if (ins==0xa4) {
			printf(" FileID=%02x%02x",buffer[3], buffer[4]);
		} else {
			//printf("\n");
		}
	}
}

void simResponse_Process(unsigned char type, unsigned char *buffer, size_t length)
{
    size_t x;
// for status process :
    int RFU1,mem,fileID,fileT;
    int lofd; 		//length of following data

/* SIM responses to these commands:
    - select	  (for MF/DF files: fileID,memory space available, CHV (en/)disable
				    indicator, CHV status, GSM specific data.
		   for EF files: fileID,fileSize,access conditions, valid/invalid
				    indicator, structure of EF and length of
				    records if they exist)
    - status	  (fileID,memory space available, CHV en/disable indicator,
		    CHV status, GSM specific data like select command)
    - read binary (string of bytes)
    - read record (contents of the record)
    - seek	  (only for seek type 2 response = 1 byte, the record number)
    - increase		( etc.. etc.. described in GSM 11.11)
    - run gsm algorithm
    - get response
    - envelope
    - fetch
*/
	switch(type) {
	case 0x02:
		printf("(Read Binary) Binary's Data: ");
		for(x=0; x<length; x++) printf("%02x ",buffer[x]&0xFF);
		//printf("\n");
		break;
	case 0x03:
		printf("(Read Record) Record's Data:\n");
		for(x=0; x<length; x++) printf("%02x ",buffer[x]&0xFF);
		//printf("\n");
		break;
	case 0x05:
		printf("Get Response Data: ");
		for(x=0; x<length; x++) printf("%02x ",buffer[x]&0xFF);
		//printf("\n");
		break;
	case 0x06:
		printf("Status Response : ");
		RFU1   = ((buffer[0]&0xFF)<<8)|(buffer[1]&0xFF);
		mem    = ((buffer[2]&0xFF)<<8)|(buffer[3]&0xFF);
		fileID = ((buffer[4]&0xFF)<<8)|(buffer[5]&0xFF);
		fileT  = (buffer[6]&0xFF);
		//RFU2 = (((buffer[7]&0xFF)<<8)|(buffer[8]&0xFF))<<8|(buffe
		lofd   = (buffer[12]&0xFF);
		printf("RFU=%04x, mem=%04x, fileID=%04x,\nfileType=%02x, RFU=%02x%02x%02x%02x%02x, ",
			RFU1, mem, fileID, fileT,
			buffer[7], buffer[8], buffer[9], buffer[10], buffer[11] // RFU2
			); // not the proper way ;)
		printf("%02x bytes of GSM specific data follows: ", lofd);

		/* we could analyze these too but there is no meaning or time */
		for(x=0;x<(unsigned int)lofd;x++) printf("%02x ", buffer[13+x]);
		//printf("\n");
		break;
	case 0x09:
		printf("Select Response");
		// select response always has 00 bytes length
		break;
	default:
	    	printf("Unknown SIM Response : ");
		for(x=0; x<length; x++) printf("%02x ",buffer[x]&0xFF);
		//printf("\n");
		break;
	}
}

void simAnswer_Process(unsigned char type, unsigned char *buffer, size_t length)
{
	size_t x;

	switch(type) {
	case 0x00:
		/* in some sim commands (i.e. status or read record) the answer
		 * is more than 2 bytes. we could analyse all the bytes but the
		 * usefull ones are only the last 2 of them.
		 * Should we make a switch() for the answer or would it be
		 * too much??
		 */
		printf("SIM answer: ");
		for(x = MAX(2, length) - 2; x < length; x++) {
			printf("%02x ", buffer[x]&0xFF);
		}
		//printf("\n");
		break;
	case 0x01:
		/* 0x25XX sim command. I don't know why they re-write/send the command :-\
			for(x=0;x<length;x++) printf("%02x ", buffer[x]&0xFF);
			printf("\n");
		*/
		break;
	default:
		printf("Uknown subtype! dumping data: ");
		for(x=0;x<length;x++) printf("%02x ", buffer[x]&0xFF);
		//printf("\n");
		break;
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
