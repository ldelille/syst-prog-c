#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <netinet/in.h>

#define PORT 8080
bool running;

static void stop_handler(int sig)
{
    running = false;
    printf("Detected signal number: %d, received by process %d\n", sig, getpid());
}

void exit_message()
{
    printf("End of the process: %d\n", getpid());
}

int main(int argc, char const *argv[])
{
    int sfd, cfd, n, i;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //localhost
    address.sin_port = htons(PORT);       //defining unprivileged source port
    int addrlen = sizeof(address);

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = stop_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGTERM, &sigIntHandler, NULL); // Catching kill signal impossible to catch kill -9
    sigaction(SIGINT, &sigIntHandler, NULL);  // Catching CTRL-C interruption
    sigaction(SIGPIPE, &sigIntHandler, NULL); // detect read-end close

    i = atexit(exit_message);
    if (i != 0)
    {
        fprintf(stderr, "cannot set exit function\n");
        exit(EXIT_FAILURE);
    }

    running = true;

    // create a socket over IPV4 using stream

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket init failed");
        exit(EXIT_FAILURE);
    }
    //bind to port
    if (bind(sfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    //listen on that port
    if (listen(sfd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((cfd = accept(sfd, (struct sockaddr *)&address,
                      (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (running)
    {
        int status = 0;
        n = rand() % 100;
        status = send(cfd, &n, sizeof(n), 0); 

        if (status <= 0)
        {
            printf("Write failed : %d\n", status);
            running = false;
        }
        printf("Server wrote random number: %d\n", n);
        sleep(1);
    }

    return 0;
}
