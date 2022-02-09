// Braden Steller
// Jacob O'Quinn

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include <math.h>

#define MAX_SYMBOL_TABLE_SIZE 500

instruction * code;
int codeIdx = 0;
char *errormsg[] = 
{
    "",
    "Error : program must end with period\n",                                                   // 1
    "Error : const, var, procedure, call, and read keywords must be followed by identifier\n",  // 2
    "Error : competing symbol declarations at the same level\n",                                // 3
    "Error : constants must be assigned with =\n",                                              // 4
    "Error : constants must be assigned an integer value\n",                                    // 5
    "Error : symbol declarations must be followed by a semicolon\n",                            // 6
    "Error : undeclared variable or constant in equation\n",                                    // 7
    "Error : only variable values may be altered\n",                                            // 8
    "Error : assignment statements must use :=\n",                                              // 9
    "Error : begin must be followed by end\n",                                                  // 10
    "Error : if must be followed by then\n",                                                    // 11
    "Error : while must be followed by do\n",                                                   // 12
    "Error : condition must contain comparison operator\n",                                     // 13
    "Error : right parenthesis must follow left parenthesis\n",                                 // 14
    "Error : arithmetic equations must contain operands, parentheses, numbers, or symbols\n",   // 15
    "Error : undeclared procedure for call\n",                                                  // 16
    "Error : parameters may only be specified by an identifier\n",                              // 17
    "Error : parameters must be declared\n",                                                    // 18
    "Error : cannot return from main\n",                                                        // 19
    "Error : cannot find procedure\n",                                                          // 20
    "Error : statement must be end with a semicolon\n"                                          // 21
};

lexeme token;
lexeme * tList;
int tokenIdx = 0;

symbol symTable[MAX_SYMBOL_TABLE_SIZE];
int tableIdx = 0;

int procedureCount = 0;

void getNextToken();
void saveToTable(int kind, char name[], int val, int level, int addr, int mark, int param);
void emit(int opcode, char op[], int l, int m);
int symbolTableCheck (char string[], int lexLevel);
int symbolTableSearch (char string[], int lexLevel, int kind);
int findProcedure (int index);
void mark (int count);
void program();
void block(int lexLevel, int param, int prodIdx);
int constDeclaration(int lexLevel);
int varDeclaration(int lexLevel, int param);
int procDeclaration(int lexLevel);
void statement(int lexLevel);
void condition(int lexLevel);
void expression(int lexLevel);
void term(int lexLevel);
void factor(int lexLevel);

instruction * parse(lexeme * tokenList, int print)
{
    code = malloc(500 * sizeof(instruction));

    tList = tokenList;
    token = tList[tokenIdx];

    program();

    if (print)
    {
        printf("Generated Assembly:\nLine%6s%5s%5s\n", "OP", "L", "M");
        for (int i = 0; i < codeIdx; i++)
        {
            printf("%3d%7s%5d%5d\n", i, code[i].op, code[i].l, code[i].m);
        }
        printf("\n\n");
    }

    return code;
}

void getNextToken()
{
    token = tList[++tokenIdx];
}

void saveToTable(int kind, char name[], int val, int level, int addr, int mark, int param)
{
    symTable[tableIdx].kind = kind;
    strcpy(symTable[tableIdx].name, name);
    symTable[tableIdx].val = val;
    symTable[tableIdx].level = level;
    symTable[tableIdx].addr = addr;
    symTable[tableIdx].mark = mark;
    symTable[tableIdx].param = param;
    tableIdx++;
}

void emit(int opcode, char op[], int l, int m)
{
    code[codeIdx].opcode = opcode;
    strcpy(code[codeIdx].op, op);
    code[codeIdx].l = l;
    code[codeIdx].m = m;
    codeIdx++;
}

int symbolTableCheck (char string[], int lexLevel)
{
    // linear search looking at name and level (unmarked)
    for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
    {
        if (!strcmp(symTable[i].name, string) 
            && symTable[i].level == lexLevel
            && symTable[i].mark == 0)
            return i;
    }

    return -1;
}

int symbolTableSearch (char string[], int lexLevel, int kind)
{
    int bestIndex = -1;

    // linear search looking at name and kind (unmarked)
    for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
    {
        if (!strcmp(symTable[i].name, string) 
            && symTable[i].kind == kind
            && symTable[i].mark == 0
            && symTable[i].level <= lexLevel)
            // finding the index of the symbol with the closest lexLevel
            bestIndex = (symTable[i].level > symTable[bestIndex].level) ? i : bestIndex;
    }
    
    return bestIndex;
}


int findProcedure (int index) 
{
    // linear search looking at kind 3 (procedure) and value attribute
    for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
    {
        if (symTable[i].kind == 3
            && symTable[i].val == index)
            return i;
    }

    return -1;
}

