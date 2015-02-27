
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <pwd.h>


int status;
pid_t pid_1;

int pipefd1[2];

int main()
{
    
    while(true)
    {
		char testStr[PATH_MAX +1]; 
		getcwd(testStr, PATH_MAX +1);
        std::cout << "[" << testStr << "]"<< "$ ";
        char input[2097152];
        
        fgets(input, 2097152, stdin);
        
        char* args[100];
        int i = 0;
        char* tempArg = strtok(input, " \n");
        int numOfArgs;
        
        
        while(tempArg)
        {
            args[i++] = tempArg;
            tempArg = strtok(NULL, " \n");
        }
        numOfArgs = (i -1);
        
        if(!strcmp(args[0],"cd"))
        {
			
            char *dirToEnter;
            if(numOfArgs == 0)
            {

                dirToEnter = getenv("HOME");
		        chdir (dirToEnter);
            }
            else if(numOfArgs == 1)
            {
                dirToEnter = args[1];
            	chdir (dirToEnter);
				
            }
		
        }
		else if(!strcmp(args[0], "q"))
		{
			break;
		}
		else
		{
			
			
			if(pipe(pipefd1) == -1)
			{
				perror("pipe1");
				exit(EXIT_FAILURE);
			}

			pid_1 = fork();

			if(pid_1 == 0)
			{
				close(pipefd1[0]);
				close(pipefd1[1]);

				if(access(args[0], F_OK) == 0) //In cur directory
				{
				    execvpe(args[0], args, environ);
				}
				else                               //Not in cur directory
				{
					int exit = 0;
				    char copyEnv[2097152];
				    
				    strcpy(copyEnv, getenv("PATH"));
				    
				    char* tempPath = strtok(copyEnv, ":");
				    
				    while(tempPath)
				    {
				        char tempExec[256];
				        strcpy(tempExec, tempPath);
				        strcat(tempExec, "/");
				        strcat(tempExec, args[0]);
				        
				        if(access(tempExec, F_OK) == 0) //Found correct Path Directory
				        {
				            execvpe(args[0], args, environ);
				            exit = 1;
							break;
				    	}
				        
				        tempPath = strtok(NULL, ":");
				    }
				    if(exit == 0)
			   		{
				    	std::cout << "Error! Executable " << args[0] << " is not in current directory or path." << std::endl;
					}
				}

				exit(0);
			}

			close(pipefd1[0]);
			close(pipefd1[1]);
		    
			if ((waitpid(pid_1, &status, 0)) == -1) {
				fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
				return EXIT_FAILURE;
			}

		}
		int j = 0;
		while(args[j] != NULL)
		{
			args[j++] = NULL;
		}
    }
    
}
