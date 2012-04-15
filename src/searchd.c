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
#include"socket_wrap.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>

struct wchar{
char s[2];
};
const int DIFF_ERR=100;
const double SIMILIAR=0.75; 
const int bignum=10000;
FILE *sitef[MAXSITE];  /*the data file of all the sites */
char site[MAXSITE][50]; /*site name */
struct searchd_nodent list[MAXFILE];  /* The most important data structure which  store all the data in memory.*/
int list_start_pos,nsite;
int file_pos_se[MAXSITE][2];   /*the start and end position in list */
char ci_query[MAXBUF],lcs_buf[MAXBUF]; /* case insenstive query lcs buf */
struct wchar wchar1[MAXBUF],wchar2[MAXBUF];
int wl1,wl2;
int result_num,tot_found; 

struct result_node{
  int val;
  char name[NAME_MAX+1];
char path[MAXBUF];
char ori_path[MAXBUF]; /* path of original code */
  struct searchd_nodent node;
}result[MAXRET+1];
int nresult;
int css[NAME_MAX*2+1][NAME_MAX*2+1];
int from[NAME_MAX*2+1][NAME_MAX*2+1][2];
char buf[MAXBUF],buf2[MAXBUF],query[MAXBUF];

int occur_place[MAXBUF],nplaces;

void init_search()
{
  DIR *dp=opendir(DATA);
  assert(dp!=NULL);
  struct dirent *dirp;
  nsite=0;
  chdir(DATA);
  while((dirp=readdir(dp))!=NULL)
  {
    if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0) continue;

    if((sitef[nsite]=fopen(dirp->d_name,"rb"))!=NULL)
    {
      strcpy(site[nsite],dirp->d_name);
      nsite++;
    }
  }
  chdir("..");
   freopen(SEARCHD_LOG,"a",stdout);
}
inline int square(int x){return (x)*(x);}
/* discrete factor=sigma{square(p[i]-p[j]) | for i,j in NXN} */
int discrete_fact()  
{
  int i;
  int diff=0;
  for(i=1;i<nplaces;i++)
      diff+=square(occur_place[i]-occur_place[i-1])*bignum/100;
  return diff;
}
/* Core of the  algorithm. Logest common subsequence--a lazy but seems
   effect method.*/
int lcs(char *s1,char *s2,int *len1,int *len2)
{
  int i,j=1;
  wl1=wl2=0;
 for(i=0;s1[i];i++)
 if(s1[i]>0)  wchar1[wl1].s[0]='\0',wchar1[wl1++].s[1]=s1[i];
 else wchar1[wl1].s[0]=s1[i],wchar1[wl1++].s[1]=s1[++i];
 for(i=0;s2[i];i++)
 if(s2[i]>0)  wchar2[wl2].s[0]='\0', wchar2[wl2++].s[1]=s2[i];
 else wchar2[wl2].s[0]=s2[i],wchar2[wl2++].s[1]=s2[++i];
 css[0][0]=0;
 for(i=0;i<wl1;i++) 
    for(j=0;j<wl2;j++)
    {
      if(css[i+1][j]>css[i][j+1]){
        css[i+1][j+1]=css[i+1][j];
        from[i+1][j+1][0]=i+1;
         from[i+1][j+1][1]=j;
      }else {
        css[i+1][j+1]=css[i][j+1];
        from[i+1][j+1][0]=i;
         from[i+1][j+1][1]=j+1;
      }
      if(wchar1[i].s[0]==wchar2[j].s[0] && wchar1[i].s[1] ==wchar2[j].s[1]
         && css[i][j]+1>css[i+1][j+1]){
        css[i+1][j+1]=css[i][j]+1;
         from[i+1][j+1][0]=i;
         from[i+1][j+1][1]=j;
      }
    }
 *len1=wl1;  *len2=wl2; 
  return css[i][j];
}
void case_insen_trans(char *s1,char *s2)
{
  int i;
  strcpy(s1,s2);
  for(i=0;s1[i];i++)
    if(isalpha(s1[i])) s1[i]=tolower(s1[i]);
}
 /*match degree,a simple function. (I think it is awkward. But I can't find a better one. Hope you have a better idea.) */
