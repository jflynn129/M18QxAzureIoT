/**
* copyright (c) 2018, James Flynn
* SPDX-License-Identifier: MIT
*/

/**
*   @file   prettyjson.cpp
*   @brief  this function simply prints the contents of a JSON object, indenting after '{'/'[' outdenting after '}'/']'
*           and addign CRLF after the commas.  The purpose is to make reading the objects easier and nothing more. It is
*           only used when printing messages that are sent to Azure...
*
*   @author James Flynn
*
*   @date   1-Oct-2018
*/

#include <stdio.h>

void prty_json(char* src, int srclen)
{
    int indent = 0;
    for (int i = 0; i<srclen; ++i) {
        switch(src[i]) {
            case '{':
            case '[':
                indent += 2;
                printf("%c\n",src[i]);
                for( int k=0; k<indent; k++ )
                    printf(" ");
                break;
                
            case '}':
            case ']':
                printf("\n");
                if( indent >= 2 )
                    indent -= 2;
                for( int k=0; k<indent; k++ )
                    printf(" ");
                printf("%c",src[i]);
                if( src[i] == '}' )
                    printf("\n");
                break;
                
            case ',':
                printf(",\n");
                for( int k=0; k<indent; k++ )
                    printf(" ");
                break;

            default:
                printf("%c",src[i]);
                break;
            }
        }
}