void mark (int count)
{
    // starting from the end of the symbol table and looping backward
    // if entry is unmarked, mark it, count--
    // else continue
    for(int i = tableIdx; i >= 0; i--)
    {
        if (symTable[i].mark == 0)
        {
            symTable[i].mark == 1;
            count--;
        }
    }
}  

void program ()
{
    int numProc = 1;

    emit(7, "JMP", 0, -1); 

    for (int i = 0; tList[i].type != 0; i++)
    {
        if (tList[i].type == procsym)
        {
            numProc++;
            emit(7, "JMP", 0, -1); 
        }
    }
    
    saveToTable(3, "main", 0, 0, 0, 0, 0);
    procedureCount++;
    block(0, 0, 0);

    if (token.type != periodsym)
    {
        printf("%s", errormsg[1]);
        exit(0);
    }
    for (int i = 0; i < numProc; i++)
        code[i].m = symTable[findProcedure(i)].addr;
    for (int i = 0; i < codeIdx; i++)
    {
        if(code[i].opcode == 5) // call
            code[i].m = symTable[findProcedure(code[i].m)].addr;
    }

    emit(9, "SYS", 0, 3); // HALT
}

void block(int lexLevel, int param, int procIdx)
{
    int c = constDeclaration(lexLevel);
    int v = varDeclaration(lexLevel, param);
    int p = procDeclaration(lexLevel);

    symTable[procIdx].addr = codeIdx;
    emit(6, "INC", 0, 4 + v); 

    statement(lexLevel);

    mark(c + v + p);
}

int constDeclaration (int lexLevel)
{
    int numConst = 0;
    if(token.type == constsym)
    {
        do 
        {
            numConst++;
            getNextToken();
            if(token.type != identsym)
            {
                printf("%s", errormsg[2]);
                exit(0);
            }
            if(symbolTableCheck(token.name, lexLevel) != -1)
            {
                printf("%s", errormsg[3]);
                exit(0);
            }

            char constName[12];
            strcpy(constName, token.name);
            getNextToken();
            if(token.type != eqlsym)
            {
                printf("%s", errormsg[4]);
                exit(0);
            }

            getNextToken();
            if (token.type != numbersym)
            {
                printf("%s", errormsg[4]);
                exit(0);
            }
            saveToTable(1, constName, token.value, lexLevel, 0, 0, 0);
            getNextToken();
        }
        while(token.type == commasym);

        if(token.type != semicolonsym)
        {
            printf("%s", errormsg[6]);
            exit(0);
        }
        getNextToken();
    }
    
    return numConst;
}

int varDeclaration (int lexLevel, int param)
{
    int numVars = param;
    
    if (token.type == varsym)
    {
        do
        {
            numVars++;
            getNextToken();
            if (token.type != identsym)
            {
                printf("%s", errormsg[2]);
                exit(0);
            }
            if (symbolTableCheck(token.name, lexLevel) != -1)
            {
                printf("%s", errormsg[3]);
                exit(0);
            }
            saveToTable(2, token.name, 0, lexLevel, numVars + 3, 0, 0);
            getNextToken();
        }
        while(token.type == commasym);
        
        if(token.type != semicolonsym)
        {
            printf("%s", errormsg[6]);
            exit(0);
        }
        getNextToken();
    }

    return numVars;
}

int procDeclaration (int lexLevel)
{
    int numProc = 0; 
    if(token.type == procsym)
    {
        do
        {
            numProc++;
            getNextToken();
            
            if(token.type != identsym)
            {
                printf("%s", errormsg[2]);
                exit(0);
            }
            if (symbolTableCheck(token.name, lexLevel) != -1)
            {
                printf("%s", errormsg[3]);
                exit(0);
            }

            // Add procedure to symbol table    
            int procIdx = tableIdx;
            saveToTable(3, token.name, procedureCount, lexLevel, 0, 0, 0); 
            procedureCount++;
            
            getNextToken();
            if (token.type == lparentsym)
            {
                getNextToken();
                if (token.type != identsym)
                {
                    printf("%s", errormsg[17]);
                    exit(0);
                }
                saveToTable(2, token.name, 0, lexLevel + 1, 3, 0, 0); 
                symTable[tableIdx - 2].param = 1; // procedure has parameter
                getNextToken();
                if(token.type != rparentsym)
                {
                    printf("%s", errormsg[14]);
                    exit(0);
                }
                getNextToken();
                if (token.type != semicolonsym)
                {
                    printf("%s", errormsg[6]);
                    exit(0);
                }
                getNextToken();
                block(lexLevel + 1, 1, procIdx);
            }
            else
            {
                if (token.type != semicolonsym)
                {
                    printf("%s", errormsg[6]);
                    exit(0);
                }
                getNextToken();
                block(lexLevel + 1, 0, procIdx);
            }

            if (code[codeIdx - 1].opcode != 2 && code[codeIdx - 1].m != 0)
            {
                emit(1, "LIT", 0, 0);
                emit(2, "RTN", 0, 0);
            }
            if(token.type != semicolonsym)
            {
                printf("%s", errormsg[6]);
                exit(0);
            }
            
            getNextToken();
        } 
        while (token.type == procsym);
    }

    return numProc;
}

