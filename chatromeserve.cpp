#include "utility.h"



int main(int argc, char *argv[]) {
    /**
   
     * 1:创建套接字socket
     * param1:指定地址族为IPv4;param2:指定传输协议为流式套接字;param3:指定传输协议为TCP,可设为0,由系统推导

     */

    int listener = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (listener < 0) {
        error("socket error");
    }
    printf("listen socket created \n");

    //地址复用
    int on = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        error("setsockopt");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    //绑定地址
    if (bind(listener, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        error("bind error");
    }

    //监听
    if (listen(listener, SOMAXCONN) < 0) {
        error("listen error");
    }



    //在内核中创建事件表
    epfd = epoll_create(EPOLL_SIZE);
    if (epfd < 0) {
        error("epfd error");
    }
   
    static struct epoll_event events[EPOLL_SIZE];
    //往内核事件表里添加事件
    addfd(epfd, listener, true);

    //主循环
    while (1) {
        //epoll_events_count表示就绪事件的数目
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if (epoll_events_count < 0) {
            perror("epoll failure");
            break;
        }

       
        //处理这epoll_events_count个就绪事件
        for (int i = 0; i < epoll_events_count; ++i) {
            int sockfd = events[i].data.fd;
            //新用户连接
            if (sockfd == listener) {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(listener, (struct sockaddr *) &client_address, &client_addrLength);
                printf("client connection from: %s : % d(IP : port), clientfd = %d \n",
                inet_ntoa(client_address.sin_addr),
                ntohs(client_address.sin_port),
                clientfd);
                pthread_t pthid1=0;
                if(pthread_create(&pthid1,NULL,mainfunc1,(void*)(long)clientfd)!=0)
                     {  printf("create thread error");
                        return -1;
                        }
                //newclient(clientfd);
            } else {           //处理用户发来的消息，并广播，使其他用户收到信息
                int ret = sendBroadcastmessage((void *)(long)sockfd);
                if (ret < 0) {
                    error("error");
                }
            }
        }
    }
    close(listener); //关闭socket
    close(epfd);    //关闭内核
    return 0;
}
