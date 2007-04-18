/* SMS related conversions */

#include "convertors.h"

char *SMSValidityToString(GSM_SMSValidity Validity) {
    char *s;

    s = malloc(4);
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }
    s[0] = 0;
    s[3] = 0;
    switch (Validity.Format) {
        case SMS_Validity_NotAvailable:
            s[0] = 'N'; s[1] = 'A'; s[2] = 0;
            return s;
        case SMS_Validity_RelativeFormat:
            switch (Validity.Relative) {
                case SMS_VALID_1_Hour: s[0] = '0'; s[1] = '1'; s[2] = 'H'; return s;
                case SMS_VALID_6_Hours: s[0] = '0'; s[1] = '6'; s[2] = 'H'; return s;
                case SMS_VALID_1_Day: s[0] = '0'; s[1] = '1'; s[2] = 'D'; return s;
                case SMS_VALID_3_Days: s[0] = '0'; s[1] = '3'; s[2] = 'D'; return s;
                case SMS_VALID_1_Week: s[0] = '0'; s[1] = '1'; s[2] = 'W'; return s;
                case SMS_VALID_Max_Time: s[0] = 'M'; s[1] = 'a'; s[2] = 'x'; return s;
            }
            PyErr_Format(PyExc_ValueError, "Bad value for RelativeValidity from Gammu: '%d'", Validity.Relative);
            free(s);
            return NULL;
    }
    PyErr_Format(PyExc_ValueError, "Bad value for ValidityPeriodFormat from Gammu: '%d'", Validity.Format);
    free(s);
    return NULL;
}

GSM_SMSValidity StringToSMSValidity(char *s){
    GSM_SMSValidity Validity = {0,0};
    int             i;
    
    if (strcmp(s, "NA") == 0) {
        Validity.Format = SMS_Validity_NotAvailable;
    } else if (strlen(s) == 3) {
        Validity.Format = SMS_Validity_RelativeFormat;
        if (strcmp(s, "Max") == 0) {
            Validity.Relative = SMS_VALID_Max_Time;
        } else if (s[2] == 'D') {
            i = atoi(s);
            switch (i) {
                case 1:
                    Validity.Relative = SMS_VALID_1_Day;
                    break;
                case 3:
                    Validity.Relative = SMS_VALID_3_Days;
                    break;
                default:
                    PyErr_Format(PyExc_ValueError, "Bad relative validity in days: '%d'", i);
                    Validity.Format = 0;
            }
        } else if (s[2] == 'H') {
            i = atoi(s);
            switch (i) {
                case 1:
                    Validity.Relative = SMS_VALID_1_Hour;
                    break;
                case 6:
                    Validity.Relative = SMS_VALID_6_Hours;
                    break;
                case 24:
                    Validity.Relative = SMS_VALID_1_Day;
                    break;
                case 72:
                    Validity.Relative = SMS_VALID_3_Days;
                    break;
                default:
                    PyErr_Format(PyExc_ValueError, "Bad relative validity in hours: '%d'", i);
                    Validity.Format = 0;
            }
        } else if (s[2] == 'W') {
            i = atoi(s);
            switch (i) {
                case 1:
                    Validity.Relative = SMS_VALID_1_Week;
                    break;
                default:
                    PyErr_Format(PyExc_ValueError, "Bad relative validity in weeks: '%d'", i);
                    Validity.Format = 0;
            }
        }
    }
      
    return Validity;
}

char *SMSFormatToString(GSM_SMSFormat f) {
    char *err = "Err";
    char *s = err;

    switch (f) {
        case SMS_FORMAT_Pager: s = strdup("Pager"); break;
        case SMS_FORMAT_Fax: s = strdup("Fax"); break;
        case SMS_FORMAT_Email: s = strdup("Email"); break;
        case SMS_FORMAT_Text: s = strdup("Text"); break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for SMSFormat from Gammu: '%d'", f);
        return NULL;
    }
    
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_SMSFormat StringToSMSFormat(char *s){
    if (strcmp("Text", s) == 0) return SMS_FORMAT_Text;
    else if (strcmp("Pager", s) == 0) return SMS_FORMAT_Pager;
    else if (strcmp("Fax", s) == 0) return SMS_FORMAT_Fax;
    else if (strcmp("Email", s) == 0) return SMS_FORMAT_Email;
    else {
        PyErr_Format(PyExc_MemoryError, "Bad value for SMS Format '%s'", s);
        return 0;
    }
}
   

PyObject *SMSCToPython(GSM_SMSC *smsc) {
    PyObject            *ret;
    Py_UNICODE          *name, *number, *defaultn;
    char                *val, *fmt;

    name = strGammuToPython(smsc->Name);
    if (name  == NULL) return NULL;

    number = strGammuToPython(smsc->Number);
    if (number == NULL) return NULL;

    defaultn = strGammuToPython(smsc->DefaultNumber);
    if (defaultn == NULL) return NULL;

    val = SMSValidityToString(smsc->Validity);
    if (val == NULL) return NULL;

    fmt = SMSFormatToString(smsc->Format);
    if (fmt == NULL) return NULL;
    
    ret = Py_BuildValue("{s:i,s:u,s:s,s:s,s:u,s:u}",
            "Location",         smsc->Location,
            "Name",             name,
            "Format",           fmt,
            "Validity",         val,
            "Number",           number,
            "DefaultNumber",    defaultn);

    free(val);
    free(fmt);
    free(name);
    free(number);
    free(defaultn);

    return ret;
}

int SMSCFromPython(PyObject* dict, GSM_SMSC *smsc) {
    char    *s;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "SMSC is not a dictionary");
        return 0;
    }

    memset(smsc, 0, sizeof(smsc));
    
    smsc->Location = GetIntFromDict(dict, "Location");
    if (smsc->Location == INT_INVALID) return 0;

    if (!CopyStringFromDict(dict, "Number", GSM_MAX_NUMBER_LENGTH, smsc->Number))
        return 0;
    if (!CopyStringFromDict(dict, "Name", GSM_MAX_SMSC_NAME_LENGTH, smsc->Name))
        return 0;
    if (!CopyStringFromDict(dict, "DefaultNumber", GSM_MAX_NUMBER_LENGTH, smsc->DefaultNumber))
        return 0;

    s = GetCharFromDict(dict, "Format");
    if (s == NULL) return 0;
    smsc->Format = StringToSMSFormat(s);
    if (smsc->Format == 0) return 0;
    
    s = GetCharFromDict(dict, "Validity");
    if (s == NULL) return 0;
    smsc->Validity = StringToSMSValidity(s);
    if (smsc->Validity.Format == 0) return 0;

    return 1;
}

