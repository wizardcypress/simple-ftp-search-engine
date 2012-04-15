#include"config.h"

#define MOD 1000003
#define HSKEY 3
#define STRCPY(dest,src) strncpy((dest),(src),sizeof(dest)-1)
const int HS_EXP[HSKEY]={13,19,29};

struct  key{
  int tab_pos;
    int hash_key[HSKEY]; 
};
struct hash_node{
  int order;
  struct key hash_key;
  int next;
};

int hash[MOD],tab_len;
struct hash_node hash_tab[MAXFILE];

char buf[MAXBUF],buf2[MAXBUF];

struct key gethash(char *str)
{
  struct key res;
  int i,j;
  res.tab_pos=0;
  for(i=0;i<HSKEY;i++) res.hash_key[i]=0;
  for(i=0;str[i];i++)
  {
    res.tab_pos=(res.tab_pos*26+str[i])%MOD;
    for(j=0;j<HSKEY;j++)
      res.hash_key[j]=(res.hash_key[j]*HS_EXP[j])+str[i];
  }
  return res;
}
void add_hash(int order, struct key *hash_key)
{
  hash_tab[tab_len].order=order;
  hash_tab[tab_len].hash_key=*hash_key;
  hash_tab[tab_len].next=hash[hash_key->tab_pos];
  hash[hash_key->tab_pos]=tab_len++;
  assert(tab_len<=MAXFILE);
}
int  match_key(struct key *k1,struct key *k2)
{
  int i;
  for(i=0;i<HSKEY;i++)
    if(k1->hash_key[i]!=k2->hash_key[i]) return 0;
  return 1;
}
int hash_getorder(struct key *hash_key)
{
  int it;
  for(it=hash[hash_key->tab_pos];it!=-1;it=hash_tab[it].next)
    if(match_key(&hash_tab[it].hash_key,hash_key))
      return hash_tab[it].order;
  return -1;
}
char *cons(const char *s1,const char *s2)
{
  STRCPY(buf,s1);
  if(strcmp(s1,"/")!=0) strcat(buf,"/");
  strcat(buf,s2);
  return buf;
}

void num2str(int num,char *str)
{
  int len=0,i;
  char sw;
  while(num)
  {
    str[len++]='0'+(num % 10);
    num/=10;
  }
  for(i=0;i<len/2;i++)
    sw=str[i],str[i]=str[len-i-1],str[len-i-1]=sw;
  str[len]='\0';
}

