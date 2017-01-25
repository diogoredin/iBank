#ifndef COMMANDLINEREADER_H
#define COMMANDLINEREADER_H

/****************************************************************************************
*
*	PROGRAMA I-BANCO - PROJETO SO DEI/IST/ULisboa 2016-17
*	Command Line Reader Header
*
*****************************************************************************************/

/*
Reads up to 'vectorSize' space-separated arguments from the standard input
and saves them in the entries of the 'argVector' argument.
This function returns once enough arguments are read or the end of the line 
is reached

Arguments:
 'argVector' should be a vector of char* previously allocated with
 as many entries as 'vectorSize'
 'vectorSize' is the size of the above vector. A vector of size N allows up to 
 N-1 arguments to be read; the entry after the last argument is set to NULL.
 'buffer' is a buffer with 'buffersize' bytes, which will be 
 used to hold the strings of each argument.  

Return value:
 The number of arguments that were read, or -1 if some error occurred.
*/

/****************************************************************************************
*
*   1. Bibliotecas
*
*****************************************************************************************/

#include <string.h>
#include <stdio.h>

/****************************************************************************************
*
*   2. Prototipos
*
*****************************************************************************************/

int readLineArguments(char **argVector, int vectorSize, char *buffer, int buffersize);

#endif