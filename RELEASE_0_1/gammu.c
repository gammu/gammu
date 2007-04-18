/* FIXME: check before call whether we are initialised, otherwise segfault follows! */

/* Python includes */
#include <Python.h>

/* Gammu includes */
#include <gammu.h>

/* Convertors between Gammu and Python types */
#include "convertors.h"

/* Length of buffers used in most of code */
#define BUFFER_LENGTH 255

static PyObject *GammuError;

static bool
checkError(GSM_StateMachine *s, GSM_Error error, char *where) {
    PyObject    *val;
    
    if (error == GE_NONE) return true;
    val = Py_BuildValue("{s:s,s:s,s:i}",
            "Text", print_error(error, s->di.df, s->msg),
            "Where", where,
            "Code", error);

    if (val == NULL) {
        PyErr_Format(GammuError, "GSM Error %d (%s) in %s", error, print_error(error, s->di.df, s->msg), where);
    } else {
        PyErr_SetObject(GammuError, val);
    }
                

    return false;
}
    

/* ----------------------------------------------------- */

/* Declarations for objects of type StateMachine */

typedef struct {
	PyObject_HEAD

    GSM_StateMachine s;
} StateMachineObject;

static PyTypeObject StateMachineType;



/* ---------------------------------------------------------------- */

/* Declarations for objects of type MemoryEntry */

typedef struct {
	PyObject_HEAD
    
    GSM_MemoryEntry entry;
} MemoryEntryObject;

static PyTypeObject MemoryEntryType;



/* ---------------------------------------------------------------- */

static char StateMachine_ReadConfig__doc__[] = 
"Reads specified sectin of gammurc"
;