/*make a temp directory to store new data*/
int init_store()
{
  char cmd[256]="rm -rf ";
  strcat(cmd,tempdir);
  system(cmd);
  STRCPY(cmd,"mkdir ");
  strcat(cmd,tempdir);
  return system(cmd);
}
struct Mtime dos_time(char *m1,char *m2)
{
  struct Mtime t;
  sscanf(m1,"%d-%d-%d",&t.month,&t.day,&t.year);
  sscanf(m2,"%d:%d",&t.hour,&t.min);
  if(strstr(m2,"PM")) t.hour+=12;
  return t;
}
void complete_name(char *line,char *msg)
{
  char *startpos=strstr(line,msg);
  if(startpos==line)
    startpos=strstr(line+strlen(msg),msg);
  int len=0;
  while(*startpos)
    msg[len++]=*startpos++;
  msg[len]='\0';
}
void replace(char *str,char ch1,char ch2)
{
  int i;
  for(i=0;str[i];i++) if(str[i]==ch1) str[i]=ch2;  
}
void dos_fmt(int par_order,char *pardir,char *line,FILE *lfile,FILE *dfile)
{
  char mesg[4][257];
  char orgname[NAME_MAX+1];
  struct key curkey;
  struct nodent nod;
  sscanf(line,"%s%s%s%s%s",orgname,mesg[0],mesg[1],mesg[2],mesg[3]);
  
 replace(orgname,'$',' ');
  
  if(strcmp(mesg[3],".")==0 || strcmp(mesg[3],"..")==0) return ;

  complete_name(line,mesg[3]);
  
  nod.mtime=dos_time(mesg[0],mesg[1]);
  if(strcmp(mesg[2],"<DIR>")==0) nod.size=0,nod.filetype='d';
  else nod.size=atoi(mesg[2]),nod.filetype='f';
  STRCPY(nod.ori_name,orgname);
  STRCPY(nod.name,mesg[3]);
  
  curkey=gethash(cons(pardir,mesg[3]));
  add_hash(tab_len,&curkey);
  nod.pare=par_order;
  fwrite(&nod,sizeof(nod),1,dfile);
}
int this_year()
{
  time_t tp; 
  time(&tp);
  return localtime(&tp)->tm_year+1900;
}
struct Mtime unix_time(char *m1,char *m2,char *m3)
{
   static char *m[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec", NULL };
   int i;
   struct Mtime t;
   for(i=0;i<12;i++)
     if (strcmp(m[i],m1)==0) t.month=i+1;
   t.day=atoi(m2);
   if(strchr(m3,':')!=NULL) {
     sscanf(m3,"%d:%d",&t.hour,&t.min);
     t.year=this_year();
   }else t.year=atoi(m3),t.hour=t.min=0;
   return t;
}
void unix_fmt(int par_order,char *pardir,char *line,FILE *lfile,FILE *dfile)
{
  char mesg[9][NAME_MAX+2];
  char orgname[NAME_MAX+1];
  struct key curkey;
  struct nodent nod;
  sscanf(line,"%s%s%s%s%s%s%s%s%s%s",orgname,mesg[0],mesg[1],mesg[2],mesg[3],
                                                                    mesg[4],mesg[5],mesg[6],mesg[7],mesg[8]);
  
  if(strcmp(mesg[8],".")==0 || strcmp(mesg[8],"..")==0) return ;

  replace(orgname,'$',' ');
  complete_name(line,mesg[8]);

  nod.mtime=unix_time(mesg[5],mesg[6],mesg[7]);
  STRCPY(nod.ori_name,orgname);
  if(mesg[0][0]=='d') nod.filetype='d';
  else nod.filetype='f';
  nod.size=atoi(mesg[4]);
  STRCPY(nod.name,mesg[8]);
  
  curkey=gethash(cons(pardir,mesg[8]));
  add_hash(tab_len,&curkey);
  nod.pare=par_order;
  fwrite(&nod,sizeof(nod),1,dfile);
}
int store_list2data(FILE *lfile,FILE *dfile)
{
  char pardir[MAXBUF];
  char line[MAXBUF];
  struct key par_key;
  fgets(pardir,sizeof(pardir),lfile);
  pardir[strlen(pardir)-1]='\0';
  par_key=gethash(pardir);
  int par_order=hash_getorder(&par_key);
  if(par_order==-1) return -1;
  
  while(fgets(line,sizeof(line),lfile))
  {
    if (feof(lfile)) break;
    line[strlen(line)-1]='\0';
    sscanf(line,"%s%s",buf2,buf);
    if(strlen(buf2)>NAME_MAX/2) continue; /* if file name is too long ,ignore it */
    if (isdigit(buf[0])) dos_fmt(par_order,pardir,line,lfile,dfile);
    else unix_fmt(par_order,pardir,line,lfile,dfile);
  }
  return 0;
}

/*store new data into new data directory*/
int store()
{
   FILE *lfile,*dfile;
  DIR *site_dp=opendir(TMP);
  struct dirent *site_dirp;
  int lfile_len,i;
  char exam[50],file_name[50],num[10];
   
  while((site_dirp=readdir(site_dp))!=NULL)
  {    
    if(strcmp(site_dirp->d_name,".")==0 ||
       strcmp(site_dirp->d_name,"..")==0) continue;
    /*a data file for each site*/   
    printf("%s\n",site_dirp->d_name);
    dfile=fopen(cons(tempdir,site_dirp->d_name),"wb");
       if(dfile==NULL) continue;
       
    chdir(TMP);
    DIR *file_dp=opendir(site_dirp->d_name);
    chdir(site_dirp->d_name);
    struct dirent *file_dirp;
    lfile_len=0;
    while((file_dirp=readdir(file_dp))!=NULL)
    {
      if(strcmp(file_dirp->d_name,".")==0 ||
       strcmp(file_dirp->d_name,"..")==0) continue;
      lfile_len++;
      if(lfile_len==1) STRCPY(exam,file_dirp->d_name);
    }
    for(i=0;exam[i];i++) if (isdigit(exam[i])) exam[i]='\0';
    
    /*handle each file*/
    tab_len=0;
    memset(hash,255,sizeof(hash));
    struct key root_key=gethash("/");
    add_hash(0,&root_key);/* add_hash(order,key) */
    struct nodent root;
    root.size=0; root.pare=-1; STRCPY(root.name,"/");
    fwrite(&root,sizeof(root),1,dfile);
    
    for(i=1;i<=lfile_len;i++)
    {
     num2str(i,num);
     STRCPY(file_name,exam);
     strcat(file_name,num);
     
     lfile=fopen(file_name,"r");
     if(lfile==NULL) continue;
     store_list2data(lfile,dfile);
     fclose(lfile);
    }
    fclose(dfile);
    chdir("..");
    chdir("..");
  }  
  return 0;
}
int main(int argc,char *argv[])
{
  if (init_store()==-1) printf("init store fail\n");
  if(store()==-1) printf("store fail\n");  
  return 0;
}
