/*
 *  This is a simple shell program from
 *  rik0.altervista.org/snippetss/csimpleshell.html
 *  It's been modified a bit and comments were added.
 *
 *  Alternations were made by: Wendy Chong
 *  EE 468
 *  Due: September 11, 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define BUFFER_SIZE 255 //size of buffer string
#define COMMAND_SIZE 255 //size of command string
#define ARGS_SIZE 255 // number of args per command
#define CMDS_SIZE 10 // NUMBER OF COMMANDS YOU CAN USE WITH PIPES
#define PIPE_SIZE 65536
#define DEBUG 1  /* In case you want debug messages */

typedef struct Command {
    char cmd[COMMAND_SIZE];
    char *args[ARGS_SIZE];
    size_t nargs;
    int *ret_status;
    int in[2];
    int out[2];
} Command;

void parse_cmds(char* buffer, Command* commands, size_t command_size, size_t cmds_size, size_t* ncmds)
{
    //temporary
    /*strncpy(commands[0].cmd, "ps -ax", COMMAND_SIZE-1);
    strncpy(commands[1].cmd, "grep launchd", COMMAND_SIZE-1);
    strncpy(commands[2].cmd, "awk '{print $2}'", COMMAND_SIZE-1);*/

    //printf("Parsing cmds of \"%s\": ", buffer);

    *ncmds = 0;

    char *buf_args[cmds_size];
    char **cp;  // This is used as a pointer into the string array
    char *wbuf = buffer;  // String variable that has the command line

    buf_args[0]=buffer;
    //commands[0]=buffer;
    for(cp=buf_args; (*cp=strsep(&wbuf, "|")) != NULL ;)
    {
        //printf("\"%s\" ",*cp);
        if ((*cp != '\0') && (++cp >= &buf_args[cmds_size]))
            break;
    }

    /*
     * Copy 'buf_args' into 'args'
     */
    size_t i, j;
    for (j=i=0; buf_args[i]!=NULL; i++)
    {
        if(strlen(buf_args[i])>0)  /* Store only non-empty tokens */
        {
            //printf("\"%s\" ",buf_args[i]);
            strncpy(commands[j++].cmd,buf_args[i],COMMAND_SIZE-1);
        }
        if (j >= CMDS_SIZE)
            break;
    }
    *ncmds = j;

    //printf("\n");

    //*ncmds = 2;
}

void parse_args(char* buffer, char** args, size_t args_size, size_t* nargs);
void error(char *s);

int main(int argc, char *argv[], char *envp[]){
    char buffer[BUFFER_SIZE]; //holds the string of commands
    Command commands[CMDS_SIZE];

    size_t ncmds = 0;

    while(1)
    {
        printf("ee468>> "); // Prompt
        fgets(buffer, BUFFER_SIZE, stdin); // Read in command line
        parse_cmds(buffer, commands, COMMAND_SIZE, CMDS_SIZE, &ncmds);

        //printf("commands: %d\n", ncmds);

        if (ncmds == 0) continue;

        for (int i=0; i < ncmds; i++)
        {
            Command* command = &commands[i];
            parse_args(command->cmd, command->args, ARGS_SIZE, &(command->nargs));
        }

        if (commands[0].args[0]==NULL) continue;

        if (!strcmp(commands[0].args[0], "exit")) exit(0);

        char data[PIPE_SIZE];
        pid_t pid;
        int *ret_status;

        for (int i=0; i<ncmds; i++)
        {
            Command* command = &commands[i];

            if (pipe(command->in) < 0) error("pipe in failed\n");
            if (pipe(command->out) < 0) error("pipe out failed\n");

            if ((pid=fork())==-1) error("fork failed\n");

            if (pid==0)
            {
                //printf("Child process %d start\n",i);

                // close stdin/out/err so we can hook them up to pipes
                //close(STDIN); close(STDOUT); close(STDERR);

                // hook up pipes to stdin/out/err (dup2 auto-closes according to linux man)
                dup2(command->in[0],STDIN);
                dup2(command->out[1],STDOUT);
                dup2(command->out[1],STDERR);

                // close parent's usable end of pipes so they can use it
                close(command->in[1]); close(command->out[0]);

                if (execvp(command->args[0],command->args)<0)
                {
                    error("Couldn't execute child process");
                }
            }
            else
            {
                // close child's usable end of pipes so they can use it
                close(command->in[0]);
                close(command->out[1]);

                //printf("Parent process waiting\n");

                //pid = wait(ret_status);

                //printf("Child process %d says hello\n", i);

                if (i > 0) // if there's data to write
                {
                    write(command->in[1], data, strlen(data));
                    //printf("Wrote data to child %d:\n %s\n", i, data);
                }
                // close child's input (sends EOF)
                close(command->in[1]);

                //pid = wait (ret_status);

                //printf("Child process %d finished\n", i);

                // read data from child
                int n = read(command->out[0], data, PIPE_SIZE-1);
                data[n] = '\0';

                //printf("Read data from child %d:\n %s\n", i, data);
            }
        }
        // and now we're all done
        printf("%s",data);
    }
    return 0;
    /*char *args[ARGS_SIZE]; //points to positions in buffer
    int *ret_status;
    size_t nargs;
    pid_t pid;
    while(1)
    {
        printf("ee468>> "); // Prompt
        fgets(buffer, BUFFER_SIZE, stdin); // Read in command line
        //parse_cmds
        parse_args(buffer, args, ARGS_SIZE, &nargs); // parse the command
        if (nargs==0) continue; // Nothing entered so prompt again
        if (!strcmp(args[0], "exit" )) exit(0);
        pid = fork();
        if (pid)
        {  //The parent
                #ifdef DEBUG
                printf("Waiting for child (%d)\n", pid);
                #endif
            pid = wait(ret_status);
                #ifdef DEBUG
                printf("Child (%d) finished\n", pid);
                #endif
        }
        else
        {  //The child executing the command
            if( execvp(args[0], args))
            {
                puts(strerror(errno));
                exit(127);
            }
        }
    }
    return 0;
    */
}



