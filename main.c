
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
#include <fcntl.h>


int status;
pid_t pid_1;

int pipefd1[2];

int main()
{
    bool redirected = false;
    
    if(!isatty(STDIN_FILENO)) //A file has been redirected to stdin
    {
        redirected = true;
    }
    
    while(true)
    {
        if(!redirected)
        {
            char testStr[PATH_MAX +1];
            getcwd(testStr, PATH_MAX +1);
            std::cout << "[" << testStr << "]"<< "$ ";
        }

        char input[2097152];
        
        if(fgets(input, 2097152, stdin) == NULL)
		{
        	break;
		}
        
        char* args[100];
        int i = 0;
        char* tempArg = strtok(input, " \n");
        int numOfArgs;

		char* inputFrom = NULL;
		int savedInput = dup(STDIN_FILENO);
		char* outputTo = NULL;
		int savedOutput = dup(STDOUT_FILENO);
        
        while(tempArg)
        {
            if(!strcmp(tempArg, ">"))
            {
                outputTo = strtok(NULL, " \n");
                if(outputTo == NULL)
                {
                    std::cout << "Error! No output file specified!" << std::endl;
                    exit(0);
                }
            }
            else if(!strcmp(tempArg, "<"))
            {
                inputFrom = strtok(NULL, " \n");
                if(inputFrom == NULL)
                {
                    std::cout << "Error! No input file specified!" << std::endl;
                    exit(0);
                }
            }
            else
            {
                args[i++] = tempArg;
            }
            
            tempArg = strtok(NULL, " \n");
        }

		int inFile;
		bool inFileUsed = false;
		int outFile;
		bool outFileUsed = false;

		if(inputFrom != NULL) //Input file specified, redirect
		{
			inFileUsed = true;
			inFile = open(inputFrom, O_RDONLY | O_CREAT);
			if(inFile < 0)
			{
				std::cout << "Error opening " << inputFrom << "." << std::endl;
				exit(0);
			}
			
			dup2(inFile, STDIN_FILENO);
		}
		
		if(outputTo != NULL) //Output file specified, redirect
		{
			outFileUsed = true;
			outFile = open(outputTo, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if(outFile < 0)
			{
				std::cout << "Error opening " << outputTo << "." << std::endl;
				exit(0);
			}

			dup2(outFile, STDOUT_FILENO);
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
        else if(!strcmp(args[0], "set"))
        {
            char* tokenizer = strtok(args[1], "=");
            char *dirToSet = getenv(tokenizer);
	    	strcpy(dirToSet,strtok(NULL, "="));
        }       
        else if((!strcmp(args[0], "quit")) || (!strcmp(args[0], "exit")))
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

		if(inFileUsed)
		{
			dup2(savedInput ,STDIN_FILENO);
			close(inFile);
			inputFrom = NULL;
		}
		if(outFileUsed)
		{
			dup2(savedOutput, STDOUT_FILENO);
			close(outFile);
			outputTo = NULL;
		}
		
		close(savedInput);
		close(savedOutput);
		int j = 0;
		while(args[j] != NULL)
		{
			args[j++] = NULL;
		}
    }
    
}

