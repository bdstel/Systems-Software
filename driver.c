// Braden Steller
// Jacob O'Quinn

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

int main(int argc, char **argv)
{
    int lflag = 0, aflag = 0, vflag = 0; // xflag == 1 => print x
    FILE *ifp;
    char *inputfile;
    char c;
    int i;
    
    if (argc < 2) // filename declared as second arg
    {
        printf("Error : please include the file name");
        return 0;
    }
    for (int i = 2; i < argc; i++)
    {
        if (!strcmp(argv[i], "-l"))
            lflag = 1;
        if (!strcmp(argv[i], "-a"))
            aflag = 1;
        if (!strcmp(argv[i], "-v"))
            vflag = 1;
    }
    
    // read in file to a string
    ifp = fopen(argv[1], "r");
    inputfile = malloc(500 * sizeof(char));
    c = fgetc(ifp);
    i = 0;
    while (1)
    {
        inputfile[i++] = c;
        c = fgetc(ifp);
        if (c == EOF)
            break;
    }
    inputfile[i] = '\0';

    lexeme * list = lex_analyze(inputfile, lflag);
    instruction * code = parse(list, aflag);
    execute(code, vflag);

    return 0;
}