GSM_UDH StringToUDHType(const char *s) {
	if (strcmp("NoUDH", s) == 0) return UDH_NoUDH;
	else if (strcmp("ConcatenatedMessages", s) == 0) return UDH_ConcatenatedMessages;
	else if (strcmp("ConcatenatedMessages16bit", s) == 0) return UDH_ConcatenatedMessages16bit;
	else if (strcmp("DisableVoice", s) == 0) return UDH_DisableVoice;
	else if (strcmp("DisableFax", s) == 0) return UDH_DisableFax;
	else if (strcmp("DisableEmail", s) == 0) return UDH_DisableEmail;
	else if (strcmp("EnableVoice", s) == 0) return UDH_EnableVoice;
	else if (strcmp("EnableFax", s) == 0) return UDH_EnableFax;
	else if (strcmp("EnableEmail", s) == 0) return UDH_EnableEmail;
	else if (strcmp("VoidSMS", s) == 0) return UDH_VoidSMS;
	else if (strcmp("NokiaRingtone", s) == 0) return UDH_NokiaRingtone;
	else if (strcmp("NokiaRingtoneLong", s) == 0) return UDH_NokiaRingtoneLong;
	else if (strcmp("NokiaOperatorLogo", s) == 0) return UDH_NokiaOperatorLogo;
	else if (strcmp("NokiaOperatorLogoLong", s) == 0) return UDH_NokiaOperatorLogoLong;
	else if (strcmp("NokiaCallerLogo", s) == 0) return UDH_NokiaCallerLogo;
	else if (strcmp("NokiaWAP", s) == 0) return UDH_NokiaWAP;
	else if (strcmp("NokiaWAPLong", s) == 0) return UDH_NokiaWAPLong;
	else if (strcmp("NokiaCalendarLong", s) == 0) return UDH_NokiaCalendarLong;
	else if (strcmp("NokiaProfileLong", s) == 0) return UDH_NokiaProfileLong;
	else if (strcmp("NokiaPhonebookLong", s) == 0) return UDH_NokiaPhonebookLong;
	else if (strcmp("UserUDH", s) == 0) return UDH_UserUDH;
	else if (strcmp("MMSIndicatorLong", s) == 0) return UDH_MMSIndicatorLong;
    
    PyErr_Format(PyExc_ValueError, "Bad value for UDH Type '%s'", s);
    return 0;
}

char *UDHTypeToString(GSM_UDH type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case UDH_NoUDH: 
            s = strdup("NoUDH");
            break;
        case UDH_ConcatenatedMessages:
            s = strdup("ConcatenatedMessages");
            break;
        case UDH_ConcatenatedMessages16bit:
            s = strdup("ConcatenatedMessages16bit");
            break;
        case UDH_DisableVoice: 
            s = strdup("DisableVoice");
            break;
        case UDH_DisableFax:
            s = strdup("DisableFax");
            break;
        case UDH_DisableEmail:
            s = strdup("DisableEmail");
            break;
        case UDH_EnableVoice:
            s = strdup("EnableVoice");
            break;
        case UDH_EnableFax:
            s = strdup("EnableFax");
            break;
        case UDH_EnableEmail:
            s = strdup("EnableEmail");
            break;
        case UDH_VoidSMS:
            s = strdup("VoidSMS");
            break;
        case UDH_NokiaRingtone:
            s = strdup("NokiaRingtone");
            break;
        case UDH_NokiaRingtoneLong:
            s = strdup("NokiaRingtoneLong");
            break;
        case UDH_NokiaOperatorLogo:
            s = strdup("NokiaOperatorLogo");
            break;
        case UDH_NokiaOperatorLogoLong:
            s = strdup("NokiaOperatorLogoLong");
            break;
        case UDH_NokiaCallerLogo:
            s = strdup("NokiaCallerLogo");
            break;
        case UDH_NokiaWAP:
            s = strdup("NokiaWAP");
            break;
        case UDH_NokiaWAPLong:
            s = strdup("NokiaWAPLong");
            break;
        case UDH_NokiaCalendarLong:
            s = strdup("NokiaCalendarLong");
            break;
        case UDH_NokiaProfileLong:
            s = strdup("NokiaProfileLong");
            break;
        case UDH_NokiaPhonebookLong:
            s = strdup("NokiaPhonebookLong");
            break;
        case UDH_UserUDH:
            s = strdup("UserUDH");
            break;
        case UDH_MMSIndicatorLong:
            s = strdup("MMSIndicatorLong");
            break;
    }
    
    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for UDHType from Gammu: '%d'", type);
        return NULL;
    }
    
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_SMSMessageType StringToSMSType(const char *s) {
	if (strcmp("Deliver", s) == 0) return SMS_Deliver;
	else if (strcmp("Status_Report", s) == 0) return SMS_Status_Report;
	else if (strcmp("Submit", s) == 0) return SMS_Submit;

    PyErr_Format(PyExc_ValueError, "Bad value for SMSType: '%s'", s);
    return 0;
}

char *SMSTypeToString(GSM_SMSMessageType type) {
    char *err = "Err";
    char *s = err;
    
    switch (type) {
        case SMS_Deliver: 
            s = strdup("Deliver");
            break;
        case SMS_Status_Report: 
            s = strdup("Status_Report");
            break;
        case SMS_Submit: 
            s = strdup("Submit");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for SMSMessageType from Gammu: '%d'", type);
        return NULL;
    }
    
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

GSM_Coding_Type StringToSMSCoding(const char *s) {
	if (strcmp("Unicode", s) == 0) return SMS_Coding_Unicode;
	else if (strcmp("Default", s) == 0) return SMS_Coding_Default;
	else if (strcmp("8bit", s) == 0) return SMS_Coding_8bit;
    
    PyErr_Format(PyExc_ValueError, "Bad value for SMSCoding: '%s'", s);
    return 0;
}

char *SMSCodingToString(GSM_Coding_Type type) {
    char *err = "Err";
    char *s = NULL;
    
    switch (type) {
        case SMS_Coding_Unicode: 
            s = strdup("Unicode");
            break;
        case SMS_Coding_Default: 
            s = strdup("Default");
            break;
        case SMS_Coding_8bit: 
            s = strdup("8bit");
            break;
    }
    
    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for Coding_Type from Gammu: '%d'", type);
        return NULL;
    }

    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}