void statement(int lexLevel)
{
    if(token.type == identsym)
    {
        int symIdx = symbolTableSearch(token.name, lexLevel, 2); // var
        if(symIdx == -1)
        {
            printf("%s", errormsg[7]);
            exit(0);
        }
        if(symTable[symIdx].kind != 2)
        {
            printf("%s", errormsg[8]);
            exit(0);
        }
            
        getNextToken();
        if(token.type != becomessym)
        {
            printf("%s", errormsg[9]);
            exit(0);
        }

        getNextToken();
        expression(lexLevel);
        emit(4, "STO", lexLevel - symTable[symIdx].level, symTable[symIdx].addr);

        return;
    }
    if(token.type == callsym)
    {
        getNextToken();
        if(token.type != identsym)
        {
            printf("%s", errormsg[2]);
            exit(0);
        }
        int symIdx = symbolTableSearch(token.name, lexLevel, 3); // procedure
        if(symIdx == -1)
        {
            printf("%s", errormsg[7]);
            exit(0);
        }

        getNextToken();
        if (token.type == lparentsym)
        {
            getNextToken();
            if(symTable[symIdx].param != 1)
            {
                printf("%s", errormsg[18]);
                exit(0);
            }
            expression(lexLevel);
            if (token.type != rparentsym)
            {
                printf("%s", errormsg[14]);
                exit(0);
            }
            getNextToken();
        }
        else
        {
            emit(1, "LIT", 0, 0);
        }
        emit(5, "CAL", lexLevel - symTable[symIdx].level, symTable[symIdx].val);

        return;
    }
    if(token.type == returnsym)
    {
        if(lexLevel == 0)
        {
            printf("%s", errormsg[19]);
            exit(0);
        }

        getNextToken();
        if(token.type == lparentsym)
        {
            getNextToken();
            expression(lexLevel);
            
            emit(2, "RTN", 0, 0);
    
            if(token.type != rparentsym)
            {
                printf("%s", errormsg[14]);
                exit(0);
            }
            getNextToken();
        }
        else
        {
            emit(1, "LIT", 0, 0);
            emit(2, "RTN", 0, 0);
        }
        
        return;
    }
    if(token.type == beginsym)
    {
        do
        {
            getNextToken();
            statement(lexLevel);
        }
        while(token.type == semicolonsym);
        
        if (token.type != endsym)
        {
            printf("%s", errormsg[10]);
            exit(0);
        }

        getNextToken();

        return;
    }
    if(token.type == ifsym)
    {
        getNextToken();
        condition(lexLevel);
        
        int jpcIdx = codeIdx;
        emit(8, "JPC", 0, jpcIdx);
        
        if (token.type != thensym)
        {
            printf("%s", errormsg[11]);
            exit(0);
        }

        getNextToken();
        statement(lexLevel);
        
        if(token.type == elsesym)
        {
            getNextToken();
            int jmpIdx = codeIdx;
            
            emit(7, "JMP", 0, jmpIdx);
            code[jpcIdx].m = codeIdx;
            statement(lexLevel);
            code[jmpIdx].m = codeIdx;
        }
        else
            code[jpcIdx].m = codeIdx;

        return;   
    }
    if(token.type == whilesym)
    {
        getNextToken();
        int loopIdx = codeIdx;
        condition(lexLevel);
        
        if(token.type != dosym)
        {
            printf("%s", errormsg[12]);
            exit(0);
        }
        
        getNextToken();
        int jpcIdx = codeIdx;
        emit(8, "JPC", 0, jpcIdx);
        statement(lexLevel);
        emit(7, "JMP", 0, loopIdx);
        code[jpcIdx].m = codeIdx;
        
        return;
    }
    if (token.type == readsym)
    {
        getNextToken();
        if(token.type != identsym)
        {
            printf("%s", errormsg[2]);
            exit(0);
        }
        int symIdx = symbolTableSearch(token.name, lexLevel, 2); // var
        if(symIdx == -1)
        {
            printf("%s", errormsg[7]);
            exit(0);
        }
        if (symTable[symIdx].kind != 2)
        {
            printf("%s", errormsg[8]);
            exit(0);
        }

        getNextToken();
        emit(9, "SYS", 0, 2); // READ
        emit(4, "STO", lexLevel - symTable[symIdx].level, symTable[symIdx].addr);

        return;
    }
    if(token.type == writesym)
    {
        getNextToken();
        expression(lexLevel);
        emit(9, "SYS", 0, 1); // WRITE
        return;
    }
}

