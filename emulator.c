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


    // Read network topology
    struct entry **topology = readtopology(filename);
    printTopology(topology);

    // Create network routes from topology
    createRoutes(topology);


    // TODO: this is temporary, need to get sender address info from fwd table
    // Setup sender address info 
    struct addrinfo ehints;
    bzero(&ehints, sizeof(struct addrinfo));
    ehints.ai_family   = AF_INET;
    ehints.ai_socktype = SOCK_DGRAM;
    ehints.ai_flags    = 0;

    // Setup emu sending socket
    struct sockaddr_in emuaddr;

    int sockfd;

    if( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1){
        perrorExit("Socket error");
    }

    emuaddr.sin_family = AF_INET;
    emuaddr.sin_port = htons(port);
    emuaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(emuaddr.sin_zero), 8);


    if (bind(sockfd, (struct sockaddr*)&emuaddr, sizeof(emuaddr)) == -1) {
        close(sockfd);
        perrorExit("Bind error");
    }

    printf("Emulator socket created on port:%d\n", port );

    //-------------------------------------------------------------------------
    // BEGIN NETWORK EMULATION LOOP
    puts("Emulator waiting for packets...\n");

    struct packet *pkt = NULL;
    struct sockaddr_in recvAddr;
    struct addrinfo *np;

    socklen_t recvLen = sizeof(recvAddr);
    //socklen_t sendLen = sizeof(sendAddr);
    // HACK: Don't like hard coding this, but don't know any other way
    size_t MAX_HOST_LEN = 256;
    char name[MAX_HOST_LEN];
    gethostname(name, MAX_HOST_LEN);
    //Need to just get lowest level dns name i.e. mumble-30
    char *pch;
    pch = strtok(name, ".");

    while (1) {
        void *msg = malloc(sizeof(struct packet));
        bzero(msg, sizeof(struct packet));
        size_t bytesRecvd;
        bytesRecvd = recvfrom(sockfd, msg, sizeof(struct packet), 0,
                              (struct sockaddr *)&recvAddr, &recvLen);
        struct forward_entry *curEntry;
        if (bytesRecvd != -1) {
            //printf("Received %d bytes\n", (int)bytesRecvd);
            // Deserialize the message into a packet 
            struct packet *pkt = malloc(sizeof(struct packet));
            bzero(pkt, sizeof(struct packet));
            deserializePacket(msg, pkt);


            //TODO: Consult forwarding table to see if packet is to be
            //forwarded, then enqueue it
            //
            // for now, just forward it directly to sender...
            //sendPacketTo(sockfd, pkt, (struct sockaddr *)sp->ai_addr);


            curEntry = head;
            unsigned long curEntryIP = 0;

            struct addrinfo entryHints;
            bzero(&entryHints, sizeof(struct addrinfo));
            entryHints.ai_family   = AF_INET;
            entryHints.ai_socktype = SOCK_DGRAM;
            entryHints.ai_flags    = 0;

            struct addrinfo *entryInfo;
            int dstsockfd = 0;
            int errcode = getaddrinfo(curEntry->dst_hostname, NULL, &entryHints, &entryInfo);
        }	
    }
    if (close(sockfd) == -1) perrorExit("Close error");
    else                     puts("Connection closed.\n");

    // Cleanup address info data
    freeaddrinfo(emuinfo);

    // All done!
    exit(EXIT_SUCCESS);


}


struct entry **readtopology(char *filename){
    FILE *file = fopen(filename, "r");
    if (file == NULL) perrorExit("File open error");
    else              printf("Opened file \"%s\" for reading.\n". filename);

    struct entry **ret = malloc(20 * sizeof(struct *entry));

    int lineCount = 0;

    char *line = NULL;
    size_t lineLen = 0;
    size_t bytesRead = getLine(&line, &lineLen, file);
    if (bytesRead == -1) perrorExit("Getline error");
    while (bytesRead != -1){
        if (lineCount > MAX_TOP_SIZE - 1){
            perrorExit("More than 20 lines in topology file. Exiting.");
        }
        //TODO: Do some data validation

        ret[lineCount] = malloc(sizeof(struct entry));
        int n = 0;
        char *tokens[MAX_TOP_SIZE - 1];
        char *tok = strtok(line, " ");
        while (tok != NULL) {
            tokens[n++] = tok;
            tok = strtok(NULL, " ");
        }
        // Line done being parsed
        int m = 0;
        ret[lineCount]->parent          =   malloc(sizeof(struct entry));
        ret[lineCount]->parent->ip      =   strtok(tokens[m], ",");
        ret[lineCount]->parent->port    =   strtok(NULL, ",");
        m++;
        ret[lineCount]->children = malloc(sizeof(struct *entry) * MAX_TOP_SIZE);
        while ( m <= n){
            ret[lineCount]->children[m]     = malloc(sizeof(struct entry));
            ret[lineCount]->children[m]->ip = strtok(tokens[m], ",");
            ret[lineCount]->children[m]->port = strtok(NULL, ",");
            m++;
        }
        ret[lineCount]->children[m] = NULL;
        free(line);
        bytesRead  = getline(&line, &lineLen, file);
    }
    free(line);

    if (fclose(file) != 0) perrorExit("Topology close error");
    else                   printf("Topology file closed");

    return ret;
}

void printTopology(struct entry **topology){
    int i = 0;
    int j = 0;
    for(i = 0; i < MAX_TOP_SIZE - 1; ++i){
        printf("%s:%d", topology[i]->parent->ip, topology[i]->parent->port);
        for(j = 0; topology[i]->children != NULL; ++j){
            printf("%s:%d", topology[i]->children[j]->ip, topology[i]->children[j]->port);
        }
        printf("\n");
    }
}
