// Jacob O'Quinn
// Braden Steller

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "compiler.h"

#define MAX_STACK_HEIGHT 50
#define MAX_CODE_LENGTH 100

int base(int stack[], int level, int BP);

void execute(instruction * code, int print)
{
    // PM/0 Initial/Default Values:
    int sp = -1;
    int bp = 0;
    int pc = 0;
    int l, m;
    instruction ir;
    int stack[MAX_STACK_HEIGHT] = {0};

    // Print Initial values
    if (print)
    {
        printf("%18s%5s%5s%8s\n", "PC", "BP", "SP", "stack");
        printf("Initial values:%2d%5d %5d   \n", pc, bp, sp);
    }

    // Fetch loop
    bool haltFlag = 1;
    int prevPC; // keeps track of previous program counter
    int ar[MAX_STACK_HEIGHT] = {-1};
    int arPos = 0;
    while (haltFlag)
    {
        // FETCH CYCLE
        ir = code[pc];
        prevPC = pc++;
        l = ir.l;
        m = ir.m;

        // EXECUTE CYCLE
        switch (ir.opcode)
        {
            // LIT	0, M		Pushes a constant value (literal) M onto the stack
            case 1:
                sp++;
                stack[sp] = m;
                break;

            // OPR	0, M		Operation to be performed on the data at the top of the stack.
            case 2:
                switch (m)
                {
                    case 0: //RTN
                        stack[bp - 1] = stack[sp];
                        sp = bp - 1;
                        bp = stack[sp + 2];
                        pc = stack[sp + 3];
                        ar[arPos] = -1;
                        arPos--;
                        break;
                    case 1: //NEG
                        stack[sp] *= -1;
                        break;
                    case 2: //ADD
                        sp = sp - 1;
                        stack[sp] += stack[sp + 1];
                        break;
                    case 3: //SUB
                        sp = sp - 1;
                        stack[sp] -= stack[sp + 1];
                        break;
                    case 4: //MUL
                        sp--;
                        stack[sp] *= stack[sp + 1];
                        break;
                    case 5: //DIV
                        sp = sp - 1;
                        stack[sp] /= stack[sp + 1];
                        break;
                    case 6: //ODD
                        stack[sp] = stack[sp] % 2;
                        break;
                    case 7: //MOD
                        sp--;
                        stack[sp] = stack[sp] % stack[sp + 1];
                        break;
                    case 8: //EQL
                        sp--;
                        stack[sp] = (stack[sp] == stack[sp + 1]);
                        break;
                    case 9: //NEQ
                        sp--;
                        stack[sp] = (stack[sp] != stack[sp + 1]);
                        break;
                    case 10: //LSS
                        sp--;
                        stack[sp] = (stack[sp] < stack[sp + 1]);
                        break;
                    case 11: //LEQ
                        sp--;
                        stack[sp] = (stack[sp] <= stack[sp + 1]);
                        break;
                    case 12: //GTR
                        sp--;
                        stack[sp] = (stack[sp] > stack[sp + 1]);
                        break;
                    case 13: //GEQ
                        sp--;
                        stack[sp] = (stack[sp] >= stack[sp + 1]);
                        break;
                }
                break;

            // LOD	L, M		Load value to top of stack from the stack location at
            //                  offset M from L lexicographical levels down
            case 3:
                sp++;
                stack[sp] = stack[base(stack, l, bp) + m];
                break;

            // STO	L, M		Store value at top of stack in the stack location at offset M
            //                  from L lexicographical levels down
            case 4:
                stack[base(stack, l, bp) + m] = stack[sp];
                sp--;
                break;

            // CAL	L, M		Call procedure at code index M (generates new
            //                  Activation Record and PC = M)
            case 5:
                stack[sp + 1] = base(stack, l, bp); // static link (SL)
                stack[sp + 2] = bp;                 // dynamic link (DL)
                stack[sp + 3] = pc;                 // return address (RA)
                stack[sp + 4] = stack[sp];          // parameter (P)
                bp = sp + 1;
                pc = m;
                arPos++;
                ar[arPos] = sp + 1;

                break;

            // INC	0, M		Allocate M memory words (increment SP by M). First four
            //  				are reserved to   Static Link (SL), Dynamic Link (DL),
            //                  Return Address (RA), and Parameter (P)
            case 6:
                sp += m;
                break;

            // JMP	0, M		Jump to instruction M (PC = M)
            case 7:
                pc = m;
                break;

            // JPC 0, M		    Jump to instruction M if top stack element is 0
            case 8:
                if (stack[sp] == 0)
                {
                    pc = m;
                }
                sp--;

                break;

            // SYS
            case 9:
                switch (ir.m)
                {
                    case 1:
                        printf("Top of Stack Value: %d\n", stack[sp]);
                        sp--;
                        break;

                    case 2:
                        sp++;
            
                        printf("Please Enter an Integer: ");
                        scanf("%d", &stack[sp]);
                        
                        break;

                    case 3: // Set Halt flag to zero (End of program).
                        haltFlag = 0;
                        break;
                }
                break;
            default:
                printf("ERROR DEFAULT STATE REACHED!!!\n");
                break;
        }

        // Print out current stack
        if (print)
        {
            printf("%2d%4s%3d%3d%6d%5d%5d  ", prevPC, ir.op, ir.l, ir.m, pc, bp, sp);

            int arPos2 = 1;
            for (int i = 0; i <= sp; i++)
            {
                if (i == ar[arPos2])
                {
                    if (ar[arPos2] != 0)
                        printf(" |");
                    arPos2++;
                }
                printf(" %d", stack[i]);
            }
            printf("\n");
        }
    }
}

// Find base L levels down
int base(int stack[], int level, int BP)
{
    int base = BP;
    int counter = level;
    while (counter > 0)
    {
        base = stack[base];
        counter--;
    }
    return base;
}