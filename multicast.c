#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h> 

#define HELLO_PORT 12345
#define MSGBUFSIZE 256
#define EXAMPLE_PORT 4242
#define EXAMPLE_GROUP "225.0.0.37"
#define HELLO_GROUP "225.0.0.37"

void *start_listening_loop(void *argument)
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
        // exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(EXAMPLE_PORT);
    addrlen = sizeof(addr);

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        perror("!! Reusing ADDR failed");
    }

    // addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
    // addr.sin_port = htons(HELLO_PORT);

    /* bind to receive address */
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        // exit(1);
    }

    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr = inet_addr(HELLO_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *) &mreq, sizeof(mreq)) < 0)
    {
        perror("setsockopt");
        // exit(1);
    }

    /* now just enter a read-print loop */
    while (1)
    {
        addrlen = sizeof(addr);
        if ((nbytes=recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, &addrlen)) < 0)
        {
            perror("recvfrom");
            // exit(1);
        }
        puts(msgbuf);
    }
}

int main(int argc)
{
    struct sockaddr_in addr;
    int addrlen, sock, return_value, i;
    struct ip_mreq mreq;
    char message[50], c;

    // 1) Configurar o socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // bzero((char *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(EXAMPLE_PORT);
    addrlen = sizeof(addr);

    // start listening thread
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, start_listening_loop, NULL);
    pthread_join(thread_id, NULL);

    while(1) {
        // 2) Digitar o texto
        i = 0;
        scanf("%c", &c);
        while (c != '\n') {
            message[i] = c;
            i++;
            scanf("%c", &c);
        }
        message[i] = '\0';

        // 3) Transmitir o texto
        addr.sin_addr.s_addr = inet_addr(EXAMPLE_GROUP);
        time_t t = time(0);
        sprintf(message, "time is %-24.24s", ctime(&t));
        printf("sending: %s\n", message);
        return_value = sendto(sock, message, sizeof(message), 0, (struct sockaddr *)&addr, addrlen);
        if (return_value < 0) {
            perror("ERROR: sendto failled");
            // return 1;
        }

        // 6) Voltar ao passo 2
    }

    return 0;
}
