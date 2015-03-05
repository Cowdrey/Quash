#include<stdio.h>
#include<stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>

struct link {
  struct link *next = NULL;
  int pid;
  int jobid;
  char command[2097152];
};


struct link *ProcessHead = NULL;
struct link *ProcessTail = NULL;
int jobidvar = 0;

void insert_link(struct link *newlink) 
{
	if(ProcessHead == NULL)
	{
		ProcessHead = newlink;
		ProcessTail = newlink;
	}
	else
	{
  		ProcessTail->next = newlink;
  		ProcessTail = ProcessTail->next;
	}
}

struct link * get_link(int pidp) 
{
  	for(struct link *iter = ProcessHead; iter != NULL; iter = iter->next) {
    	if(iter->pid == pidp)
    	{
    		return iter;
    	}
    }
    return NULL;
}

void remove_link(struct link ** listP,int value)
{
  struct link *currP, *prevP;

  prevP = NULL;

  for (currP = *listP;
	currP != NULL;
	prevP = currP, currP = currP->next) {

    if (currP->pid == value) {  /* Found it. */
      printf("[%ld] %ld finished %s\n",  currP->jobid, currP->pid,  currP->command);
      if (prevP == NULL) {
        *listP = currP->next;
      } else {
        prevP->next = currP->next;
      }

      free(currP);

      return;
    }
  }
}


bool isBackground(int thisPid)
{
	for(struct link *iter = ProcessHead; iter != NULL; iter = iter->next) {
    	if(iter->pid == thisPid)
    	{
    		return true;
    	}
    }
    return false;
}



void printJobs() 
{
  	for(struct link *iter = ProcessHead; iter != NULL; iter = iter->next) {
		std::cout << "[" << iter->jobid << "] "<< iter->pid <<
		 " " << iter->command<<
		  " \n";
    }
}
int status;
pid_t pid_1, pidbg, pid_p;

int pipefd1[2], pipefdP[2];
bool backproc = false;

int main()
{
    bool redirected = false;
    
    if(!isatty(STDIN_FILENO)) //A file has been redirected to stdin
    {
    	
        redirected = true;
    }
    
    while(!backproc)
    {
		int savedInputPipe = dup(STDIN_FILENO);

		bool pipeDetected = false;

    	struct link* newProc =NULL;

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

		int kidpid;
		int status;
		while ((kidpid = waitpid(-1, &status, WNOHANG)) > 0)
		{
		 	 remove_link(&ProcessHead,kidpid);
		}

		bool lastBackProc = false;
		
		if(strlen(input) == 1)continue;
		
		if(input[strlen(input)-2] == '&')
		{
			lastBackProc = true;
		}

		bool contiBool = false;
		char* tempProc;
			tempProc = strtok(input, "&\n");
		
		while(tempProc)
		{
			char* nextProc;
			nextProc = strtok(NULL, "&\n");

			if(nextProc == NULL)
			{
				if(lastBackProc)
				{
					
					pidbg = fork();

					if(pidbg != 0)
					{
						newProc = (struct link*) malloc(sizeof(struct link));
					    newProc->next = NULL;
					    newProc->jobid = jobidvar;
					    jobidvar++;
					    newProc->pid = pidbg;
			            insert_link(newProc); 
						strcpy(newProc->command, tempProc);
						std::cout << "[" << newProc->jobid << "]"<< newProc->pid << " running in background\n";
						redirected = false;
						lastBackProc = false;
						contiBool = true;
						break;
					}
					backproc = true;
				}
			}
			else
			{
				pidbg = fork();

				if(pidbg != 0)
				{
					newProc = (struct link*) malloc(sizeof(struct link));
				    newProc->next = NULL;
				    newProc->jobid = jobidvar;
				    jobidvar++;
				    newProc->pid = pidbg;
		            insert_link(newProc);
					strcpy(newProc->command, tempProc);
					std::cout << "[" << newProc->jobid << "]"<< newProc->pid << " running in background\n";
					tempProc = nextProc;
					continue;
				}
				backproc = true;
			}

			strcpy(input, tempProc);
			

			break;
		}

		if(contiBool)
		{
			contiBool = false;
			continue;
		}

		if(strchr(input, '|'))
		{
			pipeDetected = true;

			char * tempCommand1 = strtok(input, "|");
			char * tempCommand2 = strtok(NULL, "|");	

			if(pipe(pipefdP) == -1)
			{
				perror("pipefdP");
				exit(EXIT_FAILURE);
			}

			pid_p = fork();



			if(pid_p == 0) //left command
			{
				close(pipefdP[0]);
				strcpy(input, tempCommand1);
				dup2(pipefdP[1], STDOUT_FILENO);
			}
			else
			{
				close(pipefdP[1]);
				strcpy(input, tempCommand2);
				dup2(pipefdP[0], STDIN_FILENO);
			}
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
        
		if(pipeDetected && pid_p != 0)
		{
			if ((waitpid(pid_p, &status, 0)) == -1) 
			{
				fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
				return EXIT_FAILURE;
			} 
		}
		
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
        else if(!strcmp(args[0], "jobs"))
        {
        	printJobs();
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
			if ((waitpid(pid_1, &status, 0)) == -1) 
			{
				fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
				return EXIT_FAILURE;
			} 
			
        }

		
		if(inFileUsed)
		{
			dup2(savedInput, STDIN_FILENO);
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

		if(pipeDetected)
		{
			if(pid_p == 0)
			{
				close(pipefdP[1]);
				close(savedInputPipe);
				exit(0);
			}
		
			dup2(savedInputPipe, STDIN_FILENO);
			close(pipefdP[0]);
		}


		close(savedInputPipe);

	}
    
}
