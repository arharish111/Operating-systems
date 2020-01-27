/*
    Name: Harish Harish
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
        token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
        if( strlen( token[token_count] ) == 0 )
        {
            token[token_count] = NULL;
        }
        token_count++;
    }
    //To make sure user has entered input and if it's quit or exit,stop executing the program
    if(token[0] != NULL)
    {
        int res = strcmp(token[0],"quit");
        int res1 = strcmp(token[0],"exit");
        if(res==0 || res1==0)
        {
            exit(EXIT_SUCCESS);
        }
    }
    //Forking a process to execute commands using execvp
    pid_t child_pid = fork();
    int status,i;

    if( child_pid == 0 )
   {
        i=execvp(token[0], token);
        if(i==-1)
        {
            printf("%s- command not found\n",token[0]);
        }
        exit( EXIT_SUCCESS );
    }

    waitpid( child_pid, &status, 0 );

    free( working_root );

  }
  return 0;
}
