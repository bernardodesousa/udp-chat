#define LINUX

#ifdef WINDOWS
#include <windows.h>
#include <winsock.h>
#include <ws2tcpip.h>
#define socklen_t int
#endif

#ifdef LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#define closesocket(f) close(f)
#define SOCKET int
#endif

#include <stdio.h>

#define HELLO_PORT 12345
#define HELLO_GROUP "225.0.0.37"
#define MSGBUFSIZE 256

#include <iostream>
#include <cstdlib>

using namespace std;

void listener()
{
    struct sockaddr_in addr;
    int fd, nbytes;
    socklen_t addrlen;
    struct ip_mreq mreq;
    char msgbuf[MSGBUFSIZE];

    const char yes = (const char)1;

#ifdef WINDOWS
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);
#endif

    /* create what looks like an ordinary UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }

    /**** MODIFICATION TO ORIGINAL */
    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        perror("Reusing ADDR failed");
        exit(1);
    }
    /*** END OF MODIFICATION TO ORIGINAL */

    /* set up destination address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
    addr.sin_port = htons(HELLO_PORT);

    /* bind to receive address */
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr = inet_addr(HELLO_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *) &mreq, sizeof(mreq)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    /* now just enter a read-print loop */
    while (1)
    {
        addrlen=sizeof(addr);
        if ((nbytes=recvfrom(fd,msgbuf,MSGBUFSIZE,0,
                             (struct sockaddr *) &addr,&addrlen)) < 0)
        {
            perror("recvfrom");
            exit(1);
        }
        puts(msgbuf);
    }
}

void sender()
{
    struct sockaddr_in addr;
    int fd;
    char message[] = "Hello, World!";
    char buffer[256];
    int count = 1;

#ifdef WINDOWS
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);
#endif

    /* create what looks like an ordinary UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
	    exit(1);
    }

    /* set up destination address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(HELLO_GROUP);
    addr.sin_port = htons(HELLO_PORT);

    /* now just sendto() our destination! */
    while (1) {
        memset(buffer,'\0',256);
        sprintf(buffer,"%d - %s",count++,message);
	    if (sendto(fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	        perror("sendto");
	        exit(1);
	    }

        sleep(1);
    }
}

int main(int argc, char** argv) {
    int opc;

    std::cout << "Informe (1) para sender e (2) para o listener: ";
    std::cin >> opc;

    if (opc == 1)
    {
        sender();
    } else if (opc == 2) {
        listener();
    } else {
        std::cerr << "\n\nInvalid option!";
    }

    return 0;
}