GSM_SMS_State StringToSMSState(const char *s){
	if (strcmp("Sent", s) == 0) return SMS_Sent;
	else if (strcmp("UnSent", s) == 0) return SMS_UnSent;
	else if (strcmp("Read", s) == 0) return SMS_Read;
	else if (strcmp("UnRead", s) == 0) return SMS_UnRead;
    
    PyErr_Format(PyExc_ValueError, "Bad value for SMS_State: '%s'", s);
    return 0;
}

char *SMSStateToString(GSM_SMS_State type) {
    char *err = "Err";
    char *s = NULL;
    
    switch (type) {
        case SMS_Sent: 
            s = strdup("Sent");
            break;
        case SMS_UnSent: 
            s = strdup("UnSent");
            break;
        case SMS_Read: 
            s = strdup("Read");
            break;
        case SMS_UnRead: 
            s = strdup("UnRead");
            break;
    }

    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for SMS_State from Gammu: '%d'", type);
        return NULL;
    }
    
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

int UDHFromPython(PyObject *dict, GSM_UDHHeader *udh) {
    char *s;
    
    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "UDH is not a dictionary");
        return 0;
    }
    
    memset(udh, 0, sizeof(udh));
    
    udh->ID8bit = GetIntFromDict(dict, "ID8bit");
    if (udh->ID8bit == INT_INVALID) {
        udh->ID8bit = -1;
        PyErr_Clear();
    }
    
    udh->ID16bit = GetIntFromDict(dict, "ID16bit");
    if (udh->ID16bit == INT_INVALID) {
        udh->ID16bit = -1;
        PyErr_Clear();
    }
    
    udh->PartNumber = GetIntFromDict(dict, "PartNumber");
    if (udh->PartNumber == INT_INVALID) {
        udh->PartNumber = -1;
        PyErr_Clear();
    }
    
    udh->AllParts = GetIntFromDict(dict, "AllParts");
    if (udh->AllParts == INT_INVALID) {
        udh->AllParts = -1;
        PyErr_Clear();
    }

    s = GetCharFromDict(dict, "Type");
    if (s == NULL) return 0;
    udh->Type = StringToUDHType(s);
    if (udh->Type == 0) return 0;

    s = GetDataFromDict(dict, "Text", &(udh->Length));
    if (s == NULL) return 0;

    if (udh->Length > GSM_MAX_UDH_LENGTH) {
        printf("WARNING: UDH too large, truncating!");
        udh->Length = GSM_MAX_UDH_LENGTH;
    }
    
    memcpy(udh->Text, s, udh->Length);

    return 1;
}

PyObject *UDHToPython(GSM_UDHHeader *udh) {
    char        *type;
    PyObject    *val;

    if (udh->Type == UDH_NoUDH) {
        /* No UDH => Nothing to return */
        Py_INCREF(Py_None);
        return Py_None;
    }

    type = UDHTypeToString(udh->Type);
    if (type == NULL) return NULL;

    val = Py_BuildValue("{s:s,s:s#,s:i,s:i,s:i,s:i}",
            "Type",         type,
            "Text",         udh->Text, udh->Length,
            "ID8bit",       udh->ID8bit,
            "ID16bit",      udh->ID16bit,
            "PartNumber",   udh->PartNumber,
            "AllParts",     udh->AllParts);
    
    free(type);

    return val;
}

