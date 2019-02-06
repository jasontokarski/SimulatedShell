/****************************************************************
PROGRAM:   Microshell
AUTHOR:    Jason Tokarski

FUNCTION:  This program will fork two times, creating two seperate
child processes from a single parent process. The first child process
will execute a shell command, then the output will be redirected
into a pipe which will then be read by child process 2 and execute
a second command using the input from the first child process.
****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0

int main(void)
{
    char buf[1024] = {'\0'}; //Hold the entire command entered by a user
    char *command[10] = {'\0'}; //Each string holds a command on either side of the || symbol
    char *arguments[9]  = {'\0'}; //Each string holds an argument for an individual command
    char *token_ptr; //An iterator for the strtok function
    pid_t pid1, pid2; //Used to hold the process ID of the two children
    /*Various indexes, flags, and variables used throughout the program*/
    int status = 0, command_index = 0, argument_index = 0;
	bool quit = FALSE, display_shell = TRUE;
    /* Index 0 = Read end of Pipe
	   Index 1 = Write end of pipe*/
	int	pipe_fd[2];

    /*Remove any output buffering*/
    setbuf(stdout, NULL);

    /*If quit flag is true then end the shell*/
    while(!quit){
        /*Initial shell name*/
        if(display_shell)
        {   
            printf("MicroShell>");
            display_shell = FALSE;
        }
        /*Check for any user input*/
        while (fgets(buf,1024,stdin) != NULL)
        {
            /*Create a new pipe each time a command is read*/
            if(pipe(pipe_fd) < 0)
            {
                perror("piping error");
                exit(EXIT_FAILURE);
            }

            /*Reset our command/argument indexes*/
            command_index = 0;
            argument_index = 0;

            /*Set the second to last character to null*/
            buf[strlen(buf)-1] = 0;
            /*Allow user to end 480shell with q or quit*/
            if(!(strcmp(buf, "q")) || !(strcmp(buf, "quit")))
            {
                quit = TRUE;
                break;
            }

            /*Seperate the commands on both ends of the || symbol into two character arrays*/
            command[0] = strtok(buf,"||");
            while((token_ptr = strtok(NULL, "||")) != NULL)
            {
                command_index++;
                command[command_index] = token_ptr;
            }

            /*Create the first child process to send the command execution output to the pipe*/
            pid1 = fork();
            if (pid1 < 0)
            {
		        perror("Failure on fork for child 1.\n");
		        exit(-1);
            }
            /*Child 1 (Execute a command and send to child 2)*/
            if (pid1 == 0)
            { 
                /*Check if pipe command was used*/
                if(command_index > 0)
                {
                    /*Break up the first command into individual character arrays for each argument*/
                    arguments[0] = strtok(command[0], " ");
                    while((token_ptr = strtok(NULL, " ")) != NULL)
                    {
                        argument_index++;
                        arguments[argument_index] = token_ptr;
                    }
                    /*Add a null terminator to the end of our argument list*/
                    argument_index++;
                    arguments[argument_index] = NULL;
                    /*Close the read end of our pipe*/
                    close(pipe_fd[0]);
                    /*Redirect stdout to the write end of our pipe*/
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    /*Since our file descriptor is duplicated we no longer need the write end of the pipe*/
                    close(pipe_fd[1]);
                    /*Execute the first command, output will be sent to the pipe*/
                    if(execvp(*arguments, arguments) < 0)
                    {
                        perror("First command failed to execute.\n");
		                exit(-1);
                    }
                }
                /*Execute a single command with no pipe*/
                else
                {
                    /*Break up the first command into individual character arrays for each argument*/
                    arguments[0] = strtok(command[0], " ");
                    while((token_ptr = strtok(NULL, " ")) != NULL)
                    {
                        argument_index++;
                        arguments[argument_index] = token_ptr;
                    }
                    /*Add a null terminator to the end of our argument list*/
                    argument_index++;
                    arguments[argument_index] = NULL;
                    /*Execute our command, output goes to standard output*/
                    if(execvp(*arguments, arguments) < 0)
                    {
                        perror("Command failed to execute.\n");
		                exit(-1);
                    }
                }
            }
            else
            {
                /*Create a second child process to receive input from our pipe*/
                pid2 = fork();
                if (pid2 < 0)
                {
		            perror("Failure on fork for child 2.\n");
		            exit(-1);
                }
                //Child 2 (Receives the output from the child 1)
                if (pid2 == 0) 
                {
                    /*Break up the second command into individual character arrays for each argument*/
                    arguments[0] = strtok(command[1], " ");
                    while((token_ptr = strtok(NULL, " ")) != NULL)
                    {
                        argument_index++;
                        arguments[argument_index] = token_ptr;
                    }
                    /*Add a null terminator to the end of our argument list*/
                    argument_index++;
                    arguments[argument_index] = NULL;
                    /*Close the write end of our pipe*/
                    close(pipe_fd[1]);
                    /*Use input from our pipe rather than standard input*/
                    dup2(pipe_fd[0], STDIN_FILENO);
                    /*Since the read end of our file descriptor was duplicated we no longer need it*/
                    close(pipe_fd[0]);
                    /*Execute the second command, using output from our first command*/
                    if(execvp(*arguments, arguments) < 0)
                    {
                        perror("Second command failed to execute.\n");
		                exit(-1);
                    }
                }
                else
                {
                    /*Reset our command/argument buffers*/
                    memset(arguments, 0, sizeof(arguments));
                    memset(command, 0, sizeof(command));
                    /*Close both ends of the pipe*/
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);
                    /*Wait for Child 1 and Child 2 to finish*/
                    waitpid(pid1, &status, 0);
                    waitpid(pid2, &status, 0);
					/*Prompt the user*/
                    printf("MicroShell>");
                    display_shell = TRUE;
                }
            }
        }
    }
    exit(0);
} 
