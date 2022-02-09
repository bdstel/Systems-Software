// Braden Steller
// Jacob O'Quinn

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include "compiler.h"

// Error codes
#define INVALIDSYMBOL "INVALIDSYMBOL"
#define EXCEED11CHARS "EXCEED11CHARS"
#define NUMLENERROR "NUMLENERROR"
#define IDENTERROR "IDENTERROR"
#define ENDOFLEXEME "ENDOFLEXEME"

const static struct {
    token_type val;
    const char *str;
} conversion [] = {
    {modsym, "%"}, {identsym, "identsym"}, {numbersym, "numbersym"}, {plussym, "+"},
    {minussym, "-"}, {multsym, "*"}, {slashsym, "/"}, {oddsym, "odd"},
    {eqlsym, "="}, {neqsym, "<>"}, {lessym, "<"}, {leqsym, "<="},
    {gtrsym, ">"}, {geqsym, ">="}, {lparentsym, "("}, {rparentsym, ")"},
    {commasym, ","}, {semicolonsym, ";"}, {periodsym, "."}, {becomessym, ":="},
    {beginsym, "begin"}, {endsym, "end"}, {ifsym, "if"}, {thensym, "then"},
    {whilesym, "while"}, {dosym, "do"}, {callsym, "call"}, {constsym, "const"}, 
    {varsym, "var"}, {procsym, "procedure"}, {writesym, "write"}, {readsym, "read"}, 
    {elsesym, "else"}, {returnsym, "return"}
};

token_type str2enum (const char *str)
{
    for (int j = 0;  j < sizeof (conversion) / sizeof (conversion[0]); j++)
        if (!strcmp (str, conversion[j].str))
            return conversion[j].val;
    return -1;
}

lexeme * tokenList;

// Convert chars to words
char ** charToLexemeArray(char * input);
// Print tokens & add to tokenList
void printLexemeTable(char** lexemeList, int print);

lexeme* lex_analyze(char *inputfile, int print)
{
    char ** lexemeList = charToLexemeArray(inputfile);
    printLexemeTable(lexemeList, print);

    free(lexemeList);

    return tokenList;
}

char ** charToLexemeArray(char * input)
{
    int lexemeListSize = 500;
    int maxIdentifierLength = 50;
    char** lexemeList = calloc(lexemeListSize, sizeof(char*));
    for(int i = 0; i < lexemeListSize; i++){
        lexemeList[i] = calloc(maxIdentifierLength, sizeof(char));
        if(!lexemeList[i])
        {
            printf("LexemeList alloc error");
        }
    }   
    
    int cPos;
    int i;
    for (i = 0, cPos = 0; input[cPos] != '\0'; i++)
    {
        if(i == lexemeListSize - 2)
        {
            lexemeListSize *= 2;
            if(!realloc(lexemeList, lexemeListSize * sizeof(char*)))
            {
                printf("lexemeListSize = %d, i = %d, LexemeList Reallocation Failed\n", lexemeListSize, i);
            }
        }
        int temp;
        
        switch (input[cPos])
        {
            case '%': case '+': case '-': case '*': case '=': // add to lexemeList & increment
            case '(': case ')': case ',': case ';': case '.':
                strncat(lexemeList[i], &input[cPos], 1);
                cPos++;
                break;
            case '/':
                if (input[cPos+1] == '*') // ignore comments, increment
                {
                    cPos++;
                    int j = cPos;
                    while(!(input[j] == '*' && input[j+1] == '/'))
                    {
                        j++;
                    }
                    j += 2;
                    cPos = j;
                    i--;
                }
                else // add to lexemeList & increment
                {
                    strncat(lexemeList[i], &input[cPos], 1);
                    cPos++;
                }
                break;
            case '<': // add to lexemeList & increment
                strncat(lexemeList[i], &input[cPos], 1); 
                cPos++;
                if (input[cPos] == '>' || input[cPos] == '=') // also add <> or <=
                {
                    strncat(lexemeList[i], &input[cPos], 1);
                    cPos++;
                }
                break;
            case '>': // add to list & increment
                strncat(lexemeList[i], &input[cPos], 1);
                cPos++;
                if (input[cPos+1] == '=') // also add >=
                {
                    strncat(lexemeList[i], &input[cPos], 1);
                    cPos++;
                }
                break;
            case ':':
                strncat(lexemeList[i], &input[cPos], 1);
                cPos++;
                if (input[cPos] == '=') // only add if its :=
                {
                    strncat(lexemeList[i], &input[cPos], 1);
                    cPos++;
                }
                else // otherwise its invalid
                {
                    strcpy(lexemeList[i], INVALIDSYMBOL);
                }
                break;
            default: // whitespace, numbers, or identifiers
                if (input[cPos] == ' ' || iscntrl(input[cPos])) // ignore whitespace
                {
                    i--;
                    cPos++;
                    break;
                }
                else if(isdigit(input[cPos])) // check for numbers & invalid identifiers
                {
                    int j;
                    // if followed by letters w/ no whitespace, invalid identifier
                    for(j = cPos; isdigit(input[j]); j++){}
                    if(isalpha(input[j]))
                    {
                        int k = j;
                        for(;isdigit(input[j]) || isalpha(input[j]); j++){}
                        if(k - cPos > 5)
                        {
                            strcpy(lexemeList[i], NUMLENERROR);
                        }
                        else
                        {
                            strcpy(lexemeList[i], IDENTERROR);
                        }
                    }
                    // if followed by only numbers w/ no whitespace, it is a number
                    else
                    {
                        strncat(lexemeList[i], &input[cPos], j - cPos);
                        if(j - cPos > 5)
                        {
                            strcpy(lexemeList[i], NUMLENERROR);
                        }
                    }
                    cPos = j;
                    
                }
                else if(isalpha(input[cPos]))
                {
                    int j;
                    // if starts with letters or has only letters, store into list
                    // other functions determine difference between identifier & reserved word
                    for(j = cPos; isdigit(input[j]) || isalpha(input[j]); j++){}
                    if(j - cPos > 11)
                    {
                        strcpy(lexemeList[i], EXCEED11CHARS);
                    }
                    else
                    {
                        strncat(lexemeList[i], &input[cPos], j - cPos);
                    }
                    cPos = j;
                }
                else // store any other symbol into list, will be marked as invalid later
                {
                    strncat(lexemeList[i], &input[cPos], 1);
                    cPos++;
                }
        }
    }
    
    // Mark end of lexeme
    strcpy(lexemeList[i], ENDOFLEXEME);

    return lexemeList;
}

