#include <stdio.h>

#ifdef WIN32
#  include <windows.h>
#  pragma comment(lib, "libmysql.lib")
#  include <mysql.h>
#  include <mysqld_error.h>
#  include <process.h>
#else
#  include <stdio.h>
#  include <time.h>
#  include <mysql/mysql.h>
#  include <mysql/mysqld_error.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#define TRUE 1
#define FALSE 0

MYSQL 		DB,DB2;
unsigned char 	Pass[50];
unsigned char 	User[50];
unsigned char 	DBName[50];
unsigned char 	PC[50];
MYSQL_RES 	*Res,*Res2,*Res3,*Res4;
MYSQL_ROW 	Row,Row2,Row3,Row4;
int 		first_connect;

void connect_to_db(unsigned char *U,unsigned char *P,unsigned char *S,unsigned char *D)
{
	if (strcmp(PC,S)  !=0 || strcmp(User,U)  !=0 ||
	    strcmp(Pass,P)!=0 || strcmp(DBName,D)!=0) {
		strcpy(PC,S);
		strcpy(User,U);
		strcpy(Pass,P);
		strcpy(DBName,D);
		if (first_connect==TRUE) {
			first_connect=FALSE;
			mysql_close(&DB2);
		}
		if (!mysql_real_connect(&DB2,PC,User,Pass,DBName,0,NULL,0)) return;
	}
}

int check_if_avail(unsigned char *buf)
{
	int found;

	if (mysql_real_query(&DB,buf,strlen(buf))) {
		return FALSE;
	}
	if (!(Res2 = mysql_store_result(&DB))) {
		return FALSE;
	}
	found = TRUE;
	while ((Row2 = mysql_fetch_row(Res2))) {
		connect_to_db(Row2[4],Row2[5],Row2[6],Row2[2]);//user,pass,pc,db
		if (mysql_real_query(&DB2,Row2[3],strlen(Row2[3]))) {
			mysql_free_result(Res2);
			return FALSE;
		}
		if (!(Res3 = mysql_store_result(&DB2))) {
			mysql_free_result(Res2);
			return FALSE;
		}
		Row3 = mysql_fetch_row(Res3);
		if (Row3 == NULL) {
			found=FALSE;
			break;
		}

//		printf("%s\n",Row2[3]);
	}
	mysql_free_result(Res2);
	mysql_free_result(Res3);
	return found;
}

void execute_one_action(unsigned char *buf3)
{
	unsigned char buf4[5000];
	unsigned char *b;
	int	 num,num2,i;
	char 	 *args[10];
	unsigned char buffe[10][200];

	buf4[0]     = 0;
	b 	    = buf3;
	num	    = 0;
	buffe[0][0] = 0;
	while (b[0] != 0) {
		if (strstr(buf3,"daemon_run_program") != NULL) {
			if (b[0] == '<') {
				if (num!=0) args[num-1] = buffe[num];
				num++;
				buffe[num][0] = 0;
				b++;
			} else {
				if (strstr(b,"daemon_column") == b) {
					b+=13;
					num2=atoi(b);
					while (b[0] >= '0' && b[0] <= '9') b++;
                    strcat(buffe[num],Row3[num2-1]);
				} else if (strstr(b,"daemon2_column") == b) {
					b+=14;
					num2=atoi(b);
					while (b[0] >= '0' && b[0] <= '9') b++;
                    strcat(buffe[num],Row4[num2-1]);
				} else {
					buffe[num][strlen(buffe[num])+1] = 0;
					buffe[num][strlen(buffe[num])]   = b[0];
					b++;
				}
			}
		} else {
			if (strstr(b,"daemon_column") == b) {
				b+=13;
				num2=atoi(b);
				while (b[0] >= '0' && b[0] <= '9') b++;
				strcat(buf4,Row3[num2-1]);
			} else if (strstr(b,"daemon2_column") == b) {
				b+=14;
				num2=atoi(b);
				while (b[0] >= '0' && b[0] <= '9') b++;
				strcat(buf4,Row4[num2-1]);
			} else {
				buf4[strlen(buf4)+1] = 0;
				buf4[strlen(buf4)]   = b[0];
			}
		}
	}
	if (strstr(buf3,"daemon_run_program") != NULL) {
		args[num-1] = NULL;
//		for (i=0;i<num-1;i++) {
//			printf("arg %i \"%s\"\n",i,args[i]);
//		}
//		if (num >= 0) {
#ifdef WIN32
			spawnv(_P_NOWAIT, buffe[1], args);
#else
			if (fork()==0) execve(buffe[1],args,NULL);
#endif
//		}
	} else {
		connect_to_db(Row2[11],Row2[12],Row2[14],Row2[13]);//user,pass,pc,db
		mysql_query(&DB2,buf4);
	}
}

