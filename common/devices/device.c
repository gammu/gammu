/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef WIN32
  #include "devices/unixserial.h"
  #include "devices/unixirda.h"
  #include "devices/tekram.h"
  #include <sys/ioctl.h>
  #include <pthread.h>
  #include <termios.h>
  #include <signal.h>
  #include <errno.h>
#else
  #include <windows.h>
  #include "devices/winserial.h"
#endif

#include "gsm-api.h"
#include "devices/device.h"
#include "files/cfgreader.h"

#include <string.h>

#ifdef VC6
  /* for VC6 make scripts save VERSION constant in mversion.h file */
  #include "mversion.h"
#endif

char PortDevice[GSM_MAX_DEVICE_NAME_LENGTH]={0x00};

bool duringwrite;

#ifndef WIN32

char *lockfile;

//pthread_t Thread;
#if defined(__svr4__) || defined(__FreeBSD__)
  pthread_t selThread;
#endif

int device_portfd = -1;

#define max_buf_len 128
#define lock_path "/var/lock/LCK.."

/* Lock the device. Return allocated string with a lock name */
char *lock_device(const char* port)
{
	char *lock_file = NULL;
	char buffer[max_buf_len];
	const char *aux = rindex(port, '/');
	int fd, len = strlen(aux) + strlen(lock_path);

	/* Remove leading '/' */
	if (aux) aux++;
	else aux = port;

	memset(buffer, 0, sizeof(buffer));
	lock_file = calloc(len + 1, 1);
	if (!lock_file) {
		fprintf(stderr, _("Out of memory error while locking device\n"));
		return NULL;
	}
	/* I think we don't need to use strncpy, as we should have enough
	 * buffer due to strlen results
	 */
	strcpy(lock_file, lock_path);
	strcat(lock_file, aux);

	/* Check for the stale lockfile.
	 * The code taken from minicom by Miquel van Smoorenburg */
	if ((fd = open(lock_file, O_RDONLY)) >= 0) {
		char buf[max_buf_len];
		int pid, n = 0;

		n = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (n > 0) {
			pid = -1;
			if (n == 4)
				/* Kermit-style lockfile. */
				pid = *(int *)buf;
			else {
				/* Ascii lockfile. */
				buf[n] = 0;
				sscanf(buf, "%d", &pid);
			}
			if (pid > 0 && kill((pid_t)pid, 0) < 0 && errno == ESRCH) {
				fprintf(stderr, _("Lockfile %s is stale. Overriding it..\n"), lock_file);
				sleep(1);
				if (unlink(lock_file) == -1) {
					fprintf(stderr, _("Overriding failed, please check the permissions\n"));
					fprintf(stderr, _("Cannot lock device\n"));
					goto failed;
				}
			} else {
				fprintf(stderr, _("Device already locked.\n"));
				goto failed;
			}
		}
		/* this must not happen. because we could open the file   */
		/* no wrong permissions are set. only reason could be     */
		/* flock/lockf or a empty lockfile due to a broken binary */
		/* which is more likely (like gnokii 0.4.0pre11 ;-)       */
		if (n == 0) {
			fprintf(stderr, _("Unable to read lockfile %s.\n"), lock_file);
			fprintf(stderr, _("Please check for reason and remove the lockfile by hand.\n"));
			fprintf(stderr, _("Cannot lock device\n"));
			goto failed;
		}
	}

	/* Try to create a new file, with 0644 mode */
	fd = open(lock_file, O_CREAT | O_EXCL | O_WRONLY, 0644);
	if (fd == -1) {
		if (errno == EEXIST)
			fprintf(stderr, _("Device seems to be locked by unknown process\n"));
		else if (errno == EACCES)
			fprintf(stderr, _("Please check permission on lock directory\n"));
		else if (errno == ENOENT)
			fprintf(stderr, _("Cannot create lockfile %s. Please check for existence of path"), lock_file);
		goto failed;
	}
	sprintf(buffer, "%10ld gnokii\n", (long)getpid());
	write(fd, buffer, strlen(buffer));
	close(fd);
	return lock_file;
failed:
	free(lock_file);
	return NULL;
}

/* Removes lock and frees memory */
bool unlock_device(char *lock_file)
{
	int err;

	if (!lock_file) {
//		fprintf(stderr, _("Cannot unlock device\n"));
		return false;
	}
	err = unlink(lock_file);
	free(lock_file);
	return (err + 1);
}

/*
 * Structure to store the filedescriptor we use.
 *
 */
