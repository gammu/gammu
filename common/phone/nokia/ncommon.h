/* (c) 2001-2005 by Marcin Wiacek */

#ifndef phone_nokia_common_h
#define phone_nokia_common_h

#include <gammu-limits.h>

#define N6110_FRAME_HEADER 0x00, 0x01, 0x00
#define N7110_FRAME_HEADER 0x00, 0x01, 0x01

typedef enum {
	MEM7110_CG		 = 0xf0, /* Caller groups memory 		*/
	MEM7110_SP		 = 0xf1, /* Speed dial memory 			*/

	MEM6510_CG2		 = 0x23  /* Caller groups 2 memory 		*/
} GSM_71_65MemoryType;

typedef enum {
	/* DCT3 and DCT4 */
	N7110_PBK_SIM_SPEEDDIAL	 = 0x04, /* Speed dial on SIM			*/
	N7110_PBK_NAME		 = 0x07, /* Text: name (always the only one) 	*/
	N7110_PBK_EMAIL		 = 0x08, /* Text: email adress			*/
	N7110_PBK_POSTAL	 = 0x09, /* Text: postal address 		*/
	N7110_PBK_NOTE		 = 0x0A, /* Text: note		 		*/
	N7110_PBK_NUMBER 	 = 0x0B, /* Phone number 			*/
	N7110_PBK_RINGTONE_ID	 = 0x0C, /* Ringtone ID 			*/
	N7110_PBK_DATETIME    	 = 0x13, /* Call register: date and time	*/
	N7110_PBK_UNKNOWN1	 = 0x19, /* Call register: with missed calls	*/
	N7110_PBK_SPEEDDIAL	 = 0x1A, /* Speed dial 				*/
	N7110_PBK_GROUPLOGO	 = 0x1B, /* Caller group: logo 			*/
	N7110_PBK_LOGOON	 = 0x1C, /* Caller group: is logo on ?		*/
	N7110_PBK_GROUP		 = 0x1E, /* Caller group number in pbk entry	*/

	/* DCT4 only */
	N6510_PBK_URL		 = 0x2C, /* Text: URL address 			*/
	N6510_PBK_SMSLIST_ID	 = 0x2E, /* SMS list assigment			*/
	N6510_PBK_VOICETAG_ID	 = 0x2F, /* Voice tag assigment			*/
	N6510_PBK_PICTURE_ID	 = 0x33, /* Picture ID assigment		*/
	N6510_PBK_RINGTONEFILE_ID= 0x37, /* Ringtone ID from filesystem/internal*/
	N6510_PBK_USER_ID        = 0x38, /* Text: user ID                       */
	N6510_PBK_UNKNOWN2	 = 0x3B, /* conversation list ID ?		*/
	N6510_PBK_UNKNOWN3	 = 0x3C, /* Instant Messaging service list ID ?	*/
	N6510_PBK_UNKNOWN4	 = 0x3D, /* presence list ID ? 			*/
	N6510_PBK_PUSHTOTALK_ID	 = 0x3F, /* SIP Address (Push to Talk address)	*/
	N6510_PBK_UNKNOWN5	 = 0x42, /* Unknown so far			*/
	N6510_PBK_GROUP2_ID	 = 0x43, /* Caller group type 2 ID (6230i,later)*/

	/* Series 40 3.0 */
	S4030_PBK_CALLLENGTH	 = 0x44,
	S4030_PBK_FIRSTNAME	 = 0x46,
	S4030_PBK_LASTNAME	 = 0x47,
	S4030_PBK_POSTAL	 = 0x4A,

	S4030_PBK_FORMALNAME      = 0x52,
	S4030_PBK_JOBTITLE        = 0x54,
	S4030_PBK_COMPANY         = 0x55,
	S4030_PBK_NICKNAME        = 0x56,
	S4030_PBK_BIRTHDAY        = 0x57,

	/* Unknown series */
	N2630_PBK_FAVMESSAGING	 = 0x65,
} GSM_71_65_Phonebook_Entries_Types;

typedef enum {
	N7110_PBK_NUMBER_HOME	 = 0x02,
	N7110_PBK_NUMBER_MOBILE	 = 0x03,
	N7110_PBK_NUMBER_FAX	 = 0x04,
	N7110_PBK_NUMBER_WORK	 = 0x06,
	N7110_PBK_NUMBER_GENERAL = 0x0A
} GSM_71_65_Phonebook_Number_Types;

typedef enum {
	S4030_PBK_POSTAL_EXTADDRESS 	= 0x4B,
	S4030_PBK_POSTAL_STREET 	= 0x4C,
	S4030_PBK_POSTAL_CITY 		= 0x4D,
	S4030_PBK_POSTAL_STATE 		= 0x4E,
	S4030_PBK_POSTAL_POSTAL 	= 0x4F,
	S4030_PBK_POSTAL_COUNTRY 	= 0x50
} GSM_Series40_30_Phonebook_Postal_Types;

typedef struct {
	unsigned int	Location[GSM_PHONE_MAXSMSINFOLDER]; /* locations of SMS messages in that folder */
	int		Number;				/* number of SMS messages in that folder */
} GSM_NOKIASMSFolder;

typedef struct {
	int		Location[GSM_MAXCALENDARTODONOTES];
	int		Number;
} GSM_NOKIACalToDoLocations;

#define NOKIA_PRESSPHONEKEY   0x01
#define NOKIA_RELEASEPHONEKEY 0x02

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
