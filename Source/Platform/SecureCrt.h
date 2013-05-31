/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SECURECRT_H
#define SECURECRT_H

#include "PlatformDefines.h"
#ifndef EMUNISCE_COMPILER_MSVC

#ifndef nullptr
#define nullptr 0
#endif

//todo: the files that include SecureCrt.h should also be responsible for including the necessary dependencies.
//      in other words, these includes should not be here.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


inline int freopen_s(FILE** outFile, const char* path, const char* mode, FILE* stream)
{
    //todo

    FILE* result = freopen(path, mode, stream);

    if(outFile != nullptr)
        *outFile = result;

    return 0;
}

inline int strcat_s(char* destination, unsigned int sizeBytes, const char* source)
{
    //todo
    strcat(destination, source);
    return 0;
}

inline int strcpy_s(char* destination, unsigned int sizeBytes, const char* source)
{
    if(destination == nullptr)
    {
        return -1;
    }

    if(source == nullptr)
    {
        destination[0] = 0;
        return -1;
    }

    if(strlen(source)+1 > sizeBytes)
    {
        destination[0] = 0;
        return -2;
    }

    strcpy(destination, source);
    return 0;
}

//todo
#define sprintf_s(buffer, bufferSize, formatString, ...) sprintf(buffer, formatString, __VA_ARGS__)

inline char* strtok_s(char* token, const char* delimiters, char** context)
{
    //todo

    return strtok(token, delimiters);
}


#endif // EMUNISCE_COMPILER_MSVC

#endif // SECURECRT_H
