#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void doit(int connfd);
void parse_uri(char *uri,char *hostname,char *path,int *port);
void build_the_header(char *http_header,char *hostname,char *path,int port,rio_t *client_rio);
int connect_endServer(char *hostname,int port,char *http_header);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main(int argc, char **argv) {

  int listenfd, connfd;
  socklen_t clientlen;
  char hostname[MAXLINE], port[MAXLINE];

  struct sockaddr_storage clientaddr;
  if (argc != 2){
    fprintf(stderr, "usage :%s <port> \n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]); // 포트번호를 가지고 듣기소켓 오픈
  while(1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);

    Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s).\n", hostname, port);
    
    doit(connfd);
    Close(connfd);
  }
  
  return 0;
}

void doit(int connfd){
  
  int end_serverfd; /* 최종 서버 fd */

  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char endserver_http_header[MAXLINE];

  char hostname[MAXLINE], filepath[MAXLINE];
  int port;

  rio_t rio, server_rio;
  /*
    rio : client's rio
    server_rio : endserver's rio
  */
  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("버퍼 확인용 :%s\n", buf); // 나중에 지워야함
  sscanf(buf, "%s %s %s", method, uri, version); // 공백으로 구분된 3개의 문자열을 구분하여 각각에 저장

  //GET http://hostname:port/path HTTP/1.0

  if (strcasecmp(method, "GET")) {
    printf("Proxy does not implement the method");
    return;
  }

  /* parse the uri to get hostname, filepath(filename), port */
  // http://hostname:port/path
  parse_uri(uri, hostname, filepath, &port);

  /* 최종 서버에 전송될 http header 만들기 */
  build_the_header(endserver_http_header, hostname, filepath, port, &rio);

  /* 최종 서버에 연결하기 */
  end_serverfd = connect_endServer(hostname, port, endserver_http_header);
  if (end_serverfd<0) {
    printf("connection failed\n");
    return;
  }

  Rio_readinitb(&server_rio, end_serverfd);

  /* 최종 서버에 http header 'write' */
  Rio_writen(end_serverfd, endserver_http_header, strlen(endserver_http_header));

  /* 최종서버로부터 응답메세지 받고 그것을 클라이언트에 전송 */
  size_t n;
  while((n=Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) { //Rio_readlineb는 읽은 바이트 수를 반환
    printf("Proxy received %d bytes, then send\n", n);
    Rio_writen(connfd, buf, n); // 성공하면 쓰여진 바이트 수 반환
  }
  Close(end_serverfd);
}

/* 최종 서버에 전송될 헤더 만들기 */
void build_the_header(char * http_header, char * hostname, char * filepath, int port, rio_t *client_rio) {
  char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];
  /* 요청 라인 */
  sprintf(request_hdr, "GET %s HTTP/1.0\r\n", filepath);
  /* get other request header for client rio and change it */
  while(Rio_readlineb(client_rio,buf,MAXLINE)>0)
    {
        if(strcmp(buf,"\r\n")==0) break;/*EOF*/

        if(!strncasecmp(buf,"HOST",strlen("HOST")))/*Host:*/
        {
            strcpy(host_hdr,buf);
            continue;
        }

        if(strncasecmp(buf,"Connection",strlen("Connection"))
                &&strncasecmp(buf,"Proxy-connection",strlen("Proxy-connection"))
                &&strncasecmp(buf,"User_agent",strlen("User_agent")))
        {
            strcat(other_hdr,buf);
        }
    }
    if(strlen(host_hdr)==0)
    {
        sprintf(host_hdr,"Host: %s\r\n",hostname);
    }
    sprintf(http_header,"%s%s%s%s%s%s%s",
            request_hdr,
            host_hdr,
            "Connection: close\r\n",
            "Proxy-Connection: close\r\n",
            user_agent_hdr,
            other_hdr,
            "\r\n");
    return ;
}
/*Connect toa the end server*/
inline int connect_endServer(char *hostname,int port,char *http_header){
    char portStr[100];
    sprintf(portStr,"%d",port);
    return Open_clientfd(hostname,portStr);
}

/*parse the uri to get hostname,file path ,port*/
void parse_uri(char *uri,char *hostname,char *path,int *port)
{
  // http://hostname:port/path
    *port = 80;
    char* pos = strstr(uri,"//");

    pos = pos!=NULL? pos+2:uri;

    char*pos2 = strstr(pos,":");
    if(pos2!=NULL)// : 있을때
    {   
        *pos2 = '\0';
        // pos = hostname\0  port  /path
        sscanf(pos,"%s",hostname);
        sscanf(pos2+1,"%d%s",port,path);
    }
    else // : 없을때
    {   // pos = hostname/path
        pos2 = strstr(pos,"/"); // *pos2 = / 
        if(pos2!=NULL)
        {
            *pos2 = '\0';
            sscanf(pos,"%s",hostname);
            *pos2 = '/';
            sscanf(pos2,"%s",path);
        }
        else // path 가 아예 없을때
        {
            sscanf(pos,"%s",hostname);
        }
    }
    return;
}