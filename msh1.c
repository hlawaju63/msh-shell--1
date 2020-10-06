
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define WHITESPACE " \t\n" 	//Whitespace will seperate our tokens


#define MAX_COMMAND_SIZE 255 	//The maximum command-line size

#define MAX_NUM_ARGUMENTS 10	//MAV shell only supports five arguments 

int main()
{
	//declaring space for a variable to for users input
	char *cmd_str =(char*) malloc(MAX_COMMAND_SIZE);
	
	//declaring variables used for pid_history
	int pid_index = 0;		
	int pid_lt_15 = 1;
	pid_t pid_history[15];
	
	//declaring variables used for command_history
	int command_index = 0;		
	int command_lt_15 = 1;
	char command_history[15][50];

	//declaring variables for nth command user input
	int nth_command;
   	char str[5] = {};
	int command_num ;		

	while(1)
	{
		//Print out the msh prompt
		printf("msh> ");
	
		//Read the command from	commandline. The
		//maximum command that will be read is MAX_COMMAND_SIZE
		//THIS while command will wait here until the user
		//inputs something since fets returns NULL when there
		// is no input
		
		while(!fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );


		/* PArse input */
		char *token[MAX_NUM_ARGUMENTS];

		int token_count = 0;

		//Pointer to point to the token
		// parses by strsep
		char *argument_ptr;

		char *working_str = strdup( cmd_str );

		// we are going to	move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		
		char *working_root = working_str;
		
		// Tokenize the input string with whitespace used as the delimeter
		
		while( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
		(token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0)
			{
				token[token_count] = NULL;
			}
			token_count++;
		}//string tokenize while loop


		//Now print the tokenized input as a debug check

		//asking for user input again if user enters WHITESPACES only	
		if(token[0] == NULL)
		{
			continue;

		}

		//Exiting command if user enters quit or exit
		if( strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0 )
		{ 
			free(cmd_str);
			exit(0);
		}
	

		// Checking if user wants to change directory,
		//and chdnging directory if need to

		if( strcmp( token[0],  "cd" ) == 0)
                {
	          chdir( token[1] );			
                  continue;
                }
		
		//checking if user inputs showpids, if yes, then looping 
		//to show the the 15 recent pid history
		
		if( strcmp( token[0], "showpids" )  ==  0 )
		{
			for(int i = 0; i <  15; i ++)
			{
				// if pid less than 15 then only showing those pids
				if(pid_lt_15)
				{
					if(i >=  pid_index) break;
					printf("%d: %d \n", i, pid_history[i]);
				}		
				else // if > 15 rearranging index to show recent 15 history
				{
					printf("%d: %d \n", i, pid_history[pid_index++]);
					if(pid_index > 14 )  pid_index = 0;
					
				}
			}continue;//going back to top of while loop after for
				
		}
			
		//checking if user inputs history command, then looping to show the
		//user input recent 15 history command

		if( strcmp( token[0], "history" )  ==  0 )
		{
			for(int i = 0; i <  15; i ++)
			{
				if(command_lt_15)
				{
					if(i >=  command_index) break;
					printf("%d: %s \n", i, command_history[i]);
				}		
				else //if > 15 rearranging index to show recent 15 history
				{
					printf("%d: %s \n", i, command_history[command_index++]);
					if(command_index > 14 )  command_index = 0;
					
				}
			}continue;//going back to top of while loop after the loop
				
		}
		
		//checking to see if user input to see nth command from the 
		//command history list

		if( token[0][0] == '!' )
		{
			//with 2 cases if string is 0-9 or 10-14
			//converting str to int		
	
			if( strlen( token[0]) == 2 )
			{
				str[0] = token[0][1] ; 
				str[1] = '\0';
				nth_command = atoi(str);
			}	
			else if( strlen( token[0] ) >=2 )
			{
				str[0] = token[0][1];
				str[1] = token[0][2];
				str[2] = '\0';
				
				nth_command = atoi(str);
			}

			// managing command num depending on command < 15 condition
			// to get proper command history index 
			if(command_lt_15)
			{
				command_num = nth_command;
			}
			else
			{
				command_num = nth_command + command_index;
			}
		
		}
		//forking a new child
 
		pid_t child_pid = fork();
		

 		//Assigning PID every time new process is forked	

		pid_history[pid_index++] = child_pid;

		//assgining command in the list every time new prcess is forked
		strcpy( command_history[command_index++], token[0]);


		//since we only care about 15 pid and command history, 
		//we are going back to 0 if it exceeds this num

		if(pid_index > 14)
		{
			pid_index = 0;
			pid_lt_15 = 0;
		}
		
		if(command_index > 14 )
		{
			command_index = 0;
			command_lt_15 = 0;
		}


		// if we are inside child processes

		if( child_pid == 0 )
		{
			int ret;
			
			//using exec and creating child process different than parent processes
			// in else condition
			// if condition is for nth_commad from history
			
			if( token[0][0] == '!' )
			{
				ret = execlp(command_history[command_num],
 					command_history[command_num],NULL);
			}
			else
			{
				ret = execvp(token[0], &token[0] );
			}

			if(ret == -1)
			{
 					printf(" %s: Command not found\n", token[0]);		 
			}
		
		}
		else
		{
			int status;
			wait( &status);	
		}

		free(working_root );

	
	}//outmost while

	

	free(cmd_str);
	return 0;
}


