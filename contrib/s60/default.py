# Copyright (c) 2008 - 2010 Lukas Hetzenecker <LuHe@gmx.at>
# Copyright (c) 2011 - 2012 Michal Čihař <michal@cihar.com>

import sys

sys.path.append(r"e:\python\libs")
sys.path.append(r"c:\python\libs")

import math
import os.path
import time

import contacts
import e32
import graphics
import inbox
import location
import md5
import messaging
import sysinfo
import telephone
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

import pickle
import socket as pysocket

VERSION = "0.6.0"
PORT = 18

# For hash functions
CONTACT_SEP = chr(0x1F)  # Unit Separator
ENTRY_SEP = chr(0x1F)  # Unit Separator
FIELD_SEP = chr(0x1E)  # Record Separator
INFO_SEP = chr(0x1D)  # Group Separator


class Mobile:
    def __init__(self):
        self.connected = False
        self.service = False
        self.useCanvas = True
        self.client = None
        self.port = PORT

        self.loadConfig()
        self.initUi()

        self.inbox = inbox.Inbox(inbox.EInbox)
        self.sent = inbox.Inbox(inbox.ESent)
        self.contactDb = contacts.open()
        self.calendarDb = calendar.open()
        self.currentMessage = None
        self.__partialMessage = ""

        self.inbox.bind(self.newMessage)
        # telephone.call_state(self.handleCall)

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
        return self.getFilename("gammu-s60.cfg")

    def getScreenshotFilename(self):
        return self.getFilename("screenshot.png")

    def loadConfig(self):
        try:
            f = file(self.getConfigFilename(), "rb")
            conf = pickle.load(f)
            f.close()
            if "port" in conf:
                self.port = conf["port"]
            if "useCanvas" in conf:
                self.useCanvas = conf["useCanvas"]
        except OSError as r:
            pass

    def saveConfig(self):
        try:
            f = file(self.getConfigFilename(), "wb")
            conf = {
                "port": self.port,
                "useCanvas": self.useCanvas,
            }
            pickle.dump(conf, f)
            f.close()
        except OSError as r:
            pass

    def initUi(self):
        app.title = "Gammu S60 Remote"

        if self.useCanvas:
            self.canvas = Canvas(redraw_callback=self.statusUpdate)
            app.body = self.canvas

        self.lock = e32.Ao_lock()
        app.exit_key_handler = self.exitHandler
        app.screen = "normal"
        app.menu = [
            ("About", self.aboutHandler),
            ("Change port", self.portHandler),
            ("Toggle Bluetooth", self.toggleHandler),
            ("Exit", self.exitHandler),
        ]

    def statusUpdate(self, rect=None):
        if not self.useCanvas:
            return

        self.canvas.clear((255, 255, 255))
        if self.service:
            self.canvas.text(
                (1, 14), "Service started at port %s" % self.port, 0xFF0000
            )
        else:
            self.canvas.text((1, 14), "Service stopped", 0xFF0000)

        if self.connected:
            self.canvas.text((1, 34), "Connected to: " + self.client[1], 0x0000FF)
        else:
            self.canvas.text((1, 34), "No active connection", 0x0000FF)

    def startService(self):
        self.service = True
        self.statusUpdate()

        self.sock = socket.socket(socket.AF_BT, socket.SOCK_STREAM)
        self.sock.bind(("", self.port))
        self.sock.listen(1)

        socket.set_security(self.sock, socket.AUTH | socket.AUTHOR)
        socket.bt_advertise_service("pys60_remote", self.sock, True, socket.RFCOMM)
        note("Listenning on port %d" % self.port)

        self.listen()

    def stopService(self):
        if self.service:
            self.service = False
            self.statusUpdate()

            socket.bt_advertise_service("pys60_remote", self.sock, False, socket.RFCOMM)
            self.sock.close()
            self.sock = None

    def disconnect(self):
        if self.connected:
            self.connected = False

            try:
                self.fos.close()
            except OSError:
                pass
            except pysocket.error:
                pass

            try:
                self.fis.close()
            except OSError:
                pass
            except pysocket.error:
                pass

            try:
                self.client[0].close()
            except OSError:
                pass
            except pysocket.error:
                pass
            self.client = None

            self.statusUpdate()

    def listen(self):
        while self.service:
            self.client = self.sock.accept()

            self.connected = True
            self.statusUpdate()
            address = str(self.client[1])
            note("Connected client %s" % address)

            self.fos = self.client[0].makefile("w")
            self.fis = self.client[0].makefile("r")

            try:
                self.send(NUM_CONNECTED, PROTOCOL_VERSION)
                self.wait()
            except OSError:
                pass
            except pysocket.error:
                pass

            self.disconnect()
            note("Disconnected client %s" % address)

    def send(self, header, *message):
        new_message = ""

        if len(message) == 1:
            new_message = str(message[0])
        else:
            for part in message:
                new_message += str(part) + str(NUM_SEPERATOR)

        length = 1000
        if len(new_message) > length:
            parts = int(math.ceil(len(new_message) / float(length)))
            sentParts = 0
            for i in range(parts):
                part = new_message[sentParts * length : sentParts * length + length]
                if sentParts == parts - 1:
                    self.send(header, part)
                else:
                    self.send(NUM_PARTIAL_MESSAGE, part)
                sentParts += 1
            return

        self.fos.write(
            str(
                str(header) + str(NUM_END_HEADER) + new_message + str(NUM_END_TEXT)
            ).encode("utf8")
        )
        self.fos.flush()

    def wait(self):
        while True:
            data = self.fis.readline()
            parts = data.split(NUM_END_HEADER)

            header = int(parts[0])
            message = str(parts[1], "utf8")
            message_parts = message.split(NUM_SEPERATOR)

            if header != NUM_PARTIAL_MESSAGE and self.__partialMessage:
                message = self.__partialMessage + message
                self.__partialMessage = ""

            if header == NUM_PARTIAL_MESSAGE:
                self.__partialMessage += message

            elif header == NUM_HELLO_REQUEST:
                self.send(NUM_HELLO_REPLY)

            elif header == NUM_SYSINFO_REQUEST:
                full = bool(int(message_parts[0]))
                self.sendSysinfo(full)

            elif header == NUM_LOCATION_REQUEST:
                self.sendLocation()

            elif header == NUM_DIAL:
                try:
                    telephone.dial(message_parts[0])
                except:
                    pass

            elif header == NUM_HANGUP:
                try:
                    telephone.hang_up()
                except:
                    pass

            elif header == NUM_CONTACTS_REQUEST_HASH_ALL:
                self.sendContactHash()

            elif header == NUM_CONTACTS_REQUEST_HASH_SINGLE:
                self.sendContactHashSingle()

            elif header == NUM_CONTACTS_REQUEST_COUNT:
                self.send(NUM_CONTACTS_REPLY_COUNT, len(self.contactDb))

            elif header == NUM_CONTACTS_REQUEST_CONTACT:
                key = int(message_parts[0])
                try:
                    contact = self.contactDb[key]
                    self.sendContact(contact)
                except:
                    self.send(NUM_CONTACTS_REPLY_CONTACT_NOT_FOUND)

            elif header == NUM_CONTACTS_REQUEST_CONTACTS_ALL:
                self.sendAllContacts()

            elif header == NUM_CONTACTS_ADD:
                contact = self.contactDb.add_contact()
                contact.commit()
                self.send(NUM_CONTACTS_ADD_REPLY_ID, contact.id)

            elif header == NUM_CONTACTS_DELETE:
                id = int(message)
                if id in list(self.contactDb.keys()):
                    del self.contactDb[id]

            elif header == NUM_CONTACTS_CHANGE_ADDFIELD:
                id = int(message_parts[0])
                type = str(message_parts[1])
                location = str(message_parts[2])
                value = str(message_parts[3])
                self.modifyContact("add", id, type, location, value)

            elif header == NUM_CONTACTS_CHANGE_REMOVEFIELD:
                id = int(message_parts[0])
                type = str(message_parts[1])
                location = str(message_parts[2])
                value = str(message_parts[3])
                self.modifyContact("remove", id, type, location, value)

            elif header == NUM_CALENDAR_REQUEST_COUNT:
                self.sendCalendarCount()

            elif header == NUM_CALENDAR_REQUEST_HASH_ALL:
                self.sendCalendarHash()

            # elif (header == NUM_CALENDAR_REQUEST_HASH_SINGLE):
            #    self.sendCalendarHashSingle()

            elif header == NUM_CALENDAR_REQUEST_ENTRY:
                key = int(message_parts[0])
                try:
                    entry = self.calendarDb[key]
                    self.sendCalendarEntry(entry)
                except:
                    self.send(NUM_CALENDAR_REPLY_ENTRY_NOT_FOUND)

            elif header == NUM_CALENDAR_REQUEST_ENTRIES_ALL:
                self.sendAllCalendarEntries()

            elif header == NUM_CALENDAR_ENTRY_DELETE:
                id = int(message)
                try:
                    del self.calendarDb[id]
                except RuntimeError:
                    # no such entry
                    pass

            elif (
                header == NUM_CALENDAR_ENTRY_CHANGE or header == NUM_CALENDAR_ENTRY_ADD
            ):
                if header == NUM_CALENDAR_ENTRY_CHANGE:
                    id = int(message_parts[0])
                elif header == NUM_CALENDAR_ENTRY_ADD:
                    type = str(message_parts[0])

                content = str(message_parts[1])
                location = str(message_parts[2])
                # start = float(message_parts[3]) if message_parts[3] else 0
                if message_parts[3]:
                    start = float(message_parts[3])
                else:
                    start = 0.0
                # end = float(message_parts[4]) if message_parts[4] else None
                if message_parts[4]:
                    end = float(message_parts[4])
                else:
                    end = None
                replication = str(message_parts[5])
                # alarm = float(message_parts[6]) if message_parts[6] else None
                if message_parts[6]:
                    alarm = float(message_parts[6])
                else:
                    alarm = None
                priority = int(message_parts[7])
                repeat_type = str(message_parts[8])
                repeat_days = str(message_parts[9])
                repeat_exceptions = str(message_parts[10])
                # repeat_start = float(message_parts[11]) if message_parts[11] else 0
                if message_parts[11]:
                    repeat_start = float(message_parts[11])
                else:
                    repeat_start = 0.0
                # repeat_end = float(message_parts[12]) if message_parts[12] else None
                if message_parts[12]:
                    repeat_end = float(message_parts[12])
                else:
                    repeat_end = None
                # repeat_interval = int(message_parts[13]) if message_parts[13] else 1
                if message_parts[13]:
                    repeat_interval = int(message_parts[13])
                else:
                    repeat_interval = 1

                if header == NUM_CALENDAR_ENTRY_CHANGE:
                    self.modifyCalendarEntry(
                        id,
                        content,
                        location,
                        start,
                        end,
                        replication,
                        alarm,
                        priority,
                        repeat_type,
                        repeat_days,
                        repeat_exceptions,
                        repeat_start,
                        repeat_end,
                        repeat_interval,
                    )
                elif header == NUM_CALENDAR_ENTRY_ADD:
                    self.addCalendarEntry(
                        type,
                        content,
                        location,
                        start,
                        end,
                        replication,
                        alarm,
                        priority,
                        repeat_type,
                        repeat_days,
                        repeat_exceptions,
                        repeat_start,
                        repeat_end,
                        repeat_interval,
                    )

            elif header == NUM_MESSAGE_REQUEST:
                lastId = int(message_parts[0])
                self.sendAllMessages(lastId)

            elif header == NUM_MESSAGE_REQUEST_ONE:
                lastId = int(message_parts[0])
                self.sendOneMessage(lastId)

            elif header == NUM_MESSAGE_REQUEST_UNREAD:
                self.sendUnreadMessages()

            elif header == NUM_MESSAGE_REQUEST_LIST:
                self.sendMessagesList()

            elif header == NUM_MESSAGE_REQUEST_COUNT:
                self.sendMessagesCount()

            elif header == NUM_MESSAGE_SEND_REQUEST:
                name = str(message_parts[0])
                phone = str(message_parts[1])
                enc = str(message_parts[2])
                msg = str(message_parts[3]).decode("string_escape")
                self.sendMessage(name, phone, enc, msg)

            elif header == NUM_SET_READ:
                id = int(message_parts[0])
                state = bool(message_parts[1])
                self.setRead(id, state)

            elif header == NUM_MESSAGE_DELETE:
                id = int(message_parts[0])
                self.messageDelete(id)

            elif header == NUM_SCREENSHOT:
                self.sendScreenshot()

            elif header == NUM_QUIT:
                self.send(NUM_QUIT)
                break

    def sendSysinfo(self, full):
        self.send(NUM_SYSINFO_REPLY_START)
        self.send(NUM_SYSINFO_REPLY_LINE, "program_version", VERSION)
        self.send(NUM_SYSINFO_REPLY_LINE, "battery", sysinfo.battery())
        self.send(NUM_SYSINFO_REPLY_LINE, "active_profile", sysinfo.active_profile())
        self.send(NUM_SYSINFO_REPLY_LINE, "free_ram", sysinfo.free_ram())
        self.send(NUM_SYSINFO_REPLY_LINE, "pys60_version", e32.pys60_version)

        if sysinfo.active_profile() == "offline":
            # Return an error code if the phone is in offline mode
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_dbm", -1)
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_bars", -1)
        else:
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_dbm", sysinfo.signal_dbm())
            self.send(NUM_SYSINFO_REPLY_LINE, "signal_bars", sysinfo.signal_bars())

        for drive, free in sysinfo.free_drivespace().items():
            self.send(NUM_SYSINFO_REPLY_LINE, "free_drivespace", str(drive) + str(free))

        if full:
            self.send(
                NUM_SYSINFO_REPLY_LINE,
                "display",
                str(sysinfo.display_pixels()[0])
                + "x"
                + str(sysinfo.display_pixels()[1]),
            )
            self.send(NUM_SYSINFO_REPLY_LINE, "imei", sysinfo.imei())
            self.send(NUM_SYSINFO_REPLY_LINE, "model", sysinfo.sw_version())
            self.send(
                NUM_SYSINFO_REPLY_LINE,
                "s60_version",
                e32.s60_version_info[0],
                e32.s60_version_info[1],
            )
            self.send(NUM_SYSINFO_REPLY_LINE, "total_ram", sysinfo.total_ram())
            self.send(NUM_SYSINFO_REPLY_LINE, "total_rom", sysinfo.total_rom())

        self.send(NUM_SYSINFO_REPLY_END)

    def sendScreenshot(self):
        fn = self.getScreenshotFilename()
        shot = graphics.screenshot()
        shot.save(fn)
        note("Saved screenshot as %s" % fn)
        f = file(fn, "rb")
        self.send(NUM_SCREENSHOT_REPLY, f.read().encode("base64"))
        f.close()

    def sendLocation(self):
        loc = location.gsm_location()
        if loc is None:
            self.send(NUM_LOCATION_REPLY_NA)
        else:
            mcc, mnc, lac, cellid = loc
            self.send(
                NUM_LOCATION_REPLY,
                "%03d" % mcc,
                "%02d" % mnc,
                "%X" % lac,
                "%X" % cellid,
            )

    def contactDict(self):
        keys = list(self.contactDb.keys())

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
                value = str(value)
                value = value.replace(
                    "\u2029", "\n"
                )  # PARAGRAPH SEPARATOR (\u2029) replaced by LINE FEED (\u000a)
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

                contactDict[contact.id].append((_type, location, value))
            contactDict[contact.id].sort()

        return contactDict

    def sendContactHash(self):
        contacts = self.contactDict()
        keys = list(contacts.keys())
        keys.sort()

        hash = ""

        for key in keys:
            hash += str(key)
            hash += FIELD_SEP
            for _type, location, value in contacts[key]:
                hash += _type + INFO_SEP + location + INFO_SEP + value
                hash += FIELD_SEP
            hash += CONTACT_SEP

        hash = hash.encode("utf8")
        hash = md5.md5(hash).hexdigest()
        self.send(NUM_CONTACTS_REPLY_HASH_ALL, hash)

    def sendContactHashSingle(self):
        self.send(NUM_CONTACTS_REPLY_HASH_SINGLE_START)

        contacts = self.contactDict()
        keys = list(contacts.keys())
        keys.sort()

        for key in keys:
            hash = ""
            for _type, location, value in contacts[key]:
                hash += _type + INFO_SEP + location + INFO_SEP + value
                hash += FIELD_SEP

            hash = hash.encode("utf8")
            hash = md5.md5(hash).hexdigest()
            self.send(NUM_CONTACTS_REPLY_HASH_SINGLE_LINE, key, hash)

        self.send(NUM_CONTACTS_REPLY_HASH_SINGLE_END)

    def sendAllContacts(self):
        keys = list(self.contactDb.keys())

        for key in keys:
            contact = self.contactDb[key]
            self.sendContact(contact)
        self.send(NUM_CONTACTS_REPLY_CONTACTS_ALL_END)

    def sendContact(self, contact):
        # There could be an empty entry in the contact database
        # In this case contact.title would report the following error:
        # File "c:\resource\contacts.py", line 293, in _get_title
        #   title_str += self._contact.get_field(index)['value'] + u" "
        # TypeError: unsupported operand types for +: 'NoneType' and 'unicode'
        #
        # I think the best way is to ignore such errors...
        try:
            self.send(NUM_CONTACTS_REPLY_CONTACT_START, contact.id, contact.title)
        except TypeError:
            return

        for field in contact:
            _type = field.type
            value = field.value
            value = str(value)
            value = value.replace(
                "\u2029", "\n"
            )  # PARAGRAPH SEPARATOR (\u2029) replaced by LINE FEED (\u000a)
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

            self.send(
                NUM_CONTACTS_REPLY_CONTACT_LINE, contact.id, _type, location, value
            )
        self.send(NUM_CONTACTS_REPLY_CONTACT_END, contact.id)

    def modifyContact(self, modification, id, type, location, value):
        try:
            contact = self.contactDb[id]
        except:
            return

        if type == "thumbnail_image":
            if modification == "remove":
                self.setContactThumbnail(contact)
            else:
                self.setContactThumbnail(contact, value)
            return
        elif type == "date":
            if modification == "remove":
                self.setContactBirthday(contact)
            else:
                self.setContactBirthday(contact, value)
            return

        contact.begin()

        if modification == "add":
            contact.add_field(type, value, location=location)
        elif modification == "remove":
            index = -1
            for field in contact.find(type, location):
                if field.value == value:
                    index = field.index
                    break

            if index != -1:
                del contact[index]

        contact.commit()

    def getDetailFromVcard(self, contact, detail, delimiter="\r\n"):
        # This is an ugly hack, needed for some fields that cannot be handled using the contact object
        try:
            value = str(contact.as_vcard(), "utf8")
            value = value.split(detail + ":")[1].split(delimiter)[0]
            return value
        except:
            return

    def setDetailFromVcard(self, contact, detail, value, delimiter="\r\n"):
        # This is an ugly hack, needed for some fields that cannot be handled using the contact object
        card = contact.as_vcard()

        new = ""
        for line in card.split("\r\n"):
            if (
                line.startswith("BEGIN:")
                or line.startswith("VERSION:")
                or line.startswith("REV:")
                or line.startswith("UID:")
            ):
                new += line + "\r\n"

        # Format value: New line (\r\n) after 64 chars, followed by 4 spaces
        if len(value) > 64:
            fmtvalue = "\r\n"
            for i in range(len(value) / 64 + 1):
                fmtvalue += value[i * 64 : (i + 1) * 64] + "\r\n" + 4 * " "
        else:
            fmtvalue = value

        new += detail + ":" + fmtvalue + delimiter
        new += "END:VCARD"

        changed_contact = self.contactDb.import_vcards(new)[0]
        assert changed_contact.id == contact.id

    def getContactThumbnail(self, contact):
        # Ugly workaround!
        # HACK: The value of type "thumbnail_image" is empty, it is only shown when we export the contact to a vCard
        image = self.getDetailFromVcard(
            contact, "PHOTO;TYPE=JPEG;ENCODING=BASE64", "\r\n\r\n"
        )
        if image:
            image = image.split("\r\n\r\n")[0]
            image = image.replace("\r", "").replace("\n", "").replace(" ", "")
            return image
        return

    def setContactThumbnail(self, contact, image=""):
        # Ugly workaround!
        # HACK: There seems to be new other way to update/add the contact picture
        self.setDetailFromVcard(
            contact, "PHOTO;TYPE=JPEG;ENCODING=BASE64", image, "\r\n\r\n"
        )

    def getContactBirthday(self, contact):
        return self.getDetailFromVcard(contact, "BDAY")

    def setContactBirthday(self, contact, date=""):
        # HACK: It isn't possible to set birthdays < year 1970 (before the beginning of the unix epoch)
        self.setDetailFromVcard(contact, "BDAY", date)

    def __calendarGetType(self, entry):
        if isinstance(entry, calendar.CalendarDb.AnniversaryEntry):
            return "anniversary"
        elif isinstance(entry, calendar.CalendarDb.AppointmentEntry):
            return "appointment"
        elif isinstance(entry, calendar.CalendarDb.EventEntry):
            return "event"
        elif isinstance(entry, calendar.CalendarDb.ReminderEntry):
            return "reminder"
        elif isinstance(entry, calendar.CalendarDb.TodoEntry):
            return "todo"
        return ""

    def __calendarFormatEntry(self, entry, sep):
        entryType = self.__calendarGetType(entry)
        if not entryType:
            # Type could also be TodoListDict or TodoList, which is currently not handled...
            return ""

        line = entryType + sep + entry.content + sep + entry.location + sep
        if entry.start_time:
            # None if the start datetime of the entry is not set
            line += str(int(entry.start_time))  # entry.start_time is of type float
        line += sep

        if entry.end_time:
            line += str(int(entry.end_time))
        line += sep

        line += str(int(entry.last_modified)) + sep
        line += entry.replication + sep  # open, private or restricted

        if entry.alarm:
            line += str(
                int(entry.alarm)
            )  # The alarm datetime value (float) for the entry
        line += sep

        line += str(entry.priority) + sep

        # Format repeat...
        repeat = entry.get_repeat()
        repeated = bool(repeat)

        if repeated:
            line += repeat[
                "type"
            ]  # daily, weekly, monthly_by_dates, monthly_by_days, yearly_by_date, yearly_by_day
        line += sep

        # It is really hard to extract the days...
        # We try to seperate the values by ',' and different groups by ';'
        if repeated and "days" in repeat:
            days = repeat["days"]

            if isinstance(days, (list, tuple)):
                if len(days) == 0:
                    pass
                elif isinstance(days[0], int):
                    # Which days in week? (0=Monday,...)
                    # for example: [0,1,2]
                    line += ",".join(str(day) for day in days)
                elif isinstance(days[0], dict):
                    # example: on second Tuesday and last Monday of the month
                    # days is: [{'week': 1, 'day': 1}, {'week': 4, 'day': 0}]
                    # results in: 'week:1,day:1;week:4,day:0'
                    line += ";".join(
                        ",".join(
                            key + ":" + str(value) for key, value in list(entry.items())
                        )
                        for entry in days
                    )
            elif isinstance(days, dict):
                # for example: {'week': 1, 'day': 1, 'month': 1}
                # results in: 'week:1,day:1,month:1'
                line += ",".join(key + ":" + str(value) for key, value in days.items())
        line += sep

        if repeated and "exceptions" in repeat:
            line += ",".join(str(int(day)) for day in repeat["exceptions"])
        line += sep

        if repeated and "start" in repeat:
            line += str(int(repeat["start"]))
        line += sep

        if repeated and "end" in repeat:
            end = int(repeat["end"])
            end -= time.timezone
            if end == 4102441200:
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
                line += str(int(entry.cross_out_time))
            line += sep

            # TODO lists no more supported!
            # line += entry.todo_list + sep  # The ID of the TODO list to which this entry belongs

        return line

    def calendarDict(self):
        calendarDict = dict()
        for key in self.calendarDb:
            entry = self.calendarDb[key]
            line = self.__calendarFormatEntry(entry, FIELD_SEP)
            if not line:
                continue

            calendarDict[entry.id] = line

        return calendarDict

    def sendCalendarCount(self):
        todos = 0
        calendars = 0
        for key in self.calendarDb:
            entry = self.calendarDb[key]
            entryType = self.__calendarGetType(entry)
            if entryType in ["todo"]:
                todos = todos + 1
            elif entryType != "":
                calendars = calendars + 1
        self.send(NUM_CALENDAR_REPLY_COUNT, len(self.calendarDb), calendars, todos)

    def sendCalendarHash(self):
        calendarDict = self.calendarDict()

        keys = list(calendarDict.keys())
        keys.sort()

        hash = ""
        for key in keys:
            hash += str(key)
            hash += FIELD_SEP
            hash += calendarDict[key]
            hash += FIELD_SEP
            hash += ENTRY_SEP

        # self.send(NUM_DEBUG,  "Mobile:::" + hash.replace(FIELD_SEP,  ";").replace(ENTRY_SEP,  "\n"))

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
            self.sendCalendarEntry(entry)
        self.send(NUM_CALENDAR_REPLY_ENTRIES_END)

    def sendCalendarEntry(self, entry):
        self.send(
            NUM_CALENDAR_REPLY_ENTRY,
            entry.id,
            self.__calendarFormatEntry(entry, NUM_SEPERATOR),
        )

    def modifyCalendarEntry(
        self,
        id,
        content,
        location,
        start,
        end,
        replication,
        alarm,
        priority,
        repeat_type,
        repeat_days,
        repeat_exceptions,
        repeat_start,
        repeat_end,
        repeat_interval,
    ):

        try:
            entry = self.calendarDb[id]
        except:
            return

        entry.begin()

        self.modifyCalendarEntryFields(
            entry,
            content,
            location,
            start,
            end,
            replication,
            alarm,
            priority,
            repeat_type,
            repeat_days,
            repeat_exceptions,
            repeat_start,
            repeat_end,
            repeat_interval,
        )

        entry.commit()

        self.send(
            NUM_CALENDAR_ENTRY_CHANGE_REPLY_TIME, id, str(int(entry.last_modified))
        )

    def addCalendarEntry(
        self,
        type,
        content,
        location,
        start,
        end,
        replication,
        alarm,
        priority,
        repeat_type,
        repeat_days,
        repeat_exceptions,
        repeat_start,
        repeat_end,
        repeat_interval,
    ):

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

        self.modifyCalendarEntryFields(
            entry,
            content,
            location,
            start,
            end,
            replication,
            alarm,
            priority,
            repeat_type,
            repeat_days,
            repeat_exceptions,
            repeat_start,
            repeat_end,
            repeat_interval,
        )

        entry.commit()

        self.send(NUM_CALENDAR_ENTRY_ADD_REPLY, entry.id, str(int(entry.last_modified)))

    def modifyCalendarEntryFields(
        self,
        entry,
        content,
        location,
        start,
        end,
        replication,
        alarm,
        priority,
        repeat_type,
        repeat_days,
        repeat_exceptions,
        repeat_start,
        repeat_end,
        repeat_interval,
    ):
        entry.content = content
        entry.location = location
        entry.set_time(start, end)
        entry.replication = replication
        entry.alarm = alarm
        entry.priority = priority

        # Clear the old repeat
        entry.set_repeat(None)

        entry.set_repeat(
            self.buildCalendarEntryRepeat(
                repeat_type,
                repeat_days,
                repeat_exceptions,
                repeat_start,
                repeat_end,
                repeat_interval,
            )
        )

    def buildCalendarEntryRepeat(self, type, days, exceptions, start, end, interval):
        if not type:
            return None
        if type not in (
            "weekly",
            "monthly_by_dates",
            "monthly_by_days",
            "yearly_by_day",
            "yearly_by_date",
        ):
            return None

        days = self.calendarEntryParsedDays(type, days)
        if exceptions:
            exceptions = [float(exception) for exception in exceptions.split(",")]
        else:
            exceptions = []

        if end is None:
            # 4102441200. = 2100-01-01T00:00:00
            # There is a bug in PyS60, which causes that None as end date throws as error
            # As workarond, we use 2100-01-01 as end date for eternal repeats
            # Bug report: http://developer.symbian.org/bugs/show_bug.cgi?id=3666
            # We also can't use time.mktime([2100, 1, 1, 0, 0, 0, 1, 1, -1]), cause this wouls result in
            # an integer overflow
            end = 4102441200.0

        # We need to subtract one day from the end date, because
        # endTime+=TTimeIntervalDays(1);
        # in calendarmodule.h adds one day after set_repeat is called
        end -= 24 * 60 * 60

        return {
            "type": type,
            "days": days,
            "exceptions": exceptions,
            "start": start,
            "end": end,
            "interval": interval,
        }

    def calendarEntryParsedDays(self, type, days):

        ### PLEASE COPY CHANGES IN THIS FUNCTION ALSO TO
        ### pc/lib/classes.py, class CalendarEntry function recurrenceParsedDays

        if type in ("weekly", "monthly_by_dates"):
            # 0,1,2 -> [0, 1, 2]
            return [int(day) for day in days.split(",")]
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
                key, value = subentry.split(":")
                dates[key] = int(value)
            return dates

    def sendOneMessage(self, sms):
        box = "inbox"
        if sms in self.sent.sms_messages():
            box = "sent"
        self.__sendOneMessage(sms, box, NUM_MESSAGE_REPLY_ONE)

    def __sendOneMessage(self, sms, box, code):
        id = sms
        try:
            time = self.inbox.time(sms)
        except:
            self.send(code, "")
            return
        address = self.inbox.address(sms)
        content = self.inbox.content(sms)
        content = content.replace(
            "\u2029", "\n"
        )  # PARAGRAPH SEPARATOR (\u2029) replaced by LINE FEED (\u000a)

        if self.inbox.unread(sms):
            unread = "1"
        else:
            unread = "0"

        self.send(code, box, id, time, address, content, unread)

    def sendAllMessages(self, lastId):
        messages = list()
        inbox = list()
        sent = list()
        for box in ("inbox", "sent"):
            # FIXME: I shouldn't need this
            e32.ao_sleep(1)

            if box == "inbox":
                inbox = self.inbox.sms_messages()
            else:
                sent = self.sent.sms_messages()

        messages = inbox + sent
        messages.sort()
        for sms in messages:
            if int(sms) > int(lastId):
                if sms in inbox:
                    box = "inbox"
                else:
                    box = "sent"

                self.__sendOneMessage(sms, box, NUM_MESSAGE_REPLY_LINE)

        self.send(NUM_MESSAGE_REPLY_END)

    def sendUnreadMessages(self):
        messages = list()
        inbox = self.inbox.sms_messages()
        for sms in inbox:
            if self.inbox.unread(sms):
                messages.append(sms)
        self.send(NUM_MESSAGE_REPLY_UNREAD, *messages)

    def sendMessagesList(self):
        inbox = self.inbox.sms_messages() + self.sent.sms_messages()
        for i in inbox:
            self.send(NUM_MESSAGE_REPLY_LIST, i)
        self.send(NUM_MESSAGE_REPLY_END)

    def sendMessagesCount(self):
        messages = list()
        inbox = self.inbox.sms_messages() + self.sent.sms_messages()
        for sms in inbox:
            if self.inbox.unread(sms):
                messages.append(sms)
        self.send(NUM_MESSAGE_REPLY_COUNT, len(inbox), len(messages))

    def sendMessage(self, name, phone, encoding, msg):
        try:
            messaging.sms_send(phone, msg, encoding, self.sentMessage, name)
        except RuntimeError as detail:
            if str(detail) == "Already sending":
                # Workaround for the "Already sending" bug:
                # http://discussion.forum.nokia.com/forum/showthread.php?t=141083
                messaging._sending = False
                self.send(
                    NUM_MESSAGE_SEND_REPLY_RETRY, str(detail) + "; tried workaround"
                )
            else:
                self.send(NUM_MESSAGE_SEND_REPLY_RETRY, detail)

    def sentMessage(self, status):
        if status == messaging.ECreated:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS, "Message created.")
        elif status == messaging.EMovedToOutBox:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS, "Moved to outbox.")
        elif status == messaging.EScheduledForSend:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS, "Scheduled for send.")
        elif status == messaging.ESent:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS, "Message sent.")
        elif status == messaging.EDeleted:
            self.send(
                NUM_MESSAGE_SEND_REPLY_OK,
                "The SMS message has been deleted from device's outbox queue.",
            )
        elif status == messaging.EScheduleFailed:
            self.send(NUM_MESSAGE_SEND_REPLY_FAILURE, "Schedule failed.")
        elif status == messaging.ESendFailed:
            self.send(
                NUM_MESSAGE_SEND_REPLY_FAILURE,
                "The SMS subsystem has tried to send the message several times in vain.",
            )
        elif status == messaging.ENoServiceCentre:
            self.send(NUM_MESSAGE_SEND_REPLY_FAILURE, "No service centre.")
        elif status == messaging.EFatalServerError:
            self.send(
                NUM_MESSAGE_SEND_REPLY_FAILURE,
                "SMS send failed! If the device is in offline-mode or with no network connection the message is added to the device's outgoing message queue.",
            )
        else:
            self.send(NUM_MESSAGE_SEND_REPLY_STATUS, "Unknown status: %s" % status)

    def newMessage(self, sms):
        if not self.connected:
            return

        # FIXME: I shouldn't need this
        e32.ao_sleep(1)

        id = sms
        time = self.inbox.time(sms)
        address = self.inbox.address(sms)
        content = self.inbox.content(sms)

        self.send(NUM_MESSAGE_NEW, id, time, address, content)

    def handleCall(self, handle):
        state = handle[0]
        number = handle[1]
        call_state = {
            telephone.EStatusUnknown: "unknown",
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
            telephone.EStatusTransferAlerting: "transfer alerting",
        }

        self.send(NUM_INCOMING_CALL, number, call_state[state])

    def messageDelete(self, id):
        self.inbox.delete(id)

    def setRead(self, id, state):
        state = int(not state)

        # FIXME: I shouldn't need this
        e32.ao_sleep(1)

        self.inbox.set_unread(id, state)

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
        ret = query("Enter bluetooth port to use", "number", self.port)
        if ret is not None:
            self.port = ret
            self.quit()
            self.saveConfig()
            self.startService()

    def aboutHandler(self):
        query("Gammu S60 Remote\nVersion %s\nhttps://wammu.eu/" % (VERSION), "query")

    def toggleHandler(self):
        e32.start_exe("BtToggleApp.exe", "")


# Debug of SIS applications
try:
    mobile = Mobile()
except Exception as e:
    # Oops, something wrong. Report problems to user
    # and ask him/her to send them to you.
    import traceback

    new_line = "\u2029"

    # Collecting call stack info
    info = sys.exc_info()

    # Show the last 4 lines of the call stack
    call_stack = ""
    for filename, lineno, function, text in traceback.extract_tb(info[2]):
        call_stack += filename + ": " + str(lineno) + " - " + function + new_line
        call_stack += " " + repr(text) + new_line
    call_stack += "%s: %s" % info[:2]

    # Creating a friendly user message with exception details
    err_msg = "This programs was unexpectedly closed due to the following error: "
    err_msg += str(repr(e)) + new_line
    err_msg += "Please, copy and paste the text presented here and "
    err_msg += "send it to gammu-users@lists.sourceforge.net. "
    err_msg += "Thanks in advance and sorry for this inconvenience." + new_line * 2
    err_msg += "Call stack:" + new_line + call_stack

    # Small PyS60 application
    lock = e32.Ao_lock()
    app.body = Text(err_msg)
    app.body.set_pos(0)
    app.menu = [("Exit", lambda: lock.signal())]
    lock.wait()
