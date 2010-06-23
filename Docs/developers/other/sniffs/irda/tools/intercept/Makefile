#
#/*********************************************************************
# *
# * Filename:      Makefile
# * Version:
# * Description:   Makefile for irda_intercept
# * Status:        Experimental.
# * Author:        Thomas Schneider
# * Created at:    Thomas Schneider <nok-trace-men@dev-thomynet.de>
# * Modified at:   Sun Jul 18 12:03:03 1999
# * Modified by:   Thomas Schneider <nok-trace-men@dev-thomynet.de>
# *
# *     Copyright (c) 1999 Thomas Schneider, All Rights Reserved.
# *
# *     This program is free software; you can redistribute it and/or
# *     modify it under the terms of the GNU General Public License as
# *     published by the Free Software Foundation; either version 2 of
# *     the License, or (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful, but
# * WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# * General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software 
# * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# *
# * IN NO EVENT SHALL THOMAS SCHNEIDER BE LIABLE TO ANY PARTY FOR
# * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES 
# * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
# * IF THOMAS SCHNEIDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH 
# * DAMAGE.
# *
# * THOMAS SCHNEIDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
# * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
# * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER
# * IS ON AN "AS IS" BASIS, AND THOMAS SCHNEIDER HAS NO OBLIGATION TO 
# * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR 
# * MODIFICATIONS.
# *
# *         This material is provided "AS-IS" and at no charge.
# *
# ********************************************************************/

#
# Serial port for communication
#

MODEMDEVICE=-DMODEMDEVICE="\"/dev/ttyS1\""

COMMON=-Wall -O2 \
       ${MODEMDEVICE}

all: irda_intercept


irda_intercept: irda_intercept.o ../fcs/fcs.o
	cc ${COMMON} \
			`gtk-config --cflags` `gtk-config --libs`\
			irda_intercept.o ../fcs/fcs.o\
			-o irda_intercept

irda_intercept.o: irda_intercept.c ../include/irda.h ../include/irlap.h
	cc ${COMMON} \
			`gtk-config --cflags` \
			-c irda_intercept.c

../fcs/fcs.o: ../fcs/fcs.c ../fcs/fcs.h
	cc ${COMMON} \
			`gtk-config --cflags` \
			-c $*.c -o $@