int SMSFromPython(PyObject *dict, GSM_SMSMessage *sms, int needslocation, int needsfolder, int needsnumber) {
    PyObject        *o;
    char            *s;
    GSM_DateTime    nulldt = {0,0,0,0,0,0,0};
    int             i;
    
    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "SMS is not a dictionary");
        return 0;
    }
    
    memset(sms, 0, sizeof(sms));

    o = PyDict_GetItemString(dict, "SMSC");
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Missing SMSC attribute!");
        return 0;
    }
        
    if (!PyDict_Check(o)) {
        PyErr_Format(PyExc_ValueError, "SMSC should be dictionary!");
        return 0;
    }

    if (!SMSCFromPython(o, &(sms->SMSC))) {
        return 0;
    }

    if (!CopyStringFromDict(dict, "Number", GSM_MAX_NUMBER_LENGTH, sms->Number)) {
        if (needsnumber) {
            return 0;
        } else {
            EncodeUnicode(sms->Number, "Gammu", 5);
            PyErr_Clear();
        }
    }

    if (!CopyStringFromDict(dict, "Name", GSM_MAX_SMS_NAME_LENGTH, sms->Name)) {
        PyErr_Clear();
        sms->Name[0] = 0;
        sms->Name[1] = 0;
    }
    
    o = PyDict_GetItemString(dict, "UDH");
    if (o == NULL) {
        sms->UDH.Type = UDH_NoUDH;
    } else {
        if (!PyDict_Check(o)) {
            if (o == Py_None) {
                sms->UDH.Type = UDH_NoUDH;
            } else {
                PyErr_Format(PyExc_ValueError, "UDH is not a dictionary!");
                return 0;
            }
        } else {
            if (!UDHFromPython(o, &(sms->UDH))) return 0;
        }
    }

    if (sms->UDH.Type == UDH_NoUDH || sms->UDH.Type == UDH_UserUDH) {
        /* No UDH/UserUDH => copy as text */
        if (!CopyStringFromDict(dict, "Text", GSM_MAX_SMS_LENGTH, sms->Text))
            return 0;
        sms->Length = UnicodeLength(sms->Text);
    } else {
        /* Some UDH => copy as data */
        s = GetDataFromDict(dict, "Text", &(sms->Length));
        if (s == NULL) return 0;

        if (sms->Length > GSM_MAX_SMS_LENGTH) {
            printf("WARNING: SMS text too large, truncating!\n");
            sms->Length = GSM_MAX_SMS_LENGTH;
        }
        
        memcpy(sms->Text, s, sms->Length);
    }
   
    if ((sms->Folder = GetIntFromDict(dict, "Folder")) == INT_INVALID) {
        if (needsfolder) {
            return 0;
        } else {
            PyErr_Clear();
        }
    }
    
    if ((sms->Location = GetIntFromDict(dict, "Location")) == INT_INVALID) {
        if (needslocation) {
            return 0;
        } else {
            PyErr_Clear();
        }
    }
    if ((sms->InboxFolder = GetBoolFromDict(dict, "InboxFolder")) == BOOL_INVALID) {
        sms->InboxFolder = false;
        PyErr_Clear();
    }
    if ((i = GetIntFromDict(dict, "DeliveryStatus")) == INT_INVALID) {
        sms->DeliveryStatus = 0;
        PyErr_Clear();
    } else {
        sms->DeliveryStatus = i;
    }
    
    if ((i = GetIntFromDict(dict, "ReplyViaSameSMSC")) == INT_INVALID) {
        sms->ReplyViaSameSMSC = false;
        PyErr_Clear();
    } else {
        sms->ReplyViaSameSMSC = i;
    }
    if ((i = GetIntFromDict(dict, "Class")) == INT_INVALID) {
        sms->Class = -1;
        PyErr_Clear();
    } else {
        sms->Class = i;
    }
    if ((i = GetIntFromDict(dict, "MessageReference")) == INT_INVALID) {
        sms->MessageReference = 0;
        PyErr_Clear();
    } else {
        sms->MessageReference = i;
    }
    if ((i = GetIntFromDict(dict, "ReplaceMessage")) == INT_INVALID) {
        sms->ReplaceMessage = 0;
        PyErr_Clear();
    } else {
        sms->ReplaceMessage = i;
    }
    if ((sms->RejectDuplicates = GetBoolFromDict(dict, "RejectDuplicates")) == BOOL_INVALID) {
        sms->RejectDuplicates = false;
        PyErr_Clear();
    }

    s = GetCharFromDict(dict, "Memory");
    if (s == NULL || strcmp(s, "") == 0) {
        sms->Memory = 0;
        PyErr_Clear();
    } else {
        sms->Memory = StringToMemoryType(s);
        if (sms->Memory == 0) return 0;
    }

    s = GetCharFromDict(dict, "Type");
    if (s == NULL) {
        sms->PDU = SMS_Submit;
        PyErr_Clear();
    } else {
        sms->PDU = StringToSMSType(s);
        if (sms->PDU == 0) return 0;
    }

    s = GetCharFromDict(dict, "Coding");
    if (s == NULL) {
        sms->Coding = SMS_Coding_Default;
        PyErr_Clear();
    } else {
        sms->Coding = StringToSMSCoding(s);
        if (sms->Coding == 0) return 0;
    }

    sms->DateTime = GetDateTimeFromDict(dict, "DateTime");
    if (sms->DateTime.Year == -1) {
        sms->DateTime = nulldt;
        PyErr_Clear();
    }

    sms->SMSCTime = GetDateTimeFromDict(dict, "SMSCDateTime");
    if (sms->SMSCTime.Year == -1) {
        sms->SMSCTime = nulldt;
        PyErr_Clear();
    }

    s = GetCharFromDict(dict, "State");
    if (s == NULL) {
        PyErr_Clear();
        sms->State = SMS_UnSent;
    } else {
        sms->State = StringToSMSState(s);
        if (sms->State == 0) return 0;
    }

    return 1;
}
    
PyObject *SMSToPython(GSM_SMSMessage *sms) {
    PyObject    *udh;
    PyObject    *smsc;
    char        *mt;
    Py_UNICODE  *name;
    Py_UNICODE  *number;
    PyObject    *text;
    char        *type;
    char        *coding;
    char        *state;
    PyObject    *dt;
    PyObject    *smscdt;
    
    PyObject    *val;

    smsc = SMSCToPython(&(sms->SMSC));
    if (smsc == NULL) return NULL;

    udh = UDHToPython(&(sms->UDH));
    if (udh == NULL) {
        Py_DECREF(smsc);
        return NULL;
    }

    if (sms->Memory == 0) {
        mt = strdup("");
    } else {
        mt = MemoryTypeToString(sms->Memory);
        if (mt == NULL){
            Py_DECREF(smsc);
            Py_DECREF(udh);
            return NULL;
        }
    }

    name = strGammuToPython(sms->Name);
    if (name == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        return NULL;
    }

    number = strGammuToPython(sms->Number);
    if (number == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        return NULL;
    }

    if (sms->UDH.Type == UDH_NoUDH || sms->UDH.Type == UDH_UserUDH) {
        /* No UDH/User UDH => copy as text */
        text = UnicodeStringToPython(sms->Text);
    } else {
        /* Some UDH => copy as data */
        text = PyString_FromStringAndSize(sms->Text, sms->Length);
    }
    if (text == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        return NULL;
    }

    type = SMSTypeToString(sms->PDU);
    if (type == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        Py_DECREF(text);
        return NULL;
    }
    
    coding = SMSCodingToString(sms->Coding);
    if (type == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        Py_DECREF(text);
        free(type);
        return NULL;
    }

    state = SMSStateToString(sms->State);
    if (state == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        Py_DECREF(text);
        free(type);
        free(coding);
        return NULL;
    }

    dt = BuildPythonDateTime(&(sms->DateTime));
    if (dt == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        Py_DECREF(text);
        free(type);
        free(coding);
        free(state);
        return NULL;
    }

    smscdt = BuildPythonDateTime(&(sms->SMSCTime));
    if (smscdt == NULL) {
        Py_DECREF(smsc);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        Py_DECREF(text);
        free(type);
        free(coding);
        free(state);
        Py_DECREF(dt);
        return NULL;
    }

    val = Py_BuildValue("{s:O,s:O,s:i,s:i,s:s,s:i,s:u,s:u,s:O,s:s,s:s,s:O,s:O,s:i,s:i,s:s,s:i,s:i,s:i,s:i,s:i}",
            "SMSC",             smsc,
            "UDH",              udh,
            "Folder",           (int)sms->Folder,
            "InboxFolder",      (int)sms->InboxFolder,
            "Memory",           mt,
            "Location",         (int)sms->Location,
            "Name",             name,
            "Number",           number,
            "Text",             text,
            "Type",             type,
            "Coding",           coding,
            "DateTime",         dt,
            "SMSCDateTime",     smscdt,
            "DeliveryStatus",   (int)sms->DeliveryStatus,
            "ReplyViaSameSMSC", (int)sms->ReplyViaSameSMSC,
            "State",            state,
            "Class",            (int)sms->Class,
            "MessageReference", (int)sms->MessageReference,
            "ReplaceMessage",   (int)sms->ReplaceMessage,
            "RejectDuplicates", (int)sms->RejectDuplicates,
            "Length",           sms->Length);

    Py_DECREF(smsc);
    Py_DECREF(udh);
    free(mt);
    free(name);
    free(number);
    Py_DECREF(text);
    free(type);
    free(coding);
    free(state);
    Py_DECREF(dt);
    Py_DECREF(smscdt);
    
    return val;
}

