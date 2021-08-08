/* Filename: server_5.c */
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

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
   int fd; //fifo
   int n;  //number to send
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

   fd = open("fifo_a", O_RDONLY);

   while (running)
   {
      int status = 0;
      status = read(fd, &n, sizeof(n));
      if (status <= 0)
      {
         printf("Read failed : %d\n", status);
         running = false;
      }
      else
      {
         printf("Client received number: %d\n", n);
      }
   }
   close(fd);

   return EXIT_SUCCESS;
}