// Print out table
void printLexemeTable(char** lexemeList, int print)
{
    tokenList = calloc(500, sizeof(lexeme));

    if (print)
        printf("Lexeme Table:\n%-33stoken type\n", "lexeme");
    
    int i;
    for (i = 0; strcmp(lexemeList[i], ENDOFLEXEME); i++)
    {
        char * curLexeme = lexemeList[i];

        if (!strcmp(curLexeme, IDENTERROR))
        {
            printf("Error : Identifiers cannot begin with a digit\n");
            exit(0);
        }
        else if (!strcmp(curLexeme, NUMLENERROR))
        {
            printf("Error : Numbers cannot exceed 5 digits\n");
            exit(0);
        }
        else if (!strcmp(curLexeme, EXCEED11CHARS))
        {
            printf("Error : Identifier names cannot exceed 11 characters\n");
            exit(0);
        }
        else if (!strcmp(curLexeme, INVALIDSYMBOL) || 
                ((int)str2enum(curLexeme) == -1 
                && !isdigit(curLexeme[0]) && !isalpha(curLexeme[0])))
        {
            printf("Error : Invalid Symbol\n");
            exit(0);
        }
        else
        {
            // if -1, can only be ident or num, since invalid symbols are dealt with
            if ((int)str2enum(curLexeme) == -1)
            {
                if (isdigit(curLexeme[0]))
                {
                    tokenList[i].type = (int)str2enum("numbersym");
                    tokenList[i].value = atoi(curLexeme);
                }
                else if (isalpha(curLexeme[0]))
                {
                    tokenList[i].type = (int)str2enum("identsym");
                    tokenList[i].name = malloc(12 * sizeof(char)); // allocate memory for identifier name 
                    strcpy(tokenList[i].name, curLexeme);
                }
            }
            else // otherwise simply store the type
            {   
                tokenList[i].type = (int)str2enum(curLexeme);
            }

            if (print)
                printf("%-33s%d\n", curLexeme, tokenList[i].type);
        }
    }

    if (print)
    {
        printf("\nToken List:\n");
        int tokenLen = i;
        for (int j = 0; j < tokenLen; j++)
        {
            printf("%d ", tokenList[j].type);

            if (tokenList[j].type == 2) // ident
                printf("%s ", tokenList[j].name);
            else if (tokenList[j].type == 3) // number
                printf("%d ", tokenList[j].value);
        }
        printf("\n\n\n");
    }
}