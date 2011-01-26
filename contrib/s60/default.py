# -*- coding: utf-8 -*-

# Copyright (c) 2008 - 2010 Lukas Hetzenecker <LuHe@gmx.at>

import sys
sys.path.append('e:\python\libs')
sys.path.append('c:\python\libs')

import os.path

import time
import math
import md5

import sysinfo
import e32
import inbox
import contacts
import telephone
import messaging
import location
import graphics
from appuifw import *
from status_numbers import *

if float(e32.pys60_version[:3]) >= 1.9:
    import e32calendar as calendar
else:
    import calendar

if float(e32.pys60_version[:3]) >= 1.9:
    import btsocket as socket
else:
    import socket

import cPickle


VERSION = 0.5
PORT = 18

# For hash functions
CONTACT_SEP = chr(0x1F) # Unit Separator
ENTRY_SEP = chr(0x1F) # Unit Separator
FIELD_SEP = chr(0x1E) # Record Separator
INFO_SEP = chr(0x1D) # Group Separator

class Mobile(object):
    def __init__(self):
        self.connected = False
        self.service = False
        self.useCanvas = True
        self.client = None
        self.port = PORT

        self.loadConfig()

        self.inbox = inbox.Inbox(inbox.EInbox)
        self.sent = inbox.Inbox(inbox.ESent)
        self.contactDb = contacts.open()
        self.calendarDb = calendar.open()
        self.currentMessage = None
        self.__partialMessage = ""

        self.inbox.bind(self.newMessage)
        #telephone.call_state(self.handleCall)

        self.initUi()
        self.startService()

    def getCurrentDir(self):
        try:
            return self._current_dir
        except:
            pass

        try:
            raise Exception
        except Exception:
            frame = sys.exc_info()[2].tb_frame
            path = frame.f_code.co_filename
            dirpath = os.path.split(path)[0]
            self._current_dir = os.path.realpath(dirpath)

        return self._current_dir

    def getFilename(self, name):
        return os.path.join(self.getCurrentDir(), name)

    def getConfigFilename(self):
        return self.getFilename('s60.cfg')

    def getScreenshotFilename(self):
        return self.getFilename('screenshot.png')

    def loadConfig(self):
        try:
            f = file(self.getConfigFilename(), 'rb')
            conf = cPickle.load(f)
            f.close()
            if 'port' in conf:
                self.port = conf['port']
            if 'useCanvas' in conf:
                self.useCanvas = conf['useCanvas']
        except IOError, r:
            pass

    def saveConfig(self):
        try:
            f = file(self.getConfigFilename(), 'wb')
            conf = {
                'port': self.port,
                'useCanvas': self.useCanvas,
                }
            cPickle.dump(conf, f)
            f.close()
        except IOError, r:
            pass

    def initUi(self):
        app.title = u"Series 60 - Remote"

        if self.useCanvas:
            self.canvas = Canvas(redraw_callback=self.statusUpdate)
            app.body = self.canvas

        self.lock = e32.Ao_lock()
        app.exit_key_handler = self.exitHandler
        app.screen = 'normal'
        app.menu = [(u'About', self.aboutHandler), (u'Change port', self.portHandler), (u'Exit', self.exitHandler)]

    def statusUpdate(self, rect=None):
        if not self.useCanvas:
            return

        self.canvas.clear((255,255,255))
        if self.service:
            self.canvas.text((1,14),u"Service started at port %s" % self.port,0xff0000)
        else:
            self.canvas.text((1,14),u"Service stopped",0xff0000)

        if self.connected:
            self.canvas.text((1,34), u"Connected to: " + self.client[1], 0x0000ff)
        else:
            self.canvas.text((1,34), u"No active connection", 0x0000ff)

    def startService(self):
        self.service = True
        self.statusUpdate()

        self.sock = socket.socket(socket.AF_BT, socket.SOCK_STREAM)
        self.sock.bind(('', self.port))
        self.sock.listen(1)

        socket.set_security(self.sock,  socket.AUTH | socket.AUTHOR)
        socket.bt_advertise_service(u"pys60_remote", self.sock, True, socket.RFCOMM)
        note(u'Listenning on port %d' % self.port)

        self.listen()

    def stopService(self):
        if (self.service):
            self.service = False
            self.statusUpdate()

            socket.bt_advertise_service(u"pys60_remote", self.sock, False, socket.RFCOMM)
            self.sock.close()
            self.sock = None

    def disconnect(self):
        if(self.connected):
            self.connected = False

            self.fos.close()
            self.fis.close()

            self.client[0].close()
            self.client = None

            self.statusUpdate()

    def listen(self):
        while self.service:
            self.client = self.sock.accept()

            self.connected = True
            self.statusUpdate()
            note(u'Connected client %s' % self.client[1])

            self.fos = self.client[0].makefile("w")
            self.fis = self.client[0].makefile("r")

            self.send(NUM_CONNECTED,  PROTOCOL_VERSION)

            self.wait()
            note(u'Disconnected client %s' % self.client[1])
            self.disconnect()

    def send(self, header,  *message):
        new_message = ""

        if len(message) == 1:
            new_message = unicode(message[0])
        else:
            for part in message:
                new_message += unicode(part) + str(NUM_SEPERATOR)

        length = 1000
        if len(new_message) > length:
            parts = int(math.ceil(len(new_message) / float(length)))
            sentParts = 0
            for i in range(parts):
                part = new_message[sentParts*length:sentParts*length+length]
                if sentParts == parts-1:
                    self.send(header,  part)
                else:
                    self.send(NUM_PARTIAL_MESSAGE,  part)
                sentParts += 1
            return

        self.fos.write(unicode(str(header) + str(NUM_END_HEADER) + new_message + str(NUM_END_TEXT)).encode("utf8") )
        self.fos.flush()

    def wait(self):
        while(True):
            try:
                data = self.fis.readline()
            except:
                break

            header = int(data.split(NUM_END_HEADER)[0])
            message = unicode(data.split(NUM_END_HEADER)[1],  "utf8")

            if (header != NUM_PARTIAL_MESSAGE and self.__partialMessage):
                message = self.__partialMessage + message
                self.__partialMessage = ""

            if (header == NUM_PARTIAL_MESSAGE):
                self.__partialMessage += message

            elif (header == NUM_HELLO_REQUEST):
                    self.send(NUM_HELLO_REPLY)

            elif (header == NUM_SYSINFO_REQUEST):
                full = bool(int(message.split(NUM_SEPERATOR)[0]))
                self.sendSysinfo(full)

            elif (header == NUM_LOCATION_REQUEST):
                self.sendLocation()

            elif (header == NUM_CONTACTS_REQUEST_HASH_ALL):
                self.sendContactHash()

            elif (header == NUM_CONTACTS_REQUEST_HASH_SINGLE):
                self.sendContactHashSingle()

            elif (header == NUM_CONTACTS_REQUEST_CONTACT):
                key = int(message.split(NUM_SEPERATOR)[0])
                try:
                    contact = self.contactDb[key]
                    self.sendContact(contact)
                except:
                    self.send(NUM_CONTACTS_REPLY_CONTACT_NOT_FOUND)

            elif (header == NUM_CONTACTS_REQUEST_CONTACTS_ALL):
                self.sendAllContacts()

            elif (header == NUM_CONTACTS_ADD):
                contact = self.contactDb.add_contact()
                contact.commit()
                self.send(NUM_CONTACTS_ADD_REPLY_ID,  contact.id)

            elif (header == NUM_CONTACTS_DELETE):
                id = int(message)
                if id in self.contactDb.keys():
                    del self.contactDb[id]

            elif (header == NUM_CONTACTS_CHANGE_ADDFIELD):
                id = int(message.split(NUM_SEPERATOR)[0])
                type = unicode(message.split(NUM_SEPERATOR)[1])
                location = unicode(message.split(NUM_SEPERATOR)[2])
                value = unicode(message.split(NUM_SEPERATOR)[3])
                self.modifyContact("add",  id,  type,  location,  value)

            elif (header == NUM_CONTACTS_CHANGE_REMOVEFIELD):
                id = int(message.split(NUM_SEPERATOR)[0])
                type = unicode(message.split(NUM_SEPERATOR)[1])
                location = unicode(message.split(NUM_SEPERATOR)[2])
                value = unicode(message.split(NUM_SEPERATOR)[3])
                self.modifyContact("remove",  id,  type,  location,  value)

            elif (header == NUM_CALENDAR_REQUEST_HASH_ALL):
                self.sendCalendarHash()

            #elif (header == NUM_CALENDAR_REQUEST_HASH_SINGLE):
            #    self.sendCalendarHashSingle()

            elif (header == NUM_CALENDAR_REQUEST_ENTRY):
                key = int(message.split(NUM_SEPERATOR)[0])
                try:
                    entry = self.calendarDb[key]
                    self.sendCalendarEntry(entry)
                except:
                    self.send(NUM_CALENDAR_REPLY_ENTRY_NOT_FOUND)

            elif (header == NUM_CALENDAR_REQUEST_ENTRIES_ALL):
                self.sendAllCalendarEntries()

            elif (header == NUM_CALENDAR_ENTRY_DELETE):
                id = int(message)
                try:
                    del self.calendarDb[id]
                except RuntimeError:
                    # no such entry
                    pass

            elif (header == NUM_CALENDAR_ENTRY_CHANGE or header == NUM_CALENDAR_ENTRY_ADD):
                ms = message.split(NUM_SEPERATOR)

                if (header == NUM_CALENDAR_ENTRY_CHANGE):
                    id = int(ms[0])
                elif (header == NUM_CALENDAR_ENTRY_ADD):
                    type = str(ms[0])

                content = unicode(ms[1])
                location = unicode(ms[2])
                #start = float(ms[3]) if ms[3] else 0
                if ms[3]:
                    start = float(ms[3])
                else:
                    start = 0.0
                #end = float(ms[4]) if ms[4] else None
                if ms[4]:
                    end = float(ms[4])
                else:
                    end = None
                replication = str(ms[5])
                #alarm = float(ms[6]) if ms[6] else None
                if ms[6]:
                    alarm = float(ms[6])
                else:
                    alarm = None
                priority = int(ms[7])
                repeat_type = str(ms[8])
                repeat_days = str(ms[9])
                repeat_exceptions = str(ms[10])
                #repeat_start = float(ms[11]) if ms[11] else 0
                if ms[11]:
                    repeat_start = float(ms[11])
                else:
                    repeat_start = 0.0
                #repeat_end = float(ms[12]) if ms[12] else None
                if ms[12]:
                    repeat_end = float(ms[12])
                else:
                    repeat_end = None
                #repeat_interval = int(ms[13]) if ms[13] else 1
                if ms[13]:
                    repeat_interval = int(ms[13])
                else:
                    repeat_interval = 1

                if (header == NUM_CALENDAR_ENTRY_CHANGE):
                    self.modifyCalendarEntry(id,  content,  location,  start,  end,  replication,  alarm,  priority,  repeat_type,
                                        repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval)
                elif (header == NUM_CALENDAR_ENTRY_ADD):
                    self.addCalendarEntry(type,  content,  location,  start,  end,  replication,  alarm,  priority,  repeat_type,
                                        repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval)

            elif (header == NUM_MESSAGE_REQUEST):
                lastId = int(message.split(NUM_SEPERATOR)[0])
                self.sendAllMessages(lastId)

            elif (header == NUM_MESSAGE_REQUEST_UNREAD):
                self.sendUnreadMessages()

            elif (header == NUM_MESSAGE_SEND_REQUEST):
                name = unicode(message.split(NUM_SEPERATOR)[0])
                phone = unicode(message.split(NUM_SEPERATOR)[1])
                enc = str(message.split(NUM_SEPERATOR)[2])
                msg = unicode(message.split(NUM_SEPERATOR)[3])
                self.sendMessage(name, phone, enc,  msg)

            elif (header == NUM_SET_READ):
                id = int(message.split(NUM_SEPERATOR)[0])
                state = bool(message.split(NUM_SEPERATOR)[1])
                self.setRead(id, state)

            elif (header == NUM_SCREENSHOT):
                self.sendScreenshot()

            elif (header == NUM_QUIT):
                self.send(NUM_QUIT)
                break

    def sendSysinfo(self,  full):
        self.send(NUM_SYSINFO_REPLY_START)
        self.send(NUM_SYSINFO_REPLY_LINE, "program_version", VERSION)
        self.send(NUM_SYSINFO_REPLY_LINE, "battery", sysinfo.battery())
        self.send(NUM_SYSINFO_REPLY_LINE, "active_profile", sysinfo.active_profile())
        self.send(NUM_SYSINFO_REPLY_LINE, "free_ram", sysinfo.free_ram())
        self.send(NUM_SYSINFO_REPLY_LINE, "pys60_version", e32.pys60_version)

        if sysinfo.active_profile() == u"offline":
            # Return an error code if the phone is in offline mode
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_dbm", -1)
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_bars", -1)
        else:
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_dbm", sysinfo.signal_dbm())
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_bars", sysinfo.signal_bars())

        for drive,  free in sysinfo.free_drivespace().iteritems():
            self.send(NUM_SYSINFO_REPLY_LINE, "free_drivespace", str(drive) + str(free))

        if full:
            self.send(NUM_SYSINFO_REPLY_LINE, "display", str(sysinfo.display_pixels()[0]) + "x" + str(sysinfo.display_pixels()[1]))
            self.send(NUM_SYSINFO_REPLY_LINE, "imei", sysinfo.imei())
            self.send(NUM_SYSINFO_REPLY_LINE, "model", sysinfo.sw_version())
            self.send(NUM_SYSINFO_REPLY_LINE, "s60_version", e32.s60_version_info[0],  e32.s60_version_info[1] )
            self.send(NUM_SYSINFO_REPLY_LINE, "total_ram", sysinfo.total_ram())
            self.send(NUM_SYSINFO_REPLY_LINE, "total_rom", sysinfo.total_rom())

        self.send(NUM_SYSINFO_REPLY_END)

    def sendScreenshot(self):
        fn = self.getScreenshotFilename()
        shot = graphics.screenshot()
        shot.save(fn)
        note(u'Saved screenshot as %s' % fn)
        f = file(fn, 'rb')
        self.send(NUM_SCREENSHOT_REPLY, f.read().encode('base64'))
        f.close()

    def sendLocation(self):
        loc = location.gsm_location()
        if loc is None:
            self.send(NUM_LOCATION_REPLY_NA)
        else:
            mcc, mnc, lac, cellid = loc
            self.send(NUM_LOCATION_REPLY, '%03d' % mcc, '%02d' % mnc, '%X' % lac, '%X' % cellid)

    def contactDict(self):
        keys = self.contactDb.keys()

        contactDict = dict()
        for key in keys:
            contact = self.contactDb[key]

            # Check for empty title (please look in the comment for sendContact)
            try:
                contact.title
            except TypeError:
                continue

            contactDict[contact.id] = list()
            for field in contact:
                _type = field.type
                value = field.value
                value = unicode(value)
                value = value.replace(u'\u2029',  u'\n') # PARAGRAPH SEPARATOR (\u2029) replaced by LINE FEED (\u000a)
                location = field.location

                if _type == "unknown":
                    continue
                elif _type == "thumbnail_image":
                    value = self.getContactThumbnail(contact)
                    if not value:
                        continue
                elif _type == "date":
                    value = self.getContactBirthday(contact)

                if isinstance(value, type(None)):
                   # Ignore this field
                   continue

                contactDict[contact.id].append((_type,  location,  value))
            contactDict[contact.id].sort()

        return contactDict

    def sendContactHash(self):
        contacts = self.contactDict()
        keys = contacts.keys()
        keys.sort()

        hash = unicode()

        for key in keys:
            hash += str(key)
            hash += FIELD_SEP
            for _type,  location, value in contacts[key]:
                hash += _type + INFO_SEP + location + INFO_SEP + value
                hash += FIELD_SEP
            hash += CONTACT_SEP

        hash = hash.encode("utf8")
        hash = md5.md5(hash).hexdigest()
        self.send(NUM_CONTACTS_REPLY_HASH_ALL, hash)

    def sendContactHashSingle(self):
        self.send(NUM_CONTACTS_REPLY_HASH_SINGLE_START)

        contacts = self.contactDict()
        keys = contacts.keys()
        keys.sort()

        for key in keys:
            hash = unicode()
            for _type,  location, value in contacts[key]:
                hash += _type + INFO_SEP + location + INFO_SEP + value
                hash += FIELD_SEP

            hash = hash.encode("utf8")
            hash = md5.md5(hash).hexdigest()
            self.send(NUM_CONTACTS_REPLY_HASH_SINGLE_LINE, key,  hash)

        self.send(NUM_CONTACTS_REPLY_HASH_SINGLE_END)

    def sendAllContacts(self):
        keys = self.contactDb.keys()

        for key in keys:
            contact = self.contactDb[key]
            self.sendContact(contact)
        self.send(NUM_CONTACTS_REPLY_CONTACTS_ALL_END)

    def sendContact(self,  contact):
        # There could be an empty entry in the contact database
        # In this case contact.title would report the following error:
        # File "c:\resource\contacts.py", line 293, in _get_title
        #   title_str += self._contact.get_field(index)['value'] + u" "
        # TypeError: unsupported operand types for +: 'NoneType' and 'unicode'
        #
        # I think the best way is to ignore such errors...
        try:
            self.send(NUM_CONTACTS_REPLY_CONTACT_START,  contact.id,  contact.title)
        except TypeError:
            return

        for field in contact:
            _type = field.type
            value = field.value
            value = unicode(value)
            value = value.replace(u'\u2029',  u'\n') # PARAGRAPH SEPARATOR (\u2029) replaced by LINE FEED (\u000a)
            location = field.location

            if _type == "unknown":
                continue
            elif _type == "thumbnail_image":
                value = self.getContactThumbnail(contact)
                if not value:
                    continue
            elif _type == "date":
                value = self.getContactBirthday(contact)

            if isinstance(value, type(None)):
               continue

            self.send(NUM_CONTACTS_REPLY_CONTACT_LINE,  contact.id,  _type,  location,  value)
        self.send(NUM_CONTACTS_REPLY_CONTACT_END,  contact.id)

    def modifyContact(self,  modification,  id,  type,  location,  value):
        try:
            contact = self.contactDb[id]
        except:
            return

        if type == u"thumbnail_image":
            if modification == "remove":
                self.setContactThumbnail(contact)
            else:
                self.setContactThumbnail(contact,  value)
            return
        elif type == u"date":
            if modification == "remove":
                self.setContactBirthday(contact)
            else:
                self.setContactBirthday(contact,  value)
            return

        contact.begin()

        if modification == "add":
            contact.add_field(type,  value,  location=location)
        elif modification == "remove":
            index = -1
            for field in contact.find(type,  location):
                if field.value == value:
                    index = field.index
                    break

            if index != -1:
                del contact[index]

        contact.commit()

    def getDetailFromVcard(self,  contact,  detail,  delimiter='\r\n'):
        # This is an ugly hack, needed for some fields that cannot be handled using the contact object
        try:
            value = unicode(contact.as_vcard(), 'utf8')
            value  = value.split(detail + ":")[1].split(delimiter)[0]
            return value
        except:
            return

    def setDetailFromVcard(self,  contact,  detail,  value,  delimiter='\r\n'):
        # This is an ugly hack, needed for some fields that cannot be handled using the contact object
        card = contact.as_vcard()

        new = u""
        for line in card.split("\r\n"):
            if line.startswith("BEGIN:") or line.startswith("VERSION:") or line.startswith("REV:") or line.startswith("UID:"):
                new += line + "\r\n"

        # Format value: New line (\r\n) after 64 chars, followed by 4 spaces
        if len(value) > 64:
            fmtvalue = "\r\n"
            for i in range(len(value)/64+1):
                fmtvalue += value[i*64:(i+1)*64] + "\r\n" + 4*" "
        else:
            fmtvalue = value

        new += detail + ":" + fmtvalue + delimiter
        new += "END:VCARD"

        changed_contact = self.contactDb.import_vcards(new)[0]
        assert changed_contact.id == contact.id

    def getContactThumbnail(self,  contact):
        # Ugly workaround!
        # HACK: The value of type "thumbnail_image" is empty, it is only shown when we export the contact to a vCard
        image = self.getDetailFromVcard(contact,  "PHOTO;TYPE=JPEG;ENCODING=BASE64",  "\r\n\r\n")
        if image:
            image = image.split("\r\n\r\n")[0]
            image = image.replace("\r",  "").replace("\n",  "").replace(" ",  "")
            return image
        return

    def setContactThumbnail(self,  contact,  image=""):
        # Ugly workaround!
        # HACK: There seems to be new other way to update/add the contact picture
        self.setDetailFromVcard(contact,  "PHOTO;TYPE=JPEG;ENCODING=BASE64",  image,  "\r\n\r\n")

    def getContactBirthday(self,  contact):
        return self.getDetailFromVcard(contact,  "BDAY")

    def setContactBirthday(self,  contact,  date=""):
        # HACK: It isn't possible to set birthdays < year 1970 (before the beginning of the unix epoch)
        self.setDetailFromVcard(contact,  "BDAY",  date)

    def __calendarGetType(self,  entry):
        if isinstance(entry,  calendar.CalendarDb.AnniversaryEntry):
            return "anniversary"
        elif isinstance(entry,  calendar.CalendarDb.AppointmentEntry):
            return "appointment"
        elif isinstance(entry,  calendar.CalendarDb.EventEntry):
            return "event"
        elif isinstance(entry,  calendar.CalendarDb.ReminderEntry):
            return "reminder"
        elif isinstance(entry,  calendar.CalendarDb.TodoEntry):
            return "todo"
        return ""

    def __calendarFormatEntry(self,  entry,  sep):
        entryType = self.__calendarGetType(entry)
        if not entryType:
            # Type could also be TodoListDict or TodoList, which is currently not handled...
            return str()

        line = entryType + sep + entry.content + sep + entry.location + sep
        if entry.start_time:
            # None if the start datetime of the entry is not set
            line += str(long(entry.start_time)) # entry.start_time is of type float
        line += sep

        if entry.end_time:
            line += str(long(entry.end_time))
        line += sep

        line += str(long(entry.last_modified)) + sep
        line += entry.replication + sep  # open, private or restricted

        if entry.alarm:
            line += str(long(entry.alarm)) # The alarm datetime value (float) for the entry
        line += sep

        line += str(entry.priority) + sep

        # Format repeat...
        repeat = entry.get_repeat()
        repeated = bool(repeat)

        if repeated:
            line += repeat["type"] # daily, weekly, monthly_by_dates, monthly_by_days, yearly_by_date, yearly_by_day
        line += sep

        # It is really hard to extract the days...
        # We try to seperate the values by ',' and different groups by ';'
        if repeated and "days" in repeat:
            days = repeat["days"]

            if isinstance(days,  (list,  tuple)):
                if len(days) == 0:
                    pass
                elif isinstance(days[0],  int):
                    # Which days in week? (0=Monday,...)
                    # for example: [0,1,2]
                    line += ",".join([str(day) for day in days])
                elif isinstance(days[0],  dict):
                    # example: on second Tuesday and last Monday of the month
                    # days is: [{'week': 1, 'day': 1}, {'week': 4, 'day': 0}]
                    # results in: 'week:1,day:1;week:4,day:0'
                    line += ";".join([",".join([key + ":" + str(value) for key, value in entry.items()]) for entry in days])
            elif isinstance(days,  dict):
                # for example: {'week': 1, 'day': 1, 'month': 1}
                # results in: 'week:1,day:1,month:1'
                line += ",".join([key + ":" + str(value) for key, value in days.iteritems()])
        line += sep

        if repeated and "exceptions" in repeat:
            line += ",".join([str(int(day)) for day in repeat["exceptions"]])
        line += sep

        if repeated and "start" in repeat:
            line += str(long(repeat["start"]))
        line += sep

        if repeated and "end" in repeat:
            end = long(repeat["end"])
            end -= time.timezone
            if end == 4102441200L:
                # 4102441200L = 2100-01-01T00:00:00
                # There is a bug in PyS60, which causes that None as end date throws as error
                # As workarond, we use 2100-01-01 as end date for eternal repeats
                # Bug report: http://developer.symbian.org/bugs/show_bug.cgi?id=3666
                end = ""
            line += str(end)
        line += sep

        if repeated and "interval" in repeat:
            line += str(repeat["interval"])
        line += sep

        if entryType == "todo":
            # cross out property is valid only for todo entries
            # a value that is interpreted as false means that the entry is not crossed out
            line += str(int(bool(entry.crossed_out))) + sep

            if entry.cross_out_time:
                line += str(long(entry.cross_out_time))
            line += sep

            # TODO lists no more supported!
            #line += entry.todo_list + sep  # The ID of the TODO list to which this entry belongs

        return line

    def calendarDict(self):
        calendarDict = dict()
        for key in self.calendarDb:
            entry = self.calendarDb[key]
            line = self.__calendarFormatEntry(entry,  FIELD_SEP)
            if not line:
                continue

            calendarDict[entry.id] =  line

        return calendarDict

    def sendCalendarHash(self):
        calendarDict = self.calendarDict()

        keys = calendarDict.keys()
        keys.sort()

        hash = unicode()
        for key in keys:
            hash += str(key)
            hash += FIELD_SEP
            hash += calendarDict[key]
            hash += FIELD_SEP
            hash += ENTRY_SEP

        #self.send(NUM_DEBUG,  "Mobile:::" + hash.replace(FIELD_SEP,  ";").replace(ENTRY_SEP,  "\n"))

        hash = hash.encode("utf8")
        hash = md5.md5(hash).hexdigest()
        self.send(NUM_CALENDAR_REPLY_HASH_ALL, hash)

