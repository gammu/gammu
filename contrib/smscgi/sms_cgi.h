/*
 * This file part of smscgi
 *
 * Copyright (C) 2007  Kamanashis Roy (kamanashisroy@gmail.com)
 *
 * smscgi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * smscgi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with smscgi.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SMS_CGI_H
#define SMS_CGI_H




extern GSM_Error error;
extern char cgi_path[200];

/**
 * Implementation of GSM_SMSCallback function.
 */
void cgi_enqueue(GSM_StateMachine *s, GSM_SMSMessage sms, void *user_data);

/**
 * Process the SMSs
 */
void cgi_process(GSM_StateMachine *s);

/**
 * Reset the processing queue
 */
void cgi_reset();

#endif        //  #ifndef SMS_CGI_H

