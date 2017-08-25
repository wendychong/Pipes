Project 1

Included in this directory is a very simple shell program
for Linux named 'sshell.c'.  It currently cannot do
indirection, e.g., '<', '>' and '|'.

Rewrite the program so that it can do pipes ('|').  Note
that the program must be able to parse the input line,
identify the pipe symbols ('|'), and then launch each
command as a process.  Also the processes must be connected
together with pipes.  These pipes become replacements for
stdin and stdout.  pipe.c is an example of implementing this
using dup2.  (Recall that this example came frm EE 367L.)

You may assume that the maximum number of pipe symbols ('|')
in a command line is 9.

 Note that the in[] and out[] arrays may have 
to organized into structs since you may need 9 of them.
Your shell should not create zombie processes if it properly
exits.

Hint:  To test your shell, you can use the 'more' command
which will output a file to stdout.  So in a command line
with multiple pipes, 'more' can be the first command to
create a stream of bytes.