static PyObject *
StateMachine_ReadConfig(StateMachineObject *self, PyObject *args, PyObject *kwds)
{
    int             section = 0;
    int             dst = -1;
    CFG_Header		*cfg;

    static char         *kwlist[] = {"Section", "Configuration", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|II", kwlist, &section, &dst))
		return NULL;

    /* FIXME: Maybe this should go to separate object... */
	cfg = CFG_FindGammuRC();
    if (cfg == NULL) {
        PyErr_SetString(PyExc_IOError, "Can not find gammurc");
        return NULL;
    }
    
    if (dst == -1) dst = section;

    if (!CFG_ReadConfig(cfg, &(self->s.Config[dst]), section)) {
        free(cfg);
        PyErr_SetString(PyExc_IOError, "Can not read config");
        return NULL;
    }

    self->s.ConfigNum = dst + 1;

    free(cfg);
	Py_INCREF(Py_None);
	return Py_None;
}


static char StateMachine_Init__doc__[] = 
"Initialises the connection with phone."
;

static PyObject *
StateMachine_Init(StateMachineObject *self, PyObject *args, PyObject *kwds)
{
    GSM_Error           error;
    int                 replies = 3;

	if (!PyArg_ParseTuple(args, "|I", &replies))
		return NULL;
    
    error = GSM_InitConnection(&(self->s), replies);
    if (!checkError(&(self->s), error, "Init")) return NULL;
    
	Py_INCREF(Py_None);
	return Py_None;
}


static char StateMachine_Terminate__doc__[] = 
"Terminates the connection with phone."
;

static PyObject *
StateMachine_Terminate(StateMachineObject *self, PyObject *args, PyObject *kwds)
{
    GSM_Error           error;
    
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = GSM_TerminateConnection(&(self->s));
    if (!checkError(&(self->s), error, "Terminate")) return NULL;
    
	Py_INCREF(Py_None);
	return Py_None;
}


/*****************/
/* ShowStartInfo */
/*****************/

static char StateMachine_ShowStartInfo__doc__[] = 
"Enables showing information on phone display."
;

static PyObject *
StateMachine_ShowStartInfo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    bool                i;

	if (!PyArg_ParseTuple(args, "i", &i))
		return NULL;

    error = self->s.Phone.Functions->ShowStartInfo(&(self->s), i);

    if (!checkError(&(self->s), error, "ShowStartInfo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* GetManufacturer */
/*******************/

static char StateMachine_GetManufacturer__doc__[] = 
"Reads manufacturer from phone."
;

static PyObject *
StateMachine_GetManufacturer(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetManufacturer(&(self->s));

    if (!checkError(&(self->s), error, "GetManufacturer")) return NULL;

    return Py_BuildValue("s", self->s.Phone.Data.Manufacturer);
}

/************/
/* GetModel */
/************/

static char StateMachine_GetModel__doc__[] = 
"Reads model from phone."
;

static PyObject *
StateMachine_GetModel(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetModel(&(self->s));

    if (!checkError(&(self->s), error, "GetModel")) return NULL;

    return Py_BuildValue("ss", self->s.Phone.Data.ModelInfo->model, self->s.Phone.Data.Model);
}

/***************/
/* GetFirmware */
/***************/

static char StateMachine_GetFirmware__doc__[] = 
"Reads firmware information from phone."
;

static PyObject *
StateMachine_GetFirmware(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetFirmware(&(self->s));

    if (!checkError(&(self->s), error, "GetFirmware")) return NULL;

    return Py_BuildValue("s", self->s.Phone.Data.Version);
}

/***********/
/* GetIMEI */
/***********/

static char StateMachine_GetIMEI__doc__[] = 
"Reads IMEI/serial number from phone."
;

static PyObject *
StateMachine_GetIMEI(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetIMEI(&(self->s));

    if (!checkError(&(self->s), error, "GetIMEI")) return NULL;

    return Py_BuildValue("s", self->s.Phone.Data.IMEI);
}

/*******************/
/* GetOriginalIMEI */
/*******************/

static char StateMachine_GetOriginalIMEI__doc__[] = 
"Gets date and time from phone."
;

static PyObject *
StateMachine_GetOriginalIMEI(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                buffer[BUFFER_LENGTH];

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetOriginalIMEI(&(self->s), buffer);

    if (!checkError(&(self->s), error, "GetOriginalIMEI")) return NULL;

    return Py_BuildValue("s", buffer);
}

/***********************/
/* GetManufactureMonth */
/***********************/

static char StateMachine_GetManufactureMonth__doc__[] = 
"Gets month when device was manufactured."
;

static PyObject *
StateMachine_GetManufactureMonth(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                buffer[BUFFER_LENGTH];

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetManufactureMonth(&(self->s), buffer);

    if (!checkError(&(self->s), error, "GetManufactureMonth")) return NULL;

    return Py_BuildValue("s", buffer);
}

/******************/
/* GetProductCode */
/******************/

static char StateMachine_GetProductCode__doc__[] = 
"Gets product code of device."
;

static PyObject *
StateMachine_GetProductCode(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                buffer[BUFFER_LENGTH];

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetProductCode(&(self->s), buffer);

    if (!checkError(&(self->s), error, "GetProductCode")) return NULL;

    return Py_BuildValue("s", buffer);
}

/***************/
/* GetHardware */
/***************/

static char StateMachine_GetHardware__doc__[] = 
"Gets hardware information about device."
;

static PyObject *
StateMachine_GetHardware(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                buffer[BUFFER_LENGTH];

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetHardware(&(self->s), buffer);

    if (!checkError(&(self->s), error, "GetHardware")) return NULL;

    return Py_BuildValue("s", buffer);
}

/**********/
/* GetPPM */
/**********/

static char StateMachine_GetPPM__doc__[] = 
"Gets PPM (Post Programmable Memory) from phone."
;

static PyObject *
StateMachine_GetPPM(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                buffer[BUFFER_LENGTH];

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetPPM(&(self->s), buffer);

    if (!checkError(&(self->s), error, "GetPPM")) return NULL;

    return Py_BuildValue("s", buffer);
}

/**************/
/* GetSIMIMSI */
/**************/

static char StateMachine_GetSIMIMSI__doc__[] = 
"Gets SIM IMSI from phone."
;

static PyObject *
StateMachine_GetSIMIMSI(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                buffer[BUFFER_LENGTH];

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetSIMIMSI(&(self->s), buffer);

    if (!checkError(&(self->s), error, "GetSIMIMSI")) return NULL;

    return Py_BuildValue("s", buffer);
}

/***************/
/* GetDateTime */
/***************/

static char StateMachine_GetDateTime__doc__[] = 
"Reads date and time from phone."
;

static PyObject *
StateMachine_GetDateTime(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_DateTime        dt;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetDateTime(&(self->s), &dt);

    if (!checkError(&(self->s), error, "GetDateTime")) return NULL;

    return BuildPythonDateTime(&dt);
}

/***************/
/* SetDateTime */
/***************/

static char StateMachine_SetDateTime__doc__[] = 
"Sets date and time in phone."
;

static PyObject *
StateMachine_SetDateTime(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    PyObject            *pydt;
    GSM_DateTime        dt;
    static char         *kwlist[] = {"Date", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist,&(pydt)))
		return NULL;

    if (!BuildGSMDateTime(pydt, &dt)) return NULL;
            
    error = self->s.Phone.Functions->SetDateTime(&(self->s), &dt);

    if (!checkError(&(self->s), error, "SetDateTime")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/************/
/* GetAlarm */
/************/

static char StateMachine_GetAlarm__doc__[] = 
"Reads alarm set in phone."
;

static PyObject *
StateMachine_GetAlarm(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_Alarm           alarm;
    PyObject            *dt;
    Py_UNICODE          *s;
    PyObject            *result;

    alarm.Location = 1;

	if (!PyArg_ParseTuple(args, "|I", &(alarm.Location)))
		return NULL;

    error = self->s.Phone.Functions->GetAlarm(&(self->s), &alarm);

    if (!checkError(&(self->s), error, "GetAlarm")) return NULL;

    s = strGammuToPython(alarm.Text);
    if (s == NULL) return NULL;

    if (alarm.Repeating) {
        dt = BuildPythonTime(&(alarm.DateTime));
        if (dt == NULL) return NULL;

        result = Py_BuildValue("{s:i,s:O,s:i,s:u}",
                "Location", alarm.Location,
                "DateTime", dt,
                "Repeating", alarm.Repeating,
                "Text", s);
        Py_DECREF(dt);
    } else {
        dt = BuildPythonTime(&(alarm.DateTime));
        if (dt == NULL) return NULL;

        result = Py_BuildValue("{s:i,s:O,s:i,s:u}",
                "Location", alarm.Location,
                "Time", dt,
                "Repeating", alarm.Repeating,
                "Text", s);
        Py_DECREF(dt);
    }
    free(s);
    return result;
}

/************/
/* SetAlarm */
/************/

static char StateMachine_SetAlarm__doc__[] = 
"Sets alarm in phone."
;

static PyObject *
StateMachine_SetAlarm(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_Alarm           alarm;
    PyObject            *pydt;
    Py_UNICODE          *s = NULL;
    char                *gs;
    static char         *kwlist[] = {"DateTime", "Location", "Repeating", "Text", NULL};

    alarm.Location = 1;
    alarm.Text[0] = 0;
    alarm.Repeating = true;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|IIu", kwlist,
                &pydt, &(alarm.Location), &(alarm.Repeating), &(s)))
		return NULL;
    
    if (s != NULL) {
        gs = strPythonToGammu(s);
        if (gs == NULL) return NULL;

        /* FIXME: check length */
        CopyUnicodeString(alarm.Text, gs);
        free(gs);
    }
  
    if (alarm.Repeating) {
        if (!BuildGSMDateTime(pydt, &(alarm.DateTime))) return NULL;
    } else {
        if (!BuildGSMTime(pydt, &(alarm.DateTime))) return NULL;
    }
    
    error = self->s.Phone.Functions->SetAlarm(&(self->s), &alarm);

    if (!checkError(&(self->s), error, "SetAlarm")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* GetLocale */
/*************/

static char StateMachine_GetLocale__doc__[] = 
"Gets locale from phone."
;

static PyObject *
StateMachine_GetLocale(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_Locale          locale;
    char                s[2];

    static char *dtfmts[] = {"", "DDMMYYYY", "MMDDYYYY", "YYYYMMDD"};

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetLocale(&(self->s), &locale);

    if (!checkError(&(self->s), error, "GetLocale")) return NULL;

    s[0] = locale.DateSeparator;
    s[1] = 0;
    
    return Py_BuildValue("{s:s,s:s,s:i}",
            "DateSeparator", s,
            "DateFormat", dtfmts[locale.DateFormat],
            "AMPMTime", locale.AMPMTime);
}

/*************/
/* SetLocale */
/*************/

static char StateMachine_SetLocale__doc__[] = 
"SetLocale(DateSeparator, DateFormat, AMPMTime)\n\n"
"Sets locale of phone.\n"
"DateFormat is one of 'DDMMYYYY', 'MMDDYYYY', 'YYYYMMDD'"
;

static PyObject *
StateMachine_SetLocale(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_Locale          locale;
    char                *ds;
    char                *df;
    int                 ampm;
    static char         *kwlist[] = {"DateSeparator", "DateFormat", "AMPMTime", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ssi", kwlist,
                &ds, &df, &ampm))
		return NULL;
    
    if (strcmp(df, "DDMMYYYY") == 0) locale.DateFormat = GSM_Date_DDMMYYYY;
    else if (strcmp(df, "MMDDYYYY") == 0) locale.DateFormat = GSM_Date_MMDDYYYY;
    else if (strcmp(df, "YYYYMMDD") == 0) locale.DateFormat = GSM_Date_YYYYMMDD;
    else {
        PyErr_Format(PyExc_ValueError, "Bad value for DateFormat: '%s'", df);
        return NULL;
    }
    if (strlen(ds) != 1) {
        PyErr_Format(PyExc_ValueError, "Bad value for DateSeparator: '%s'", ds);
        return NULL;
    }
    locale.DateSeparator = ds[0];
    locale.AMPMTime = ampm;
    
    error = self->s.Phone.Functions->SetLocale(&(self->s), &locale);

    if (!checkError(&(self->s), error, "SetLocale")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/************/
/* PressKey */
/************/

static char StateMachine_PressKey__doc__[] = 
"PressKey(Key, Press = 1)\n\n"
"Emulates key press."
;

static PyObject *
StateMachine_PressKey(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    int                 press = 1;
    char                *key;
    static char         *kwlist[] = {"Key", "Press", NULL};
	GSM_KeyCode	        KeyCode[1];
    int                 Length;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist,
                &key, &press))
		return NULL;
    
    if (strlen(key) != 1) {
        PyErr_Format(PyExc_ValueError, "Bad value for Key: '%s'", key);
        return NULL;
    }

    error = MakeKeySequence(key,  KeyCode, &Length);
    if (!checkError(&(self->s), error, "MakeKeySequence")) return NULL;

    error = self->s.Phone.Functions->PressKey(&(self->s), KeyCode[0], press);

    if (!checkError(&(self->s), error, "PressKey")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********/
/* Reset */
/*********/

static char StateMachine_Reset__doc__[] = 
"Reset(Hard = 0)\n\n"
"Performs phone reset."
;

static PyObject *
StateMachine_Reset(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    int                 hard = 0;
    static char         *kwlist[] = {"Hard", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
                &hard))
		return NULL;

    error = self->s.Phone.Functions->Reset(&(self->s), hard);

    if (!checkError(&(self->s), error, "Reset")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**********************/
/* ResetPhoneSettings */
/**********************/

static char StateMachine_ResetPhoneSettings__doc__[] = 
"ResetPhoneSettings(Type)\n\n"
"Resets phone settings.\n"
"Values for Type: 'PHONE', 'UIF', 'ALL', 'DEV', 'FACTORY'"
;

static PyObject *
StateMachine_ResetPhoneSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    char                *s;
    static char         *kwlist[] = {"Type", NULL};
	GSM_ResetSettingsType Type;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist,
                &s))
		return NULL;

    if (strcmp(s, "PHONE") == 0)        Type = GSM_RESET_PHONESETTINGS;
    else if (strcmp(s, "UIF") == 0)     Type = GSM_RESET_USERINTERFACE;
    else if (strcmp(s, "ALL") == 0)     Type = GSM_RESET_USERINTERFACE_PHONESETTINGS;
    else if (strcmp(s, "DEV") == 0)     Type = GSM_RESET_DEVICE;
    else if (strcmp(s, "FACTORY") == 0) Type = GSM_RESET_FULLFACTORY;
    else {
        PyErr_Format(PyExc_ValueError, "Bad value for Type: '%s'", s);
        return NULL;
    }

    error = self->s.Phone.Functions->ResetPhoneSettings(&(self->s), Type);

    if (!checkError(&(self->s), error, "ResetPhoneSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********************/
/* EnterSecurityCode */
/*********************/

static char StateMachine_EnterSecurityCode__doc__[] = 
"Entres security code (PIN, PUK,...) ."
;

static PyObject *
StateMachine_EnterSecurityCode(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_SecurityCode    Code;
    char                *s;
    char                *code;
    static char         *kwlist[] = {"Type", "Code", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist,
                &s, &code))
		return NULL;

    if (strcmp(s, "PIN") == 0)          Code.Type = GSCT_Pin;
    else if (strcmp(s, "PUK") == 0)     Code.Type = GSCT_Puk;
    else if (strcmp(s, "PIN2") == 0)    Code.Type = GSCT_Pin2;
    else if (strcmp(s, "PUK2") == 0)    Code.Type = GSCT_Puk2;
    else {
        PyErr_Format(PyExc_ValueError, "Bad value for Type: '%s'", s);
        return NULL;
    }

    strncpy(Code.Code, code, GSM_SECURITY_CODE_LEN);

    error = self->s.Phone.Functions->EnterSecurityCode(&(self->s), Code);

    if (!checkError(&(self->s), error, "EnterSecurityCode")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********************/
/* GetSecurityStatus */
/*********************/

static char StateMachine_GetSecurityStatus__doc__[] = 
"Queries whether some security code needs to be entered./"
;

static PyObject *
StateMachine_GetSecurityStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_SecurityCodeType    Status;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetSecurityStatus(&(self->s), &Status);

    if (!checkError(&(self->s), error, "GetSecurityStatus")) return NULL;

    switch (Status) {
	    case GSCT_SecurityCode: return Py_BuildValue("s", "SecurityCode");
	    case GSCT_Pin: return Py_BuildValue("s", "Pin");
	    case GSCT_Pin2: return Py_BuildValue("s", "Pin2");
	    case GSCT_Puk: return Py_BuildValue("s", "Puk");
	    case GSCT_Puk2: return Py_BuildValue("s", "Puk2");
	    case GSCT_None: Py_INCREF(Py_None); return Py_None;
    }
    return Py_None;
}

/********************/
/* GetDisplayStatus */
/********************/

static char StateMachine_GetDisplayStatus__doc__[] = 
"Acquired display status."
;

static PyObject *
StateMachine_GetDisplayStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_DisplayFeatures features;
    PyObject *          list;
    PyObject *          val;
    int                 i;
    char                *buffer;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetDisplayStatus(&(self->s), &features);

    if (!checkError(&(self->s), error, "GetDisplayStatus")) return NULL;

    list = Py_BuildValue("[]");

    for (i = 0; i < features.Number; i++) {
        buffer = "Unknown";
        switch (features.Feature[i]) {
	        case GSM_CallActive: buffer = "CallActive"; break;
	        case GSM_UnreadSMS: buffer = "UnreadSMS"; break;
	        case GSM_VoiceCall: buffer = "VoiceCall"; break;
	        case GSM_FaxCall: buffer = "FaxCall"; break;
	        case GSM_DataCall: buffer = "DataCall"; break;
	        case GSM_KeypadLocked: buffer = "KeypadLocked"; break;
	        case GSM_SMSMemoryFull: buffer = "SMSMemoryFull"; break;
        }
        val = Py_BuildValue("s", buffer);
        if (PyList_Append(list, val) != 0) {
            Py_DECREF(list);
            Py_DECREF(val);
            return NULL;
        }
        Py_DECREF(val);
    }

    return list;
}

/***********************/
/* SetAutoNetworkLogin */
/***********************/

static char StateMachine_SetAutoNetworkLogin__doc__[] = 
"Enables network auto login."
;

static PyObject *
StateMachine_SetAutoNetworkLogin(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->SetAutoNetworkLogin(&(self->s));

    if (!checkError(&(self->s), error, "SetAutoNetworkLogin")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/********************/
/* GetBatteryCharge */
/********************/

static char StateMachine_GetBatteryCharge__doc__[] = 
"Gets information about batery charge and phone charging state."
;

static PyObject *
StateMachine_GetBatteryCharge(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_BatteryCharge   bat;
    char                *buffer;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetBatteryCharge(&(self->s), &bat);

    if (!checkError(&(self->s), error, "GetBatteryCharge")) return NULL;
   
    buffer = "Unknown";
    
    switch (bat.ChargeState) {
        case GSM_BatteryPowered: buffer = "BatteryPowered"; break;
        case GSM_BatteryConnected: buffer = "BatteryConnected"; break;
        case GSM_BatteryNotConnected: buffer = "BatteryNotConnected"; break;
        case GSM_PowerFault: buffer = "PowerFault"; break;
    }

    return Py_BuildValue("{s:i,s:s}", 
            "BatteryPercent", bat.BatteryPercent, 
            "ChargeState", buffer);
}

/********************/
/* GetSignalQuality */
/********************/

static char StateMachine_GetSignalQuality__doc__[] = 
"Reads signal quality (strength and error rate)."
;

static PyObject *
StateMachine_GetSignalQuality(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_SignalQuality   sig;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetSignalQuality(&(self->s), &sig);

    if (!checkError(&(self->s), error, "GetSignalQuality")) return NULL;

    return Py_BuildValue("{s:i,s:i,s:i}"
            "SignalStrength", sig.SignalStrength,
            "SignalPercent", sig.SignalPercent,
            "BitErrorRate", sig.BitErrorRate);
}

/******************/
/* GetNetworkInfo */
/******************/

static char StateMachine_GetNetworkInfo__doc__[] = 
"Gets network information."
;

static PyObject *
StateMachine_GetNetworkInfo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_NetworkInfo	    netinfo;
    char                *buffer;
        
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

    error = self->s.Phone.Functions->GetNetworkInfo(&(self->s), &netinfo);

    if (!checkError(&(self->s), error, "GetNetworkInfo")) return NULL;

    buffer = "Unknown";
    
    switch(netinfo.State) {
	    case GSM_HomeNetwork: buffer = "HomeNetwork"; break;
	    case GSM_RoamingNetwork: buffer = "RoamingNetwork"; break;
	    case GSM_RequestingNetwork: buffer = "RequestingNetwork"; break;
	    case GSM_NoNetwork: buffer = "NoNetwork"; break;
    }

    return Py_BuildValue("{s:s,s:s,s:s,s:s,s:s}",
            "NetworkName", netinfo.NetworkName,
            "State", buffer,
            "NetworkCode", netinfo.NetworkCode,
            "CellID", netinfo.CellID,
            "LAC", netinfo.LAC);
}

/***************/
/* GetCategory */
/***************/

static char StateMachine_GetCategory__doc__[] = 
"Reads category from phone."
;

static PyObject *
StateMachine_GetCategory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_Category	    Category;
    static char         *kwlist[] = {"Type", "Location", NULL};
    char                *s;
    Py_UNICODE          *u;
    PyObject            *o;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", kwlist,
                &s, &(Category.Location)))
		return NULL;

    Category.Type = StringToCategoryType(s);
    if (Category.Type == 0) return NULL;

    error = self->s.Phone.Functions->GetCategory(&(self->s), &Category);

    if (!checkError(&(self->s), error, "GetCategory")) return NULL;
    
    u = strGammuToPython(Category.Name);
    if (u == NULL) return NULL;
    o = Py_BuildValue("u", u);
    free(u);
        
    return o;
}

/*********************/
/* GetCategoryStatus */
/*********************/

static char StateMachine_GetCategoryStatus__doc__[] = 
"Reads category status (number of used entries) from phone."
;

static PyObject *
StateMachine_GetCategoryStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_CategoryStatus  Status;
    static char         *kwlist[] = {"Type", NULL};
    char                *s;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist,
                &s))
		return NULL;
    
    Status.Type = StringToCategoryType(s);
    if (Status.Type == 0) return NULL;

    error = self->s.Phone.Functions->GetCategoryStatus(&(self->s), &Status);

    if (!checkError(&(self->s), error, "GetCategoryStatus")) return NULL;

    return Py_BuildValue("{s:i}", "Used", Status.Used);
}

