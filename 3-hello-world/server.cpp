#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void do_something(int connfd) {
    /*
        Note that read() and write() call returns the number of read or written bytes.
        A real programmer must deal with the return value of functions,
        but in this tutorial, we omit these handlings.
    */
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

/* 
    g++ -Wall -Wextra -O2 -g server.cpp -o server
    -Wall is for enabling all warnings
    -Wextra is for additional warning message
    -O2 is for optimizing
    -g is for including debugging info
*/
int main() {
    /*
        AF_INET is for IPv4
        AF_INET6 is for IPv6 or dual-stack socket

        SOCK_STREAM is for TCP
    */
    int fd = socket(AF_INET, SOCK_STREAM, 0);


    /*
        setsockopt() is used for configuring various aspects of a socket
        SO_REUSEADDR is for allowing the server to reuse address if restarted

        https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux
    */
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0

    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0) {
            continue; // err
        }

        do_something(connfd);
        close(connfd);
    }
}
