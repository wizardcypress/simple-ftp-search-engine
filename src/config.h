/*
 *    Copyright (C) 2011 Bolin Huang <bolin.huang@gmail.com>
 *    I made it just for fun -_-". And I hope it works as desired.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include<dirent.h>
#include<limits.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<ctype.h>
#include<assert.h>
#include<fcntl.h>



#define LISTENQ 20
#define SERV_PORT 54413

#define MAX_TEMPFILE 1000000
#define MAXFILE 1000000
#define  MAXBUF 1024
#define MAXSITE 1000
#define MAXRET 101

const char *SEARCHD_LOG="log/searchd.log";
const char *SERV_PORT_FILE="serv_port_file";
const char *LOCAL_HOST="127.0.0.1";
const char *TMP="tmp";
const char *DATA="data";
const char *tempdir="temp_data";
const char *LOG="log";
const char *QUERY_LOGFILE="query.log";

struct Mtime {
    int year,month,day,hour,min;
};
struct nodent{
  int size,pare;
  char ori_name[NAME_MAX/2];
  char name[NAME_MAX/2];
  char filetype;
  struct Mtime mtime;
};

struct searchd_nodent{
  int size,pare;
  char *ori_name;
  char *name;
  char filetype;
  struct Mtime mtime;
};


#define DBG_PRINTF(_x_)\
do{\
time_t now;struct tm *ti;\
time(&now);\
ti=localtime(&now);\
printf("%d-%d-%d %d:%d:%d ",ti->tm_year,ti->tm_mon,ti->tm_mday\
,ti->tm_hour,ti->tm_min,ti->tm_sec);\
printf("%s(%d)--:",__FILE__,__LINE__);\
printf("\t%s",_x_);\
 fflush(stdout);\
}while(0)

#endif

