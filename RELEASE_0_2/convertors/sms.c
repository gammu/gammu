/* SMS related convertions */

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
                case SMS_VALID_24_Hours: s[0] = '2'; s[1] = '4'; s[2] = 'H'; return s;
                case SMS_VALID_72_Hours: s[0] = '7'; s[1] = '2'; s[2] = 'H'; return s;
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
    
    if (strcmp(s, "NA") == 0) Validity.Format = SMS_Validity_NotAvailable;
    else if (strlen(s) == 3) {
        Validity.Format = SMS_Validity_RelativeFormat;
        if (strcmp(s, "Max") == 0) {
            Validity.Relative = SMS_VALID_Max_Time;
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
                    Validity.Relative = SMS_VALID_24_Hours;
                    break;
                case 72:
                    Validity.Relative = SMS_VALID_72_Hours;
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
    
    PyErr_Format(PyExc_MemoryError, "Bad value for UDH Type '%s'", s);
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

int SMSFromPython(PyObject *dict, GSM_SMSMessage *sms, int needslocation, int needsfolder) {
    PyObject        *o;
    char            *s;
    GSM_DateTime    nulldt = {0,0,0,0,0,0,0};
    int             i;
    
    memset(sms, 0, sizeof(sms));

    if (!CopyStringFromDict(dict, "SMSCNumber", GSM_MAX_NUMBER_LENGTH, sms->SMSC.Number)) {
        /* SMSC can be empty */
        PyErr_Clear();
        sms->SMSC.Number[0] = 0;
        sms->SMSC.Number[1] = 0;
    }
    if (!CopyStringFromDict(dict, "Number", GSM_MAX_NUMBER_LENGTH, sms->Number))
        return 0;

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
            PyErr_Format(PyExc_ValueError, "UDH is not a dictionary!");
            return 0;
        }
        if (!UDHFromPython(o, &(sms->UDH))) return 0;
    }
    if (!CopyStringFromDict(dict, "Text", GSM_MAX_SMS_LENGTH, sms->Text)) {
        /* Text can be empty if some UDH */
        if (sms->UDH.Type != UDH_NoUDH) {
            PyErr_Clear();
            sms->Text[0] = 0;
            sms->Text[1] = 0;
        } else {
            PyErr_Format(PyExc_ValueError, "SMS with no UDH and no Text doesn't make much sense!");
            return 0;
        }
    }

   
    if (needsfolder) {
        if ((sms->Folder = GetIntFromDict(dict, "Folder")) == INT_INVALID) {
            return 0;
        }
    }
    
    if (needslocation) {
        if ((sms->Location = GetIntFromDict(dict, "Location")) == INT_INVALID) {
            return 0;
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

    s = GetCharFromDict(dict, "Validity");
    if (s == NULL) {
        sms->SMSC.Validity.Relative  = SMS_VALID_Max_Time;
        sms->SMSC.Validity.Format    = SMS_Validity_RelativeFormat;
        PyErr_Clear();
    } else {
        sms->SMSC.Validity = StringToSMSValidity(s);
        if (sms->SMSC.Validity.Format == 0) return 0;
    }

    s = GetCharFromDict(dict, "Memory");
    if (s == NULL) {
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
        sms->State = SMS_UnSent;
    } else {
        sms->State = StringToSMSState(s);
        if (sms->State == 0) return 0;
    }

    return 1;
}
    
PyObject *SMSToPython(GSM_SMSMessage *sms) {
    PyObject    *udh;
    Py_UNICODE  *smsc;
    char        *valid;
    char        *mt;
    Py_UNICODE  *name;
    Py_UNICODE  *number;
    Py_UNICODE  *text;
    char        *type;
    char        *coding;
    char        *state;
    PyObject    *dt;
    PyObject    *smscdt;
    
    PyObject    *val;

    smsc = strGammuToPython(sms->SMSC.Number);
    if (smsc == NULL) return NULL;

    valid = SMSValidityToString(sms->SMSC.Validity);
    if (valid == NULL) {
        free(smsc);
        return NULL;
    }
    
    udh = UDHToPython(&(sms->UDH));
    if (udh == NULL) {
        free(smsc);
        free(valid);
        return NULL;
    }

    mt = MemoryTypeToString(sms->Memory);
    if (mt == NULL){
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        return NULL;
    }

    name = strGammuToPython(sms->Name);
    if (name == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        return NULL;
    }

    number = strGammuToPython(sms->Number);
    if (number == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        return NULL;
    }

    text = strGammuToPython(sms->Text);
    if (text == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        return NULL;
    }

    type = SMSTypeToString(sms->PDU);
    if (type == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        free(text);
        return NULL;
    }
    
    coding = SMSCodingToString(sms->Coding);
    if (type == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        free(text);
        free(type);
        return NULL;
    }

    state = SMSStateToString(sms->State);
    if (state == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        free(text);
        free(type);
        free(coding);
        return NULL;
    }

    dt = BuildPythonDateTime(&(sms->DateTime));
    if (dt == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        free(text);
        free(type);
        free(coding);
        free(state);
        return NULL;
    }

    smscdt = BuildPythonDateTime(&(sms->SMSCTime));
    if (smscdt == NULL) {
        free(smsc);
        free(valid);
        Py_DECREF(udh);
        free(mt);
        free(name);
        free(number);
        free(text);
        free(type);
        free(coding);
        free(state);
        Py_DECREF(dt);
        return NULL;
    }

    val = Py_BuildValue("{s:u,s:s,s:O,s:i,s:i,s:s,s:i,s:u,s:u,s:u,s:s,s:s,s:O,s:O,s:b,s:i,s:s,s:b,s:b,s:b,s:i}",
            "SMSCNumber",       smsc,
            "Validity",         valid,
            "UDH",              udh,
            "Folder",           sms->Folder,
            "InboxFolder",      sms->InboxFolder,
            "Memory",           mt,
            "Location",         sms->Location,
            "Name",             name,
            "Number",           number,
            "Text",             text,
            "Type",             type,
            "Coding",           coding,
            "DateTime",         dt,
            "SMSCDateTime",     smscdt,
            "DeliveryStatus",   sms->DeliveryStatus,
            "ReplyViaSameSMSC", sms->ReplyViaSameSMSC,
            "State",            state,
            "Class",            sms->Class,
            "MessageReference", sms->MessageReference,
            "ReplaceMessage",   sms->ReplaceMessage,
            "RejectDuplicates", sms->RejectDuplicates);

    free(smsc);
    free(valid);
    Py_DECREF(udh);
    free(mt);
    free(name);
    free(number);
    free(text);
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
            Py_DECREF(val);
            return NULL;
        }
    }

    return val;
}
