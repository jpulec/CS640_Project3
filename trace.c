#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "utilities.h"


MAX_TOP_SIZE = 20;

struct node {
    char *ip;
    int port;
};

struct entry {
    struct node *parent;
    struct node **children;
};


struct entry **readtoplogy(char *filename);


int main(int argc, char **argv) {
    // ------------------------------------------------------------------------
    // Handle commandline arguments
    if (argc != 5) {
        printf("usage: emulator -p <port> -f <filename>\n");
        exit(1);
    }

    char *portStr     = NULL;
    char *filename    = NULL;

    int cmd;
    while ((cmd = getopt(argc, argv, "p:f:")) != -1) {
        switch(cmd) {
          case 'p': portStr      = optarg; break;
          case 'f': filename     = optarg; break;
          case '?':
                    if (optopt == 'p' || optopt == 'f')
                        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                    else if (isprint(optopt))
                        fprintf(stderr, "Unknown option -%c.\n", optopt);
                    else
                        fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                    exit(EXIT_FAILURE);
                    break;
          default: 
                    printf("Unhandled argument: %d\n", cmd);
                    exit(EXIT_FAILURE); 
        }
    }

    printf("Port           : %s\n", portStr);
    printf("Filename       : %s\n", filename);

    // Convert program args to values
    int port         = atoi(portStr);

    // Validate the argument values
    if (port <= 1024 || port >= 65536)
        ferrorExit("Invalid port");
    puts("");
}