void execute_all_actions(unsigned char *buf)
{
	int 		i;
	unsigned char 	buf3[5000];

	buf3[0] = 0;
	for (i=0;i<strlen(buf);i++) {
		if (buf[i] == '{') {
			execute_one_action(buf3);
			buf3[0] = 0;
		} else {
			buf3[strlen(buf3)+1] = 0;
			buf3[strlen(buf3)] = buf[i];
		}
	}
	if (buf3[0] != 0) execute_one_action(buf3);
}

void execute_actions()
{
	int 		first,i,j;
	unsigned char 	buf3[5000],buf4[5000];

	//we search for select part
	first   = 0;
	buf3[0] = 0;
	buf4[0] = 0;
	for (i=0;i<strlen(Row2[10]);i++) {
		if (Row2[10][i] == '{') {
			if (first == 0) j = i;
			if (first == 1) break;
			first++;
		} else {
			if (first == 0) {
				buf3[strlen(buf3)+1] = 0;
				buf3[strlen(buf3)] = Row2[10][i];
			} else if (first == 1) {
				buf4[strlen(buf4)+1] = 0;
				buf4[strlen(buf4)] = Row2[10][i];
			}
		}
	}
	if (buf4[0] != 0) {
		if (strstr(buf4,"select") == NULL || strstr(buf4,"from") == NULL || strstr(buf4,"where") == NULL) {
			buf4[0] = 0;
			i = j;
		}
	}
	connect_to_db(Row2[2],Row2[4],Row2[8],Row2[6]);//user,pass,pc,db
	if (mysql_real_query(&DB2,buf3,strlen(buf3))) {
		return;
	}
	if (!(Res3 = mysql_store_result(&DB2))) {
		return;
	}
	while ((Row3 = mysql_fetch_row(Res3))) {
		//for each value we execute actions
		if (buf4[0] != 0) {
			connect_to_db(Row2[3],Row2[5],Row2[9],Row2[7]);//user,pass,pc,db
			if (mysql_real_query(&DB2,buf4,strlen(buf4))) {
				return;
			}
			if (!(Res4 = mysql_store_result(&DB2))) {
				return;
			}
			while ((Row4 = mysql_fetch_row(Res4))) {
				execute_all_actions(Row2[10]+i+1);
			}
			mysql_free_result(Res4);
		} else {
			execute_all_actions(Row2[10]+i+1);
		}
	}
	mysql_free_result(Res3);
}

void main(int argc, char *argv[])
{
	unsigned char buf[5000];

	first_connect 	= TRUE;
	DBName[0] 	= 0;
	Pass[0] 	= 0;
	User[0] 	= 0;
	PC[0] 		= 0;
	mysql_init(&DB);
	mysql_init(&DB2);

	if (argc < 4) {
		printf("Usage: PC User Password Database\n");
		return;
	}

	//connect
	if (!mysql_real_connect(&DB,argv[1],argv[2],argv[3],argv[4],0,NULL,0)) {
		printf("I can't read rules & actions database");
		return;
	}

	while (TRUE) {
		//search for rules ID
		sprintf(buf, "select ID from rules group by ID");
		if (mysql_real_query(&DB,buf,strlen(buf))) {
			return;
		}
		if (!(Res = mysql_store_result(&DB))) {
			return;
		}
		while ((Row = mysql_fetch_row(Res))) {
			//search for all rules with some ID and check them inside
			sprintf(buf, "SELECT ID,RuleID,DB,SQL,User,Pass,PC FROM `rules` WHERE ID='%s'",Row[0]);
			if (check_if_avail(buf)==FALSE) continue;

			//yes, we execute actions
			sprintf(buf, "SELECT ID,ActionID,User,User2,Pass,Pass2,DB,DB2,PC,PC2,SQL,User3,Pass3,DB3,PC3 FROM `actions` WHERE ID='%s'",Row[0]);
			if (mysql_real_query(&DB,buf,strlen(buf))) {
				return;
			}
			if (!(Res2 = mysql_store_result(&DB))) {
				return;
			}
			while ((Row2 = mysql_fetch_row(Res2))) {
				//we don't have two parts
				if (strstr(Row2[10],"{")==NULL) continue;

				execute_actions();
			}
			mysql_free_result(Res2);
		}
		mysql_free_result(Res);
#ifdef WIN32
		Sleep(500);
#else
		usleep(2000);
#endif
	}

	return;
}