int match(char *ci_query,char *match_name)
{
  int i,j,retval;
  case_insen_trans(lcs_buf,match_name);
  retval=lcs(ci_query,lcs_buf,&i,&j);
  nplaces=0;
  while(i!=0 || j!=0){
    occur_place[nplaces++]=j;
    i=from[i][j][0];
    j=from[i][j][1];
  } 
  return retval*bignum+bignum-discrete_fact();
}
void getpath(int start,struct searchd_nodent *cur)
{
  if(cur->pare!=-1){
    getpath(start,list+start+cur->pare);
    strcat(buf,"/");    strcat(buf2,"/");
    strcat(buf,cur->name);  strcat(buf2,cur->ori_name);
  }
}
/* degree of similarity  of s1 and s2 */
double similiar(char *s1,char *s2) 
{
   int len1,len2;
   int val=lcs(s1,s2,&len1,&len2);
   return val/((len1+len2)/2.0);
}
/* add cand to result and sort it.  variable start means the start position
in list of site 'site_name' */
void add_to_result(int val,int start,struct searchd_nodent *cand,char *site_name)
{
  int p=MAXRET,i;
  int lenbuf;

  strcpy(buf,site_name);
  strcpy(buf2,site_name);
  getpath(start,cand);
  lenbuf=strlen(cand->name);
  while(p>0 && val>result[p-1].val)  p--;
  if(p!=MAXRET){
    tot_found++;
    for(i=MAXRET;i>p;i--) result[i]=result[i-1];
    result[p].val=val;
    result[p].val=val;
    strcpy(result[p].name,cand->name);
    strcpy(result[p].path,buf);
    strcpy(result[p].ori_path,buf2);
    result[p].node=*cand;
  }
}
void search_site(char *query,int start,int end,int cursite)
{
  int i,val;
  for(i=start;i<end;i++)
  {
  val=match(query,list[i].name);  /* return match degree */
  if(val>100)   add_to_result(val,start,&list[i],site[cursite]);
  }
}
/* retrun parent dir */
char * paredir(struct result_node *rsn)
{
  static char buf[MAXBUF];
  if(rsn->node.filetype=='d') return rsn->path;
  else {
    int i;
    strcpy(buf,rsn->path); 
    for(i=strlen(buf)-1;i>=0;i--) if(buf[i]=='/'){
        buf[i]='\0';
        return buf;
      }
    return buf;
  }
}
char * sizeStr(int size)
{
  static char buf[MAXBUF];
  if(size==0) strcpy(buf," ");
  else if(size<1024) sprintf(buf,"%d B",size);
  else if (size<1024*1024)  sprintf(buf,"%.1lf KB",1.0*size/1024);
  else if(size<1024*1024*1024) sprintf(buf,"%.1lf MB",1.0*size/1024/1024);
  else sprintf(buf,"%.1lf GB",1.0*size/1024/1024/1024);
  return buf;
}
char rech(int val)
{
  assert(val>=0);
  assert(val<16);
  if(val<10) return '0'+val;
  else return 'A'+val-10;
}
/* parse non-ascii code to url address */
char * parse2url(char *str)  
{
  static char tmbuf[MAXBUF];
  int i,len=0,num;
  for(i=0;str[i];i++)
    {
      if(str[i]<0){
        tmbuf[len++]='%';
        num=(unsigned char)str[i];
        tmbuf[len++]=rech(num>>4);
        tmbuf[len++]=rech(num&15);
      }else tmbuf[len++]=str[i];
    }
  tmbuf[len]='\0';
  return tmbuf;
}
void output_result(FILE *fout)  
{
  int i;
  fprintf(fout,"%s\n",query);
  fprintf(fout,"%d\n",tot_found);
  fprintf(fout,"%d\n",list_start_pos); /* How many files in total */  
  for(i=0;i<result_num && result[i].val>0 ;i++) 
   {
     fprintf(fout,"<div><p><a href=\"javascript:direct('ftp://%s')\" >%s</a></p>\
      <p>%d-%d-%d  %d:%d  %s </p>\n",
     paredir(&result[i]),  //terrible ~.~  .如何解决编码问题?...
            result[i].name,
            result[i].node.mtime.year,
            result[i].node.mtime.month,
            result[i].node.mtime.day,
            result[i].node.mtime.hour,
            result[i].node.mtime.min,
             sizeStr(result[i].node.size)
            );
     fprintf(fout,"<p>ftp://%s</p></div>",result[i].path);
   }
   if(i==0) fprintf(fout,"<h2>OMG~ Noting was found .</br> \
 Please contact : abarkingdog@qq.com </br> - _ -\"<h2>\n");
   fprintf(fout,"\r\n");
   fflush(fout);
 }
 int ch(char ch)
 {
   if(ch>='A' && ch<='Z') return ch-'A'+10;
   else return ch-'0';
 }
 void querylog(char *str)
 {
   strcpy(buf,LOG); strcat(buf,"/");  strcat(buf,QUERY_LOGFILE);
   FILE *log=fopen(buf,"a");
   if(log!=NULL)
   {
     fprintf(log,"%s\n",str);
     fclose(log);
   }
 }
 void getquery(FILE *fin,char *query)   /* get query from search */
 {  
   int totline,i;
   char *ptr;
   result_num=20;
   fscanf(fin,"%d\n",&totline);
   for(i=0;i<totline;i++){    
     fgets(buf,sizeof(buf),fin);
     buf[strlen(buf)-1]='\0';
     for(ptr=buf;*ptr;ptr++)
       if(*ptr=='='){
         *ptr='\0';
         if(strcmp(buf,"q")==0) strcpy(buf2,ptr+1);
         if(strcmp(buf,"list_num")==0) sscanf(ptr+1,"%d",&result_num);
       }
    }
   /* decode query %1A%DF */
   int len=0;
   for(i=0;buf2[i];i++){
     if(buf2[i]=='%')  query[len++]=ch(buf2[i+1])*16+ch(buf2[i+2]),i+=2;
     else if(isalnum(buf2[i]))   query[len++]=buf2[i];
   }
   query[len]='\0';
   querylog(query);
 }
 /* Tell search which port to send request */
 void port_notice(int x) 
 {
   FILE *fp=fopen(SERV_PORT_FILE,"w");
   if(fp!=NULL){
     fprintf(fp,"%d",x);
     fclose(fp);
  }
}
/* resort result.remove similarity */
void resort()
{
  int reslen=1,i;
  for(i=1;i<MAXRET;i++)
     if(similiar(result[i].path,result[reslen-1].path)<SIMILIAR && i!=reslen)
     result[reslen++]=result[i];
}
void search()  
{
  int i;
  int listenfd,connfd;
  FILE *fout,*fin;
  struct sockaddr_in cliaddr,servaddr;
  socklen_t clilen;
  
  listenfd=Socket(AF_INET,SOCK_STREAM,0);
  
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);  
  servaddr.sin_port=htons(SERV_PORT);
  
  Bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
  
  Listen(listenfd,LISTENQ);
 clilen=sizeof(cliaddr);
  for(;;){
    connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);

    if(connfd<0) {
      DBG_PRINTF("accept error\n");
      continue;
    }else DBG_PRINTF("accept success\n");
    fin=fdopen(connfd,"r");
    fout=fdopen(connfd,"w");
    if(fin==NULL || fout==NULL) {
      DBG_PRINTF("fdopen error\n");
      continue;
    }
    
    getquery(fin,query);    
    sprintf(buf,"getquery :%s\n",query);
    DBG_PRINTF(buf);
    tot_found=0;
    for(i=0;i<MAXRET;i++)  result[i].val=0;
    if(query[0]!='\0')
    {
      case_insen_trans(ci_query,query); /* case insensitive transform */
      for(i=0;i<nsite;i++) /* search for each site */
        search_site(ci_query,file_pos_se[i][0],file_pos_se[i][1],i);
    }

    resort();
    output_result(fout);
    fclose(fin); 
    fclose(fout);
    close(connfd);
    DBG_PRINTF("connect closed\n\n");
  }
}
/* Dynamic allocate filename to save memory */
void copy2node(struct searchd_nodent *dest,struct nodent *src)
{
  dest->size=src->size;
  dest->pare=src->pare;
  dest->filetype=src->filetype;
  dest->mtime=src->mtime;
  dest->ori_name=(char *)malloc(strlen(src->ori_name)+1);
  strcpy(dest->ori_name,src->ori_name);
  dest->name=(char *)malloc(strlen(src->name)+1);
  strcpy(dest->name,src->name);
}
void store_file2mem()
{
  int n,i;
  list_start_pos=0;
  struct nodent tmp;
  for(i=0;i<nsite;i++)
  {
    n=0;
    while(fread(&tmp,sizeof(struct nodent),1,sitef[i]))
    {
      copy2node(list+list_start_pos+n,&tmp);
      n++;
    }
    fclose(sitef[i]);
    file_pos_se[i][0]=list_start_pos;
    file_pos_se[i][1]=list_start_pos+n;
    list_start_pos+=n;
  }
}
 /* make searchd in a daemon */
void daemonize()
{
  pid_t pid;
  int fd;
  umask(0);
  if ((pid=fork())<0) {   //first 
    DBG_PRINTF("can't fork. exit.");
    exit(0);
  } else if(pid!=0) exit(0); //parent
  setsid();    
  if ((pid=fork())<0) {   //again
    DBG_PRINTF("can't fork. exit.");
    exit(0);
  } else if(pid!=0) exit(0); //parent
  fd=open("/dev/null",O_RDWR);
  dup2(fd,0);
  close(fd);
  dup2(0,1);
  dup2(0,2); 
}
int main(int argc,char *argv[])
{
   daemonize();
  init_search();
  store_file2mem();
  search();
  return 0;
}