int device_getfd(void)
{
  return device_portfd;
}

int device_open(__const char *__file, int __with_odd_parity) {
  struct CFG_Header *cfg_info;
  char *aux;	

  cfg_info=CFG_FindGnokiirc();
  if (cfg_info!=NULL) {
    aux = CFG_Get(cfg_info, "global", "use_locking");
    if (aux) {
      if (!strcmp(aux, "yes")) {
        lockfile = lock_device(__file);
        if (!lockfile) return false;
      }
    }
  }
  
  switch (CurrentConnectionType) {
    case GCT_Tekram:
      device_portfd = tekram_open(__file);
      break;
    case GCT_Irda:
      device_portfd = irda_open();
      break;
    default:
      device_portfd = serial_opendevice(__file, __with_odd_parity, true, false);
      break;
  }
  
  return (device_portfd >= 0);
}

void device_close(void)
{
  AppendLogText("CLOSE\n",false);

  /* Now wait for thread to terminate. */
  //pthread_join(Thread, NULL);

  unlock_device(lockfile);
  
  switch (CurrentConnectionType) {
    case GCT_Tekram: tekram_close(device_portfd); break;
    case GCT_Irda  :   irda_close(device_portfd); break;
    default        : serial_close(device_portfd); break;
  }

  PortDevice[0]=0x00;
}

void device_reset(void) {
}

void device_setdtrrts(int __dtr, int __rts)
{
  switch (CurrentConnectionType) {
    case GCT_Tekram:
    case GCT_Irda:
      break;
    default:
      serial_setdtrrts(device_portfd, __dtr, __rts);
#ifdef DEBUG
      device_dumpserial();
#endif
      break;
  }
}

void device_changespeed(int __speed)
{
  switch (CurrentConnectionType) {
    case GCT_Irda:
      break;
    case GCT_Tekram:
      tekram_changespeed(device_portfd, __speed);
      break;
    default:
      serial_changespeed(device_portfd, __speed);
#ifdef DEBUG
      fprintf(stdout,_("Serial device: changing speed to %i\n"),__speed);
#endif
      break;
  }
}

size_t device_read(__ptr_t __buf, size_t __nbytes)
{
  switch (CurrentConnectionType) {
    case GCT_Tekram: return (tekram_read(device_portfd, __buf, __nbytes)); break;
    case GCT_Irda  : return (  irda_read(device_portfd, __buf, __nbytes)); break;
    default        : return (serial_read(device_portfd, __buf, __nbytes)); break;
  }
}

size_t device_write(__const __ptr_t __buf, size_t __n) {
  u8 buffer[300];
  size_t mysize;
  
  while (duringwrite) {}

  memcpy(buffer,__buf,__n);
  AppendLog(buffer,__n,true);

  duringwrite=true;
  switch (CurrentConnectionType) {
    case GCT_Irda  : mysize =   irda_write(device_portfd, __buf, __n); break;
    case GCT_Tekram: mysize = tekram_write(device_portfd, __buf, __n); break;      
    default        : mysize = serial_write(device_portfd, __buf, __n); break;
  }
  duringwrite=false;
  return mysize;
}

#ifdef DEBUG
void device_dumpserial(void)
{
  int PortFD;
  unsigned int Flags=0;

  PortFD = device_getfd();

  ioctl(PortFD, TIOCMGET, &Flags);

  fprintf(stdout, _("Serial device:"));
  fprintf(stdout,  _(" DTR is %s"), Flags&TIOCM_DTR?_("up"):_("down"));
  fprintf(stdout, _(", RTS is %s"), Flags&TIOCM_RTS?_("up"):_("down"));
  fprintf(stdout, _(", CAR is %s"), Flags&TIOCM_CAR?_("up"):_("down"));
  fprintf(stdout, _(", CTS is %s\n"), Flags&TIOCM_CTS?_("up"):_("down"));
}
#endif /* DEBUG */

void SigHandler(int status)
{

  unsigned char buffer[2048];

  int count, res;

  res = device_read(buffer, 255);

  for (count = 0; count < res ; count ++) {
    Protocol->StateMachine(buffer[count]);
  }
}


