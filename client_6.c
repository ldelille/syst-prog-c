#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

int main(int argc, char *argv[])
{
    int n; //number to send
    int i;
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

    int socket_a = 0, valread;
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if ((socket_a = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nAdress is no valid \n");
        return -1;
    }

    if (connect(socket_a, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (running)
    {
        int status = 0;
        status = read(socket_a, &n, sizeof(n));
        if (status <= 0)
        {
            printf("Read failed with status: %d\n", status);
            running = false;
        }
        else
        {
            printf("Client received number: %d\n", n);
        }
    }

    return 0;
}
