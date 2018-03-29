#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/usr/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/usr/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
  int status;
  pid_t pid_1, pid_2, pid_3, pid_4;

  if (argc != 4) {
    printf("usage: finder DIR STR NUM_FILES\n");
    exit(0);
  }

	int pipeFG[2];
	int pipeGS[2];
	int pipeSH[2];

	pipe(pipeFG);
	pipe(pipeGS);
	pipe(pipeSH);


    pid_1 = fork();
    if (pid_1 == 0) {
    /* First Child */
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);
        sprintf(cmdbuf, "%s %s -name \'*\'.[ch]", FIND_EXEC, argv[1]);

        dup2(pipeFG[1], STDOUT_FILENO);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, (char*) 0};
        if((execv(BASH_EXEC, myArgs))< 0){
            fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
            return EXIT_FAILURE;
        }

        exit(0);
    }

    pid_2 = fork();
    if (pid_2 == 0) {
    /* Second Child */
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s %s -c %s", XARGS_EXEC, GREP_EXEC, argv[2]);

        dup2(pipeFG[0], STDIN_FILENO);
        dup2(pipeGS[1], STDOUT_FILENO);

        close(pipeFG[1]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, (char*) 0};
        if((execv(BASH_EXEC, myArgs))< 0){
            fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
            return EXIT_FAILURE;
        }

        exit(0);
    }

    pid_3 = fork();
    if (pid_3 == 0) {
    /* Third Child */
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s -t : +1.0 -2.0 --numeric --reverse", SORT_EXEC);

        dup2(pipeGS[0], STDIN_FILENO);
        dup2(pipeSH[1], STDOUT_FILENO);

        close(pipeFG[1]);
        close(pipeGS[1]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, (char*) 0};
        if((execv(BASH_EXEC, myArgs))< 0){
            fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
            return EXIT_FAILURE;
        }

        exit(0);
    }

    pid_4 = fork();
    if (pid_4 == 0) {
    /* Fourth Child */
    char cmdbuf[BSIZE];
    bzero(cmdbuf, BSIZE);
    sprintf(cmdbuf, "%s --lines=%s", HEAD_EXEC, argv[3]);

        dup2(pipeSH[0], STDIN_FILENO);

        close(pipeFG[1]);
        close(pipeGS[1]);
        close(pipeSH[1]);

        char *myArgs[] = {BASH_EXEC, "-c", cmdbuf, (char*) 0};
        if((execv(BASH_EXEC, myArgs))< 0){
            fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
            return EXIT_FAILURE;
        }

        exit(0);
    }

    close(pipeFG[1]);
    close(pipeGS[1]);
    close(pipeSH[1]);

    if ((waitpid(pid_1, &status, 0)) == -1) {
        fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
        return EXIT_FAILURE;
    }
    if ((waitpid(pid_2, &status, 0)) == -1) {
        fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
        return EXIT_FAILURE;
    }
    if ((waitpid(pid_3, &status, 0)) == -1) {
        fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
        return EXIT_FAILURE;
    }
    if ((waitpid(pid_4, &status, 0)) == -1) {
        fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
        return EXIT_FAILURE;
    }

  return 0;
}