# We don't need to send a hash for every entry, it's better if we send directly the data
#    def sendCalendarHashSingle(self):
#        self.send(NUM_CALENDAR_REPLY_HASH_SINGLE_START)
#
#        calendarDict = self.calendarDict()
#        keys = calendarDict.keys()
#        keys.sort()
#
#        print "keys are of length",  len(keys)
#
#        for key in keys:
#            hash = calendarDict[key]
#            hash = hash.encode("utf8")
#            hash = md5.md5(hash).hexdigest()
#
#            self.send(NUM_CALENDAR_REPLY_HASH_SINGLE_LINE, key,  hash)
#
#        self.send(NUM_CALENDAR_REPLY_HASH_SINGLE_END)

    def sendAllCalendarEntries(self):
        self.send(NUM_CALENDAR_REPLY_ENTRIES_START)
        for key in self.calendarDb:
            entry = self.calendarDb[key]
            self.sendCalendarEntry(entry )
        self.send(NUM_CALENDAR_REPLY_ENTRIES_END)

    def sendCalendarEntry(self,  entry):
        self.send(NUM_CALENDAR_REPLY_ENTRY,  entry.id,  self.__calendarFormatEntry(entry,  NUM_SEPERATOR))

    def modifyCalendarEntry(self,  id,  content,  location,  start,  end,  replication,  alarm,  priority, repeat_type,
                            repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval):

        try:
            entry = self.calendarDb[id]
        except:
            return

        entry.begin()

        self.modifyCalendarEntryFields(entry,  content,  location,  start,  end,  replication,  alarm,  priority, repeat_type,
                            repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval)

        entry.commit()

        self.send(NUM_CALENDAR_ENTRY_CHANGE_REPLY_TIME,  id,  str(long(entry.last_modified)))

    def addCalendarEntry(self,  type,  content,  location,  start,  end,  replication,  alarm,  priority, repeat_type,
                            repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval):

        if type == "appointment":
            entry = self.calendarDb.add_appointment()
        elif type == "event":
            entry = self.calendarDb.add_event()
        elif type == "anniversary":
            entry = self.calendarDb.add_anniversary()
        elif type == "todo":
            entry = self.calendarDb.add_todo()
        else:
            return

        self.modifyCalendarEntryFields(entry,  content,  location,  start,  end,  replication,  alarm,  priority, repeat_type,
                            repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval)

        entry.commit()

        self.send(NUM_CALENDAR_ENTRY_ADD_REPLY,  entry.id,  str(long(entry.last_modified)))

    def modifyCalendarEntryFields(self,  entry,  content,  location,  start,  end,  replication,  alarm,  priority, repeat_type,
                            repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval):
        entry.content = content
        entry.location = location
        entry.set_time(start,  end)
        entry.replication = replication
        entry.alarm = alarm
        entry.priority = priority

        # Clear the old repeat
        entry.set_repeat(None)

        entry.set_repeat(self.buildCalendarEntryRepeat(repeat_type,  repeat_days,  repeat_exceptions,  repeat_start,  repeat_end,  repeat_interval))

    def buildCalendarEntryRepeat(self,   type,  days,  exceptions,  start,  end,  interval):
        if not type:
            return None
        if type not in ("weekly",  "monthly_by_dates",  "monthly_by_days",  "yearly_by_day",  "yearly_by_date"):
            return None

        days = self.calendarEntryParsedDays(type,  days)
        if exceptions:
            exceptions = [float(exception) for exception in exceptions.split(',')]
        else:
            exceptions = []

        if end is None:
            # 4102441200. = 2100-01-01T00:00:00
            # There is a bug in PyS60, which causes that None as end date throws as error
            # As workarond, we use 2100-01-01 as end date for eternal repeats
            # Bug report: http://developer.symbian.org/bugs/show_bug.cgi?id=3666
            # We also can't use time.mktime([2100, 1, 1, 0, 0, 0, 1, 1, -1]), cause this wouls result in
            # an integer overflow
            end = 4102441200.

        # We need to subtract one day from the end date, because
        # endTime+=TTimeIntervalDays(1);
        # in calendarmodule.h adds one day after set_repeat is called
        end -= 24*60*60

        return {"type" : type,
                      "days" : days,
                      "exceptions" : exceptions,
                      "start" : start,
                      "end" : end,
                      "interval" : interval}

    def calendarEntryParsedDays(self,  type,  days):

        ### PLEASE COPY CHANGES IN THIS FUNCTION ALSO TO
        ### pc/lib/classes.py, class CalendarEntry function recurrenceParsedDays

        if type in ("weekly",   "monthly_by_dates"):
            # 0,1,2 -> [0, 1, 2]
            return [int(day) for day in days.split(',')]
        if type == "monthly_by_days":
            # week:1,day:1;week:4,day:0 -> [{'week': 1, 'day': 1}, {'week': 4, 'day': 0}]
            dates = []
            for date in days.split(";"):
                tmp = dict()
                for sub in date.split(","):
                    key, value = sub.split(":")
                    tmp[key] = int(value)
                dates.append(tmp)
            return dates
        if type == "yearly_by_day":
            # week:1,day:1,month:1 -> {'week': 1, 'day': 1, 'month': 1}
            dates = {}
            for subentry in days.split(","):
                key,  value = subentry.split(":")
                dates[key] = int(value)
            return dates

    def sendAllMessages(self,  lastId):
        messages = list()
        inbox = list()
        sent = list()
        for box in ("inbox",  "sent"):
            #FIXME: I shouldn't need this
            e32.ao_sleep(1)

            if box == "inbox":
                inbox = self.inbox.sms_messages()
            else:
                sent = self.sent.sms_messages()

        messages = inbox + sent
        messages.sort()
        for sms in messages:
            if (int(sms) > int(lastId)):
                id = sms
                time = self.inbox.time(sms)
                address = self.inbox.address(sms)
                content = self.inbox.content(sms)
                content = content.replace(u'\u2029',  u'\n') # PARAGRAPH SEPARATOR (\u2029) replaced by LINE FEED (\u000a)

                if sms in inbox:
                    box = "inbox"
                else:
                    box = "sent"

                self.send(NUM_MESSAGE_REPLY_LINE,  box,  id,  time,  address,  content)

        self.send(NUM_MESSAGE_REPLY_END)

    def sendUnreadMessages(self):
        messages = list()
        inbox = self.inbox.sms_messages()
        for sms in inbox:
            if self.inbox.unread(sms):
                messages.append(sms)
        self.send(NUM_MESSAGE_REPLY_UNREAD,  *messages)

    def sendMessage(self, name, phone, encoding,  msg):
        try:
            messaging.sms_send(phone, msg,  encoding,  self.sentMessage,  name)
        except RuntimeError,  detail:
            if str(detail) == "Already sending":
                # Workaround for the "Already sending" bug:
                # http://discussion.forum.nokia.com/forum/showthread.php?t=141083
                messaging._sending = False
                self.send(NUM_MESSAGE_SEND_REPLY_RETRY,  str(detail) + "; tried workaround")
            else:
                self.send(NUM_MESSAGE_SEND_REPLY_RETRY,  detail)

    def sentMessage(self,  status):
        if status == messaging.ECreated:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS,  "Message created.")
        elif status == messaging.EMovedToOutBox:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS,  "Moved to outbox.")
        elif status == messaging.EScheduledForSend:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS,  "Scheduled for send.")
        if status == messaging.ESent:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS,  "Message sent.")
        elif status == messaging.EDeleted:
            self.send(NUM_MESSAGE_SEND_REPLY_OK,  "The SMS message has been deleted from device's outbox queue.")
        elif status == messaging.EScheduleFailed:
            self.send(NUM_MESSAGE_SEND_REPLY_FAILURE,  "Schedule failed.")
        elif status == messaging.ESendFailed:
            self.send(NUM_MESSAGE_SEND_REPLY_FAILURE,  "The SMS subsystem has tried to send the message several times in vain.")
        elif status == messaging.ENoServiceCentre:
            self.send(NUM_MESSAGE_SEND_REPLY_FAILURE,  "No service centre.")
        elif status == messaging.EFatalServerError:
            self.send(NUM_MESSAGE_SEND_REPLY_FAILURE,  "SMS send failed! If the device is in offline-mode or with no network connection the message is added to the device's outgoing message queue.")

    def newMessage(self, sms):
        if not self.connected:
            return

        #FIXME: I shouldn't need this
        e32.ao_sleep(1)

        id = sms
        time = self.inbox.time(sms)
        address = self.inbox.address(sms)
        content = self.inbox.content(sms)

        self.send(NUM_MESSAGE_NEW, id,  time,  address,  content)

    def handleCall(self,  handle):
        state = handle[0]
        number = handle[1]
        call_state = { telephone.EStatusUnknown: "unknown",
        telephone.EStatusIdle: "idle",
        telephone.EStatusDialling: "dialing",
        telephone.EStatusRinging: "ringing",
        telephone.EStatusAnswering: "answering",
        telephone.EStatusConnecting: "connecting",
        telephone.EStatusConnected: "connected",
        telephone.EStatusReconnectPending: "reconnect pending",
        telephone.EStatusDisconnecting: "disconnecting",
        telephone.EStatusHold: "hold",
        telephone.EStatusTransferring: "transferring",
        telephone.EStatusTransferAlerting: "transfer alerting" }

        self.send(NUM_INCOMING_CALL, number,  call_state[state])

    def setRead(self,  id,  state):
        state = int(not state)

        #FIXME: I shouldn't need this
        e32.ao_sleep(1)

        self.inbox.set_unread(id,  state)

    def quit(self):
        self.stopService()
        self.disconnect()

    def exitHandler(self):
        self.quit()
        self.saveConfig()

        app.exit_key_handler = None
        self.lock.signal()
        if self.useCanvas:
            self.canvas = None
        if app.full_name()[-10:] != "Python.app":
           app.set_exit()

    def portHandler(self):
        ret = query(u'Enter bluetooth port to use', 'number', self.port)
        if ret is not None:
            self.port = ret
            self.quit()
            self.saveConfig()
            self.startService()

    def aboutHandler(self):
        query(u'Series60 - remote\nVersion %s\nModified for Gammu\nhttp://wammu.eu/' % (VERSION) , 'query')

# Debug of SIS applications
try:
    mobile = Mobile()
except Exception, e:
    # Oops, something wrong. Report problems to user
    # and ask him/her to send them to you.
    import traceback

    new_line = u"\u2029"

    # Collecting call stack info
    info = sys.exc_info()

    # Show the last 4 lines of the call stack
    call_stack = u""
    for filename, lineno, function, text in traceback.extract_tb(info[2])[4:]:
        call_stack += filename + u": " + str(lineno) + u" - " + function + new_line
        call_stack += u" " + repr(text) + new_line
    call_stack +=  u"%s: %s" % info[:2]

    # Creating a friendly user message with exception details
    err_msg = u"This programs was unexpectedly closed due to the following error: "
    err_msg += unicode(repr(e)) + new_line
    err_msg += u"Please, copy and paste the text presented here and "
    err_msg += u"send it to series60-remote-devel@lists.sourceforge.net. "
    err_msg += u"Thanks in advance and sorry for this inconvenience." + new_line*2
    err_msg += u"Call stack:" + new_line + call_stack

    # Small PyS60 application
    lock = e32.Ao_lock()
    app.body = Text(err_msg)
    app.body.set_pos(0)
    app.menu = [(u"Exit", lambda: lock.signal())]
    lock.wait()
