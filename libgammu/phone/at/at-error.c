/* Vendor-aware AT error decoding. Source revisions and limitations are in
 * at-error-sources.md. Keep CME and CMS namespaces separate. */
#include <gammu-config.h>

#ifdef GSM_ENABLE_ATGEN
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "at-error.h"

typedef struct {
	int code;
	const char *text;
	GSM_Error error;
} ATErrorCode;

/* Standard CME codes; retain existing Gammu result mappings. */
static const ATErrorCode standard_cme[] = {
	{0, "phone failure", ERR_UNKNOWN},
	{1, "no connection to phone", ERR_UNKNOWN},
	{2, "phone-adaptor link reserved", ERR_UNKNOWN},
	{3, "operation not allowed", ERR_SECURITYERROR},
	{4, "operation not supported", ERR_NOTSUPPORTED},
	{5, "PH-SIM PIN required", ERR_SECURITYERROR},
	{10, "SIM not inserted", ERR_NOSIM},
	{11, "SIM PIN required", ERR_SECURITYERROR},
	{12, "SIM PUK required", ERR_SECURITYERROR},
	{13, "SIM failure", ERR_NOSIM},
	{14, "SIM busy", ERR_NOSIM},
	{15, "SIM wrong", ERR_NOSIM},
	{16, "incorrect password", ERR_SECURITYERROR},
	{17, "SIM PIN2 required", ERR_SECURITYERROR},
	{18, "SIM PUK2 required", ERR_SECURITYERROR},
	{20, "memory full", ERR_FULL},
	{21, "invalid index", ERR_INVALIDLOCATION},
	{22, "not found", ERR_EMPTY},
	{23, "memory failure", ERR_MEMORY},
	{24, "text string too long", ERR_INVALIDDATA},
	{25, "invalid characters in text string", ERR_INVALIDDATA},
	{26, "dial string too long", ERR_INVALIDDATA},
	{27, "invalid characters in dial string", ERR_INVALIDDATA},
	{30, "no network service", ERR_NETWORK_ERROR},
	{31, "network timeout", ERR_NETWORK_ERROR},
	{32, "Network not allowed - emergency calls only.", ERR_NETWORK_ERROR},
	{40, "Network personalization PIN required.", ERR_SECURITYERROR},
	{41, "Network personalization PUK required.", ERR_SECURITYERROR},
	{42, "Network subset personalization PIN required.", ERR_SECURITYERROR},
	{43, "Network subset personalization PUK required.", ERR_SECURITYERROR},
	{44, "Service provider personalization PIN required.", ERR_SECURITYERROR},
	{45, "Service provider personalization PUK required.", ERR_SECURITYERROR},
	{46, "Corporate personalization PIN required.", ERR_SECURITYERROR},
	{47, "Corporate personalization PUK required.", ERR_SECURITYERROR},
	{100, "unknown", ERR_UNKNOWN},
	{103, "Illegal MS (#3).", ERR_UNKNOWN},
	{106, "Illegal ME (#6).", ERR_UNKNOWN},
	{107, "GPRS services not allowed (#7).", ERR_UNKNOWN},
	{111, "Public Land Mobile Network (PLMN) not allowed (#11).", ERR_UNKNOWN},
	{112, "Location area not allowed (#12).", ERR_UNKNOWN},
	{113, "Roaming not allowed in this location area (#13).", ERR_UNKNOWN},
	{132, "Service option not supported (#32).", ERR_NOTSUPPORTED},
	{133, "Requested service option not subscribed (#33).", ERR_NOTSUPPORTED},
	{134, "Service option temporarily out of order (#34).", ERR_NOTSUPPORTED},
	{148, "Unspecified GPRS error.", ERR_UNKNOWN},
	{149, "PDP authentication failure.", ERR_UNKNOWN},
	{150, "Invalid mobile class.", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Standard CMS codes; retain existing Gammu result mappings. */
static const ATErrorCode standard_cms[] = {
	/* Preserve Gammu's legacy classification of numeric +CMS ERROR: 0. */
	{0, "Unspecified SMS failure", ERR_PHONE_INTERNAL},
	{1, "Unassigned (unallocated) number", ERR_UNKNOWN},
	{8, "Operator determined barring", ERR_UNKNOWN},
	{10, "Call barred", ERR_UNKNOWN},
	{21, "Short message transfer rejected", ERR_UNKNOWN},
	{27, "Destination out of service", ERR_UNKNOWN},
	{28, "Unidentified subscriber", ERR_UNKNOWN},
	{29, "Facility rejected", ERR_UNKNOWN},
	{30, "Unknown subscriber", ERR_UNKNOWN},
	{38, "Network out of order", ERR_NETWORK_ERROR},
	{41, "Temporary failure", ERR_NETWORK_ERROR},
	{42, "Congestion", ERR_NETWORK_ERROR},
	{47, "Resources unavailable, unspecified", ERR_NETWORK_ERROR},
	{50, "Requested facility not subscribed", ERR_UNKNOWN},
	{69, "Requested facility not implemented", ERR_UNKNOWN},
	{81, "Invalid short message transfer reference value", ERR_UNKNOWN},
	{95, "Invalid message, unspecified", ERR_UNKNOWN},
	{96, "Invalid mandatory information", ERR_UNKNOWN},
	{97, "Message type non-existent or not implemented", ERR_UNKNOWN},
	{98, "Message not compatible with short message protocol state", ERR_UNKNOWN},
	{99, "Information element non-existent or not implemented", ERR_UNKNOWN},
	{111, "Protocol error, unspecified", ERR_NETWORK_ERROR},
	{127, "Interworking, unspecified", ERR_UNKNOWN},
	{128, "Telematic interworking not supported", ERR_UNKNOWN},
	{129, "Short message Type 0 not supported", ERR_UNKNOWN},
	{130, "Cannot replace short message", ERR_UNKNOWN},
	{143, "Unspecified TP-PID error", ERR_UNKNOWN},
	{144, "Data coding scheme (alphabet) not supported", ERR_UNKNOWN},
	{145, "Message class not supported", ERR_UNKNOWN},
	{159, "Unspecified TP-DCS error", ERR_UNKNOWN},
	{160, "Command cannot be actioned", ERR_UNKNOWN},
	{161, "Command unsupported", ERR_UNKNOWN},
	{175, "Unspecified TP-Command error", ERR_UNKNOWN},
	{176, "TPDU not supported", ERR_UNKNOWN},
	{192, "SC busy", ERR_UNKNOWN},
	{193, "No SC subscription", ERR_UNKNOWN},
	{194, "SC system failure", ERR_UNKNOWN},
	{195, "Invalid SME address", ERR_UNKNOWN},
	{196, "Destination SME barred", ERR_UNKNOWN},
	{197, "SM Rejected-Duplicate SM", ERR_UNKNOWN},
	{198, "TP-VPF not supported", ERR_UNKNOWN},
	{199, "TP-VP not supported", ERR_UNKNOWN},
	{208, "SIM SMS storage full", ERR_UNKNOWN},
	{209, "No SMS storage capability in SIM", ERR_UNKNOWN},
	{210, "Error in MS", ERR_UNKNOWN},
	{211, "Memory capacity exceeded", ERR_FULL},
	{212, "SIM Application Toolkit Busy", ERR_UNKNOWN},
	{255, "Unspecified error cause", ERR_UNKNOWN},
	{300, "ME failure", ERR_PHONE_INTERNAL},
	{301, "SMS service of ME reserved", ERR_UNKNOWN},
	{302, "operation not allowed", ERR_SECURITYERROR},
	{303, "operation not supported", ERR_UNKNOWN},
	{304, "invalid PDU mode parameter", ERR_NOTSUPPORTED},
	{305, "invalid text mode parameter", ERR_BUG},
	{310, "SIM not inserted", ERR_UNKNOWN},
	{311, "SIM PIN required", ERR_SECURITYERROR},
	{312, "PH-SIM PIN required", ERR_SECURITYERROR},
	{313, "SIM failure", ERR_NOSIM},
	{314, "SIM busy", ERR_NOSIM},
	{315, "SIM wrong", ERR_NOSIM},
	{316, "SIM PUK required", ERR_SECURITYERROR},
	{317, "SIM PIN2 required", ERR_SECURITYERROR},
	{318, "SIM PUK2 required", ERR_SECURITYERROR},
	{320, "memory failure", ERR_PHONE_INTERNAL},
	{321, "invalid memory index", ERR_INVALIDLOCATION},
	{322, "memory full", ERR_FULL},
	{330, "SMSC address unknown", ERR_UNKNOWN},
	{331, "no network service", ERR_NETWORK_ERROR},
	{332, "network timeout", ERR_NETWORK_ERROR},
	{340, "no CNMA acknowledgement expected", ERR_UNKNOWN},
	{500, "unknown error", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Siemens TC65 V02.000, section 2.12, tables 2.4-2.7 */
static const ATErrorCode siemens_cme[] = {
	{256, "Operation temporary not allowed", ERR_BUSY},
	{257, "call barred", ERR_SECURITYERROR},
	{258, "phone busy", ERR_BUSY},
	{259, "user abort", ERR_CANCELED},
	{260, "invalid dial string", ERR_INVALIDDATA},
	{261, "ss not executed", ERR_UNKNOWN},
	{262, "SIM blocked", ERR_SECURITYERROR},
	{263, "Invalid Block", ERR_INVALIDDATA},
	{578, "GPRS - unspecified activation rejection", ERR_NETWORK_ERROR},
	{588, "GPRS - feature not supported", ERR_NOTSUPPORTED},
	{594, "GPRS - invalid address length", ERR_INVALIDDATA},
	{595, "GPRS - invalid character in address string", ERR_INVALIDDATA},
	{596, "GPRS - invalid cid value", ERR_INVALIDDATA},
	{607, "GPRS - missing or unknown APN", ERR_INVALIDDATA},
	{615, "network failure", ERR_NETWORK_ERROR},
	{616, "network is down", ERR_NETWORK_ERROR},
	{625, "GPRS - pdp type not supported", ERR_NOTSUPPORTED},
	{630, "GPRS - profile (cid) not defined", ERR_INVALIDDATA},
	{632, "GPRS - QOS not accepted", ERR_INVALIDDATA},
	{633, "GPRS - QOS validation fail", ERR_INVALIDDATA},
	{639, "service type not yet available", ERR_UNKNOWN},
	{640, "operation of service temporary not allowed", ERR_BUSY},
	{643, "GPRS - unknown PDP address or type", ERR_INVALIDDATA},
	{644, "GPRS - unknown PDP context", ERR_INVALIDDATA},
	{646, "GPRS - QOS invalid parameter", ERR_INVALIDDATA},
	{764, "missing input value", ERR_INVALIDDATA},
	{765, "invalid input value", ERR_INVALIDDATA},
	{767, "operation failed", ERR_UNKNOWN},
	{769, "unable to get control of required module", ERR_BUSY},
	{770, "SIM invalid - network reject", ERR_NOSIM},
	{771, "call setup in progress", ERR_BUSY},
	{772, "SIM powered down", ERR_NOSIM},
	{0, NULL, ERR_UNKNOWN}
};

/* Siemens TC65 V02.000, section 2.12, pp. 53-54 */
static const ATErrorCode siemens_cms[] = {
	{512, "User abort", ERR_CANCELED},
	{513, "unable to store", ERR_MEMORY},
	{514, "invalid status", ERR_INVALIDDATA},
	{515, "invalid character in address string", ERR_INVALIDDATA},
	{516, "invalid length", ERR_INVALIDDATA},
	{517, "invalid character in pdu", ERR_INVALIDDATA},
	{519, "invalid length or character", ERR_INVALIDDATA},
	{520, "invalid character in text", ERR_INVALIDDATA},
	{521, "timer expired", ERR_TIMEOUT},
	{522, "Operation temporary not allowed", ERR_BUSY},
	{532, "SIM not ready", ERR_BUSY},
	{534, "Cell Broadcast error unknown", ERR_UNKNOWN},
	{535, "PS busy", ERR_BUSY},
	{538, "invalid parameter", ERR_INVALIDDATA},
	{549, "incorrect PDU length", ERR_INVALIDDATA},
	{550, "invalid message type indication (MTI)", ERR_INVALIDDATA},
	{551, "invalid (non-hex) chars in address", ERR_INVALIDDATA},
	{553, "incorrect PDU length (UDL)", ERR_INVALIDDATA},
	{554, "incorrect SCA length", ERR_INVALIDDATA},
	{0, NULL, ERR_UNKNOWN}
};

/* Motorola g20 98-08901C68-O, table 95, pp. 197-198 */
static const ATErrorCode motorola_cme[] = {
	{6, "PH-FSIM PIN required", ERR_SECURITYERROR},
	{7, "PH-FSIM PUK required", ERR_SECURITYERROR},
	{33, "Command aborted", ERR_CANCELED},
	{34, "Numeric parameter instead of text parameter", ERR_INVALIDDATA},
	{35, "Text parameter instead of numeric parameter", ERR_INVALIDDATA},
	{36, "Numeric parameter is out of bounds", ERR_INVALIDDATA},
	{259, "Unknown calling error", ERR_UNKNOWN},
	{264, "Unlock code required", ERR_SECURITYERROR},
	{265, "Network busy", ERR_NETWORK_ERROR},
	{266, "Invalid phone number", ERR_INVALIDDATA},
	{267, "Number entry already started", ERR_UNKNOWN},
	{268, "Cancelled by user", ERR_CANCELED},
	{269, "Number entry could not be started", ERR_UNKNOWN},
	{280, "Data lost", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Motorola g20 98-08901C68-O, table 95, p. 198 */
static const ATErrorCode motorola_cms[] = {
	{512, "Network busy", ERR_NETWORK_ERROR},
	{513, "Invalid destination address", ERR_INVALIDDATA},
	{514, "Invalid message body length", ERR_INVALIDDATA},
	{515, "Phone is not in service", ERR_NETWORK_ERROR},
	{516, "Invalid preferred memory storage", ERR_INVALIDLOCATION},
	{517, "User terminated", ERR_CANCELED},
	{518, "Inactive socket", ERR_UNKNOWN},
	{519, "Socket already open", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Huawei UMTS M2M V100R001 issue 18, section 21.2 */
static const ATErrorCode huawei_cme[] = {
	{257, "network rejected request", ERR_NETWORK_ERROR},
	{258, "retry operation", ERR_BUSY},
	{259, "invalid deflected to number", ERR_INVALIDDATA},
	{260, "deflected to own number", ERR_INVALIDDATA},
	{261, "unknown subscriber", ERR_UNKNOWN},
	{262, "service not in use", ERR_UNKNOWN},
	{263, "unknown class", ERR_INVALIDDATA},
	{264, "unknown network message", ERR_UNKNOWN},
	{65280, "call index error", ERR_INVALIDDATA},
	{65281, "call state error", ERR_UNKNOWN},
	{65282, "sys state error", ERR_UNKNOWN},
	{65283, "parameters error", ERR_INVALIDDATA},
	{65284, "spn file wrong", ERR_UNKNOWN},
	{65285, "spn file accessed denied", ERR_SECURITYERROR},
	{65286, "spn file not exist", ERR_FILENOTEXIST},
	{65287, "another SPN query operation still not finished", ERR_BUSY},
	{0, NULL, ERR_UNKNOWN}
};

/* Falcom SAMBA75 version 1.00, section 1.12.1, tables 1.2-1.5 */
static const ATErrorCode falcom_cme[] = {
	{6, "PH-FSIM PIN required", ERR_SECURITYERROR},
	{7, "PH-FSIM PUK required", ERR_SECURITYERROR},
	{48, "Master Phone Code required", ERR_SECURITYERROR},
	{256, "Operation temporary not allowed", ERR_BUSY},
	{257, "Call barred", ERR_SECURITYERROR},
	{258, "Phone is busy", ERR_BUSY},
	{259, "User abort", ERR_CANCELED},
	{260, "Invalid dail string", ERR_INVALIDDATA},
	{261, "Ss not executed", ERR_UNKNOWN},
	{262, "SIM blocked", ERR_SECURITYERROR},
	{263, "Invalid Block", ERR_INVALIDDATA},
	{588, "GPRS - feature not supported", ERR_NOTSUPPORTED},
	{594, "GPRS - invalid address length", ERR_INVALIDDATA},
	{595, "GPRS - invalid character in address string", ERR_INVALIDDATA},
	{596, "GPRS - invalid cid value", ERR_INVALIDDATA},
	{607, "GPRS - missing or unknown APN", ERR_INVALIDDATA},
	{625, "GPRS - pdp type not supported", ERR_NOTSUPPORTED},
	{630, "GPRS - profile (cid) not defined", ERR_INVALIDDATA},
	{632, "GPRS - QOS not accepted", ERR_INVALIDDATA},
	{633, "GPRS -QOS validation fail", ERR_INVALIDDATA},
	{643, "GPRS - unknown PDP address or type", ERR_INVALIDDATA},
	{644, "GPRS - unknown PDP context", ERR_INVALIDDATA},
	{646, "GPRS - QOS invalid parameter", ERR_INVALIDDATA},
	{0, NULL, ERR_UNKNOWN}
};

/* iTegno 38XX v1.0, section 19.1 */
static const ATErrorCode itegno_cme[] = {
	{512, "Failed to Abort", ERR_UNKNOWN},
	{513, "Lower Layer Failure (for SMS)", ERR_NETWORK_ERROR},
	{514, "SIM Busy with SIM Application Toolkit", ERR_BUSY},
	{0, NULL, ERR_UNKNOWN}
};

/* iTegno 38XX v1.0, section 19.2.2 */
static const ATErrorCode itegno_cms[] = {
	{512, "Failed to Abort", ERR_UNKNOWN},
	{513, "ACM Reset Needed", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Quectel EC25/EC21 v1.3, section 21.5 */
static const ATErrorCode quectel_cme[] = {
	{901, "Audio unknown error", ERR_UNKNOWN},
	{902, "Audio invalid parameters", ERR_INVALIDDATA},
	{903, "Audio operation not supported", ERR_NOTSUPPORTED},
	{904, "Audio device busy", ERR_BUSY},
	{0, NULL, ERR_UNKNOWN}
};

/* Quectel EC25/EC21 v1.3, section 21.6 */
static const ATErrorCode quectel_cms[] = {
	{512, "SIM not ready", ERR_BUSY},
	{513, "Message length exceeds", ERR_INVALIDDATA},
	{514, "Invalid request parameters", ERR_INVALIDDATA},
	{515, "ME storage failure", ERR_MEMORY},
	{517, "Invalid service mode", ERR_INVALIDDATA},
	{528, "More message to send state error", ERR_UNKNOWN},
	{529, "MO SMS is not allow", ERR_SECURITYERROR},
	{530, "GPRS is suspended", ERR_NETWORK_ERROR},
	{531, "ME storage full", ERR_FULL},
	{0, NULL, ERR_UNKNOWN}
};

/* SIMCom SIM800 v1.01, section 19.1 */
static const ATErrorCode simcom_cme[] = {
	{6, "PH-FSIM PIN required", ERR_SECURITYERROR},
	{7, "PH-FSIM PUK required", ERR_SECURITYERROR},
	{99, "resource limitation", ERR_UNKNOWN},
	{160, "DNS resolve failed", ERR_NETWORK_ERROR},
	{161, "Socket open failed", ERR_NETWORK_ERROR},
	{171, "MMS task is busy now", ERR_BUSY},
	{172, "The mms data is oversize", ERR_UNKNOWN},
	{173, "The operation is overtime", ERR_TIMEOUT},
	{174, "There is no mms receiver", ERR_UNKNOWN},
	{175, "The storage for address is full", ERR_FULL},
	{176, "Not find the address", ERR_EMPTY},
	{177, "The connection to network is failed", ERR_NETWORK_ERROR},
	{178, "Failed to read push message", ERR_UNKNOWN},
	{179, "This is not a push message", ERR_UNKNOWN},
	{180, "gprs is not attached", ERR_NETWORK_ERROR},
	{181, "tcpip stack is busy", ERR_BUSY},
	{182, "The mms storage is full", ERR_FULL},
	{183, "The box is empty", ERR_EMPTY},
	{184, "failed to save mms", ERR_MEMORY},
	{185, "It is in edit mode", ERR_BUSY},
	{186, "It is not in edit mode", ERR_UNKNOWN},
	{187, "No content in the buffer", ERR_EMPTY},
	{188, "Not find the file", ERR_FILENOTEXIST},
	{189, "Failed to receive mms", ERR_UNKNOWN},
	{190, "Failed to read mms", ERR_UNKNOWN},
	{191, "Not M-Notification.ind", ERR_UNKNOWN},
	{192, "The mms inclosure is full", ERR_UNKNOWN},
	{193, "Unknown", ERR_UNKNOWN},
	{753, "missing required cmd parameter", ERR_INVALIDDATA},
	{754, "invalid SIM command", ERR_INVALIDDATA},
	{755, "invalid File Id", ERR_INVALIDDATA},
	{756, "missing required P1/2/3 parameter", ERR_INVALIDDATA},
	{757, "invalid P1/2/3 parameter", ERR_INVALIDDATA},
	{758, "missing required command data", ERR_INVALIDDATA},
	{759, "invalid characters in command data", ERR_INVALIDDATA},
	{765, "Invalid input value", ERR_INVALIDDATA},
	{766, "Unsupported mode", ERR_NOTSUPPORTED},
	{767, "Operation failed", ERR_UNKNOWN},
	{768, "Mux already running", ERR_BUSY},
	{769, "Unable to get control", ERR_BUSY},
	{770, "SIM network reject", ERR_NOSIM},
	{771, "Call setup in progress", ERR_BUSY},
	{772, "SIM powered down", ERR_NOSIM},
	{773, "SIM file not present", ERR_FILENOTEXIST},
	{791, "Param count not enough", ERR_INVALIDDATA},
	{792, "Param count beyond", ERR_INVALIDDATA},
	{793, "Param value range beyond", ERR_INVALIDDATA},
	{794, "Param type not match", ERR_INVALIDDATA},
	{795, "Param format invalid", ERR_INVALIDDATA},
	{796, "Get a null param", ERR_INVALIDDATA},
	{797, "CFUN state is 0 or 4", ERR_UNKNOWN},
	{810, "No Error", ERR_UNKNOWN},
	{811, "Unrecognized Command", ERR_NOTSUPPORTED},
	{812, "Return Value Error", ERR_UNKNOWN},
	{813, "Syntax Error", ERR_INVALIDDATA},
	{814, "Unspecified Error", ERR_UNKNOWN},
	{815, "Data Transfer Already", ERR_UNKNOWN},
	{816, "Action Already", ERR_UNKNOWN},
	{817, "Not At Cmd", ERR_UNKNOWN},
	{818, "Multi Cmd too long", ERR_UNKNOWN},
	{819, "Abort Cops", ERR_UNKNOWN},
	{820, "No Call Disc", ERR_UNKNOWN},
	{821, "BT SAP Undefined", ERR_UNKNOWN},
	{822, "BT SAP Not Accessible", ERR_UNKNOWN},
	{823, "BT SAP Card Removed", ERR_NOSIM},
	{824, "AT Not Allowed By Customer", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* SIMCom SIM800 v1.01, section 19.2 */
static const ATErrorCode simcom_cms[] = {
	{3, "No route to destination", ERR_UNKNOWN},
	{6, "Channel unacceptable", ERR_UNKNOWN},
	{16, "Normal call clearing", ERR_UNKNOWN},
	{17, "User busy", ERR_BUSY},
	{18, "No user responding", ERR_UNKNOWN},
	{19, "User alerting, no answer", ERR_UNKNOWN},
	{22, "Number changed", ERR_UNKNOWN},
	{25, "Pre-emption", ERR_UNKNOWN},
	{26, "Non-selected user clearing", ERR_UNKNOWN},
	{32, "Normal, unspecified", ERR_UNKNOWN},
	{34, "No circuit/channel available", ERR_UNKNOWN},
	{43, "Access information discarded", ERR_UNKNOWN},
	{44, "Requested circuit/channel not available", ERR_UNKNOWN},
	{49, "Quality of service unavailable", ERR_UNKNOWN},
	{55, "Requested facility not subscribed", ERR_UNKNOWN},
	{57, "Bearer capability not authorized", ERR_SECURITYERROR},
	{58, "Bearer capability not presently available", ERR_UNKNOWN},
	{63, "Service or option not available, unspecified", ERR_UNKNOWN},
	{65, "Bearer service not implemented", ERR_NOTSUPPORTED},
	{68, "ACM equal or greater than ACM maximum", ERR_UNKNOWN},
	{70, "Only restricted digital information bearer capability is available", ERR_UNKNOWN},
	{79, "Service or option not implemented, unspecified", ERR_NOTSUPPORTED},
	{87, "User not member of CUG", ERR_UNKNOWN},
	{88, "Incompatible destination", ERR_UNKNOWN},
	{91, "Invalid transit network selection", ERR_UNKNOWN},
	{100, "Conditional information element error", ERR_UNKNOWN},
	{101, "Message not compatible with protocol", ERR_UNKNOWN},
	{102, "Recovery on timer expiry", ERR_UNKNOWN},
	{213, "SIM data download error", ERR_UNKNOWN},
	{224, "CP retry exceed", ERR_NETWORK_ERROR},
	{225, "RP trim timeout", ERR_NETWORK_ERROR},
	{226, "SMS connection broken", ERR_NETWORK_ERROR},
	{323, "invalid input parameter", ERR_INVALIDDATA},
	{324, "invalid input format", ERR_INVALIDDATA},
	{325, "invalid input value", ERR_INVALIDDATA},
	{512, "SMS no error", ERR_UNKNOWN},
	{513, "Message length exceeds maximum length", ERR_INVALIDDATA},
	{514, "Invalid request parameters", ERR_INVALIDDATA},
	{515, "ME storage failure", ERR_MEMORY},
	{516, "Invalid bearer service", ERR_INVALIDDATA},
	{517, "Invalid service mode", ERR_INVALIDDATA},
	{518, "Invalid storage type", ERR_INVALIDDATA},
	{519, "Invalid message format", ERR_INVALIDDATA},
	{520, "Too many MO concatenated messages", ERR_FULL},
	{521, "SMSAL not ready", ERR_BUSY},
	{522, "SMSAL no more service", ERR_UNKNOWN},
	{523, "Not support TP-Status-Report & TP-Command in storage", ERR_NOTSUPPORTED},
	{524, "Reserved MTI", ERR_INVALIDDATA},
	{525, "No free entity in RL layer", ERR_FULL},
	{526, "The port number is already registerred", ERR_UNKNOWN},
	{527, "There is no free entity for port number", ERR_FULL},
	{528, "More Message to Send state error", ERR_UNKNOWN},
	{529, "MO SMS is not allow", ERR_SECURITYERROR},
	{530, "GPRS is suspended", ERR_NETWORK_ERROR},
	{531, "ME storage full", ERR_FULL},
	{532, "Doing SIM refresh", ERR_BUSY},
	{0, NULL, ERR_UNKNOWN}
};

/* Telit 80617ST10991A rev. 9, section 3.2.2.1 */
static const ATErrorCode telit_cme[] = {
	{6, "PH-FSIM PIN required", ERR_SECURITYERROR},
	{7, "PH-FSIM PUK required", ERR_SECURITYERROR},
	{34, "numeric parameter instead of text parameter", ERR_INVALIDDATA},
	{35, "text parameter instead of numeric parameter", ERR_INVALIDDATA},
	{36, "numeric parameter out of bounds", ERR_INVALIDDATA},
	{37, "text string too short", ERR_INVALIDDATA},
	{38, "The GPIO Pin is already used", ERR_BUSY},
	{49, "EAP method not supported", ERR_NOTSUPPORTED},
	{50, "Invalid EAP parameter", ERR_INVALIDDATA},
	{51, "Parameter length error for all Auth commands", ERR_INVALIDDATA},
	{52, "Temporary error for all Auth command", ERR_BUSY},
	{53, "not verified hidden key", ERR_SECURITYERROR},
	{257, "network rejected request", ERR_NETWORK_ERROR},
	{258, "retry operation", ERR_BUSY},
	{259, "invalid deflected to number", ERR_INVALIDDATA},
	{260, "deflected to own number", ERR_INVALIDDATA},
	{261, "unknown subscriber", ERR_UNKNOWN},
	{262, "service not available", ERR_UNKNOWN},
	{263, "unknown class", ERR_INVALIDDATA},
	{264, "unknown network message", ERR_UNKNOWN},
	{273, "Minimum TFT per PDP address error", ERR_INVALIDDATA},
	{274, "Duplicate TFT eval prec index", ERR_INVALIDDATA},
	{275, "Invalid TFT param combination", ERR_INVALIDDATA},
	{277, "Invalid number of parameters", ERR_INVALIDDATA},
	{278, "Invalid Parameter", ERR_INVALIDDATA},
	{320, "Call index error", ERR_INVALIDDATA},
	{321, "Call state error", ERR_UNKNOWN},
	{322, "Sys state error", ERR_UNKNOWN},
	{323, "Parameters error", ERR_INVALIDDATA},
	{550, "generic undocumented error", ERR_UNKNOWN},
	{551, "wrong state", ERR_UNKNOWN},
	{552, "wrong mode", ERR_UNKNOWN},
	{553, "context already activated", ERR_BUSY},
	{554, "stack already active", ERR_BUSY},
	{555, "activation failed", ERR_UNKNOWN},
	{556, "context not opened", ERR_UNKNOWN},
	{557, "can not setup socket", ERR_NETWORK_ERROR},
	{558, "can not resolve DN", ERR_NETWORK_ERROR},
	{559, "time-out in opening socket", ERR_TIMEOUT},
	{560, "can not open socket", ERR_NETWORK_ERROR},
	{561, "remote disconnected or time-out", ERR_NETWORK_ERROR},
	{562, "connection failed", ERR_NETWORK_ERROR},
	{563, "tx error", ERR_UNKNOWN},
	{564, "already listening", ERR_BUSY},
	{565, "socket disconnection", ERR_NETWORK_ERROR},
	{566, "can not resume socket", ERR_NETWORK_ERROR},
	{567, "ip version type incompatible", ERR_UNKNOWN},
	{568, "ipv6 not enabled", ERR_UNKNOWN},
	{600, "Generic undocumented error", ERR_UNKNOWN},
	{601, "wrong state", ERR_UNKNOWN},
	{602, "Can not activate", ERR_UNKNOWN},
	{603, "Can not resolve name", ERR_NETWORK_ERROR},
	{604, "Can not allocate control socket", ERR_UNKNOWN},
	{605, "Can not connect control socket", ERR_NETWORK_ERROR},
	{606, "Bad or no response from server", ERR_NETWORK_ERROR},
	{607, "Not connected", ERR_NETWORK_ERROR},
	{608, "Already connected", ERR_BUSY},
	{609, "Context down", ERR_NETWORK_ERROR},
	{612, "Resource used by other instance", ERR_BUSY},
	{613, "Data socket yet opened in cmdmode", ERR_BUSY},
	{614, "FTP CmdMode data socket closed", ERR_UNKNOWN},
	{615, "FTP not connected", ERR_NETWORK_ERROR},
	{616, "FTP disconnected", ERR_NETWORK_ERROR},
	{617, "FTP read command closed", ERR_UNKNOWN},
	{618, "FTP read command error", ERR_UNKNOWN},
	{619, "FTP write command closed", ERR_UNKNOWN},
	{620, "FTP write command error", ERR_UNKNOWN},
	{621, "FTP read data closed", ERR_UNKNOWN},
	{622, "FTP read data error", ERR_UNKNOWN},
	{623, "FTP write data closed", ERR_UNKNOWN},
	{624, "FTP write data error", ERR_UNKNOWN},
	{625, "FTP host not found", ERR_NETWORK_ERROR},
	{626, "FTP accept failure", ERR_NETWORK_ERROR},
	{627, "FTP listen failure", ERR_NETWORK_ERROR},
	{628, "FTP bind failure", ERR_NETWORK_ERROR},
	{629, "FTP file create failure", ERR_UNKNOWN},
	{630, "FTP file get failure", ERR_UNKNOWN},
	{631, "FTP file put failure", ERR_UNKNOWN},
	{632, "FTP file not found", ERR_FILENOTEXIST},
	{633, "FTP timed out", ERR_TIMEOUT},
	{634, "FTP login incorrect", ERR_SECURITYERROR},
	{635, "FTP close error", ERR_UNKNOWN},
	{636, "FTP server not ready", ERR_BUSY},
	{637, "FTP server shutdown", ERR_NETWORK_ERROR},
	{638, "FTP unexpected reply", ERR_UNKNOWN},
	{639, "FTP user ID and password don't match", ERR_SECURITYERROR},
	{640, "FTP user ID and password don't match", ERR_SECURITYERROR},
	{641, "FTP user already logged in", ERR_BUSY},
	{642, "FTP open channel timeout", ERR_TIMEOUT},
	{643, "FTP communication timeout", ERR_TIMEOUT},
	{644, "FTP unknown error", ERR_UNKNOWN},
	{657, "Network survey error (No Carrier)", ERR_NETWORK_ERROR},
	{658, "Network survey error (Busy)", ERR_BUSY},
	{659, "Network survey error (Wrong request)", ERR_INVALIDDATA},
	{660, "Network survey error (Aborted)", ERR_CANCELED},
	{680, "LU processing", ERR_BUSY},
	{681, "Network search aborted", ERR_CANCELED},
	{682, "PTM mode", ERR_UNKNOWN},
	{683, "Network search terminated", ERR_UNKNOWN},
	{684, "CSG Search processing", ERR_BUSY},
	{690, "Active call state", ERR_BUSY},
	{691, "RR connection established", ERR_UNKNOWN},
	{770, "SIM invalid", ERR_NOSIM},
	{900, "No Response for AT Command", ERR_TIMEOUT},
	{1000, "SSL not activated", ERR_UNKNOWN},
	{1001, "SSL certs and keys wrong or not stored", ERR_SECURITYERROR},
	{1002, "SSL generic error", ERR_UNKNOWN},
	{1003, "SSL already activated", ERR_BUSY},
	{1004, "SSL error during handshake", ERR_UNKNOWN},
	{1005, "SSL socket error", ERR_NETWORK_ERROR},
	{1006, "SSL invalid state", ERR_UNKNOWN},
	{1007, "SSL cannot activate", ERR_UNKNOWN},
	{1008, "SSL not connected", ERR_NETWORK_ERROR},
	{1009, "SSL already connected", ERR_BUSY},
	{1010, "SSL error enc/dec data", ERR_UNKNOWN},
	{1011, "SSL disconnected", ERR_NETWORK_ERROR},
	{1100, "Model not recognized", ERR_UNKNOWN},
	{1101, "Model information missing", ERR_UNKNOWN},
	{1102, "Unable to open the file", ERR_UNKNOWN},
	{1103, "Unable to close the file", ERR_UNKNOWN},
	{1104, "Unable to read the nv file", ERR_UNKNOWN},
	{1105, "Unable to write the nv file", ERR_UNKNOWN},
	{1106, "Input pattern is wrong", ERR_INVALIDDATA},
	{1113, "Call establishment failed", ERR_NETWORK_ERROR},
	{1114, "File name already exist", ERR_FILEALREADYEXIST},
	{0, NULL, ERR_UNKNOWN}
};

/* Telit 80617ST10991A rev. 9, section 3.2.2.2 */
static const ATErrorCode telit_cms[] = {
	{510, "msg blocked", ERR_UNKNOWN},
	{512, "No SM resources", ERR_BUSY},
	{513, "TR1M timeout", ERR_NETWORK_ERROR},
	{514, "LL error", ERR_NETWORK_ERROR},
	{515, "No response from network", ERR_NETWORK_ERROR},
	{0, NULL, ERR_UNKNOWN}
};

/* Sierra Wireless HL78xx 41111821 rev. 11, section 14.3.1 */
static const ATErrorCode sierra_cme[] = {
	{48, "Hidden key required", ERR_SECURITYERROR},
	{49, "EAP method not supported", ERR_NOTSUPPORTED},
	{50, "Incorrect parameters", ERR_INVALIDDATA},
	{60, "Internal system failure", ERR_PHONE_INTERNAL},
	{99, "Resource limitation", ERR_UNKNOWN},
	{201, "Alternate SIM conflict", ERR_UNKNOWN},
	{500, "CTS Handover on Progress", ERR_BUSY},
	{501, "Cellular Protocol Stack Out of service state", ERR_NETWORK_ERROR},
	{502, "CTS Unspecified Error", ERR_UNKNOWN},
	{650, "General AVMS error", ERR_UNKNOWN},
	{651, "Communication error", ERR_NETWORK_ERROR},
	{652, "Session in progress", ERR_BUSY},
	{654, "RDMS services are in \"deactivated\" state", ERR_UNKNOWN},
	{655, "RDMS services are in \"prohibited\" state", ERR_UNKNOWN},
	{656, "RDMS services are in \"to be provisioned\" state; no available NAP", ERR_UNKNOWN},
	{800, "SIM Security unspecified error", ERR_SECURITYERROR},
	{902, "No more sockets available; the maximum number has been reached", ERR_FULL},
	{903, "Memory problem", ERR_MEMORY},
	{904, "DNS error", ERR_NETWORK_ERROR},
	{905, "TCP disconnection by the server", ERR_NETWORK_ERROR},
	{906, "TCP/UDP connection error", ERR_NETWORK_ERROR},
	{907, "Generic error", ERR_UNKNOWN},
	{908, "Fail to accept client request's", ERR_UNKNOWN},
	{909, "Data send by KTCPSND/KUDPSND are incoherent", ERR_INVALIDDATA},
	{910, "Bad session ID", ERR_INVALIDDATA},
	{911, "Session is already running", ERR_BUSY},
	{912, "No more sessions can be used (maximum session is 6)", ERR_FULL},
	{913, "Socket connection timer timeout", ERR_NETWORK_ERROR},
	{914, "Control socket connection timer timeout", ERR_NETWORK_ERROR},
	{915, "A parameter is not expected", ERR_INVALIDDATA},
	{916, "A parameter has an invalid range of values", ERR_INVALIDDATA},
	{917, "A parameter is missing", ERR_INVALIDDATA},
	{918, "Feature is not supported", ERR_NOTSUPPORTED},
	{919, "Feature is not available", ERR_UNKNOWN},
	{920, "Protocol is not supported", ERR_NOTSUPPORTED},
	{921, "Error due to invalid state of bearer connection", ERR_UNKNOWN},
	{922, "Error due to invalid state of session", ERR_UNKNOWN},
	{923, "Error due to invalid state of terminate port data mode", ERR_UNKNOWN},
	{924, "Error due to session busy, retry later", ERR_BUSY},
	{925, "Failed to decode HTTP header's name, missing ':'", ERR_INVALIDDATA},
	{926, "Failed to decode HTTP header's value, missing 'cr/lf'", ERR_INVALIDDATA},
	{927, "HTTP header's name is an empty string", ERR_INVALIDDATA},
	{928, "HTTP header's value is an empty string", ERR_INVALIDDATA},
	{929, "Format of input data is invalid", ERR_INVALIDDATA},
	{930, "Content of input data is invalid or not supported", ERR_INVALIDDATA},
	{931, "The length of a parameter is invalid", ERR_INVALIDDATA},
	{932, "The format of a parameter is invalid", ERR_INVALIDDATA},
	{0, NULL, ERR_UNKNOWN}
};

/* Sierra Wireless HL78xx 41111821 rev. 11, section 14.3.3 */
static const ATErrorCode sierra_cms[] = {
	{606, "ME Busy - CM server request already pending", ERR_BUSY},
	{0, NULL, ERR_UNKNOWN}
};

/* u-blox LENA-R8 UBX-22016905 R04, appendix A.1 */
static const ATErrorCode ublox_cme[] = {
	{6, "PH-FSIM PIN required", ERR_SECURITYERROR},
	{7, "PH-FSIM PUK required", ERR_SECURITYERROR},
	{50, "Incorrect parameters", ERR_INVALIDDATA},
	{51, "Command implemented but currently disabled", ERR_UNKNOWN},
	{52, "Command aborted by user", ERR_CANCELED},
	{53, "Not attached to network due to MT functionality restrictions", ERR_UNKNOWN},
	{54, "Modem not allowed - MT restricted to emergency calls only", ERR_UNKNOWN},
	{55, "Operation not allowed because of MT functionality restrictions", ERR_UNKNOWN},
	{56, "Fixed dial number only allowed - called number is not a fixed dial number", ERR_UNKNOWN},
	{57, "Temporarily out of service due to other MT usage", ERR_UNKNOWN},
	{108, "GPRS and non GPRS services not allowed", ERR_UNKNOWN},
	{114, "GPRS services not allowed in this PLMN", ERR_UNKNOWN},
	{115, "No Suitable Cells In Location Area", ERR_UNKNOWN},
	{122, "Congestion", ERR_UNKNOWN},
	{125, "Not authorized for this CSG", ERR_SECURITYERROR},
	{126, "Insufficient resources", ERR_UNKNOWN},
	{127, "Missing or unknown APN", ERR_UNKNOWN},
	{128, "Unknown PDP address or PDP type", ERR_UNKNOWN},
	{129, "User authentication failed", ERR_SECURITYERROR},
	{130, "Request rejected by Serving GW or PDN GW", ERR_UNKNOWN},
	{131, "Request rejected, unspecified", ERR_UNKNOWN},
	{135, "NS-api already used", ERR_UNKNOWN},
	{137, "EPS QoS not accepted", ERR_UNKNOWN},
	{138, "Network failure", ERR_NETWORK_ERROR},
	{140, "Feature not supported", ERR_NOTSUPPORTED},
	{141, "Semantic error in the TFT operation", ERR_UNKNOWN},
	{142, "Syntactical error in the TFT operation", ERR_UNKNOWN},
	{143, "Unknown PDP context", ERR_UNKNOWN},
	{144, "Semantic errors in packet filter(s)", ERR_UNKNOWN},
	{145, "Syntactical errors in packet filter(s)", ERR_UNKNOWN},
	{146, "PDP context without TFT already activated", ERR_UNKNOWN},
	{147, "PTI mismatch", ERR_UNKNOWN},
	{153, "ESM information not received", ERR_UNKNOWN},
	{154, "PDN connection does not exist", ERR_UNKNOWN},
	{155, "Multiple PDN connections for a given APN not allowed", ERR_UNKNOWN},
	{156, "User Busy", ERR_BUSY},
	{159, "Uplink Busy/ Flow Control", ERR_BUSY},
	{160, "Bearer handling not supported", ERR_NOTSUPPORTED},
	{165, "Maximum number of EPS bearers reached", ERR_UNKNOWN},
	{166, "Requested APN not supported in current RAT and PLMN combination", ERR_NOTSUPPORTED},
	{168, "Network failure", ERR_NETWORK_ERROR},
	{169, "IMSI unknown in VLR", ERR_UNKNOWN},
	{170, "Congestion", ERR_UNKNOWN},
	{171, "Last PDN disconnection not allowed", ERR_UNKNOWN},
	{172, "Semantically incorrect message", ERR_UNKNOWN},
	{173, "Mandatory information element error", ERR_UNKNOWN},
	{174, "Information element non-existent or not implemented", ERR_UNKNOWN},
	{175, "Conditional IE error", ERR_UNKNOWN},
	{176, "Protocol error, unspecified", ERR_UNKNOWN},
	{177, "Operator determined barring", ERR_SECURITYERROR},
	{178, "Maximum number of PDP contexts reached", ERR_UNKNOWN},
	{179, "Requested APN not supported in current RAT and PLMN combination", ERR_NOTSUPPORTED},
	{180, "Request rejected, bearer control mode violation", ERR_UNKNOWN},
	{181, "Invalid PTI value", ERR_INVALIDDATA},
	{189, "Semantically incorrect message", ERR_UNKNOWN},
	{190, "Invalid mandatory IE", ERR_UNKNOWN},
	{191, "Message type non existent", ERR_UNKNOWN},
	{192, "Message type not compatible", ERR_UNKNOWN},
	{193, "IE non existent", ERR_UNKNOWN},
	{194, "Conditional IE error", ERR_UNKNOWN},
	{195, "Message not compatible", ERR_UNKNOWN},
	{197, "Protocol error unspecified", ERR_UNKNOWN},
	{203, "Multiple PDN connections for a given APN not allowed", ERR_UNKNOWN},
	{254, "Invalid error mapping", ERR_UNKNOWN},
	{255, "Internal error", ERR_PHONE_INTERNAL},
	{262, "SIM blocked", ERR_SECURITYERROR},
	{300, "ME failure", ERR_PHONE_INTERNAL},
	{301, "SMS service of ME reserved", ERR_UNKNOWN},
	{302, "Operation not allowed", ERR_UNKNOWN},
	{303, "Operation not supported", ERR_NOTSUPPORTED},
	{304, "Invalid PDU mode parameter", ERR_INVALIDDATA},
	{305, "Invalid text mode parameter", ERR_INVALIDDATA},
	{306, "PDN invalid type", ERR_UNKNOWN},
	{307, "PDN no parameter", ERR_UNKNOWN},
	{308, "UE fail", ERR_UNKNOWN},
	{309, "PDP APN and PDP type duplicate used", ERR_UNKNOWN},
	{310, "(U)SIM not inserted", ERR_NOSIM},
	{311, "(U)SIM PIN required", ERR_SECURITYERROR},
	{312, "PH-(U)SIM PIN required", ERR_SECURITYERROR},
	{313, "(U)SIM failure", ERR_NOSIM},
	{314, "(U)SIM busy", ERR_BUSY},
	{315, "(U)SIM wrong", ERR_NOSIM},
	{316, "(U)SIM PUK required", ERR_SECURITYERROR},
	{317, "(U)SIM PIN2 required", ERR_SECURITYERROR},
	{318, "(U)SIM PUK2 required", ERR_SECURITYERROR},
	{320, "Memory failure", ERR_MEMORY},
	{321, "Invalid memory index", ERR_INVALIDLOCATION},
	{322, "Memory full", ERR_FULL},
	{330, "SMSC address unknown", ERR_UNKNOWN},
	{331, "No network service", ERR_NETWORK_ERROR},
	{332, "Network timeout", ERR_NETWORK_ERROR},
	{340, "No +CNMA acknowledgement expected", ERR_UNKNOWN},
	{401, "Invalid PLMN", ERR_UNKNOWN},
	{402, "PLMN duplicate used", ERR_UNKNOWN},
	{403, "PLMN not found", ERR_UNKNOWN},
	{404, "PLMN table empty", ERR_UNKNOWN},
	{405, "PLMN table full", ERR_UNKNOWN},
	{406, "No ESM cause", ERR_UNKNOWN},
	{407, "Unknown ESM cause", ERR_UNKNOWN},
	{408, "Command not successful", ERR_UNKNOWN},
	{500, "Unknown error", ERR_UNKNOWN},
	{608, "Voice call active", ERR_UNKNOWN},
	{701, "Incorrect security code", ERR_SECURITYERROR},
	{702, "Max attempts reached", ERR_SECURITYERROR},
	{1001, "Unassigned (unallocated) number", ERR_UNKNOWN},
	{1003, "No route to destination", ERR_UNKNOWN},
	{1006, "Channel unacceptable", ERR_UNKNOWN},
	{1008, "Operator determined barring", ERR_SECURITYERROR},
	{1016, "Normal call clearing", ERR_UNKNOWN},
	{1017, "User busy", ERR_BUSY},
	{1018, "No user responding", ERR_UNKNOWN},
	{1019, "User alerting, no answer", ERR_UNKNOWN},
	{1021, "Call rejected", ERR_UNKNOWN},
	{1022, "Number changed", ERR_UNKNOWN},
	{1026, "Non selected user clearing", ERR_UNKNOWN},
	{1027, "Destination out of order", ERR_UNKNOWN},
	{1028, "Invalid number format (incomplete number)", ERR_INVALIDDATA},
	{1029, "Facility rejected", ERR_UNKNOWN},
	{1030, "Response to STATUS ENQUIRY", ERR_UNKNOWN},
	{1031, "Normal, unspecified", ERR_UNKNOWN},
	{1034, "No circuit/channel available", ERR_NETWORK_ERROR},
	{1038, "Network out of order", ERR_NETWORK_ERROR},
	{1041, "Temporary failure", ERR_NETWORK_ERROR},
	{1042, "Switching equipment congestion", ERR_NETWORK_ERROR},
	{1043, "Access information discarded", ERR_UNKNOWN},
	{1044, "requested circuit/channel not available", ERR_NETWORK_ERROR},
	{1047, "Resources unavailable, unspecified", ERR_NETWORK_ERROR},
	{1049, "Quality of service unavailable", ERR_UNKNOWN},
	{1050, "Requested facility not subscribed", ERR_UNKNOWN},
	{1055, "Incoming calls barred within the CUG", ERR_UNKNOWN},
	{1056, "Collision with network initiated request", ERR_UNKNOWN},
	{1057, "Bearer capability not authorized", ERR_SECURITYERROR},
	{1058, "Bearer capability not presently available", ERR_UNKNOWN},
	{1059, "Unsupported QCI value", ERR_NOTSUPPORTED},
	{1063, "Service or option not available, unspecified", ERR_UNKNOWN},
	{1065, "Bearer service not implemented", ERR_NOTSUPPORTED},
	{1068, "ACM equal to or greater than ACMmax", ERR_UNKNOWN},
	{1069, "Requested facility not implemented", ERR_NOTSUPPORTED},
	{1070, "Only restricted digital information bearer capability is available", ERR_UNKNOWN},
	{1079, "Service or option not implemented, unspecified", ERR_NOTSUPPORTED},
	{1081, "Invalid transaction identifier value", ERR_INVALIDDATA},
	{1087, "User not member of CUG", ERR_UNKNOWN},
	{1088, "Incompatible destination", ERR_UNKNOWN},
	{1091, "Invalid transit network selection", ERR_UNKNOWN},
	{1095, "Semantically incorrect message", ERR_UNKNOWN},
	{1096, "Invalid mandatory information", ERR_UNKNOWN},
	{1097, "Message type non-existent or not implemented", ERR_UNKNOWN},
	{1098, "Message type not compatible with protocol state", ERR_UNKNOWN},
	{1099, "Information element non-existent or not implemented", ERR_UNKNOWN},
	{1100, "Conditional IE error", ERR_UNKNOWN},
	{1101, "Message not compatible with protocol state", ERR_UNKNOWN},
	{1102, "Recovery on timer expiry", ERR_UNKNOWN},
	{1111, "Protocol error, unspecified", ERR_UNKNOWN},
	{1112, "APN restriction value incompatible with active EPS bearer context", ERR_UNKNOWN},
	{1127, "Interworking, unspecified", ERR_UNKNOWN},
	{1142, "Network Error", ERR_NETWORK_ERROR},
	{1143, "Invalid EPS bearer identity", ERR_UNKNOWN},
	{1149, "Last PDN disconnection not allowed", ERR_UNKNOWN},
	{1243, "Emm Error Unspecified", ERR_UNKNOWN},
	{1244, "Esm Error Unspecified", ERR_UNKNOWN},
	{1279, "Number not allowed", ERR_UNKNOWN},
	{1283, "CCBS possible", ERR_UNKNOWN},
	{1400, "Wrong RAT", ERR_UNKNOWN},
	{1500, "Wrong GPIO identifier", ERR_UNKNOWN},
	{1501, "Set GPIO default error", ERR_UNKNOWN},
	{1502, "Select GPIO mode error", ERR_UNKNOWN},
	{1503, "Read GPIO error", ERR_UNKNOWN},
	{1504, "Write GPIO error", ERR_UNKNOWN},
	{1505, "GPIO busy", ERR_BUSY},
	{1520, "Wrong ADC identifier", ERR_UNKNOWN},
	{1521, "Read ADC error", ERR_UNKNOWN},
	{1530, "IPv4 only allowed", ERR_UNKNOWN},
	{1531, "IPv6 only allowed", ERR_UNKNOWN},
	{1540, "Wrong ringer identifier", ERR_UNKNOWN},
	{1542, "LLC or SNDCP failure", ERR_UNKNOWN},
	{1543, "Regular deactivation", ERR_UNKNOWN},
	{1544, "Reactivation requested", ERR_UNKNOWN},
	{1545, "Single address bearers only allowed", ERR_UNKNOWN},
	{1546, "Invalid transaction identifier value", ERR_INVALIDDATA},
	{1547, "APN restriction val incompatible with PDP context", ERR_UNKNOWN},
	{1548, "PDP activation rejected", ERR_UNKNOWN},
	{1549, "unknown PDP address or PDP type", ERR_UNKNOWN},
	{1550, "GPRS generic operation error", ERR_UNKNOWN},
	{1551, "GPRS invalid APN", ERR_UNKNOWN},
	{1552, "GPRS authentication failure", ERR_SECURITYERROR},
	{1553, "GPRS QoS parameters inconsistent", ERR_UNKNOWN},
	{1554, "GPRS network failure", ERR_NETWORK_ERROR},
	{1555, "GPRS context busy", ERR_BUSY},
	{1556, "CSD generic operation error", ERR_UNKNOWN},
	{1557, "CSD undefined profile", ERR_UNKNOWN},
	{1558, "CSD context busy", ERR_BUSY},
	{1559, "PLMN scan not allowed", ERR_UNKNOWN},
	{1560, "PDP type IPv4 only allowed", ERR_UNKNOWN},
	{1561, "PDP type IPv6 only allowed", ERR_UNKNOWN},
	{1600, "FFS error", ERR_UNKNOWN},
	{1612, "FILE NOT FOUND", ERR_FILENOTEXIST},
	{1613, "Cannot open file", ERR_UNKNOWN},
	{1614, "TAC value not allowed", ERR_UNKNOWN},
	{1615, "OTP failure", ERR_UNKNOWN},
	{1616, "Wrong Check Digit", ERR_UNKNOWN},
	{1620, "Buffer full", ERR_FULL},
	{1621, "FFS initializing", ERR_BUSY},
	{1622, "FFS already open file", ERR_UNKNOWN},
	{1623, "FFS not open file", ERR_UNKNOWN},
	{1624, "FFS file not found", ERR_FILENOTEXIST},
	{1625, "FFS file already created", ERR_UNKNOWN},
	{1626, "FFS illegal id", ERR_UNKNOWN},
	{1627, "FFS illegal file handle", ERR_UNKNOWN},
	{1628, "FFS illegal type", ERR_UNKNOWN},
	{1629, "FFS illegal mode", ERR_UNKNOWN},
	{1630, "FFS file range", ERR_UNKNOWN},
	{1631, "FFS operation not possible", ERR_UNKNOWN},
	{1632, "FFS write error", ERR_UNKNOWN},
	{1633, "FFS user id error", ERR_UNKNOWN},
	{1634, "FFS internal fatal error", ERR_UNKNOWN},
	{1635, "FFS memory resource error", ERR_MEMORY},
	{1636, "FFS maximum number of files exceeded", ERR_FULL},
	{1637, "FFS memory not available", ERR_MOREMEMORY},
	{1638, "FFS invalid filename", ERR_UNKNOWN},
	{1639, "FFS streaming not enabled", ERR_UNKNOWN},
	{1640, "FFS operation not allowed on static file", ERR_UNKNOWN},
	{1641, "FFS memory table inconsistency", ERR_UNKNOWN},
	{1642, "FFS not a factory default file", ERR_UNKNOWN},
	{1643, "FFS requested memory temporary not available", ERR_UNKNOWN},
	{1644, "FFS operation not allowed for a directory", ERR_UNKNOWN},
	{1645, "FFS directory space not available", ERR_UNKNOWN},
	{1646, "FFS too many streaming files open", ERR_UNKNOWN},
	{1647, "FFS requested dynamic memory temporary not available", ERR_UNKNOWN},
	{1648, "FFS user provided a NULL parameter instead of a suitable buffer", ERR_UNKNOWN},
	{1649, "FFS timeout", ERR_UNKNOWN},
	{1650, "Command line too long", ERR_UNKNOWN},
	{1660, "Call barred - Fixed dialing numbers only", ERR_SECURITYERROR},
	{1670, "SEC remote object wrong state", ERR_UNKNOWN},
	{1671, "SEC ROT not personalized", ERR_UNKNOWN},
	{1672, "SEC loss of connectivity", ERR_UNKNOWN},
	{1673, "SEC service not authorized", ERR_SECURITYERROR},
	{1674, "SEC FW package installation required", ERR_UNKNOWN},
	{1675, "SEC FW package not valid", ERR_UNKNOWN},
	{1676, "SEC resource not available", ERR_UNKNOWN},
	{1677, "SEC data not available", ERR_UNKNOWN},
	{1678, "SEC timeout", ERR_UNKNOWN},
	{1679, "SEC data inconsistent or unsupported", ERR_NOTSUPPORTED},
	{1680, "SEC pspk lock pending", ERR_UNKNOWN},
	{1681, "SEC C2C already paired", ERR_UNKNOWN},
	{1682, "SEC C2C channels consumed", ERR_UNKNOWN},
	{1683, "SEC C2C pairing not present", ERR_UNKNOWN},
	{1684, "SEC busy", ERR_BUSY},
	{1685, "SEC connection failed due to a DNS resolution error", ERR_NETWORK_ERROR},
	{1686, "SEC restore pending", ERR_UNKNOWN},
	{1687, "SEC RoT IO error", ERR_UNKNOWN},
	{1688, "SEC RoT IO pending", ERR_UNKNOWN},
	{1689, "SEC disabled", ERR_UNKNOWN},
	{1700, "GPS GPIO not configured", ERR_UNKNOWN},
	{1701, "GPS GPIO ownership error", ERR_UNKNOWN},
	{1702, "Invalid operation with GPS ON", ERR_UNKNOWN},
	{1703, "Invalid operation with GPS OFF", ERR_UNKNOWN},
	{1704, "Invalid GPS aiding mode", ERR_UNKNOWN},
	{1705, "Reserved GPS aiding mode", ERR_UNKNOWN},
	{1706, "GPS aiding mode already set", ERR_UNKNOWN},
	{1707, "Invalid GPS trace mode", ERR_UNKNOWN},
	{1708, "Parameter valid only in case of GPS OTA", ERR_UNKNOWN},
	{1709, "GPS trace invalid server", ERR_UNKNOWN},
	{1710, "Invalid TimeZone", ERR_UNKNOWN},
	{1711, "Invalid value", ERR_INVALIDDATA},
	{1712, "Invalid parameter", ERR_INVALIDDATA},
	{1713, "Invalid operation with LOC running / GPS Busy", ERR_BUSY},
	{1800, "No ongoing call", ERR_UNKNOWN},
	{1801, "IBM busy / eCall already armed/active", ERR_BUSY},
	{1802, "IBM feature off / eCall feature off", ERR_UNKNOWN},
	{1803, "Wrong IBM requested", ERR_UNKNOWN},
	{1804, "Audio resource not available", ERR_UNKNOWN},
	{1805, "ECALL restriction", ERR_UNKNOWN},
	{1806, "eCall invalid dial number", ERR_UNKNOWN},
	{1900, "No SAP Server Connection", ERR_UNKNOWN},
	{1901, "SAP Protocol Error", ERR_UNKNOWN},
	{1902, "SAP Connection failure", ERR_UNKNOWN},
	{1903, "SAP Server Disconnection", ERR_UNKNOWN},
	{1904, "SAP Other terminal using service", ERR_UNKNOWN},
	{1910, "USECMNG import timeout expired (no input for > 20 s)", ERR_UNKNOWN},
	{1911, "USECMNG import file size exceeds limit", ERR_UNKNOWN},
	{1912, "USECMNG no memory available", ERR_MOREMEMORY},
	{1913, "USECMNG invalid certificate/key format", ERR_INVALIDDATA},
	{1914, "USECMNG database full", ERR_FULL},
	{1915, "USECMNG database internal error", ERR_UNKNOWN},
	{1916, "USECMNG internal name not found", ERR_UNKNOWN},
	{1917, "USECMNG internal name already present", ERR_UNKNOWN},
	{1918, "USECMNG private key format error", ERR_UNKNOWN},
	{1919, "USECMNG MD5 checksum error", ERR_UNKNOWN},
	{1920, "USECMNG PEM DER conversion error", ERR_UNKNOWN},
	{1921, "USECMNG certificate date error", ERR_UNKNOWN},
	{1922, "USECMNG certificate common name error", ERR_UNKNOWN},
	{1950, "CDC-ECM is not available", ERR_UNKNOWN},
	{1951, "CDC-ECM is busy", ERR_BUSY},
	{1952, "No DHCP Packets received from the DTE", ERR_UNKNOWN},
	{2000, "Command timeout", ERR_TIMEOUT},
	{3000, "Command aborted", ERR_CANCELED},
	{4000, "APN configuration mismatch", ERR_UNKNOWN},
	{4001, "IP type configuration mismatch", ERR_UNKNOWN},
	{5000, "FOTA package download state or name mismatch", ERR_UNKNOWN},
	{5001, "FOTA package data corrupted", ERR_UNKNOWN},
	{5002, "FOTA memory is in use", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* u-blox LENA-R8 UBX-22016905 R04, appendix A.2 (first meaning for duplicate codes) */
static const ATErrorCode ublox_cms[] = {
	{5, "Delta firmware unavailable on FOTA server", ERR_UNKNOWN},
	{17, "Network failure", ERR_NETWORK_ERROR},
	{22, "Memory capacity exceeded", ERR_FULL},
	{213, "SIM data download error", ERR_UNKNOWN},
	{287, "Network failure unspecified", ERR_NETWORK_ERROR},
	{290, "Network no resource", ERR_NETWORK_ERROR},
	{296, "Radio Resources not Available due to DUAL SIM operation", ERR_UNKNOWN},
	{297, "Out of service due to DUAL SIM operation", ERR_NETWORK_ERROR},
	{350, "Unassigned (unallocated) number", ERR_UNKNOWN},
	{351, "Operator determined barring", ERR_SECURITYERROR},
	{352, "Call barred", ERR_SECURITYERROR},
	{353, "ME failure", ERR_UNKNOWN},
	{354, "Short message transfer rejected", ERR_UNKNOWN},
	{355, "Number changed", ERR_UNKNOWN},
	{356, "Destination out of order", ERR_UNKNOWN},
	{357, "Unidentified subscriber", ERR_UNKNOWN},
	{358, "Facility rejected", ERR_UNKNOWN},
	{359, "Unknown subscriber", ERR_UNKNOWN},
	{360, "Network out of order", ERR_NETWORK_ERROR},
	{361, "Temporary failure", ERR_NETWORK_ERROR},
	{362, "Congestion", ERR_NETWORK_ERROR},
	{363, "Resources unavailable, unspecified", ERR_NETWORK_ERROR},
	{364, "Requested facility not subscribed", ERR_UNKNOWN},
	{365, "Requested facility not implemented", ERR_NOTSUPPORTED},
	{366, "Invalid short message transfer reference value", ERR_INVALIDDATA},
	{367, "Invalid message, unspecified", ERR_UNKNOWN},
	{368, "Invalid mandatory information", ERR_UNKNOWN},
	{369, "Message type non-existent or not implemented", ERR_UNKNOWN},
	{370, "Message not compatible with short message protocol state", ERR_UNKNOWN},
	{371, "Information element non-existent or not implemented", ERR_UNKNOWN},
	{372, "Protocol error, unspecified", ERR_UNKNOWN},
	{373, "Interworking, unspecified", ERR_UNKNOWN},
	{512, "Relay Protocol Acknowledgement", ERR_UNKNOWN},
	{513, "SMS timer expired", ERR_TIMEOUT},
	{514, "SMS forwarding availability failed", ERR_UNKNOWN},
	{515, "SMS forwarding availability aborted", ERR_UNKNOWN},
	{516, "MS invalid TP-Message-Type-Indicator", ERR_UNKNOWN},
	{517, "MS no TP-Status-Report in Phase 1", ERR_NOTSUPPORTED},
	{518, "MS no TP-Reject-Duplicate in phase 1", ERR_NOTSUPPORTED},
	{519, "MS no TP-Replay-Path in Phase 1", ERR_NOTSUPPORTED},
	{520, "MS no TP-User-Data-Header in Phase 1", ERR_NOTSUPPORTED},
	{521, "MS missing TP-Validity-Period", ERR_INVALIDDATA},
	{522, "MS invalid TP-Service-Centre-Time-Stamp", ERR_INVALIDDATA},
	{523, "MS missing TP-Destination-Address", ERR_INVALIDDATA},
	{524, "MS invalid TP-Destination-Address", ERR_INVALIDDATA},
	{525, "MS missing Service-Centre-Address", ERR_INVALIDDATA},
	{526, "MS invalid Service-Centre-Address", ERR_INVALIDDATA},
	{527, "MS invalid alphabet", ERR_INVALIDDATA},
	{528, "MS invalid TP-User-Data-length", ERR_INVALIDDATA},
	{529, "MS missing TP-User-Data", ERR_INVALIDDATA},
	{530, "MS TP-User-Data to long", ERR_INVALIDDATA},
	{531, "MS no Command-Request in Phase 1", ERR_NOTSUPPORTED},
	{532, "MS Cmd-Req invalid TP-Destination-Address", ERR_INVALIDDATA},
	{533, "MS Cmd-Req invalid TP-User-Data-Length", ERR_INVALIDDATA},
	{534, "MS Cmd-Req invalid TP-User-Data", ERR_INVALIDDATA},
	{535, "MS Cmd-Req invalid TP-Command-Type", ERR_INVALIDDATA},
	{536, "MN MNR creation failed", ERR_UNKNOWN},
	{537, "MS CMM creation failed", ERR_UNKNOWN},
	{538, "MS network connection lost", ERR_NETWORK_ERROR},
	{539, "MS pending MO SM transfer", ERR_BUSY},
	{540, "RP-Error OK", ERR_UNKNOWN},
	{541, "RP-Error OK no icon display", ERR_UNKNOWN},
	{542, "SMS-PP Unspecified", ERR_UNKNOWN},
	{543, "SMS rejected By SMS CONTROL", ERR_SECURITYERROR},
	{544, "Service Centre Address(SCA) FDN failed", ERR_SECURITYERROR},
	{545, "Destination Address(DA) FDN failed", ERR_SECURITYERROR},
	{546, "BDN check failed", ERR_SECURITYERROR},
	{547, "Unspecified SMS PP error", ERR_UNKNOWN},
	{548, "Undefined Result", ERR_UNKNOWN},
	{549, "Channel Unacceptable", ERR_UNKNOWN},
	{555, "No Circuit/Channel Available", ERR_UNKNOWN},
	{556, "Access Information Discarded", ERR_UNKNOWN},
	{557, "Requested Circuit/Channel Not Available By Other Side", ERR_UNKNOWN},
	{558, "Quality Of Service Unavailable", ERR_UNKNOWN},
	{560, "Bearer Capability Not Authorized", ERR_SECURITYERROR},
	{561, "Bearer Capability Not Presently Available", ERR_UNKNOWN},
	{562, "Service or Option Not Available, Unspecified", ERR_UNKNOWN},
	{563, "Bearer Service Not Implemented", ERR_NOTSUPPORTED},
	{564, "ACM Equal to or Greater Than ACMmax", ERR_UNKNOWN},
	{565, "Only Restricted Digital Information Bearer Capability Is Available", ERR_UNKNOWN},
	{566, "Service or Option Not Implemented, Unspecified", ERR_NOTSUPPORTED},
	{567, "User Not Member of CUG", ERR_UNKNOWN},
	{568, "Incompatible By Destination", ERR_UNKNOWN},
	{569, "Invalid Transit Network Selection", ERR_UNKNOWN},
	{571, "Message Not Compatible With Protocol State", ERR_UNKNOWN},
	{572, "Recovery On Timer Expiry", ERR_UNKNOWN},
	{576, "Data Call Active", ERR_BUSY},
	{577, "Speech Call Active", ERR_BUSY},
	{579, "MOC Setup Rejected Due to Missing ACM Info", ERR_UNKNOWN},
	{580, "Temporary Forbidden Call Attempt", ERR_UNKNOWN},
	{581, "Called Party is Blacklisted", ERR_SECURITYERROR},
	{583, "Temporary Forbidden Call Attempt No Service", ERR_UNKNOWN},
	{584, "Temporary Forbidden Call Attempt Limited Service", ERR_UNKNOWN},
	{585, "Client Temporary Barred", ERR_UNKNOWN},
	{586, "Dual Service Call Active", ERR_BUSY},
	{587, "Atc Fclass Not Speech", ERR_UNKNOWN},
	{590, "Client Not Registrated", ERR_UNKNOWN},
	{591, "Active Client Gone", ERR_UNKNOWN},
	{595, "Rejected By Call Control", ERR_UNKNOWN},
	{601, "Invalid ALS Line", ERR_UNKNOWN},
	{604, "MM No Service (out of coverage)", ERR_NETWORK_ERROR},
	{605, "MM Access Class Barred (RR_REL_IND During RR Conn. Establishment)", ERR_SECURITYERROR},
	{606, "ME Busy -CM Service Request Already Pending", ERR_BUSY},
	{608, "Rejected Due To SUP Timer Expiry", ERR_TIMEOUT},
	{609, "Rejected Due To USSD Busy", ERR_BUSY},
	{610, "Rejected Due To SS Busy", ERR_BUSY},
	{612, "SIM Toolkit Request Is Rejected, Because Another SIM Toolkit Request Is Pending", ERR_BUSY},
	{614, "Rejected Because SIM Toolkit Request Is Not Yet Answered By The User", ERR_BUSY},
	{615, "MN Setup SS Error", ERR_UNKNOWN},
	{616, "Call Controller Blocked (Other Call Command Pending)", ERR_BUSY},
	{618, "Environment Parameter Not Set Correctly (Fclass/Cmod)", ERR_UNKNOWN},
	{619, "Other Blocking Call Present", ERR_BUSY},
	{620, "Lower Layer Failure", ERR_NETWORK_ERROR},
	{621, "The Authentication Procedure Failed", ERR_SECURITYERROR},
	{622, "The Packet-Switched Registration Procedure Failed", ERR_NETWORK_ERROR},
	{623, "CM Service Reject From The Network", ERR_NETWORK_ERROR},
	{624, "The ABORT Message Was Received From The Network", ERR_UNKNOWN},
	{625, "Timer Expiry", ERR_TIMEOUT},
	{626, "IMSI Detach Was Initiated", ERR_UNKNOWN},
	{627, "Normal RR Connection Release (2G)", ERR_UNKNOWN},
	{628, "Registration Failed", ERR_NETWORK_ERROR},
	{630, "Failure Due To Handover", ERR_NETWORK_ERROR},
	{631, "Link Establishment Failure", ERR_NETWORK_ERROR},
	{632, "Random Access Failure", ERR_NETWORK_ERROR},
	{633, "Radio Link Aborted", ERR_UNKNOWN},
	{634, "Lower Layer Failure in Layer 1", ERR_NETWORK_ERROR},
	{635, "Immediate Assignment Reject", ERR_UNKNOWN},
	{636, "Failure Due To Paging", ERR_UNKNOWN},
	{637, "Abnormal Release Unspecified", ERR_UNKNOWN},
	{638, "Abnormal Release Channel Unacceptable", ERR_NETWORK_ERROR},
	{639, "Abnormal Release Timer Expired", ERR_TIMEOUT},
	{640, "Abnormal Release No Act On Radio Path", ERR_NETWORK_ERROR},
	{641, "Preemptive Release", ERR_UNKNOWN},
	{642, "UTRAN Configuration Unknown", ERR_UNKNOWN},
	{643, "Handover Impossible", ERR_NETWORK_ERROR},
	{644, "Channel Mode Unacceptable", ERR_UNKNOWN},
	{647, "Lower Layer Failure From NW", ERR_NETWORK_ERROR},
	{649, "Conditional IE Error", ERR_UNKNOWN},
	{650, "No Cell Allocation Available", ERR_NETWORK_ERROR},
	{653, "Re Establishment Reject", ERR_NETWORK_ERROR},
	{654, "Directed Sigconn Re Establishment", ERR_UNKNOWN},
	{656, "Release of RRC connection Without Network Activity(3G) Lower Layer Failure Downlink", ERR_NETWORK_ERROR},
	{657, "Lower Layer Failure Uplink", ERR_NETWORK_ERROR},
	{658, "Cell Barred Due To Authentication Failure", ERR_SECURITYERROR},
	{659, "Signalling Connection Release", ERR_UNKNOWN},
	{660, "CS Connection Release Triggered By MM", ERR_UNKNOWN},
	{661, "RRC Connection Establishment Failure", ERR_NETWORK_ERROR},
	{662, "RRC Connection Establishment Reject With Redirection", ERR_NETWORK_ERROR},
	{663, "Resource Conflict", ERR_BUSY},
	{664, "Lower Layer Failure in Layer 2", ERR_NETWORK_ERROR},
	{665, "L2 Cause T200 Expiry N200 Plus 1 Times", ERR_UNKNOWN},
	{669, "RR Connection Release Due to BAND Change (2G)", ERR_UNKNOWN},
	{670, "Release of the RRC Connection Due to Out of Service in Cell_Fach (3G)", ERR_NETWORK_ERROR},
	{671, "Release of the RRC Connection Due to Not Matching PLMN in Shared Networks(3G)", ERR_NETWORK_ERROR},
	{672, "Error Happens While Call Is Already Disconnected / Late Error", ERR_UNKNOWN},
	{674, "SIM Toolkit Cannot Initiate A Call, Because MMI Is Not Registered", ERR_UNKNOWN},
	{675, "SIM Toolkit Call Setup Request Is Rejected Due User Did Not Accept", ERR_UNKNOWN},
	{676, "Proactive SIM Appl Terminated By User", ERR_UNKNOWN},
	{677, "SIM Toolkit Originated SIM Reset (Refresh Request)", ERR_UNKNOWN},
	{680, "Dial String/Number Incorrect", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Wavecom AT commands interface, section 16.3 */
static const ATErrorCode wavecom_cms[] = {
	{512, "MM establishment failure (for SMS)", ERR_NETWORK_ERROR},
	{513, "Lower layer failure (for SMS)", ERR_NETWORK_ERROR},
	{514, "CP error (for SMS)", ERR_NETWORK_ERROR},
	{515, "Please wait, init or command processing in progress", ERR_BUSY},
	{0, NULL, ERR_UNKNOWN}
};

/* Wavecom AT commands interface, section 16.3 */
static const ATErrorCode wavecom_cme[] = {
	{515, "Please wait, init or command processing in progress", ERR_BUSY},
	{516, "USSD error", ERR_UNKNOWN},
	{0, NULL, ERR_UNKNOWN}
};

/* Existing Gammu Samsung empty-phonebook-location workaround */
static const ATErrorCode samsung_cme[] = {
	{-1, "[Samsung] Empty location", ERR_EMPTY},
	{0, NULL, ERR_UNKNOWN}
};

/* Existing Gammu Sony Ericsson P1i phonebook workaround */
static const ATErrorCode ericsson_cme[] = {
	{601, "Operation not supported", ERR_NOTSUPPORTED},
	{0, NULL, ERR_UNKNOWN}
};

typedef struct {
	GSM_AT_Manufacturer manufacturer;
	const ATErrorCode *cme;
	const ATErrorCode *cms;
} ATErrorVendor;

static const ATErrorVendor vendors[] = {
	{AT_Ericsson, ericsson_cme, NULL},
	{AT_Falcom, falcom_cme, NULL},
	{AT_Huawei, huawei_cme, NULL},
	{AT_ITegno, itegno_cme, itegno_cms},
	{AT_Motorola, motorola_cme, motorola_cms},
	{AT_Quectel, quectel_cme, quectel_cms},
	{AT_SIMCom, simcom_cme, simcom_cms},
	{AT_Samsung, samsung_cme, NULL},
	{AT_Siemens, siemens_cme, siemens_cms},
	{AT_Sierra, sierra_cme, sierra_cms},
	{AT_Telit, telit_cme, telit_cms},
	{AT_UBlox, ublox_cme, ublox_cms},
	{AT_Wavecom, wavecom_cme, wavecom_cms},
};

static const ATErrorCode *vendor_table(const GSM_Phone_ATGENData *priv)
{
	size_t i;
	for (i = 0; i < sizeof(vendors) / sizeof(vendors[0]); i++) {
		if (vendors[i].manufacturer == priv->Manufacturer) {
			return priv->ReplyState == AT_Reply_CMEError ? vendors[i].cme : vendors[i].cms;
		}
	}
	return NULL;
}

static const ATErrorCode *standard_table(const GSM_Phone_ATGENData *priv)
{
	return priv->ReplyState == AT_Reply_CMEError ? standard_cme : standard_cms;
}

static const ATErrorCode *find_code(const ATErrorCode *table, int code)
{
	if (table != NULL) {
		for (; table->text != NULL; table++) {
			if (table->code == code) {
				return table;
			}
		}
	}
	return NULL;
}

static const ATErrorCode *lookup(const GSM_Phone_ATGENData *priv, int code)
{
	const ATErrorCode *entry = find_code(vendor_table(priv), code);
	return entry != NULL ? entry : find_code(standard_table(priv), code);
}

/* Complete, case-insensitive comparison, with surrounding whitespace removed
 * by the caller. Do not accept a known description as a prefix of another. */
static gboolean text_equal(const char *text, size_t length, const char *expected)
{
	size_t i;
	if (strlen(expected) != length) {
		return FALSE;
	}
	for (i = 0; i < length; i++) {
		if (tolower((unsigned char)text[i]) != tolower((unsigned char)expected[i])) {
			return FALSE;
		}
	}
	return TRUE;
}

void ATGEN_ParseError(GSM_Phone_ATGENData *priv, const char *text)
{
	const ATErrorCode *entry = NULL, *table;
	const ATErrorCode *vendor = vendor_table(priv);
	char *end;
	long code;
	size_t length;
	int pass;

	priv->ErrorCode = -1;
	priv->ErrorCodeValid = FALSE;
	priv->ErrorText = NULL;
	while (isspace((unsigned char)*text)) {
		text++;
	}
	length = strlen(text);
	while (length > 0 && isspace((unsigned char)text[length - 1])) {
		length--;
	}
	if (length == 0) {
		return;
	}

	if (isdigit((unsigned char)*text) || *text == '-' || *text == '+') {
		errno = 0;
		code = strtol(text, &end, 10);
		if (end == text || errno == ERANGE || code < INT_MIN || code > INT_MAX ||
			(size_t)(end - text) != length) {
			return;
		}
		priv->ErrorCode = (int)code;
		priv->ErrorCodeValid = TRUE;
		entry = lookup(priv, priv->ErrorCode);
	} else {
		for (pass = 0; pass < 2 && entry == NULL; pass++) {
			table = pass == 0 ? vendor : standard_table(priv);
			if (table == NULL) {
				continue;
			}
			for (; table->text != NULL; table++) {
				/* A standard description must not resurrect a vendor-overridden code. */
				if (pass == 1 && find_code(vendor, table->code) != NULL) {
					continue;
				}
				if (text_equal(text, length, table->text)) {
					entry = table;
					priv->ErrorCode = entry->code;
					priv->ErrorCodeValid = TRUE;
					break;
				}
			}
		}
	}
	if (entry != NULL) {
		priv->ErrorText = entry->text;
	}
}

GSM_Error ATGEN_ErrorResult(const GSM_Phone_ATGENData *priv)
{
	const ATErrorCode *entry;
	if (!priv->ErrorCodeValid) {
		return ERR_UNKNOWN;
	}
	entry = lookup(priv, priv->ErrorCode);
	return entry != NULL ? entry->error : ERR_UNKNOWN;
}
#endif