/*******************/
/* GetMemoryStatus */
/*******************/

static char StateMachine_GetMemoryStatus__doc__[] = 
"Gets memory (phonebooks or calls) status (eg. number of used andfree entries)."
;

static PyObject *
StateMachine_GetMemoryStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_MemoryStatus	Status;
    static char         *kwlist[] = {"Type", NULL};
    char                *s;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist,
                &s))
		return NULL;
    
    Status.MemoryType = StringToMemoryType(s);
    if (Status.MemoryType == 0) return NULL;

    error = self->s.Phone.Functions->GetMemoryStatus(&(self->s), &Status);

    if (!checkError(&(self->s), error, "GetMemoryStatus")) return NULL;

    return Py_BuildValue("{s:i,s:i}", "Used", Status.Used, "Free", Status.Free);
}

/*************/
/* GetMemory */
/*************/

static char StateMachine_GetMemory__doc__[] = 
"Reads entry from memory (phonebooks or calls). Which entry shouldbe read is defined in entry."
;

static PyObject *
StateMachine_GetMemory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_MemoryEntry     entry;
    static char         *kwlist[] = {"Type", "Location", NULL};
    char                *s;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", kwlist,
                &s, &(entry.Location)))
		return NULL;
    
    entry.MemoryType = StringToMemoryType(s);
    if (entry.MemoryType == 0) return NULL;

    error = self->s.Phone.Functions->GetMemory(&(self->s), &entry);

    if (!checkError(&(self->s), error, "GetMemory")) return NULL;

    return BuildPythonMemoryEntry(&entry);
}

/*****************/
/* GetNextMemory */
/*****************/

static char StateMachine_GetNextMemory__doc__[] = 
"Reads entry from memory (phonebooks or calls). Which entry shouldbe read is defined in entry. This can be easily used for reading all entries."
;

static PyObject *
StateMachine_GetNextMemory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_MemoryEntry     entry;
    static char         *kwlist[] = {"Start", "Type", "Location", NULL};
    char                *s = NULL;
    bool                start = false;

    entry.Location = -1;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|isi", kwlist,
                &start, &s, &(entry.Location)))
		return NULL;

    if (s == NULL || (!start && entry.Location == -1)) {
        PyErr_SetString(PyExc_TypeError, "you have to specify Type and (Start or Location)");
        return NULL;
    }

    entry.MemoryType = StringToMemoryType(s);
    if (entry.MemoryType == 0) return NULL;

    error = self->s.Phone.Functions->GetNextMemory(&(self->s), &entry, start);

    if (!checkError(&(self->s), error, "GetNextMemory")) return NULL;

    return BuildPythonMemoryEntry(&entry);
}

