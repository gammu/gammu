SAMSUNG Organizer AT commands
=============================

Get organizer information
+++++++++++++++++++++++++

Invocation::

    AT+ORGI?

Example::

    AT+ORGI?
    +ORGI: 84,400,30,100,30
    OK

Return 5 values:

par1
    ( 84) Busy entries (1 to par1 of par2 possibles entries)
par2
    (400) Max possible entries
par3
    ( 30) Unknown
par4
    (100) Unknown
par5
    ( 30) Unknown


Get organizer details
+++++++++++++++++++++

Invocation::

    AT+ORGR=number

Get organizer details for index entry "number"
Returns 24 values:

Example 1::

    AT+ORGR=10
    +ORGR: 161,1,"Comprar lagrimas artificiales","Farmacia",2,4,2009,9,0,2,4,2009,9,10,"Farmacia",1,1,0,3,,,29,1,2010
    OK

Example 2::

    AT+ORGR=15
    +ORGR: 67,2,,"Laura Santiesteban Cabrera",3,11,2009,9,0,,,,,,,1,3,0,4,,,,,
    OK

Example 3::

    AT+ORGR=19
    +ORGR: 205,3,,"Cemento",13,3,2009,10,35,13,3,2009,,,,1,3,0,0,1,0,,,
    OK

Example 4::

    AT+ORGR=23
    +ORGR: 235,4,"Curso","Averiguar",13,3,2009,9,50,13,3,2009,9,59,,1,1,0,,,,,,
    OK

+ORGR: AT+ORGR answer header

par01
    Pointer to real memory position
par02
    Organizer entry type (1=appointments, 2=aniversaries, 3=tasks, 4=miscellany )

If par02 =1, appointment entry type

par03
    Organizer entry short name
par04
    Organizer entry detailed description
par05
    Start day
par06
    Start month
par07
    Start year
par08
    Start hour
par09
    Start minute
par10
    End day
par11
    End month
par12
    End year
par13
    End hour
par14
    End minute
par15
    Location
par16
    Alarm flag (0=no, 1=yes)
par17
    Alarm time unit (1=minutes, 2=hours, days, 4=weeks)
par18
    Alarm items quantity
par19
    Alarm repeat flag (0 or empty=no, 2=yes)
par20
    Empty
par21
    Empty
par22
    Repeat until day
par23
    Repeat until month
par24
    Repeat until year

If par02 = 2, aniversary entry type

par03
    Empty
par04
    Ocassion name
par05
    Alarm day
par06
    Alarm month
par07
    Alarm year
par08
    Alarm hour
par09
    Alarm minutes
par10
    Empty
par11
    Empty
par12
    Empty
par13
    Empty
par14
    Empty
par15
    Empty
par16
    Alarm flag (0=no, 1=yes)
par17
    Alarm time unit (1=minutes, 2=hours, days, 4=weeks)
par18
    Alarm items quantity
par19
    Repeat each year (0=no, 4=yes)
par20
    Empty
par21
    Empty
par22
    Empty
par23
    Empty
par24
    Empty

If par02 = 3, task entry type

par03
    Empty
par04
    Task name
par05
    Start day
par06
    Start month
par07
    Start year
par08
    Alarm hour
par09
    Alarm minute
par10
    Due day
par11
    Due month
par12
    Due year
par13
    Empty
par14
    Empty
par15
    Empty
par16
    Alarm flag (0=no, 1=yes)
par17
    Alarm time unit (1=minutes, 2=hours, days, 4=weeks)
par18
    Alarm items quantity
par19
    Empty
par20
    Task priority (1=high, 2=normal, 3=low)
par21
    Task status (0=undone, 1=done)
par22
    Empty
par23
    Empty
par24
    Empty

If par02 = 4, miscellany entry type

par03
    Entry name
par04
    Details
par05
    Start day
par06
    Start month
par07
    Start year
par08
    Start hour
par09
    Start minutes
par10
    End day
par11
    End month
par12
    End year
par13
    End hour
par14
    End minutes
par15
    Empty
par16
    Alarm flag (0=no, 1=yes)
par17
    Alarm time unit (1=minutes, 2=hours, days, 4=weeks)
par18
    Alarm items quantity
par19
    Empty
par20
    Empty
par21
    Empty
par22
    Empty
par23
    Empty
par24
    Empty

Write organizer entry
+++++++++++++++++++++

Invocation::

    AT+ORGW=par0,par1,par2...par24

Write organizer entry in memory location par0

If par0=65535 then locate next empty entry on memory

Example::

    AT+ORGW=65535,0,4,"p2","p2",14,3,2009,2,23,14,3,2009,3,23,,0,0,0,,,,,,
    +ORGW: 253,253
    OK

par1 to par24 has the same significance than in the AT+ORGR command

Delete organizer entry
++++++++++++++++++++++

Invocation::

    AT+ORGD=number

Delete organizer entry of index "number"

Example::

    AT+ORGD=21

    OK

Notes
+++++

Read command use index reference.

Write command uses index and direct memory reference with special 65535 value to locate empty memory position.

Delete command use direct memory reference, index are automatically reorganized.

Hint: After create or delete an organizer entry, reread full information to update index table.

