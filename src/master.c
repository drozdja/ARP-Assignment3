#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "./../include/common.h"

int spawn(const char *program, char *arg_list[])
{

  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
    return -1;
  }

  else if (child_pid != 0)
  {
    return child_pid;
  }

  else
  {
    if (execvp(program, arg_list) < 0)
      ;
    perror("Exec failed");
    return -1;
  }
}

int main()
{
  char ip[20] = {};
  char port[20] = {};
  int choice;

  printf("Enter execution type:\n1-Normal\n2-Server\n3-Client\n");
  scanf("%d", &choice);

  switch (choice)
  {
  case 1:
    break;
  case 2:
  case 3:
    printf("Enter the port (0 for default):\n");
    scanf("%s", port);
    printf("Enter the server IP (0 for local IP):\n");
    scanf("%s", ip);
    break;
  default:
    printf("Invalid input.\n");
    exit(1);
  }

  char mode[2];
  sprintf(mode, "%d", choice);

  // Opening semaphore:
  sem_t *sem = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
  if (sem == SEM_FAILED)
  {
    perror("Error opening semaphore");
    exit(1);
  }

  // Initializing semaphore:
  if (sem_init(sem, 1, 0) < 0)
  {
    perror("Error initializing semaphore");
    sem_close(sem);
    sem_unlink(SEM_PATH);
    exit(1);
  }

  char *arg_list_A[] = {"/usr/bin/konsole", "-e", "./bin/processA", mode, port, ip, NULL};
  char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", mode, NULL};
  pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
  pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  int status;
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);

  // Closing sempahore
  sem_close(sem);
  sem_unlink(SEM_PATH);

  printf("Main program exiting with status %d\n", status);
  return 0;
}