PyObject *MultiSMSToPython(GSM_MultiSMSMessage *sms) {
    PyObject    *val;
    PyObject    *item;
    int         i;

    val = PyList_New(0);
    if (val == NULL) return NULL;

    for (i=0; i < sms->Number; i++) {
        item = SMSToPython(&(sms->SMS[i]));
        if (item == NULL) {
            Py_DECREF(val);
            return NULL;
        }
        if (PyList_Append(val, item) != 0) {
            Py_DECREF(item);
            Py_DECREF(val);
            return NULL;
        }
        Py_DECREF(item);
    }

    return val;
}

int MultiSMSFromPython(PyObject *list, GSM_MultiSMSMessage *sms) {
    PyObject    *item;
    int         len;
    int         i;

    if (!PyList_Check(list)) {
        PyErr_Format(PyExc_ValueError, "Multi SMS is not a list");
        return 0;
    }
    
    len = PyList_Size(list);

    if (len > MAX_MULTI_SMS) {
        printf("WARNING: truncating MultiSMS entries to %d entries! (from %d))\n", MAX_MULTI_SMS, len);
        len = MAX_MULTI_SMS;
    }
    sms->Number = len;

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(list, i);
        if (item == NULL) return 0;
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %i in Messages is not dictionary", i);
            return 0;
        }
        if (!SMSFromPython(item, &(sms->SMS[i]), 0, 0, 0)) return 0;
    }

    return 1;
}

int MultiSMSListFromPython(PyObject *list, GSM_MultiSMSMessage ***sms) {
    PyObject    *item;
    int         len;
    int         i;
    int         j;

    if (!PyList_Check(list)) {
        PyErr_Format(PyExc_ValueError, "Multi SMS list is not a list");
        return 0;
    }
    
    len = PyList_Size(list);

    *sms = (GSM_MultiSMSMessage **)malloc((len + 1) * sizeof(GSM_MultiSMSMessage *));
    if (*sms == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
        return 0;
    }

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(list, i);
        if (item == NULL) {
            for (j = 0; j < i; j++) free((*sms)[j]);
            free((*sms));
            return 0;
        }
        if (!PyList_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %i in Messages is not list", i);
            for (j = 0; j < i; j++) free((*sms)[j]);
            free((*sms));
            return 0;
        }
        
        (*sms)[i] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
        if ((*sms)[i] == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            for (j = 0; j < i; j++) free((*sms)[j]);
            free((*sms));
            return 0;
        }

        if (!MultiSMSFromPython(item, (*sms)[i])) {
            for (j = 0; j <= i; j++) free((*sms)[j]);
            free((*sms));
            return 0;
        }
    }
    (*sms)[len] = NULL;

    return 1;
}

PyObject *MultiSMSListToPython(GSM_MultiSMSMessage **sms) {
    PyObject    *val;
    PyObject    *item;
    int         i = 0;

    val = PyList_New(0);
    if (val == NULL) return NULL;

    while (sms[i] != NULL) {
        item = MultiSMSToPython(sms[i]);
        if (item == NULL) {
            Py_DECREF(val);
            return NULL;
        }
        if (PyList_Append(val, item) != 0) {
            Py_DECREF(val);
            Py_DECREF(item);
            return NULL;
        }
        Py_DECREF(item);
        i++;
    }

    return val;
}


