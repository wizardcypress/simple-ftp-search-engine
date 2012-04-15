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
#include"config.h"
#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/time.h>
#include<math.h>

int sockfd;
struct sockaddr_in servaddr;
FILE *consult,*result;
char buf[MAXBUF];
struct timeval tvbef,tvafter;
struct timezone tz;

void put_head()
{
  printf("Content-Type: text/html; charset=GBK\r\n\r\n");
  printf("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=GBK\">\r\n");
  printf("<TITLE>%s</TITLE></head>\n","Result");
 printf("<script language=\"javascript\">function direct(site)\
{ window.location=site;}</script>");
    fflush(stdout);
}
void err_sys(char *msg)
{
  printf("<h2>%s<h2>",msg);
  exit(0);
}
void output_result()
{
  int n,tot_found,tot_file;
  result=fdopen(sockfd,"r");
  if(result==NULL) err_sys("Open socket  error");
  
  fgets(buf,sizeof(buf),result);
  buf[strlen(buf)-1]='\0';
  printf("<H1>Simple Ftp Search Engine</H1>\
<form action=\"/cgi-bin/search\">\
	<input type=\"text\" maxlength=\"20\" value=\"%s\" name=\"q\"/>\
<select name=\"list_num\">\
	  <option value =\"20\">20</option>\
	  <option value =\"50\">50</option>\
	  <option value=\"100\">100</option>\
	</select>\
	<input type=\"submit\" />\
</form> ",buf);
  
  fscanf(result,"%d",&tot_found);
  fscanf(result,"%d",&tot_file);
  gettimeofday(&tvafter,&tz);
  double tot_time=(1.0*tvafter.tv_sec-tvbef.tv_sec)
                                    +1.0*(tvafter.tv_usec-tvbef.tv_usec)/1000000;
  printf("<pre>Time cost: %.6lfs       %d / %d files were found.</pre>",tot_time,tot_found,tot_file);
  printf("<hr/>");
  while((n=fread(buf,sizeof(char),sizeof(buf)-1,result))>0)
  {
    buf[n]='\0';
    printf("%s",buf);
  }
  printf("<hr/><h2>Any suggestion?</h2>\
<form action=\"/cgi-bin/addsuggest.py\" method=\"POST\">\
<textarea rows=\"5\" cols=\"50\" name=\"suggest\">\
</textarea>\
<input type=\"submit\" value=\"submit\"/>\
</form>");
  fflush(stdout);
}
int ch(char ch)
{
  if(ch>='A' && ch<='Z') return ch-'A'+10;
  else return ch-'0';
}
void send_query()
{
  int i,argc;
  char *q=getenv("QUERY_STRING");
  /* FILE *portfile=fopen(SERV_PORT_FILE,"r");*/
  
  /* Get searchd port */
/* if(portfile==NULL) err_sys("Can not open daemon port\n");
  fscanf(portfile,"%d",&port);
  fclose(portfile);*/
  
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_port=htons(SERV_PORT);
  inet_pton(AF_INET,LOCAL_HOST,&servaddr.sin_addr);

  if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
    err_sys("Connect to daemon error. (Maybe searchd is not running~.~)\n");

  if((consult=fdopen(sockfd,"w"))==NULL)
     err_sys("Connect to daemon error.(Can not fetch result.)\n");
  
  argc=1;
  for(i=0;q[i];i++) if(q[i]=='&') argc++;

  fprintf(consult,"%d\n",argc);
  
  for(i=0;q[i];i++)
    if(q[i]=='&')  fputc('\n',consult);
    else fputc(q[i],consult);
  fputc('\n',consult);
  fflush(consult);
  
  gettimeofday(&tvbef,&tz);  
}
int main(int argc,char *argv[])
{
  put_head();
  send_query();
  output_result();
  return 0;
}
