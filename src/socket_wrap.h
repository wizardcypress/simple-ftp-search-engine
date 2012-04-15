#ifndef SOCKET_WRAP_H
#define SOCKET_WRAP_H

#include"config.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

int Socket(int family,int type,int protocol)
{
  int sockfd=socket(family,type,protocol);
  if(sockfd<0)  {
    DBG_PRINTF("socket error\n");
      exit(1);
  } else DBG_PRINTF("socket success\n");
  return sockfd;
}


int Bind(int sockfd,const struct sockaddr *myaddr,socklen_t addrlen)
{
  int res=bind(sockfd,myaddr,addrlen);
  if(res<0) {
    DBG_PRINTF("bind error\n");
      exit(1);
  }else DBG_PRINTF("bind success\n");
  return res;
}

int Listen(int sockfd,int backlog)
{
  int res=listen(sockfd,backlog);
  if(res<0) {
    DBG_PRINTF("listen error\n");
      exit(1);
  }else DBG_PRINTF("listen success\n");
  return res;
}


#endif