char *MultiPartSMSIDToString(EncodeMultiPartSMSID type) {
    char *err = "Err";
    char *s = err;

    switch (type) {
        case SMS_Text:
            s = strdup("Text");
            break;
        case SMS_ConcatenatedTextLong:
            s = strdup("ConcatenatedTextLong");
            break;
        case SMS_ConcatenatedAutoTextLong:
            s = strdup("ConcatenatedAutoTextLong");
            break;
        case SMS_ConcatenatedTextLong16bit:
            s = strdup("ConcatenatedTextLong16bit");
            break;
        case SMS_ConcatenatedAutoTextLong16bit:
            s = strdup("ConcatenatedAutoTextLong16bit");
            break;
        case SMS_NokiaProfileLong:
            s = strdup("NokiaProfileLong");
            break;
        case SMS_NokiaPictureImageLong:
            s = strdup("NokiaPictureImageLong");
            break;
        case SMS_NokiaScreenSaverLong:
            s = strdup("NokiaScreenSaverLong");
            break;
        case SMS_NokiaRingtone:
            s = strdup("NokiaRingtone");
            break;
        case SMS_NokiaRingtoneLong:
            s = strdup("NokiaRingtoneLong");
            break;
        case SMS_NokiaOperatorLogo:
            s = strdup("NokiaOperatorLogo");
            break;
        case SMS_NokiaOperatorLogoLong:
            s = strdup("NokiaOperatorLogoLong");
            break;
        case SMS_NokiaCallerLogo:
            s = strdup("NokiaCallerLogo");
            break;
        case SMS_NokiaWAPBookmarkLong:
            s = strdup("NokiaWAPBookmarkLong");
            break;
        case SMS_NokiaWAPSettingsLong:
            s = strdup("NokiaWAPSettingsLong");
            break;
        case SMS_NokiaMMSSettingsLong:
            s = strdup("NokiaMMSSettingsLong");
            break;
        case SMS_NokiaVCARD10Long:
            s = strdup("NokiaVCARD10Long");
            break;
        case SMS_NokiaVCARD21Long:
            s = strdup("NokiaVCARD21Long");
            break;
        case SMS_NokiaVCALENDAR10Long:
            s = strdup("NokiaVCALENDAR10Long");
            break;
        case SMS_NokiaVTODOLong:
            s = strdup("NokiaVTODOLong");
            break;
        case SMS_VCARD10Long:
            s = strdup("VCARD10Long");
            break;
        case SMS_VCARD21Long:
            s = strdup("VCARD21Long");
            break;
        case SMS_DisableVoice:
            s = strdup("DisableVoice");
            break;
        case SMS_DisableFax:
            s = strdup("DisableFax");
            break;
        case SMS_DisableEmail:
            s = strdup("DisableEmail");
            break;
        case SMS_EnableVoice:
            s = strdup("EnableVoice");
            break;
        case SMS_EnableFax:
            s = strdup("EnableFax");
            break;
        case SMS_EnableEmail:
            s = strdup("EnableEmail");
            break;
        case SMS_VoidSMS:
            s = strdup("VoidSMS");
            break;
        case SMS_EMSSound10:
            s = strdup("EMSSound10");
            break;
        case SMS_EMSSound12:
            s = strdup("EMSSound12");
            break;
        case SMS_EMSSonyEricssonSound:
            s = strdup("EMSSonyEricssonSound");
            break;
        case SMS_EMSSound10Long:
            s = strdup("EMSSound10Long");
            break;
        case SMS_EMSSound12Long:
            s = strdup("EMSSound12Long");
            break;
        case SMS_EMSSonyEricssonSoundLong:
            s = strdup("EMSSonyEricssonSoundLong");
            break;
        case SMS_EMSPredefinedSound:
            s = strdup("EMSPredefinedSound");
            break;
        case SMS_EMSPredefinedAnimation:
            s = strdup("EMSPredefinedAnimation");
            break;
        case SMS_EMSAnimation:
            s = strdup("EMSAnimation");
            break;
        case SMS_EMSFixedBitmap:
            s = strdup("EMSFixedBitmap");
            break;
        case SMS_EMSVariableBitmap:
            s = strdup("EMSVariableBitmap");
            break;
        case SMS_EMSVariableBitmapLong:
            s = strdup("EMSVariableBitmapLong");
            break;
        case SMS_MMSIndicatorLong:
            s = strdup("MMSIndicatorLong");
            break;
        case SMS_AlcatelMonoBitmapLong:
            s = strdup("AlcatelMonoBitmapLong");
            break;
        case SMS_AlcatelMonoAnimationLong:
            s = strdup("AlcatelMonoAnimationLong");
            break;
        case SMS_AlcatelSMSTemplateName:
            s = strdup("AlcatelSMSTemplateName");
            break;
    }
    
    if (s == err) {
        PyErr_Format(PyExc_ValueError, "Bad value for MultiPartSMSID from Gammu: '%d'", type);
        return NULL;
    }
    
    if (s == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate string");
        return NULL;
    }

    return s;
}

EncodeMultiPartSMSID StringToMultiPartSMSID(char *s){
	if (strcmp("Text", s) == 0) return SMS_Text;
	else if (strcmp("ConcatenatedTextLong", s) == 0) return SMS_ConcatenatedTextLong;
	else if (strcmp("ConcatenatedAutoTextLong", s) == 0) return SMS_ConcatenatedAutoTextLong;
	else if (strcmp("ConcatenatedTextLong16bit", s) == 0) return SMS_ConcatenatedTextLong16bit;
	else if (strcmp("ConcatenatedAutoTextLong16bit", s) == 0) return SMS_ConcatenatedAutoTextLong16bit;
	else if (strcmp("NokiaProfileLong", s) == 0) return SMS_NokiaProfileLong;
	else if (strcmp("NokiaPictureImageLong", s) == 0) return SMS_NokiaPictureImageLong;
	else if (strcmp("NokiaScreenSaverLong", s) == 0) return SMS_NokiaScreenSaverLong;
	else if (strcmp("NokiaRingtone", s) == 0) return SMS_NokiaRingtone;
	else if (strcmp("NokiaRingtoneLong", s) == 0) return SMS_NokiaRingtoneLong;
	else if (strcmp("NokiaOperatorLogo", s) == 0) return SMS_NokiaOperatorLogo;
	else if (strcmp("NokiaOperatorLogoLong", s) == 0) return SMS_NokiaOperatorLogoLong;
	else if (strcmp("NokiaCallerLogo", s) == 0) return SMS_NokiaCallerLogo;
	else if (strcmp("NokiaWAPBookmarkLong", s) == 0) return SMS_NokiaWAPBookmarkLong;
	else if (strcmp("NokiaWAPSettingsLong", s) == 0) return SMS_NokiaWAPSettingsLong;
	else if (strcmp("NokiaMMSSettingsLong", s) == 0) return SMS_NokiaMMSSettingsLong;
	else if (strcmp("NokiaVCARD10Long", s) == 0) return SMS_NokiaVCARD10Long;
	else if (strcmp("NokiaVCARD21Long", s) == 0) return SMS_NokiaVCARD21Long;
	else if (strcmp("NokiaVCALENDAR10Long", s) == 0) return SMS_NokiaVCALENDAR10Long;
	else if (strcmp("NokiaVTODOLong", s) == 0) return SMS_NokiaVTODOLong;
	else if (strcmp("VCARD10Long", s) == 0) return SMS_VCARD10Long;
	else if (strcmp("VCARD21Long", s) == 0) return SMS_VCARD21Long;
	else if (strcmp("DisableVoice", s) == 0) return SMS_DisableVoice;
	else if (strcmp("DisableFax", s) == 0) return SMS_DisableFax;
	else if (strcmp("DisableEmail", s) == 0) return SMS_DisableEmail;
	else if (strcmp("EnableVoice", s) == 0) return SMS_EnableVoice;
	else if (strcmp("EnableFax", s) == 0) return SMS_EnableFax;
	else if (strcmp("EnableEmail", s) == 0) return SMS_EnableEmail;
	else if (strcmp("VoidSMS", s) == 0) return SMS_VoidSMS;
	else if (strcmp("EMSSound10", s) == 0) return SMS_EMSSound10;
	else if (strcmp("EMSSound12", s) == 0) return SMS_EMSSound12;
	else if (strcmp("EMSSonyEricssonSound", s) == 0) return SMS_EMSSonyEricssonSound;
	else if (strcmp("EMSSound10Long", s) == 0) return SMS_EMSSound10Long;
	else if (strcmp("EMSSound12Long", s) == 0) return SMS_EMSSound12Long;
	else if (strcmp("EMSSonyEricssonSoundLong", s) == 0) return SMS_EMSSonyEricssonSoundLong;
	else if (strcmp("EMSPredefinedSound", s) == 0) return SMS_EMSPredefinedSound;
	else if (strcmp("EMSPredefinedAnimation", s) == 0) return SMS_EMSPredefinedAnimation;
	else if (strcmp("EMSAnimation", s) == 0) return SMS_EMSAnimation;
	else if (strcmp("EMSFixedBitmap", s) == 0) return SMS_EMSFixedBitmap;
	else if (strcmp("EMSVariableBitmap", s) == 0) return SMS_EMSVariableBitmap;
	else if (strcmp("EMSVariableBitmapLong", s) == 0) return SMS_EMSVariableBitmapLong;
	else if (strcmp("MMSIndicatorLong", s) == 0) return SMS_MMSIndicatorLong;
	else if (strcmp("AlcatelMonoBitmapLong", s) == 0) return SMS_AlcatelMonoBitmapLong;
	else if (strcmp("AlcatelMonoAnimationLong", s) == 0) return SMS_AlcatelMonoAnimationLong;
    else if (strcmp("AlcatelSMSTemplateName", s) == 0) return SMS_AlcatelSMSTemplateName;
    
    PyErr_Format(PyExc_ValueError, "Bad value for MultiPartSMSID '%s'", s);
    return 0;
}

