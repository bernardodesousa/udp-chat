#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>

#define EXAMPLE_PORT 4242
#define EXAMPLE_GROUP "225.0.0.37"

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

        // 4) Receber mensagem externa
        return_value = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
        if (return_value < 0) {
            perror("ERROR: bind failled");
            // return 1;
        }

        mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        return_value = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
        if (return_value < 0) {
            perror("ERROR: setsockopt failled");
            // return 1;
        }

        return_value = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &addr, &addrlen);
        if (return_value < 0) {
            perror("ERROR: recvfrom failled");
            // return 1;
        }

        // 5) Apresentar mensagem recebida
        printf("%s: message = \"%s\"\n", inet_ntoa(addr.sin_addr), message);

        // 6) Voltar ao passo 2
    }

    return 0;
}
