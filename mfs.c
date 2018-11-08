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
#include <ctype.h>

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

int32_t currentDirectory;
char formattedDirectory[12];

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
void printDirectory();
void changeDirectory(int32_t sector);
void getDirectoryInfo();
int32_t getCluster(char *dirname);
void formatDirectory(char *dirname);
void get();

int main()
{

    //////////////////
    // PROGRAM LOOP //
    //////////////////
    formatDirectory("foo.txt");
    //FOO     TXT

    printf("%s\n", formattedDirectory);
    while (1)
    {
        getInput();
        execute();
    }
    return 0;
}

int LBAToOffset(int32_t sector)
{
    if (sector == 0)
        sector = 2;
    return ((sector - 2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
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
        if (fp != NULL)
        {
            printf("Error: File system image already open.\n");
            return;
        }

        if (token[1] != NULL && fp == NULL)
        {
            openImage(token[1]);
        }
        else if (token[1] == NULL)
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
    else if (strcmp(token[0], "ls") == 0)
    {
        printDirectory();
    }
    else if (strcmp(token[0], "cd") == 0)
    {
        if (token[1] == NULL)
        {
            printf("ERR: Please provide which directory you would like to open\n");
            return;
        }
        changeDirectory(getCluster(token[1]));
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
    currentDirectory = BPB_RootClus;

    // -- hard code get num.txt ( cluster 17, size 8 ) --
    FILE *newfp = fopen("NUM.txt", "w");
    fseek(fp, LBAToOffset(17), SEEK_SET);
    unsigned char *ptr = malloc(8);
    fread(ptr, 8, 1, fp);
    fwrite(ptr, 8, 1, newfp);
    fclose(newfp);
}

void get(char *dirname)
{
    // -- hard code get num.txt ( cluster 17, size 8 ) --
    FILE *newfp = fopen("NUM.txt", "w");
    fseek(fp, LBAToOffset(17), SEEK_SET);
    unsigned char *ptr = malloc(8);
    fread(ptr, 8, 1, fp);
    fwrite(ptr, 8, 1, newfp);
    fclose(newfp);
}

void formatDirectory(char *dirname)
{
    char IMG_Name[11] = "FOO     TXT";

    char input[7] = "foo.txt";

    char expanded_name[12];
    memset(expanded_name, ' ', 12);

    printf("Wassup\n");
    char *token = strtok(dirname, ".");

    strncpy(expanded_name, token, strlen(token));

    token = strtok(NULL, ".");

    if (token)
    {
        strncpy((char *)(expanded_name + 8), token, strlen(token));
    }

    expanded_name[11] = '\0';

    int i;
    for (i = 0; i < 11; i++)
    {
        expanded_name[i] = toupper(expanded_name[i]);
    }

    strncpy(formattedDirectory, expanded_name, 12);
    //formattedDirectory = expanded_name;
}

int32_t getCluster(char *dirname)
{
    //Compare dirname to directory name (attribute), if same, cd into FirstClusterLow
    return -1;
}

void changeDirectory(int32_t cluster)
{
    int offset = LBAToOffset(cluster);
    currentDirectory = cluster;
    fseek(fp, offset, SEEK_SET);
}

void getDirectoryInfo()
{
    int i;
    for (i = 0; i < 16; i++)
    {
        fread(&dir[i], 32, 1, fp);
    }
}

void printDirectory()
{
    int offset = LBAToOffset(currentDirectory);
    fseek(fp, offset, SEEK_SET);

    int i;
    for (i = 0; i < 16; i++)
    {
        fread(&dir[i], 32, 1, fp);

        if ((dir[i].DIR_Name[0] != (char)0xe5) &&
            (dir[i].DIR_Attr == 0x1 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20))
        {
            char *directory = malloc(11);
            memset(directory, '\0', 11);
            memcpy(directory, dir[i].DIR_Name, 11);
            printf("Directory Name: %s\n", directory);
            printf("Filesize: %d\n", dir[i].DIR_FileSize);
            printf("Cluster: %d\n", dir[i].DIR_FirstClusterLow);
        }
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