/*************/
/* SetMemory */
/*************/

static char StateMachine_SetMemory__doc__[] = 
"Sets memory (phonebooks or calls) entry."
;

static PyObject *
StateMachine_SetMemory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    PyObject            *value;
    GSM_MemoryEntry     entry;
    static char         *kwlist[] = {"Value", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist,
                &PyDict_Type, &(value)))
		return NULL;

    if (!FillGSMMemoryEntry(value, &entry, true)) return NULL;

    error = self->s.Phone.Functions->SetMemory(&(self->s), &entry);

    if (!checkError(&(self->s), error, "SetMemory")) return NULL;

    return PyInt_FromLong(entry.Location);
}

/*************/
/* AddMemory */
/*************/

static char StateMachine_AddMemory__doc__[] = 
"Adds memory (phonebooks or calls) entry."
;

static PyObject *
StateMachine_AddMemory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    PyObject            *value;
    GSM_MemoryEntry     entry;
    static char         *kwlist[] = {"Value", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist,
                &PyDict_Type, &(value)))
		return NULL;

    if (!FillGSMMemoryEntry(value, &entry, false)) return NULL;

    error = self->s.Phone.Functions->AddMemory(&(self->s), &entry);

    if (!checkError(&(self->s), error, "AddMemory")) return NULL;

    return PyInt_FromLong(entry.Location);
}

/****************/
/* DeleteMemory */
/****************/

static char StateMachine_DeleteMemory__doc__[] = 
"Deletes memory (phonebooks or calls) entry."
;

