#ifndef CHAT_UTILITY_H
#define CHAT_UTILITY_H
#include<map>
#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;
pthread_mutex_t mutex;//charge socketfd
pthread_mutex_t mutex2;//charge useralive
void initserve(){
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&mutex2,NULL);
}
void *mainfunc1(void *arg);
void quitfun1(void *arg);
void huncfun1(int sig);
void newclient(void* clientfd);
int addAliveusr(int flag,int uid,int socketfd);
int queryAliveuse(int uid);//return socketfd
//clients_list save all the clients's socket


/***** macro defintion *****/
//server ip
#define SERVER_IP "127.0.0.1"

//server port
#define SERVER_PORT 8888

//epoll size
#define EPOLL_SIZE 5000

//message buffer size
#define BUF_SIZE 0xFFFF

#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is: Client #%d"

#define SERVER_MESSAGE "ClientID %d say >> %s"

//exit
#define EXIT "EXIT"

#define CAUTION "There is only ont int the char root!"
#define error(msg) \
    do {perror(msg); exit(EXIT_FAILURE); } while (0)

#define loginmsg 1
#define logoutmsg 2
#define chatmsg 3
#define addfriendmsg 4
#define deletefriendmsg 5

/****** some function *****/
/**
 *设置非阻塞
 */
 int epfd;
typedef struct header{
    int cmd;
    int len;
    int sig;
    
}header;
typedef struct chat{
    header head;
    int from;
    int to;
    char msg[50] ;
}chat;
 typedef struct login{
    header head;
    int uid;
    int pwd;
 }login;
typedef struct aliveusrinfo{
    int flag;
    int socketfd;
}aliveusrinfo;

map<long,aliveusrinfo> clients_list;
int setnonblockint(int sockfd) {
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
    return 0;
}
void *mainfunc1(void *arg)
{
    pthread_detach(pthread_self());
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
   
    newclient(arg);
    pthread_exit(0);
}
/**
 * 将文件描述符 fd 添加到 epollfd 标示的内核事件表中,
 * 并注册 EPOLLIN 和 EPOOLET 事件,
 * EPOLLIN 是数据可读事件；EPOOLET 表明是 ET 工作方式。
 * 最后将文件描述符设置非阻塞方式
 * @param epollfd:epoll句柄
 * @param fd:文件描述符
 * @param enable_et:enable_et = true,
 * 是否采用epoll的ET(边缘触发)工作方式；否则采用LT(水平触发)工作方式
 */
void addfd(int epollfd, int fd, bool enable_et) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (enable_et) {
        ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblockint(fd);
    printf("fd added to epoll!\n\n");
}

//发送广播
int verifyusr(int uid,int pwd){
    return 1;
}
int sendBroadcastmessage(void * arg) {
    char buf[BUF_SIZE];
    char message[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    bzero(buf, BUF_SIZE);
    long clientfd =(long)arg;
 
    int len = recv(clientfd, buf, BUF_SIZE, 0);
    header * servehead= (header *)(buf);
    //printf("%d cmd %d len\n",servehead->cmd,servehead->len);
    //chat * servechat = (chat *)servehead;
    //printf("%d to,%s msg",servechat->to,servechat->msg);
    switch(servehead->cmd){
        case loginmsg:
            if(verifyusr(((login *)buf)->uid,((login *)buf)->pwd)!=1){
                int loginfail=0;
                send(clientfd,&loginfail,sizeof(loginfail),0);
                return 1;
            }

            else{
                int response=(addAliveusr(clientfd,((login *)buf)->uid,clientfd));
                printf("response %d",response);
                header temphead={0,sizeof(header),response};
                write(clientfd,&temphead,sizeof(header));
                return 1;
            }
        case chatmsg:
            chat * servechat=(chat *)servehead;
            printf("%d  %d %s\n",servechat->to,servechat->from,servechat->msg);
            std::map<long,aliveusrinfo>::targetcientinfo=clients_list.find(servechat->to);
            long socketfd=targetcientinfo.
            fflush(stdout);
    }
    if (0 == len) {
        close(clientfd);
         pthread_mutex_lock(&mutex);
        clients_list.erase(clientfd);
         pthread_mutex_unlock(&mutex);
        printf("ClientID = %d closed.\n now there are %d client in the char room\n",
        clientfd, (int)clients_list.size());
    } else {
        if (1 == clients_list.size()) {
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return 0;
        }
        sprintf(message, SERVER_MESSAGE, clientfd, buf);
         pthread_mutex_lock(&mutex);
        list<long>::iterator it;
        for (it = clients_list.begin(); it != clients_list.end(); ++it) {
            if (*it != clientfd) {
                if (send(*it, message, BUF_SIZE, 0) < 0) {
                    perror("error");
                    exit(-1);
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return len;
}
void newclient(void * arg){

    long clientfd =(long)arg;
    addfd(epfd, clientfd, true);
    aliveusrinfo usr={0,0};
    // 服务端用list保存用户连接
    pthread_mutex_lock(&mutex);
    clients_list.insert(clientfd,aliveusrinfo);
    pthread_mutex_unlock(&mutex);
    printf("Add new clientfd = %d to epoll\n", clientfd);
    printf("Now there are %d clients int the chat room\n", (int) clients_list.size());


}
#endif //CHAT_UTILITY_H
int addAliveusr(int flag,int uid,int socketfd){
 //aliveusrinfo  newaliveusr={flag,uid};
 pthread_mutex_lock(&mutex);
 clients_list.find(socketfd)->second.flag=flag;
 clients_list.find(socketfd)->second.uid=uid;
 pthread_mutex_unlock(&mutex);
 return socketfd;
}
