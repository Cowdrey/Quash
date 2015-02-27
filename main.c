
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

int main()
{
    
    while(true)
    {
        std::cout << "[" << getcwd(NULL, PATH_MAX +1) << "]"<< "$ ";
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
            char *dirToEnter = args[1];
            if(numOfArgs == 0)
            {
                struct passwd *pw = getpwuid(getuid());
                char *homedir = pw->pw_dir;
                dirToEnter = homedir;
            }
            else if(numOfArgs == 1)
            {
                
            }
            int ret = chdir (dirToEnter);
        }
        else if(access(args[0], F_OK) == 0) //In cur directory
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
        
    }
    
}