PyObject *SMSPartToPython(MultiPartSMSEntry *entry) {
    char        *t;
    PyObject    *r;
    PyObject    *v;

    t = MultiPartSMSIDToString(entry->ID);

    r = Py_BuildValue("{s:s,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i}",
            "ID",               t,
            "Left",             (int)entry->Left,
            "Right",            (int)entry->Right,
            "Center",           (int)entry->Center,
            "Large",            (int)entry->Large,
            "Small",            (int)entry->Small,
            "Bold",             (int)entry->Bold,
            "Italic",           (int)entry->Italic,
            "Underlined",       (int)entry->Underlined,
            "Strikethrough",    (int)entry->Strikethrough,
            "RingtoneNotes",    (int)entry->RingtoneNotes,
            "Protected",        (int)entry->Protected,
            "Number",           entry->Number);
    free(t);

    if (entry->Ringtone != NULL) {
        v = RingtoneToPython(entry->Ringtone);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Ringtone", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    if (entry->Bitmap != NULL) {
        v = MultiBitmapToPython(entry->Bitmap);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Bitmap", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

#if 0
    if (entry->Bookmark != NULL) {
        v = BookmarkToPython(entry->Bookmark);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Bookmark", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    if (entry->Settings != NULL) {
        v = SettingsToPython(entry->Settings);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Settings", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    if (entry->MMSIndicator != NULL) {
        v = MMSIndicatorToPython(entry->MMSIndicator);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "MMSIndicator", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);
#endif

    if (entry->Phonebook != NULL) {
        v = MemoryEntryToPython(entry->Phonebook);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Phonebook", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    if (entry->Calendar != NULL) {
        v = CalendarToPython(entry->Calendar);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Calendar", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    if (entry->ToDo != NULL) {
        v = TodoToPython(entry->ToDo);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "ToDo", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    if (entry->Buffer != NULL) {
        v = UnicodeStringToPython(entry->Buffer);
        if (v == NULL) return NULL;
    } else {
        Py_INCREF(Py_None);
        v = Py_None;
    }
    if (PyDict_SetItemString(r, "Buffer", v) != 0) {
        Py_DECREF(v);
        Py_DECREF(r);
        return NULL;
    }
    Py_DECREF(v);

    return r;
}

PyObject *SMSInfoToPython(GSM_MultiPartSMSInfo *smsinfo) {
    PyObject    *v;
    PyObject    *f;
    PyObject    *r;
    int         i;

    v = PyList_New(0);
    if (v == NULL) return NULL;
    
	for (i = 0; i < smsinfo->EntriesNum; i++) {
        f = SMSPartToPython(&(smsinfo->Entries[i]));
        if (f == NULL) {
            Py_DECREF(v);
            return NULL;
        }
        if (PyList_Append(v, f) != 0) {
            Py_DECREF(f);
            Py_DECREF(v);
            return NULL;
        }
        Py_DECREF(f);
    }

    r = Py_BuildValue("{s:i,s:i,s:i,s:i,s:O}",
            "Class",            (int)smsinfo->Class,
            "Unknown",          (int)smsinfo->Unknown,
            "ReplaceMessage",   (int)smsinfo->ReplaceMessage,
            "Unicode",          (int)smsinfo->UnicodeCoding,
            "Entries", v
            );
    Py_DECREF(v);
    return r;
}


int SMSPartFromPython(PyObject *dict, MultiPartSMSEntry *entry) {
    char            *s;
    int             i;
    PyObject        *o;

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "SMS info part is not a dictionary");
        return 0;
    }
    
    s = GetCharFromDict(dict, "ID");
    if (s == NULL) return 0;

    entry->ID = StringToMultiPartSMSID(s);
    if (entry->ID == 0) return 0;


    i = GetBoolFromDict(dict, "Left");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Left = i;
    }
    i = GetBoolFromDict(dict, "Right");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Right = i;
    }
    i = GetBoolFromDict(dict, "Center");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Center = i;
    }
    i = GetBoolFromDict(dict, "Large");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Large = i;
    }
    i = GetBoolFromDict(dict, "Small");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Small = i;
    }
    i = GetBoolFromDict(dict, "Bold");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Bold = i;
    }
    i = GetBoolFromDict(dict, "Italic");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Italic = i;
    }
    i = GetBoolFromDict(dict, "Underlined");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Underlined = i;
    }
    i = GetBoolFromDict(dict, "Strikethrough");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Strikethrough = i;
    }
    i = GetBoolFromDict(dict, "RingtoneNotes");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->RingtoneNotes = i;
    }
    i = GetBoolFromDict(dict, "Protected");
    if (i == BOOL_INVALID) {
        PyErr_Clear();
    } else {
       entry->Protected = i;
    }
    i = GetIntFromDict(dict, "Number");
    if (i == INT_INVALID) {
        PyErr_Clear();
    } else {
       entry->Number = i;
    }

    o = PyDict_GetItemString(dict, "Ringtone");
    if (o != NULL && o != Py_None) {
        entry->Ringtone = (GSM_Ringtone *)malloc(sizeof(GSM_Ringtone));
        if (entry->Ringtone == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!RingtoneFromPython(o, entry->Ringtone)) {
            return 0;
        }
    }

    o = PyDict_GetItemString(dict, "Bitmap");
    if (o != NULL && o != Py_None) {
        entry->Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
        if (entry->Bitmap == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!MultiBitmapFromPython(o, entry->Bitmap)) {
            return 0;
        }
    }