void parse_args(char *buffer, char** args, size_t args_size, size_t *nargs)
{
    //printf("Parsing args of \"%s\": ", buffer);
    char *buf_args[args_size];
    char **cp;  // This is used as a pointer into the string array
    char *wbuf = buffer;  // String variable that has the command line

    buf_args[0]=buffer;
    args[0]=buffer;
    /*
     * Now 'wbuf' is parsed into the string array 'buf_args'
     *
     * The for-loop uses a string.h function
     *   char *strsep(char **stringp, const char *delim);
     *
     *   Description:
     *   If *stringp = NULL then it returns NULL and does
     *   nothing else.  Otherwise the function finds the first token in
     *   the string *stringp, where tokens are delimited by symbols
     *   in the string 'delim'.
     *
     *   In the example below, **stringp is &wbu, and
     *   the delim = ' ', '\n', and '\t'.  So there are three possible
     *   delimiters.
     *
     *   So in the string " Aloha World\n", the spaces and "\n" are
     *   delimiters.  Thus, there are three delimiters.  The tokens
     *   are what's between the delimiters.  So the first token is
     *   "", which is nothing because a space is the first delimiter.
     *   The second token is "Aloha", and the third token is "World".
     *
     *   The function will scan a character string starting from
     *   *stringp, search for the first delimiter.  It replaces
     *   the delimiter with '\0', and *stringp is updated to point
     *   past the token.  In case no delimiter was found, the
     *   token is taken to be the entire string *stringp, and *stringp
     *   is made NULL.   Strsep returns a pointer to the token.
     *
     *   Example:  Suppose *stringp -> " Aloha World\n"
     *
     *   The first time strsep is called, the string is "\0Aloha World\n",
     *   and the pointer value returned = 0.  Note the token is nothing.
     *
     *   The second time it is called, the string is "\0Aloha\0World\n",
     *   and the pointer value returned = 1  Note that 'Aloha' is a token.
     *
     *   The third time it is called, the string is '\0Aloha\0World\0',
     *   and the pointer value returned is 7.  Note that 'World' is a token.
     *
     *   The fourth time it is called, it returns NULL.
     *
     *   The for-loop, goes through buffer starting at the beginning.
     *   wbuf is updated to point to the next token, and cp is
     *   updated to point to the current token, which terminated by '\0'.
     *   Note that pointers to tokens are stored in array buf_args through cp.
     *   The loop stops if there are no more tokens or exceeded the
     *   array buf_args.
     */
    /* cp is a pointer to buff_args */
    for(cp=buf_args; (*cp=strsep(&wbuf, " \n\t")) != NULL ;)
    {
        //printf("\"%s\" ",*cp);
        if ((*cp != '\0') && (++cp >= &buf_args[args_size]))
            break;
    }

    /*
     * Copy 'buf_args' into 'args'
     */
    size_t i, j;
    for (j=i=0; buf_args[i]!=NULL; i++)
    {
        if(strlen(buf_args[i])>0)  /* Store only non-empty tokens */
        {
            //printf("\"%s\" ",buf_args[i]);
            args[j++]=buf_args[i];
        }
    }

    *nargs=j;

    // must be finished with a null ptr
    args[j]=NULL;
    //printf("\n");
}

void error(char *s)
{
    perror(s);
    exit(1);
}