static PyObject *
StateMachine_DeleteMemory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
    GSM_MemoryEntry     entry;
    static char         *kwlist[] = {"Type", "Location", NULL};
    char                *s;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", kwlist,
                &s, &(entry.Location)))
		return NULL;
    
    entry.MemoryType = StringToMemoryType(s);
    if (entry.MemoryType == 0) return NULL;

    error = self->s.Phone.Functions->DeleteMemory(&(self->s), &entry);

    if (!checkError(&(self->s), error, "DeleteMemory")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* DeleteAllMemory */
/*******************/

static char StateMachine_DeleteAllMemory__doc__[] = 
"Deletes all memory (phonebooks or calls) entries of specified type."
;

static PyObject *
StateMachine_DeleteAllMemory(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;
	GSM_MemoryType		MemoryType;
    static char         *kwlist[] = {"Type", NULL};
    char                *s;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist,
                &s))
		return NULL;
    
    MemoryType = StringToMemoryType(s);
    if (MemoryType == 0) return NULL;

    error = self->s.Phone.Functions->DeleteAllMemory(&(self->s), MemoryType);

    if (!checkError(&(self->s), error, "DeleteAllMemory")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

#if 0
/****************/
/* GetSpeedDial */
/****************/

static char StateMachine_GetSpeedDial__doc__[] = 
"Gets speed dial."
;

static PyObject *
StateMachine_GetSpeedDial(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetSpeedDial(&(self->s));

    if (!checkError(&(self->s), error, "GetSpeedDial")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/****************/
/* SetSpeedDial */
/****************/

static char StateMachine_SetSpeedDial__doc__[] = 
"Sets speed dial."
;

static PyObject *
StateMachine_SetSpeedDial(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetSpeedDial(&(self->s));

    if (!checkError(&(self->s), error, "SetSpeedDial")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* GetSMSC */
/***********/

static char StateMachine_GetSMSC__doc__[] = 
"Gets SMS Service Center number and SMS settings."
;

static PyObject *
StateMachine_GetSMSC(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetSMSC(&(self->s));

    if (!checkError(&(self->s), error, "GetSMSC")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* SetSMSC */
/***********/

static char StateMachine_SetSMSC__doc__[] = 
"Sets SMS Service Center number and SMS settings."
;

static PyObject *
StateMachine_SetSMSC(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetSMSC(&(self->s));

    if (!checkError(&(self->s), error, "SetSMSC")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/****************/
/* GetSMSStatus */
/****************/

static char StateMachine_GetSMSStatus__doc__[] = 
"Gets information about SMS memory (read/unread/size of memory forboth SIM and phone)."
;

static PyObject *
StateMachine_GetSMSStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetSMSStatus(&(self->s));

    if (!checkError(&(self->s), error, "GetSMSStatus")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**********/
/* GetSMS */
/**********/

static char StateMachine_GetSMS__doc__[] = 
"Reads SMS message."
;

static PyObject *
StateMachine_GetSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetSMS(&(self->s));

    if (!checkError(&(self->s), error, "GetSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* GetNextSMS */
/**************/

static char StateMachine_GetNextSMS__doc__[] = 
"Reads next (or first if start set) SMS message. This might befaster for some phones than using @ref GetSMS for each message."
;

static PyObject *
StateMachine_GetNextSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetNextSMS(&(self->s));

    if (!checkError(&(self->s), error, "GetNextSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**********/
/* SetSMS */
/**********/

static char StateMachine_SetSMS__doc__[] = 
"Sets SMS."
;

static PyObject *
StateMachine_SetSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetSMS(&(self->s));

    if (!checkError(&(self->s), error, "SetSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**********/
/* AddSMS */
/**********/

static char StateMachine_AddSMS__doc__[] = 
"Adds SMS to specified folder."
;

static PyObject *
StateMachine_AddSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AddSMS(&(self->s));

    if (!checkError(&(self->s), error, "AddSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* DeleteSMS */
/*************/

static char StateMachine_DeleteSMS__doc__[] = 
"Deletes SMS."
;

static PyObject *
StateMachine_DeleteSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteSMS(&(self->s));

    if (!checkError(&(self->s), error, "DeleteSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* SendSMS */
/***********/

static char StateMachine_SendSMS__doc__[] = 
"Sends SMS."
;

static PyObject *
StateMachine_SendSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SendSMS(&(self->s));

    if (!checkError(&(self->s), error, "SendSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* SetIncomingSMS */
/******************/

static char StateMachine_SetIncomingSMS__doc__[] = 
"Enable/disable notification on incoming SMS."
;

static PyObject *
StateMachine_SetIncomingSMS(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetIncomingSMS(&(self->s));

    if (!checkError(&(self->s), error, "SetIncomingSMS")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************/
/* SetIncomingCB */
/*****************/

static char StateMachine_SetIncomingCB__doc__[] = 
"Gets network information from phone."
;

static PyObject *
StateMachine_SetIncomingCB(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetIncomingCB(&(self->s));

    if (!checkError(&(self->s), error, "SetIncomingCB")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************/
/* GetSMSFolders */
/*****************/

static char StateMachine_GetSMSFolders__doc__[] = 
"Returns SMS folders information."
;

static PyObject *
StateMachine_GetSMSFolders(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetSMSFolders(&(self->s));

    if (!checkError(&(self->s), error, "GetSMSFolders")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/****************/
/* AddSMSFolder */
/****************/

static char StateMachine_AddSMSFolder__doc__[] = 
"Creates SMS folder."
;

static PyObject *
StateMachine_AddSMSFolder(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AddSMSFolder(&(self->s));

    if (!checkError(&(self->s), error, "AddSMSFolder")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* DeleteSMSFolder */
/*******************/

static char StateMachine_DeleteSMSFolder__doc__[] = 
"Deletes SMS folder."
;

static PyObject *
StateMachine_DeleteSMSFolder(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteSMSFolder(&(self->s));

    if (!checkError(&(self->s), error, "DeleteSMSFolder")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* DialVoice */
/*************/

static char StateMachine_DialVoice__doc__[] = 
"Dials number and starts voice call."
;

static PyObject *
StateMachine_DialVoice(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DialVoice(&(self->s));

    if (!checkError(&(self->s), error, "DialVoice")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* AnswerCall */
/**************/

static char StateMachine_AnswerCall__doc__[] = 
"Accept current incoming call."
;

static PyObject *
StateMachine_AnswerCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AnswerCall(&(self->s));

    if (!checkError(&(self->s), error, "AnswerCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* CancelCall */
/**************/

static char StateMachine_CancelCall__doc__[] = 
"Deny current incoming call."
;

static PyObject *
StateMachine_CancelCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->CancelCall(&(self->s));

    if (!checkError(&(self->s), error, "CancelCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/************/
/* HoldCall */
/************/

static char StateMachine_HoldCall__doc__[] = 
"Holds call."
;

static PyObject *
StateMachine_HoldCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->HoldCall(&(self->s));

    if (!checkError(&(self->s), error, "HoldCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* UnholdCall */
/**************/

static char StateMachine_UnholdCall__doc__[] = 
"Unholds call."
;

static PyObject *
StateMachine_UnholdCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->UnholdCall(&(self->s));

    if (!checkError(&(self->s), error, "UnholdCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* ConferenceCall */
/******************/

static char StateMachine_ConferenceCall__doc__[] = 
"Initiates conference call."
;

static PyObject *
StateMachine_ConferenceCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->ConferenceCall(&(self->s));

    if (!checkError(&(self->s), error, "ConferenceCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* SplitCall */
/*************/

static char StateMachine_SplitCall__doc__[] = 
"Splits call."
;

static PyObject *
StateMachine_SplitCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SplitCall(&(self->s));

    if (!checkError(&(self->s), error, "SplitCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/****************/
/* TransferCall */
/****************/

static char StateMachine_TransferCall__doc__[] = 
"Transfers call."
;

static PyObject *
StateMachine_TransferCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->TransferCall(&(self->s));

    if (!checkError(&(self->s), error, "TransferCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* SwitchCall */
/**************/

static char StateMachine_SwitchCall__doc__[] = 
"Switches call."
;

static PyObject *
StateMachine_SwitchCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SwitchCall(&(self->s));

    if (!checkError(&(self->s), error, "SwitchCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************/
/* GetCallDivert */
/*****************/

static char StateMachine_GetCallDivert__doc__[] = 
"Gets call diverts."
;

static PyObject *
StateMachine_GetCallDivert(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetCallDivert(&(self->s));

    if (!checkError(&(self->s), error, "GetCallDivert")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************/
/* SetCallDivert */
/*****************/

static char StateMachine_SetCallDivert__doc__[] = 
"Sets call diverts."
;

static PyObject *
StateMachine_SetCallDivert(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetCallDivert(&(self->s));

    if (!checkError(&(self->s), error, "SetCallDivert")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/********************/
/* CancelAllDiverts */
/********************/

static char StateMachine_CancelAllDiverts__doc__[] = 
"Cancels all diverts."
;

static PyObject *
StateMachine_CancelAllDiverts(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->CancelAllDiverts(&(self->s));

    if (!checkError(&(self->s), error, "CancelAllDiverts")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* SetIncomingCall */
/*******************/

static char StateMachine_SetIncomingCall__doc__[] = 
"Activates/deactivates noticing about incoming calls."
;

static PyObject *
StateMachine_SetIncomingCall(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetIncomingCall(&(self->s));

    if (!checkError(&(self->s), error, "SetIncomingCall")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* SetIncomingUSSD */
/*******************/

static char StateMachine_SetIncomingUSSD__doc__[] = 
"Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services)."
;

static PyObject *
StateMachine_SetIncomingUSSD(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetIncomingUSSD(&(self->s));

    if (!checkError(&(self->s), error, "SetIncomingUSSD")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/************/
/* SendDTMF */
/************/

static char StateMachine_SendDTMF__doc__[] = 
"Sends DTMF (Dual Tone Multi Frequency) tone."
;

static PyObject *
StateMachine_SendDTMF(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SendDTMF(&(self->s));

    if (!checkError(&(self->s), error, "SendDTMF")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* GetRingtone */
/***************/

static char StateMachine_GetRingtone__doc__[] = 
"Gets ringtone from phone."
;

static PyObject *
StateMachine_GetRingtone(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetRingtone(&(self->s));

    if (!checkError(&(self->s), error, "GetRingtone")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* SetRingtone */
/***************/

static char StateMachine_SetRingtone__doc__[] = 
"Sets ringtone in phone."
;

static PyObject *
StateMachine_SetRingtone(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetRingtone(&(self->s));

    if (!checkError(&(self->s), error, "SetRingtone")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/********************/
/* GetRingtonesInfo */
/********************/

static char StateMachine_GetRingtonesInfo__doc__[] = 
"Acquires ringtone informaiton."
;

static PyObject *
StateMachine_GetRingtonesInfo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetRingtonesInfo(&(self->s));

    if (!checkError(&(self->s), error, "GetRingtonesInfo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********************/
/* DeleteUserRingtones */
/***********************/

static char StateMachine_DeleteUserRingtones__doc__[] = 
"Deletes user defined ringtones from phone."
;

static PyObject *
StateMachine_DeleteUserRingtones(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteUserRingtones(&(self->s));

    if (!checkError(&(self->s), error, "DeleteUserRingtones")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/************/
/* PlayTone */
/************/

static char StateMachine_PlayTone__doc__[] = 
"Plays tone."
;

static PyObject *
StateMachine_PlayTone(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->PlayTone(&(self->s));

    if (!checkError(&(self->s), error, "PlayTone")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* GetWAPBookmark */
/******************/

static char StateMachine_GetWAPBookmark__doc__[] = 
"Reads WAP bookmark."
;

static PyObject *
StateMachine_GetWAPBookmark(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetWAPBookmark(&(self->s));

    if (!checkError(&(self->s), error, "GetWAPBookmark")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* SetWAPBookmark */
/******************/

static char StateMachine_SetWAPBookmark__doc__[] = 
"Sets WAP bookmark."
;

static PyObject *
StateMachine_SetWAPBookmark(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetWAPBookmark(&(self->s));

    if (!checkError(&(self->s), error, "SetWAPBookmark")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********************/
/* DeleteWAPBookmark */
/*********************/

static char StateMachine_DeleteWAPBookmark__doc__[] = 
"Deletes WAP bookmark."
;

static PyObject *
StateMachine_DeleteWAPBookmark(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteWAPBookmark(&(self->s));

    if (!checkError(&(self->s), error, "DeleteWAPBookmark")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* GetWAPSettings */
/******************/

static char StateMachine_GetWAPSettings__doc__[] = 
"Acquires WAP settings."
;

static PyObject *
StateMachine_GetWAPSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetWAPSettings(&(self->s));

    if (!checkError(&(self->s), error, "GetWAPSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* SetWAPSettings */
/******************/

static char StateMachine_SetWAPSettings__doc__[] = 
"Changes WAP settings."
;

static PyObject *
StateMachine_SetWAPSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetWAPSettings(&(self->s));

    if (!checkError(&(self->s), error, "SetWAPSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* GetMMSSettings */
/******************/

static char StateMachine_GetMMSSettings__doc__[] = 
"Acquires MMS settings."
;

static PyObject *
StateMachine_GetMMSSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetMMSSettings(&(self->s));

    if (!checkError(&(self->s), error, "GetMMSSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* SetMMSSettings */
/******************/

static char StateMachine_SetMMSSettings__doc__[] = 
"Changes MMS settings."
;

static PyObject *
StateMachine_SetMMSSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetMMSSettings(&(self->s));

    if (!checkError(&(self->s), error, "SetMMSSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* GetBitmap */
/*************/

static char StateMachine_GetBitmap__doc__[] = 
"Gets bitmap."
;

static PyObject *
StateMachine_GetBitmap(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetBitmap(&(self->s));

    if (!checkError(&(self->s), error, "GetBitmap")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* SetBitmap */
/*************/

static char StateMachine_SetBitmap__doc__[] = 
"Sets bitmap."
;

static PyObject *
StateMachine_SetBitmap(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetBitmap(&(self->s));

    if (!checkError(&(self->s), error, "SetBitmap")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************/
/* GetToDoStatus */
/*****************/

static char StateMachine_GetToDoStatus__doc__[] = 
"Gets status of ToDos (count of used entries)."
;

static PyObject *
StateMachine_GetToDoStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetToDoStatus(&(self->s));

    if (!checkError(&(self->s), error, "GetToDoStatus")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* GetToDo */
/***********/

static char StateMachine_GetToDo__doc__[] = 
"Reads ToDo from phone."
;

static PyObject *
StateMachine_GetToDo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetToDo(&(self->s));

    if (!checkError(&(self->s), error, "GetToDo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* GetNextToDo */
/***************/

static char StateMachine_GetNextToDo__doc__[] = 
"Reads ToDo from phone."
;

static PyObject *
StateMachine_GetNextToDo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetNextToDo(&(self->s));

    if (!checkError(&(self->s), error, "GetNextToDo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* SetToDo */
/***********/

static char StateMachine_SetToDo__doc__[] = 
"Sets ToDo in phone."
;

static PyObject *
StateMachine_SetToDo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetToDo(&(self->s));

    if (!checkError(&(self->s), error, "SetToDo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* AddToDo */
/***********/

static char StateMachine_AddToDo__doc__[] = 
"Adds ToDo in phone."
;

static PyObject *
StateMachine_AddToDo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AddToDo(&(self->s));

    if (!checkError(&(self->s), error, "AddToDo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* DeleteToDo */
/**************/

static char StateMachine_DeleteToDo__doc__[] = 
"Deletes ToDo entry in phone."
;

static PyObject *
StateMachine_DeleteToDo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteToDo(&(self->s));

    if (!checkError(&(self->s), error, "DeleteToDo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************/
/* DeleteAllToDo */
/*****************/

static char StateMachine_DeleteAllToDo__doc__[] = 
"Deletes all todo entries in phone."
;

static PyObject *
StateMachine_DeleteAllToDo(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteAllToDo(&(self->s));

    if (!checkError(&(self->s), error, "DeleteAllToDo")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********************/
/* GetCalendarStatus */
/*********************/

static char StateMachine_GetCalendarStatus__doc__[] = 
"Retrieves calendar status (number of used entries)."
;

static PyObject *
StateMachine_GetCalendarStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetCalendarStatus(&(self->s));

    if (!checkError(&(self->s), error, "GetCalendarStatus")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* GetCalendar */
/***************/

static char StateMachine_GetCalendar__doc__[] = 
"Retrieves calendar entry."
;

static PyObject *
StateMachine_GetCalendar(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetCalendar(&(self->s));

    if (!checkError(&(self->s), error, "GetCalendar")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* GetNextCalendar */
/*******************/

static char StateMachine_GetNextCalendar__doc__[] = 
"Retrieves calendar entry. This is useful for continuous reading of allcalendar entries."
;

static PyObject *
StateMachine_GetNextCalendar(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetNextCalendar(&(self->s));

    if (!checkError(&(self->s), error, "GetNextCalendar")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* SetCalendar */
/***************/

static char StateMachine_SetCalendar__doc__[] = 
"Sets calendar entry"
;

static PyObject *
StateMachine_SetCalendar(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetCalendar(&(self->s));

    if (!checkError(&(self->s), error, "SetCalendar")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* AddCalendar */
/***************/

static char StateMachine_AddCalendar__doc__[] = 
"Adds calendar entry."
;

static PyObject *
StateMachine_AddCalendar(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AddCalendar(&(self->s));

    if (!checkError(&(self->s), error, "AddCalendar")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/******************/
/* DeleteCalendar */
/******************/

static char StateMachine_DeleteCalendar__doc__[] = 
"Deletes calendar entry."
;

static PyObject *
StateMachine_DeleteCalendar(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteCalendar(&(self->s));

    if (!checkError(&(self->s), error, "DeleteCalendar")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********************/
/* DeleteAllCalendar */
/*********************/

static char StateMachine_DeleteAllCalendar__doc__[] = 
"Deletes all calendar entries."
;

static PyObject *
StateMachine_DeleteAllCalendar(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteAllCalendar(&(self->s));

    if (!checkError(&(self->s), error, "DeleteAllCalendar")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********************/
/* GetCalendarSettings */
/***********************/

static char StateMachine_GetCalendarSettings__doc__[] = 
"Reads calendar settings."
;

static PyObject *
StateMachine_GetCalendarSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetCalendarSettings(&(self->s));

    if (!checkError(&(self->s), error, "GetCalendarSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********************/
/* SetCalendarSettings */
/***********************/

static char StateMachine_SetCalendarSettings__doc__[] = 
"Sets calendar settings."
;

static PyObject *
StateMachine_SetCalendarSettings(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetCalendarSettings(&(self->s));

    if (!checkError(&(self->s), error, "SetCalendarSettings")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********/
/* GetNote */
/***********/

static char StateMachine_GetNote__doc__[] = 
"Gets note."
;

static PyObject *
StateMachine_GetNote(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetNote(&(self->s));

    if (!checkError(&(self->s), error, "GetNote")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* GetProfile */
/**************/

static char StateMachine_GetProfile__doc__[] = 
"Reads profile."
;

static PyObject *
StateMachine_GetProfile(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetProfile(&(self->s));

    if (!checkError(&(self->s), error, "GetProfile")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* SetProfile */
/**************/

static char StateMachine_SetProfile__doc__[] = 
"Updates profile."
;

static PyObject *
StateMachine_SetProfile(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetProfile(&(self->s));

    if (!checkError(&(self->s), error, "SetProfile")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/****************/
/* GetFMStation */
/****************/

static char StateMachine_GetFMStation__doc__[] = 
"Reads FM station."
;

static PyObject *
StateMachine_GetFMStation(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetFMStation(&(self->s));

    if (!checkError(&(self->s), error, "GetFMStation")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/****************/
/* SetFMStation */
/****************/

static char StateMachine_SetFMStation__doc__[] = 
"Sets FM station."
;

static PyObject *
StateMachine_SetFMStation(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetFMStation(&(self->s));

    if (!checkError(&(self->s), error, "SetFMStation")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*******************/
/* ClearFMStations */
/*******************/

static char StateMachine_ClearFMStations__doc__[] = 
"Clears defined FM stations."
;

static PyObject *
StateMachine_ClearFMStations(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->ClearFMStations(&(self->s));

    if (!checkError(&(self->s), error, "ClearFMStations")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*********************/
/* GetNextFileFolder */
/*********************/

static char StateMachine_GetNextFileFolder__doc__[] = 
"Gets next filename from filesystem."
;

static PyObject *
StateMachine_GetNextFileFolder(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetNextFileFolder(&(self->s));

    if (!checkError(&(self->s), error, "GetNextFileFolder")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* GetFilePart */
/***************/

static char StateMachine_GetFilePart__doc__[] = 
"Gets file part from filesystem."
;

static PyObject *
StateMachine_GetFilePart(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetFilePart(&(self->s));

    if (!checkError(&(self->s), error, "GetFilePart")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***************/
/* AddFilePart */
/***************/

static char StateMachine_AddFilePart__doc__[] = 
"Adds file part to filesystem."
;

static PyObject *
StateMachine_AddFilePart(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AddFilePart(&(self->s));

    if (!checkError(&(self->s), error, "AddFilePart")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/***********************/
/* GetFileSystemStatus */
/***********************/

static char StateMachine_GetFileSystemStatus__doc__[] = 
"Acquires filesystem status."
;

static PyObject *
StateMachine_GetFileSystemStatus(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetFileSystemStatus(&(self->s));

    if (!checkError(&(self->s), error, "GetFileSystemStatus")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**************/
/* DeleteFile */
/**************/

static char StateMachine_DeleteFile__doc__[] = 
"Deletes file from filessytem."
;

static PyObject *
StateMachine_DeleteFile(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->DeleteFile(&(self->s));

    if (!checkError(&(self->s), error, "DeleteFile")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*************/
/* AddFolder */
/*************/

static char StateMachine_AddFolder__doc__[] = 
"Adds folder to filesystem."
;

static PyObject *
StateMachine_AddFolder(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->AddFolder(&(self->s));

    if (!checkError(&(self->s), error, "AddFolder")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**********************/
/* GetGPRSAccessPoint */
/**********************/

static char StateMachine_GetGPRSAccessPoint__doc__[] = 
"Gets GPRS access point."
;

static PyObject *
StateMachine_GetGPRSAccessPoint(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->GetGPRSAccessPoint(&(self->s));

    if (!checkError(&(self->s), error, "GetGPRSAccessPoint")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/**********************/
/* SetGPRSAccessPoint */
/**********************/

static char StateMachine_SetGPRSAccessPoint__doc__[] = 
"Sets GPRS access point."
;

static PyObject *
StateMachine_SetGPRSAccessPoint(StateMachineObject *self, PyObject *args, PyObject *kwds) {
    GSM_Error           error;

    error = self->s.Phone.Functions->SetGPRSAccessPoint(&(self->s));

    if (!checkError(&(self->s), error, "SetGPRSAccessPoint")) return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}
#endif

static struct PyMethodDef StateMachine_methods[] = {
	{"ReadConfig",	(PyCFunction)StateMachine_ReadConfig,	METH_VARARGS|METH_KEYWORDS,	StateMachine_ReadConfig__doc__},
    {"Init",	(PyCFunction)StateMachine_Init,	METH_VARARGS|METH_KEYWORDS,	StateMachine_Init__doc__},
    {"Terminate",	(PyCFunction)StateMachine_Terminate,	METH_VARARGS|METH_KEYWORDS,	StateMachine_Terminate__doc__},
    {"ShowStartInfo",	(PyCFunction)StateMachine_ShowStartInfo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_ShowStartInfo__doc__},
    {"GetManufacturer",	(PyCFunction)StateMachine_GetManufacturer,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetManufacturer__doc__},
    {"GetModel",	(PyCFunction)StateMachine_GetModel,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetModel__doc__},
    {"GetFirmware",	(PyCFunction)StateMachine_GetFirmware,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetFirmware__doc__},
    {"GetIMEI",	(PyCFunction)StateMachine_GetIMEI,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetIMEI__doc__},
    {"GetOriginalIMEI",	(PyCFunction)StateMachine_GetOriginalIMEI,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetOriginalIMEI__doc__},
    {"GetManufactureMonth",	(PyCFunction)StateMachine_GetManufactureMonth,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetManufactureMonth__doc__},
    {"GetProductCode",	(PyCFunction)StateMachine_GetProductCode,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetProductCode__doc__},
    {"GetHardware",	(PyCFunction)StateMachine_GetHardware,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetHardware__doc__},
    {"GetPPM",	(PyCFunction)StateMachine_GetPPM,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetPPM__doc__},
    {"GetSIMIMSI",	(PyCFunction)StateMachine_GetSIMIMSI,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSIMIMSI__doc__},
    {"GetDateTime",	(PyCFunction)StateMachine_GetDateTime,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetDateTime__doc__},
    {"SetDateTime",	(PyCFunction)StateMachine_SetDateTime,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetDateTime__doc__},
    {"GetAlarm",	(PyCFunction)StateMachine_GetAlarm,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetAlarm__doc__},
    {"SetAlarm",	(PyCFunction)StateMachine_SetAlarm,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetAlarm__doc__},
    {"GetLocale",	(PyCFunction)StateMachine_GetLocale,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetLocale__doc__},
    {"SetLocale",	(PyCFunction)StateMachine_SetLocale,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetLocale__doc__},
    {"PressKey",	(PyCFunction)StateMachine_PressKey,	METH_VARARGS|METH_KEYWORDS,	StateMachine_PressKey__doc__},
    {"Reset",	(PyCFunction)StateMachine_Reset,	METH_VARARGS|METH_KEYWORDS,	StateMachine_Reset__doc__},
    {"ResetPhoneSettings",	(PyCFunction)StateMachine_ResetPhoneSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_ResetPhoneSettings__doc__},
    {"EnterSecurityCode",	(PyCFunction)StateMachine_EnterSecurityCode,	METH_VARARGS|METH_KEYWORDS,	StateMachine_EnterSecurityCode__doc__},
    {"GetSecurityStatus",	(PyCFunction)StateMachine_GetSecurityStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSecurityStatus__doc__},
    {"GetDisplayStatus",	(PyCFunction)StateMachine_GetDisplayStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetDisplayStatus__doc__},
    {"SetAutoNetworkLogin",	(PyCFunction)StateMachine_SetAutoNetworkLogin,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetAutoNetworkLogin__doc__},
    {"GetBatteryCharge",	(PyCFunction)StateMachine_GetBatteryCharge,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetBatteryCharge__doc__},
    {"GetSignalQuality",	(PyCFunction)StateMachine_GetSignalQuality,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSignalQuality__doc__},
    {"GetNetworkInfo",	(PyCFunction)StateMachine_GetNetworkInfo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNetworkInfo__doc__},
    {"GetCategory",	(PyCFunction)StateMachine_GetCategory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetCategory__doc__},
    {"GetCategoryStatus",	(PyCFunction)StateMachine_GetCategoryStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetCategoryStatus__doc__},
    {"GetMemoryStatus",	(PyCFunction)StateMachine_GetMemoryStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetMemoryStatus__doc__},
    {"GetMemory",	(PyCFunction)StateMachine_GetMemory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetMemory__doc__},
    {"GetNextMemory",	(PyCFunction)StateMachine_GetNextMemory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNextMemory__doc__},
    {"SetMemory",	(PyCFunction)StateMachine_SetMemory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetMemory__doc__},
    {"AddMemory",	(PyCFunction)StateMachine_AddMemory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddMemory__doc__},
    {"DeleteMemory",	(PyCFunction)StateMachine_DeleteMemory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteMemory__doc__},
    {"DeleteAllMemory",	(PyCFunction)StateMachine_DeleteAllMemory,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteAllMemory__doc__},
#if 0
    {"GetSpeedDial",	(PyCFunction)StateMachine_GetSpeedDial,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSpeedDial__doc__},
    {"SetSpeedDial",	(PyCFunction)StateMachine_SetSpeedDial,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetSpeedDial__doc__},
    {"GetSMSC",	(PyCFunction)StateMachine_GetSMSC,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSMSC__doc__},
    {"SetSMSC",	(PyCFunction)StateMachine_SetSMSC,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetSMSC__doc__},
    {"GetSMSStatus",	(PyCFunction)StateMachine_GetSMSStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSMSStatus__doc__},
    {"GetSMS",	(PyCFunction)StateMachine_GetSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSMS__doc__},
    {"GetNextSMS",	(PyCFunction)StateMachine_GetNextSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNextSMS__doc__},
    {"SetSMS",	(PyCFunction)StateMachine_SetSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetSMS__doc__},
    {"AddSMS",	(PyCFunction)StateMachine_AddSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddSMS__doc__},
    {"DeleteSMS",	(PyCFunction)StateMachine_DeleteSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteSMS__doc__},
    {"SendSMS",	(PyCFunction)StateMachine_SendSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SendSMS__doc__},
    {"SetIncomingSMS",	(PyCFunction)StateMachine_SetIncomingSMS,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetIncomingSMS__doc__},
    {"SetIncomingCB",	(PyCFunction)StateMachine_SetIncomingCB,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetIncomingCB__doc__},
    {"GetSMSFolders",	(PyCFunction)StateMachine_GetSMSFolders,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetSMSFolders__doc__},
    {"AddSMSFolder",	(PyCFunction)StateMachine_AddSMSFolder,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddSMSFolder__doc__},
    {"DeleteSMSFolder",	(PyCFunction)StateMachine_DeleteSMSFolder,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteSMSFolder__doc__},
    {"DialVoice",	(PyCFunction)StateMachine_DialVoice,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DialVoice__doc__},
    {"AnswerCall",	(PyCFunction)StateMachine_AnswerCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AnswerCall__doc__},
    {"CancelCall",	(PyCFunction)StateMachine_CancelCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_CancelCall__doc__},
    {"HoldCall",	(PyCFunction)StateMachine_HoldCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_HoldCall__doc__},
    {"UnholdCall",	(PyCFunction)StateMachine_UnholdCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_UnholdCall__doc__},
    {"ConferenceCall",	(PyCFunction)StateMachine_ConferenceCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_ConferenceCall__doc__},
    {"SplitCall",	(PyCFunction)StateMachine_SplitCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SplitCall__doc__},
    {"TransferCall",	(PyCFunction)StateMachine_TransferCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_TransferCall__doc__},
    {"SwitchCall",	(PyCFunction)StateMachine_SwitchCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SwitchCall__doc__},
    {"GetCallDivert",	(PyCFunction)StateMachine_GetCallDivert,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetCallDivert__doc__},
    {"SetCallDivert",	(PyCFunction)StateMachine_SetCallDivert,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetCallDivert__doc__},
    {"CancelAllDiverts",	(PyCFunction)StateMachine_CancelAllDiverts,	METH_VARARGS|METH_KEYWORDS,	StateMachine_CancelAllDiverts__doc__},
    {"SetIncomingCall",	(PyCFunction)StateMachine_SetIncomingCall,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetIncomingCall__doc__},
    {"SetIncomingUSSD",	(PyCFunction)StateMachine_SetIncomingUSSD,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetIncomingUSSD__doc__},
    {"SendDTMF",	(PyCFunction)StateMachine_SendDTMF,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SendDTMF__doc__},
    {"GetRingtone",	(PyCFunction)StateMachine_GetRingtone,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetRingtone__doc__},
    {"SetRingtone",	(PyCFunction)StateMachine_SetRingtone,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetRingtone__doc__},
    {"GetRingtonesInfo",	(PyCFunction)StateMachine_GetRingtonesInfo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetRingtonesInfo__doc__},
    {"DeleteUserRingtones",	(PyCFunction)StateMachine_DeleteUserRingtones,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteUserRingtones__doc__},
    {"PlayTone",	(PyCFunction)StateMachine_PlayTone,	METH_VARARGS|METH_KEYWORDS,	StateMachine_PlayTone__doc__},
    {"GetWAPBookmark",	(PyCFunction)StateMachine_GetWAPBookmark,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetWAPBookmark__doc__},
    {"SetWAPBookmark",	(PyCFunction)StateMachine_SetWAPBookmark,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetWAPBookmark__doc__},
    {"DeleteWAPBookmark",	(PyCFunction)StateMachine_DeleteWAPBookmark,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteWAPBookmark__doc__},
    {"GetWAPSettings",	(PyCFunction)StateMachine_GetWAPSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetWAPSettings__doc__},
    {"SetWAPSettings",	(PyCFunction)StateMachine_SetWAPSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetWAPSettings__doc__},
    {"GetMMSSettings",	(PyCFunction)StateMachine_GetMMSSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetMMSSettings__doc__},
    {"SetMMSSettings",	(PyCFunction)StateMachine_SetMMSSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetMMSSettings__doc__},
    {"GetBitmap",	(PyCFunction)StateMachine_GetBitmap,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetBitmap__doc__},
    {"SetBitmap",	(PyCFunction)StateMachine_SetBitmap,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetBitmap__doc__},
    {"GetToDoStatus",	(PyCFunction)StateMachine_GetToDoStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetToDoStatus__doc__},
    {"GetToDo",	(PyCFunction)StateMachine_GetToDo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetToDo__doc__},
    {"GetNextToDo",	(PyCFunction)StateMachine_GetNextToDo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNextToDo__doc__},
    {"SetToDo",	(PyCFunction)StateMachine_SetToDo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetToDo__doc__},
    {"AddToDo",	(PyCFunction)StateMachine_AddToDo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddToDo__doc__},
    {"DeleteToDo",	(PyCFunction)StateMachine_DeleteToDo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteToDo__doc__},
    {"DeleteAllToDo",	(PyCFunction)StateMachine_DeleteAllToDo,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteAllToDo__doc__},
    {"GetCalendarStatus",	(PyCFunction)StateMachine_GetCalendarStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetCalendarStatus__doc__},
    {"GetCalendar",	(PyCFunction)StateMachine_GetCalendar,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetCalendar__doc__},
    {"GetNextCalendar",	(PyCFunction)StateMachine_GetNextCalendar,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNextCalendar__doc__},
    {"SetCalendar",	(PyCFunction)StateMachine_SetCalendar,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetCalendar__doc__},
    {"AddCalendar",	(PyCFunction)StateMachine_AddCalendar,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddCalendar__doc__},
    {"DeleteCalendar",	(PyCFunction)StateMachine_DeleteCalendar,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteCalendar__doc__},
    {"DeleteAllCalendar",	(PyCFunction)StateMachine_DeleteAllCalendar,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteAllCalendar__doc__},
    {"GetCalendarSettings",	(PyCFunction)StateMachine_GetCalendarSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetCalendarSettings__doc__},
    {"SetCalendarSettings",	(PyCFunction)StateMachine_SetCalendarSettings,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetCalendarSettings__doc__},
    {"GetNote",	(PyCFunction)StateMachine_GetNote,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNote__doc__},
    {"GetProfile",	(PyCFunction)StateMachine_GetProfile,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetProfile__doc__},
    {"SetProfile",	(PyCFunction)StateMachine_SetProfile,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetProfile__doc__},
    {"GetFMStation",	(PyCFunction)StateMachine_GetFMStation,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetFMStation__doc__},
    {"SetFMStation",	(PyCFunction)StateMachine_SetFMStation,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetFMStation__doc__},
    {"ClearFMStations",	(PyCFunction)StateMachine_ClearFMStations,	METH_VARARGS|METH_KEYWORDS,	StateMachine_ClearFMStations__doc__},
    {"GetNextFileFolder",	(PyCFunction)StateMachine_GetNextFileFolder,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetNextFileFolder__doc__},
    {"GetFilePart",	(PyCFunction)StateMachine_GetFilePart,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetFilePart__doc__},
    {"AddFilePart",	(PyCFunction)StateMachine_AddFilePart,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddFilePart__doc__},
    {"GetFileSystemStatus",	(PyCFunction)StateMachine_GetFileSystemStatus,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetFileSystemStatus__doc__},
    {"DeleteFile",	(PyCFunction)StateMachine_DeleteFile,	METH_VARARGS|METH_KEYWORDS,	StateMachine_DeleteFile__doc__},
    {"AddFolder",	(PyCFunction)StateMachine_AddFolder,	METH_VARARGS|METH_KEYWORDS,	StateMachine_AddFolder__doc__},
    {"GetGPRSAccessPoint",	(PyCFunction)StateMachine_GetGPRSAccessPoint,	METH_VARARGS|METH_KEYWORDS,	StateMachine_GetGPRSAccessPoint__doc__},
    {"SetGPRSAccessPoint",	(PyCFunction)StateMachine_SetGPRSAccessPoint,	METH_VARARGS|METH_KEYWORDS,	StateMachine_SetGPRSAccessPoint__doc__},
#endif
 
	{NULL,		NULL}		/* sentinel */
};

/* ---------- */


static PyObject *
StateMachine_getattr(StateMachineObject *self, char *name)
{
	/* XXXX Add your own getattr code here */
	return Py_FindMethod(StateMachine_methods, (PyObject *)self, name);
}

static int
StateMachine_setattr(StateMachineObject *self, char *name, PyObject *v)
{
	/* Set attribute 'name' to value 'v'. v==NULL means delete */
	
	/* XXXX Add your own setattr code here */
	return -1;
}

#if 0
static int
StateMachine_compare(StateMachineObject *v, StateMachineObject *w)
{
	/* XXXX Compare objects and return -1, 0 or 1 */
}

static PyObject *
StateMachine_repr(StateMachineObject *self)
{
	PyObject *s;

	/* XXXX Add code here to put self into s */
	return s;
}

static PyObject *
StateMachine_str(StateMachineObject *self)
{
	PyObject *s;

	/* XXXX Add code here to put self into s */
	return s;
}
#endif

static void
StateMachine_dealloc(StateMachineObject * obj)
{
    if (obj->s.opened) GSM_TerminateConnection(&(obj->s));
    obj->ob_type->tp_free(obj);
}

static char StateMachineType__doc__[] = 
"StateMachine object, that is used for communication with phone."
;

static PyTypeObject StateMachineType = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/*ob_size*/
	"StateMachine",			/*tp_name*/
	sizeof(StateMachineObject),		/*tp_basicsize*/
	0,				/*tp_itemsize*/
	/* methods */
	(destructor)StateMachine_dealloc,	/*tp_dealloc*/
	(printfunc)0,		/*tp_print*/
	(getattrfunc)StateMachine_getattr,	/*tp_getattr*/
	(setattrfunc)StateMachine_setattr,	/*tp_setattr*/
    0,
//	(cmpfunc)StateMachine_compare,		/*tp_compare*/
    0,
//	(reprfunc)StateMachine_repr,		/*tp_repr*/
	0,			/*tp_as_number*/
	0,		/*tp_as_sequence*/
	0,		/*tp_as_mapping*/
	(hashfunc)0,		/*tp_hash*/
	(ternaryfunc)0,		/*tp_call*/
    0,
//	(reprfunc)StateMachine_str,		/*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
	StateMachineType__doc__, /* Documentation string */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew                 /* tp_new */
};

/* End of code for StateMachine objects */
/* -------------------------------------------------------- */


static char gammu_Version__doc__[] =
"Returns Gammu version"
;

static PyObject *
gammu_Version(PyObject *self /* Not used */, PyObject *args)
{

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	return Py_BuildValue("s", VERSION);
}

static char gammu_SetDebugFile__doc__[] =
""
;

static PyObject *
gammu_SetDebugFile(PyObject *self /* Not used */, PyObject *args)
{

	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

static char gammu_SetDebugLevel__doc__[] =
"Sets debug level accorting to passed string."
;

static PyObject *
gammu_SetDebugLevel(PyObject *self /* Not used */, PyObject *args)
{
    char *level;
    if (!PyArg_ParseTuple(args, "s", &level))
        return NULL;
    
    if (!GSM_SetDebugLevel(level, &di)) {
        PyErr_SetString(PyExc_ValueError, level);
        return NULL;
    }
    
	Py_INCREF(Py_None);
	return Py_None;
}

static char gammu_StateMachine__doc__[] =
"StateMachine(GlobalDebug = 1) \n\n"
"Creates new StateMachine"
;

static PyObject *
gammu_StateMachine(PyObject *self /* Not used */, PyObject *args, PyObject *kwds)
{
    StateMachineObject  *sm;
    int                 global_debug = 1;
    static char         *kwlist[] = {"GlobalDebug", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|I", kwlist, &global_debug))
		return NULL;
    sm = (StateMachineObject *)PyObject_NEW(StateMachineObject, &StateMachineType);
    
    sm->s.opened      = false;
    sm->s.msg         = NULL;
    sm->s.ConfigNum   = 0;

    sm->s.di.use_global = global_debug;

    return (PyObject *)sm;
}


/* List of methods defined in the module */

static struct PyMethodDef gammu_methods[] = {
    {"Version",         (PyCFunction)gammu_Version,         METH_VARARGS,   gammu_Version__doc__},
    {"SetDebugFile",    (PyCFunction)gammu_SetDebugFile,    METH_VARARGS,   gammu_SetDebugFile__doc__},
    {"SetDebugLevel",   (PyCFunction)gammu_SetDebugLevel,   METH_VARARGS,   gammu_SetDebugLevel__doc__},
    {"StateMachine",    (PyCFunction)gammu_StateMachine,    METH_VARARGS|METH_KEYWORDS,   gammu_StateMachine__doc__},
 
	{NULL,	 (PyCFunction)NULL, 0, NULL}		/* sentinel */
};


/* Initialization function for the module (*must* be called initgammu) */

static char gammu_module_documentation[] = 
"Module wrapping Gammu functions. Gammu is software for communication with GSM phones "
"allowing work with most of data stored in them."
;

PyMODINIT_FUNC initgammu(void) {
	PyObject *m, *d;

	/* Create the module and add the functions */
	m = Py_InitModule3("gammu", gammu_methods, gammu_module_documentation);

    if (m == NULL)
        return;

    d = PyModule_GetDict(m);

    if (PyType_Ready(&MemoryEntryType) < 0)
        return;
    Py_INCREF(&MemoryEntryType);
    
    if (PyType_Ready(&StateMachineType) < 0)
        return;
    Py_INCREF(&StateMachineType);
	
	/* Add some symbolic constants to the module */
    GammuError = PyErr_NewException("gammu.GSMError", NULL, NULL);
    if (GammuError == NULL)
        return;
    PyDict_SetItemString(d, "GSMError", GammuError);

	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module gammu");

    di.df = stdout;
    /* FIXME: remove this in future! */
    di.dl = DL_TEXTALL;

}