void condition(int lexLevel)
{
    if (token.type == oddsym)
    {
        getNextToken();
        expression(lexLevel);
        emit(2, "ODD", 0, 6);
    }
    else
    {
        expression(lexLevel);
        if(token.type == eqlsym)
        {
            getNextToken();
            expression(lexLevel);
            emit(2, "EQL", 0, 8);
        }
        else if (token.type == neqsym)
        {
            getNextToken();
            expression(lexLevel);
            emit(2, "NEQ", 0, 9);
        }
        else if (token.type == lessym)
        {
            getNextToken();
            expression(lexLevel);
            emit(2, "LSS", 0, 10);
        }    
        else if (token.type == leqsym)
        {
            getNextToken();
            expression(lexLevel);
            emit(2, "LEQ", 0, 11);
        }    
        else if (token.type == gtrsym)
        {
            getNextToken();
            expression(lexLevel);
            emit(2, "GTR", 0 , 12);    
        }
        else if (token.type == geqsym)
        { 
            getNextToken();
            expression(lexLevel);
            emit(2, "GEQ", 0 ,13);
        }    
        else
        {
            printf("%s", errormsg[13]);
            exit(0);
        }
    }
}

void expression(int lexLevel)
{
    if (token.type == minussym)
    {
        getNextToken();
        term(lexLevel);
        emit(2, "NEG", 0, 1);
        
        while (token.type == plussym || token.type == minussym)
        {
            if (token.type == plussym)
            {
                getNextToken();
                term(lexLevel);
                emit(2, "ADD", 0, 2);
            }
            else
            {
                getNextToken();
                term(lexLevel);
                emit(2 ,"SUB", 0, 3);
            }
        }
    }
    else
    {
        if (token.type == plussym)
            getNextToken();
            
        term(lexLevel);
        while(token.type == plussym || token.type == minussym)
        {
            if(token.type == plussym)
            {
                getNextToken();
                term(lexLevel);
                emit(2, "ADD", 0, 2);
            }
            else
            {
                getNextToken();
                term(lexLevel);
                emit(2, "SUB", 0, 3);
            }
        }
    }
}

void term(int lexLevel)
{
    factor(lexLevel);
    while(token.type == multsym || token.type == slashsym || token.type == modsym)
    {
        if (token.type == multsym)
        {
            getNextToken();
            factor(lexLevel);
            emit(2, "MUL", 0, 4);
        }
        else if (token.type == slashsym)
        {
            getNextToken();
            factor(lexLevel);
            emit(2, "DIV", 0, 5);
        }
        else
        {
            getNextToken();
            factor(lexLevel);
            emit(2, "MOD", 0, 7);
        }
    }
}

void factor(int lexLevel)
{
    if (token.type == identsym)
    {
        int symIdxV = symbolTableSearch(token.name, lexLevel, 2); // var
        int symIdxC = symbolTableSearch(token.name, lexLevel, 1); // const
        
        if (symIdxV == -1 && symIdxC == -1)
        {
            printf("%s", errormsg[7]);
            exit(0);
        }
        else if (symIdxC == -1 || (symIdxV != -1 && symTable[symIdxV].level > symTable[symIdxC].level)) // var is closer
            emit (3, "LOD", lexLevel - symTable[symIdxV].level, symTable[symIdxV].addr);
        else // const is closer
            emit(1, "LIT", 0, symTable[symIdxC].val);

        getNextToken();
    }
    else if (token.type == numbersym)
    {
        emit(1, "LIT", 0, token.value);
        getNextToken();
    }
    else if (token.type == lparentsym)
    {
        getNextToken();
        expression(lexLevel);
        if(token.type != rparentsym)
        {
            printf("%s", errormsg[14]);
            exit(0);
        }
        getNextToken();
    }
    else if(token.type == callsym)
    {
        getNextToken();
        if(token.type != identsym)
        {
            printf("%s", errormsg[2]);
            exit(0);
        }
        int symIdx = symbolTableSearch(token.name, lexLevel, 3); // procedure
        if (symIdx == -1)
        {
            printf("%s", errormsg[7]);
            exit(0);
        }

        getNextToken();
        if(token.type == lparentsym)
        {
            getNextToken();
            if(symTable[symIdx].param != 1)
            {
                printf("%s", errormsg[18]);
                exit(0);
            }
            expression(lexLevel);
            if(token.type != rparentsym)
            {
                printf("%s", errormsg[14]);
                exit(0);
            }
            getNextToken();
        }
        else
            emit(1, "LIT", 0, 0);

        emit(5, "CAL", lexLevel - symTable[symIdx].level, symTable[symIdx].val);
    }
    else
    {
        printf("%s", errormsg[15]);
        exit(0);
    }
}