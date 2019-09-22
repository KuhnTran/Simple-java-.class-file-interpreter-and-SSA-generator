#ifndef GENSSA_H
#define GENSSA_H


#include"parsed_class_info.h"
#include"code_line.h"
#include<iostream>

using namespace std;

#define MAX_BLOCKS 64
#define MAX_LINES_PER_BLOCK 64
#define ZERO 0
#define MAX_OPERAND 4
void genSSA(parsed_class_info* &parsedClass);

enum myType { myPad ,myStack, myVar, myConst, temp, mindex, blockNum};

struct operand
{
    myType mType;
    int numbering; 
    int ssaVal;

    operand()
    {
        mType = myPad;
        numbering = 0;
        ssaVal = 0;
    }

    friend ostream&operator<<(ostream& out, operand& myOp)
    {
        switch(myOp.mType)
        {
            case myPad:
                cout << "ERROR: PADDING" << endl;
                exit(1);
                break;
            case myStack:
                out << "stack_" << myOp.numbering << "_" << myOp.ssaVal;
                break;
            case myVar:
                out << "var_" << myOp.numbering << "_" << myOp.ssaVal;;
                break;
            case myConst:
                out << "const_" << myOp.numbering;
                break;
            case temp:
                out << "temp_" << myOp.numbering;
                break;
            case mindex:
                out << "index_" << myOp.numbering;
                break;
            case blockNum:
                out << "block_" << myOp.numbering;
                break;
        }
    }

    operand operator=(operand& otherOperand)
    {
        mType = otherOperand.mType;
        numbering = otherOperand.numbering;
        ssaVal = otherOperand.ssaVal;
    }
};

struct threeAddressLine
{
    string op_code;
    operand operands[MAX_OPERAND];

    threeAddressLine()
    {
        op_code = "";
    }

    threeAddressLine operator=(threeAddressLine& otherThreeAddressLine)
    {
        op_code = otherThreeAddressLine.op_code;
        for (int i = 0; i < MAX_OPERAND; i++)
        {
            operands[i] = otherThreeAddressLine.operands[i];
        }
    }
};

struct phiNode
{

};

struct codeBlock
{
    int start;
    int end;
    int numOfLines;
    int numOfThreeAddresses;
    int resultingStackDepth;
    int numOfPhiNodes;

    code_line codes[MAX_LINES_PER_BLOCK];
    threeAddressLine threeAddress[MAX_LINES_PER_BLOCK];
};

#endif
