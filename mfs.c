/*

    Name: Zachery Gentry
    ID:   1001144385

*/

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017, 2018 Trevor Bakker, Zachery Gentry
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define WHITESPACE " \t\n" // We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.   \
                           // In this case  white space                        \
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports five arguments


// token and cmd_str used for tokenizing user input
char *token[MAX_NUM_ARGUMENTS];
char cmd_str[MAX_COMMAND_SIZE];



void getInput();

int main()
{

    //////////////////
    // PROGRAM LOOP //
    //////////////////

    while (1)
    {
        getInput();
    }
    return 0;
}


void getInput()
{
    printf("mfs> ");

    memset(cmd_str, '\0', MAX_COMMAND_SIZE);
    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
        ;
    /* Parse input */

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    memset(&token, '\0', MAX_NUM_ARGUMENTS);

    // Tokenize the input strings with whitespace used as the delimiter
    memset(&token, '\0', sizeof(MAX_NUM_ARGUMENTS));
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
        token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
        if (strlen(token[token_count]) == 0)
        {
            token[token_count] = NULL;
        }
        token_count++;
    }

    free(working_root);
}