@echo off
bcc /c gnokii.c
bcc /c ../common/gsmcomon.c
bcc /c ../common/gsmstate.c
bcc /c ../common/device/serial/dos.c
bcc /c ../common/protocol/nokia/fbus2.c
bcc /c ../common/protocol/nokia/mbus2.c
bcc /c ../common/phone/nokia/nokia.c
bcc /c ../common/phone/nokia/n6110.c
bcc /c ../common/phone/nokia/n7110.c
bcc /c ../common/misc/coding.c
bcc /c ../common/misc/misc.c
bcc /c ../common/service/gsmdate.c



tlib ala.lib +gsmcomon.obj
tlib ala.lib +gsmstate.obj
tlib ala.lib +dos.obj
tlib ala.lib +fbus2.obj
tlib ala.lib +mbus2.obj
tlib ala.lib +nokia.obj
tlib ala.lib +n6110.obj
tlib ala.lib +n7110.obj
tlib ala.lib +coding.obj
tlib ala.lib +misc.obj
tlib ala.lib +gsmdate.obj