#if 0
    o = PyDict_GetItemString(dict, "Bookmark");
    if (o != NULL && o != Py_None) {
        entry->Bookmark = (GSM_WAPBookmark *)malloc(sizeof(GSM_WAPBookmark));
        if (entry->Bookmark == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!BookmarkFromPython(o, entry->Bookmark)) {
            return 0;
        }
    }

    o = PyDict_GetItemString(dict, "Settings");
    if (o != NULL && o != Py_None) {
        entry->Settings = (GSM_WAPSettings *)malloc(sizeof(GSM_WAPSettings));
        if (entry->Settings == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!SettingsFromPython(o, entry->Settings)) {
            return 0;
        }
    }

    o = PyDict_GetItemString(dict, "MMSIndicator");
    if (o != NULL && o != Py_None) {
        entry->MMSIndicator = (GSM_MMSIndicator *)malloc(sizeof(GSM_MMSIndicator));
        if (entry->MMSIndicator == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!MMSIndicatorFromPython(o, entry->MMSIndicator)) {
            return 0;
        }
    }
#endif

    o = PyDict_GetItemString(dict, "Phonebook");
    if (o != NULL && o != Py_None) {
        entry->Phonebook = (GSM_MemoryEntry *)malloc(sizeof(GSM_MemoryEntry));
        if (entry->Phonebook == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!MemoryEntryFromPython(o, entry->Phonebook, false)) {
            return 0;
        }
    }

    o = PyDict_GetItemString(dict, "Calendar");
    if (o != NULL && o != Py_None) {
        entry->Calendar = (GSM_CalendarEntry *)malloc(sizeof(GSM_CalendarEntry));
        if (entry->Calendar == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!CalendarFromPython(o, entry->Calendar, false)) {
            return 0;
        }
    }

    o = PyDict_GetItemString(dict, "ToDo");
    if (o != NULL && o != Py_None) {
        entry->ToDo = (GSM_ToDoEntry *)malloc(sizeof(GSM_ToDoEntry));
        if (entry->ToDo == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate structure");
            return 0;
        }
        if (!TodoFromPython(o, entry->ToDo, false)) {
            return 0;
        }
    }

    entry->Buffer = GetStringFromDict(dict, "Buffer");
    if (entry->Buffer == NULL) {
        PyErr_Clear();
    }

    return 1;
}
    
int SMSInfoFromPython(PyObject *dict, GSM_MultiPartSMSInfo *entry) {
    PyObject        *o;
    PyObject        *item;
    int             len;
    int             i;
    
    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_ValueError, "SMS info is not a dictionary");
        return 0;
    }
    
    GSM_ClearMultiPartSMSInfo(entry);
    
    entry->UnicodeCoding = GetBoolFromDict(dict, "Unicode");
    if (entry->UnicodeCoding == BOOL_INVALID) {
        PyErr_Clear();
        entry->UnicodeCoding = 0;
    }

    i = GetIntFromDict(dict, "ReplaceMessage");
    if (i == INT_INVALID) {
        PyErr_Clear();
        entry->ReplaceMessage = 0;
    } else {
        entry->ReplaceMessage = i;
    }

    entry->Unknown = GetBoolFromDict(dict, "Unknown");
    if (entry->Unknown == BOOL_INVALID) {
        PyErr_Clear();
        entry->Unknown = false;
    }

    i = GetIntFromDict(dict, "Class");
    if (i == INT_INVALID) {
        PyErr_Clear();
        entry->Class = -1;
    } else {
        entry->Class = i;
    }

    o = PyDict_GetItemString(dict, "Entries");
    if (o == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not get string value for key Entries");
        return 0;
    }
    
    if (!PyList_Check(o)) {
        PyErr_Format(PyExc_ValueError, "Key Entries doesn't contain list");
        return 0;
    }
    
    len = PyList_Size(o);

    if (len > MAX_MULTI_SMS - 1) {
        printf("Warning, too many entries, truncating from %d to %d\n", len, MAX_MULTI_SMS - 1);
        len = MAX_MULTI_SMS - 1;
    }

    entry->EntriesNum = len;
    

    for (i = 0; i < len ; i++) {
        item = PyList_GetItem(o, i);
        if (item == NULL) {
            return 0;
        }
        if (!PyDict_Check(item)) {
            PyErr_Format(PyExc_ValueError, "Element %i in Entries is not dictionary", i);
            return 0;
        }
        
        if (!SMSPartFromPython(item, &(entry->Entries[i]))) {
            return 0;
        }
    }

    return 1;
}

PyObject *SMSFolderToPython(GSM_OneSMSFolder *folder) {
    Py_UNICODE      *name;
    char            *mt;
    PyObject        *result;

    name = strGammuToPython(folder->Name);
    if (name == NULL) return NULL;
    
    mt = MemoryTypeToString(folder->Memory);
    if (mt == NULL) {
        free(name);
        return NULL;
    }

    result = Py_BuildValue("{s:u,s:s,s:i}",
            "Name",     name,
            "Memory",   mt,
            "Inbox",    (int)folder->InboxFolder);
 
    free(mt);
    free(name);
    
    return result;
}


PyObject *SMSFoldersToPython(GSM_SMSFolders *folders) {
    PyObject    *val;
    PyObject    *item;
    int         i;

    val = PyList_New(0);
    if (val == NULL) return NULL;

    for (i = 0; i < folders->Number; i++) {
        item = SMSFolderToPython(&folders->Folder[i]);
        if (item == NULL) {
            Py_DECREF(val);
            return NULL;
        }
        if (PyList_Append(val, item) != 0) {
            Py_DECREF(val);
            Py_DECREF(item);
            return NULL;
        }
        Py_DECREF(item);
    }

    return val;
}