#if defined(__svr4__) || defined(__FreeBSD__)
/* thread for handling incoming data */
void SelectLoop() {
  int err;
  fd_set readfds;
  struct timeval timeout;

  FD_ZERO(&readfds);
  FD_SET(device_portfd,&readfds);
  /* set timeout to 15 seconds */
  timeout.tv_sec=15;
  timeout.tv_usec=0;
  while (!CurrentRequestTerminate) {
    err=select(device_portfd+1,&readfds,NULL,NULL,&timeout);
    if ( err > 0 ) {
      /* call singal handler to process incoming data */
      SigHandler(0);
      /* refresh timeout, just for linux :-( */
      /* required for irda */
      timeout.tv_sec=15;
    } else {
      if (err == -1)
      perror("Error in SelectLoop");
    }
  }
}
#endif

bool StartConnection (char *port_device, bool with_odd_parity, GSM_ConnectionType con)
{
  int result;

  char text[100];

#if defined(__svr4__) || defined(__FreeBSD__)
  int rtn;
#else
  struct sigaction sig_io;
#endif

#ifdef DEBUG
      if ((strstr(GSM_Info->IrdaModels,"decode")!=NULL) &&  (CurrentConnectionType == GCT_Irda))
       {
         printf("DEBUG and Irda decode Model -> not realy open ;-)\n");
         return true;
       }
#endif 

  if (PortDevice[0]!=0x00) return true;

  duringwrite=false;
  
  strncpy(PortDevice, port_device, GSM_MAX_DEVICE_NAME_LENGTH);

#ifdef DEBUG
  fprintf(stdout,_("Opening \"%s\" device...\n"),PortDevice);
#endif

  strcpy(text,"\n\n\nMygnokii ");
  sprintf(text+strlen(text), "%s",VERSION);
  strcpy(text+strlen(text),"\n");
  AppendLogText(text,false);

  strcpy(text,"Port ");
  strcpy(text+strlen(text),PortDevice);
  strcpy(text+strlen(text),"\n");
  AppendLogText(text,false);

  strcpy(text,"Connection ");
  switch (con) {
    case GCT_FBUS    :strcpy(text+strlen(text),"FBUS");break;
    case GCT_Infrared:strcpy(text+strlen(text),"infrared");break;
    case GCT_Irda    :strcpy(text+strlen(text),"irda sockets");break;
    case GCT_MBUS    :strcpy(text+strlen(text),"MBUS");break;
    case GCT_DLR3    :strcpy(text+strlen(text),"DLR3");break;
    case GCT_Tekram  :strcpy(text+strlen(text),"Tekram");break;
    case GCT_AT      :strcpy(text+strlen(text),"AT");break;
    default          :strcpy(text+strlen(text),"unknown");break;
  }
  strcpy(text+strlen(text),"\n");
  AppendLogText(text,false);

  /* Ralf Thelen: In this moment there is NO method of communication,
   which require keepalive packets and GSM->KeepAlive is
   always NULL_KeepAlive, I comment this thread, */

  /* Create and start main thread. */
//  rtn = pthread_create(&Thread, NULL,(void*)GSM->KeepAlive, (void *)NULL);
//
//  if (rtn != 0) {
//    fprintf(stdout,_("Error\n"));  
//    return false;
//  }

#if defined(__svr4__) || defined(__FreeBSD__)
#else
        /* Set up and install handler before enabling async IO on port. */
        sig_io.sa_handler = SigHandler;
        sig_io.sa_flags = 0;
        sigaction (SIGIO, &sig_io, NULL);
#endif

    /* Open device. */
    result = device_open(PortDevice, with_odd_parity);
    
    if (!result) {
      fprintf(stdout,_("Error\n"));  
      return false;
    }

#if defined(__svr4__) || defined(__FreeBSD__)
    /* create a thread to handle incoming data from mobile phone */
    rtn=pthread_create(&selThread,NULL,(void*)SelectLoop,(void*)NULL);
    if (rtn != 0) {
      fprintf(stdout,_("Error\n"));  
      return false;
    }
#endif

  return true;
}

/* ---------------- RTH:   #ifdef WIN32 ------------------ */  

#else

  extern HANDLE hPhone;

void device_close(void)
{
  AppendLogText("CLOSE\n",false);

  CloseConnection();

  PortDevice[0]=0x00;
}

