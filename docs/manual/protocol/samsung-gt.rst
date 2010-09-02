SAMSUNG GT calendar AT commands
===============================

Calendar Entries
----------------

``AT+SSHT=1`` - selects the Organizer->Calendar->Appointment entries
(Spotkania in Polish version)

``AT+SSHT=2`` - selects the Organizer->Calendar->Anniversary entries
(Rocznice in Polish version)

``AT+SSHT=5`` - selects the Organizer->Calendar->Holiday entries (Święta
in Polish version)

``AT+SSHT=6`` - selects the Organizer->Calendar->Important entries (Ważne
in Polish version)

``AT+SSHT=7`` - selects the Organizer->Calendar->Private entries (Prywatne
in Polish version)

After selection of type, we can read all items::

    AT+SSHR=0
    +SSHR:5,"5,test1","0,","0,",2010,5,12,2010,5,12,21,49,22,49,0,0,0,0,2010,5,30,,
    +SSHR:3,"1,x","0,","0,",2010,6,2,2010,6,3,0,0,0,0,0,0,0,0,2010,5,30,,
    +SSHR:1,"9,event1234","0,","0,",2010,6,7,2010,6,7,7,0,8,59,0,0,0,0,2010,5,30,,
    +SSHR:4,"7,test123","0,","0,",2010,6,14,2010,6,14,21,37,22,37,0,0,0,0,2010,5,30,,
    +SSHR:2,"7,Meeting","0,","0,",2010,6,15,2010,6,15,8,0,8,59,0,0,0,0,2010,5,30,,
    OK

Or just read a single item::

    AT+SSHR=1
    +SSHR:1,"9,Event 123","0,","0,",2010,6,7,2010,6,7,7,0,8,59,0,0,0,0,2010,5,30,,
    OK

Getting status (the last number appears to be number of notes)::

    AT+SSHR=?
    +SSHR:100,15,100,15,"1000000",2008,2024,5
    OK

You can also add or modify an item::

    AT+SSHW="7,event01","16,details of event","5,where",2010,06,03,2010,06,04,12,31,13,42,0,0,0,0,2010,05,31,,,0

It seems, that the last number in the above record specifies whether
it is addition of a new record (0), or modification of the old record
(then the number is the position of the item, as the first number
listed after ``AT+SSHR=0``). e.g.::

    AT+SSHW="13,event1234 new","0,","0,",2010,06,07,2010,06,07,07,00,08,59,0,0,0,0,2010,05,30,,,1


Please note, that the format for writing is somehow different, than
for reading - hour and minuts must be in two-digit form!
The text fields (as shown above) are formatted in the following way:
"number_of_characters_in_string,string"
In all items above the first string is the name of event, the second
string - details of event, the third one - place of event.
The numeric fields encode start date (year,month,day), end date (year,
month, day), start time (hour,minutes),
end time (hour, minutes), four unknown to me (yet?) values, date of
creation? (year month day) - the meaning of this date is not sure for
me yet.

To delete entries::

    AT+SSHD=1
    OK

Task Entries
------------

There is yet another type, that can be selected by ``AT+SSHT=3``
This is Organizer->Task::

    AT+SSHT=3
    OK
    AT+SSHR=0
    +SSHR:1,"10,Test event","10,2010-06-05",60823,11,25,60823,11,26,0,0,0,0,0,0,0,0,0
    OK

Please note, that the format of output is different, when you read the
specific task::

    AT+SSHR=1
    +SSHR:1,"10,Test event","12,Some details",2010,6,3,2010,6,5,1,2010,6,4,10,11,0,2,0

You can similarly add  a new task::

    AT+SSHW="9,New task1","10,0123456789",2010,06,21,2010,06,30,1,2010,06,27,08,07,0,2,0,0
    +SSHW:2
    OK

Read it back::

    AT+SSHR=2
    +SSHR:2,"9,New task1","10,0123456789",2010,6,21,2010,6,30,1,2010,6,27,8,7,0,2,0
    OK

And modify::

    AT+SSHW="9,New task1","11,New details",2010,06,21,2010,06,30,1,2010,06,27,08,07,0,2,0,2
    +SSHW:2
    OK
    AT+SSHR=2
    +SSHR:2,"9,New task1","11,New details",2010,6,21,2010,6,30,1,2010,6,27,8,7,0,2,0
    OK

To delete entries::

    AT+SSHT=3
    OK
    AT+SSHR=0
    +SSHR:1,"10,Test
    event","10,2010-06-05",60823,11,25,60823,11,26,0,60823,11,26,0,0,0,0,0
    +SSHR:2,"9,New task1","10,2010-06-30",60823,11,25,60823,11,26,0,60823,11,26,0,0,0,0,0
    OK
    AT+SSHD=1
    OK
    AT+SSHR=0
    +SSHR:2,"9,New task1","10,2010-06-30",60823,11,25,60823,11,26,0,0,0,0,0,0,0,0,0
    OK

Memo Notes
----------

The memo notes are accessible via ``AT+OMM???`` commands::

    AT+OMMI?
    +OMMI:4,100,100

We found, that we have 4 memos

You can add a note::

    AT+OMMW=0,"This is a note"
    +OMMW:6
    OK

You can read it::

    AT+OMMR=6
    +OMMR:"This is a note"
    OK

You can modify it::

    AT+OMMW=6,"This is a new modified note"
    +OMMW:6
    OK
    AT+OMMR=6
    +OMMR:"This is a new modified note"
    OK

To delete entries::

    AT+OMMR=3
    +OMMR:"Note number 3"
    OK
    AT+OMMD=3
    OK
    AT+OMMW=3,"New note number 3"
    +CME ERROR:29

    ERROR
