/*

    Name: Zachery Gentry, Garry Geaslin
    ID:   1001144385, 1001299734

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
#include <stdint.h>
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

char BS_OEMName[8];
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];

FILE *fp;

void getInput();
void execute();
void openImage(char file[]);
void closeImage();

int main()
{

    //////////////////
    // PROGRAM LOOP //
    //////////////////

    while (1)
    {
        getInput();
        execute();
    }
    return 0;
}

int16_t NextLB(uint32_t sector)
{
    uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (sector * 4);
    int16_t val;
    fseek(fp, FATAddress, SEEK_SET);
    fread(&val, 2, 1, fp);
    return val;
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

void execute()
{
    // If the user just hits enter, do nothing
    if (token[0] == NULL)
    {
        return;
    }

    if (strcmp(token[0], "open") == 0)
    {
        if (token[1] != NULL)
        {
            openImage(token[1]);
        }
        else
        {
            printf("ERR: Must give argument of file to open\n");
        }
    }
    else if (strcmp(token[0], "info") == 0)
    {
        printf("BPB_BytesPerSec: %d\n", BPB_BytesPerSec);
        printf("BPB_SecPerClus: %d\n", BPB_SecPerClus);
        printf("BPB_RsvdSecCnt: %d\n", BPB_RsvdSecCnt);
        printf("BPB_NumFATs: %d\n", BPB_NumFATs);
        printf("BPB_FATSz32: %d\n", BPB_FATSz32);
    }
    else if (strcmp(token[0], "close") == 0)
    {
        closeImage();
    }
}

void openImage(char file[])
{
    fp = fopen(file, "r");
    printf("%s opened.\n", file);

    fseek(fp, 3, SEEK_SET);
    fread(&BS_OEMName, 8, 1, fp);

    fseek(fp, 11, SEEK_SET);
    fread(&BPB_BytesPerSec, 2, 1, fp);
    fread(&BPB_SecPerClus, 1, 1, fp);
    fread(&BPB_RsvdSecCnt, 2, 1, fp);
    fread(&BPB_NumFATs, 1, 1, fp);
    fread(&BPB_RootEntCnt, 2, 1, fp);

    fseek(fp, 36, SEEK_SET);
    fread(&BPB_FATSz32, 4, 1, fp);

    fseek(fp, 44, SEEK_SET);
    fread(&BPB_RootClus, 4, 1, fp);


    int i;
    for (i = 0; i < 16; i++)
    {
        fread(&dir[i], 32, 1, fp);
        printf("%s\n", dir[i].DIR_Name);
        printf("%d\n", dir[i].DIR_FileSize);
    }
}

void closeImage()
{
    if (fp == NULL)
    {
        printf("Error: File system not open.");
        return;
    }

    fclose(fp);
    fp = NULL;
}