#ifdef DEBUG
void device_dumpserial(void)
{
  DCB dcb;
  
  dcb.DCBlength = sizeof(DCB);
  GetCommState(hPhone, &dcb);

  fprintf(stdout, _("Serial device:"));
  fprintf(stdout,  _(" DTR is "));
  switch (dcb.fDtrControl) {
    case DTR_CONTROL_ENABLE   : fprintf(stdout,  _("up"));       break;
    case DTR_CONTROL_DISABLE  : fprintf(stdout,  _("down"));     break;
    case DTR_CONTROL_HANDSHAKE: fprintf(stdout,  _("handshake"));break;
  }
  fprintf(stdout, _(", RTS is "));
  switch (dcb.fRtsControl) {
    case RTS_CONTROL_ENABLE   : fprintf(stdout,  _("up\n"));       break;
    case RTS_CONTROL_DISABLE  : fprintf(stdout,  _("down\n"));     break;
    case RTS_CONTROL_HANDSHAKE: fprintf(stdout,  _("handshake\n"));break;
    case RTS_CONTROL_TOGGLE   : fprintf(stdout,  _("toggle\n"));   break;
  }
}
#endif /* DEBUG */

void device_setdtrrts(int __dtr, int __rts)
{
  DCB dcb;
  
  dcb.DCBlength = sizeof(DCB);
  GetCommState(hPhone, &dcb);

  if (__dtr==1) dcb.fDtrControl = DTR_CONTROL_ENABLE;
           else dcb.fDtrControl = DTR_CONTROL_DISABLE;

   if (__rts==1) dcb.fRtsControl = RTS_CONTROL_ENABLE;
            else dcb.fRtsControl = RTS_CONTROL_DISABLE;

   SetCommState(hPhone, &dcb);

#ifdef DEBUG
  device_dumpserial();
#endif
}

void device_changespeed(int __speed)
{

  DCB dcb;

  dcb.DCBlength = sizeof(DCB);
  GetCommState(hPhone, &dcb);

  switch (__speed) {
    case 115200: dcb.BaudRate=CBR_115200; break;
    case 19200 : dcb.BaudRate=CBR_19200;  break;
    case 9600  : dcb.BaudRate=CBR_9600;   break;
   }
   
   SetCommState(hPhone, &dcb);

#ifdef DEBUG
   fprintf(stdout,_("Serial device: changing speed to %i\n"),__speed);
#endif
}

bool StartConnection (char *port_device, bool with_odd_parity, GSM_ConnectionType con)
{
  DCB dcb;
  char text[100];

  int rtn;

  if (PortDevice[0]!=0x00) return true;  

  duringwrite=false;
  strncpy(PortDevice, port_device, GSM_MAX_DEVICE_NAME_LENGTH);

#ifdef DEBUG
  fprintf(stdout,_("Opening \"%s\" device...\n"),PortDevice);
#endif

  strcpy(text,"\n\n\nMygnokii ");
  sprintf(text+strlen(text), "%s",VERSION);
  strcpy(text+strlen(text),"\n");
  AppendLogText(text,false);

  strcpy(text,"Port ");
  strcpy(text+strlen(text),PortDevice);
  strcpy(text+strlen(text),"\n");
  AppendLogText(text,false);

  strcpy(text,"Connection ");
  switch (con) {
    case GCT_FBUS    :strcpy(text+strlen(text),"FBUS");break;
    case GCT_Infrared:strcpy(text+strlen(text),"infrared");break;
    case GCT_Irda    :strcpy(text+strlen(text),"irda sockets");break;
    case GCT_MBUS    :strcpy(text+strlen(text),"MBUS");break;
    case GCT_DLR3    :strcpy(text+strlen(text),"DLR3");break;
    case GCT_Tekram  :strcpy(text+strlen(text),"Tekram");break;
    case GCT_AT      :strcpy(text+strlen(text),"AT");break;
    default          :strcpy(text+strlen(text),"unknown");break;
  }
  strcpy(text+strlen(text),"\n");
  AppendLogText(text,false);

  CurrentDisableKeepAlive = true;

  /* Create and start main thread. */
  rtn = ! OpenConnection(PortDevice, Protocol->StateMachine, GSM->KeepAlive);

  if (rtn != 0) {

    fprintf(stdout,_("Error\n"));  
    return false;
  } else {
    CurrentDisableKeepAlive = false;
  }

  if (with_odd_parity) {
    dcb.DCBlength = sizeof(DCB);
    GetCommState(hPhone, &dcb);

    dcb.Parity=ODDPARITY;

    SetCommState(hPhone, &dcb);
  }
  
  return true;
}

size_t device_write(const __ptr_t __buf, size_t __n) {
  int i;
  while (duringwrite) {}
  duringwrite=true;
  AppendLog(__buf,__n,true);
  i=WriteCommBlock(__buf,__n);
  duringwrite=false;
  if (i) return __n; else return 0;
}

#endif /*WIN32*/
