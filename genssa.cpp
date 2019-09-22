#include<iostream>
#include"genssa.h"
#include"interpreter.h"
#include<stack>

using namespace std;

parsed_class_info* class_infoSSA;
cp_info** constant_poolSSA;
method_info** methodsSSA;

const string INIT = "<init>";

string MAIN_METHODSSA = "main";

void parseMyCode(code_line* codes, code_attribute* codeAttr, int & numOfLines)
{
    int n = 0;
    while(n < codeAttr->get_code_length())
    {
        codes[n].op_code = (*(codeAttr->get_code() + n));
        switch(codes[n].op_code)
        {
            //Without extra bytes:
            case CODE_RETURN:
            case CODE_IRETURN:
                
            case CODE_ICONST_M1:
            case CODE_ICONST_0:
            case CODE_ICONST_1:
            case CODE_ICONST_2:
            case CODE_ICONST_3:
            case CODE_ICONST_4:
            case CODE_ICONST_5:

            case CODE_ILOAD_0:
            case CODE_ILOAD_1:
            case CODE_ILOAD_2:
            case CODE_ILOAD_3:

            case CODE_ISTORE_0:
            case CODE_ISTORE_1:
            case CODE_ISTORE_2:
            case CODE_ISTORE_3:

            case CODE_IADD:
            case CODE_IMUL:
            case CODE_ISUB:
            case CODE_ISHL:
            case CODE_ISHR:
                codes[n].para = 0;
                break;
            //With 1 extra byte
            case CODE_BIPUSH:
                codes[n].para = (*(codeAttr->get_code() + n + 1));
                ++n;
                break;
            //With 2 extra bytes
            case CODE_IINC:

            case CODE_ICMPEQ:
            case CODE_ICMPNE:
            case CODE_ICMPLT:
            case CODE_ICMPGE:
            case CODE_ICMPGT:
            case CODE_ICMPLE:

            case CODE_IFEQ:
            case CODE_IFNE:
            case CODE_IFLT:
            case CODE_IFGE:
            case CODE_IFGT:
            case CODE_IFLE:

            case CODE_GOTO:
            case CODE_GETSTATIC:
            case CODE_INVOKESTATIC:
            case CODE_INVOKEVIRTUAL:
                codes[n].para = (((*(codeAttr->get_code() + n + 1)) << 8)
                    | (*(codeAttr->get_code() + n + 2)));
                n += 2;
            break;
        }
        ++n;
    }
    numOfLines = n;
}

void findLeaderLines(int (&leaderMarker)[MAX_LINES], 
    const code_line (&codes)[MAX_LINES], int &blockCount, int numOfLines)
{
    int16_t temp;
    int16_t offset;

    leaderMarker[0] = 1;
    for (int i = 0; i < numOfLines; i++)
    {
        switch(codes[i].op_code)
        {
            case CODE_ICMPEQ:
            case CODE_ICMPNE:
            case CODE_ICMPLT:
            case CODE_ICMPGE:
            case CODE_ICMPGT:
            case CODE_ICMPLE:

            case CODE_IFEQ:
            case CODE_IFNE:
            case CODE_IFLT:
            case CODE_IFGE:
            case CODE_IFGT:
            case CODE_IFLE:

            case CODE_GOTO:
                leaderMarker[i] = 0;
                offset = (int16_t)codes[i].para;
                temp = i + offset;
                leaderMarker[temp] = 1;

                ++i;
                while(codes[i].op_code == 0)
                    ++i;
                leaderMarker[i] = 1;
            break;

            default: 
                if (leaderMarker[i] != 1)
                    leaderMarker[i] = 0; 
            break;
        }
    }
    for (int i = 0; i < numOfLines; i++)
        if (leaderMarker[i] == 1)
            ++blockCount;
    //for (int i = 0; i < numOfLines;i++)
    //    cout << i << "-" <<leaderMarker[i] << " ";
    //cout << endl;
}

void buildControlFlowGraph(int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS],
    const codeBlock (&codeBlocks)[MAX_BLOCKS], int blockCount)
{
    int k;
    int tempOPCode;
    int16_t offset;
    int16_t temp;
    //cout << blockCount << endl;
    //controlFlowGraph[FROM][TO]
    for (int i = 0; i < blockCount; i++)
    {
        k = 1;
        while (codeBlocks[i].codes[codeBlocks[i].numOfLines - k].op_code == 0) ++k;
    //    cout << codeBlocks[i].codes[codeBlocks[i].numOfLines - k].op_code << endl;
        switch (codeBlocks[i].codes[codeBlocks[i].numOfLines - k].op_code)
        {
            case CODE_ICMPEQ:
            case CODE_ICMPNE:
            case CODE_ICMPLT:
            case CODE_ICMPGE:
            case CODE_ICMPGT:
            case CODE_ICMPLE:

            case CODE_IFEQ:
            case CODE_IFNE:
            case CODE_IFLT:
            case CODE_IFGE:
            case CODE_IFGT:
            case CODE_IFLE:

                controlFlowGraph[i][i+1] = 1;
                offset = (int16_t)codeBlocks[i].codes[codeBlocks[i].numOfLines-k].para;
                temp = (int16_t)codeBlocks[i].end + offset;

                for (int n = 0; n < blockCount;n++) 
                {              
                    if ((int16_t)codeBlocks[n].start == temp)          
                        controlFlowGraph[i][n] = 1;
                }
                break;

            case CODE_GOTO:
                offset = (int16_t)codeBlocks[i].codes[codeBlocks[i].numOfLines-k].para;
                temp = (int16_t)codeBlocks[i].end + offset;

                for (int n = 0; n < blockCount;n++) 
                {              
                    if ((int16_t)codeBlocks[n].start == temp)          
                        controlFlowGraph[i][n] = 1;
                }
            break;    

            default:
                controlFlowGraph[i][i+1] = 1;
        }
    }
}

void switchTo(int i, const int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS],
                 int blockCount, int (&visited)[MAX_BLOCKS])
{
    visited[i] = 1;
    for (int n = 0; n < blockCount; n++)
    {
        if (controlFlowGraph[i][n] == 1)
        {
            if (visited[n] == 0)
            {
                switchTo(n,controlFlowGraph,blockCount,visited);
            }
        }
    }
}

void DFS(const int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS],
            int (&visited)[MAX_BLOCKS], int blockCount)
{
    for (int i = 0; i < MAX_BLOCKS; i++) 
    {
        visited[i] = 0;
    }

    //START WITH NODE 0
    visited[ZERO] = 1;
    for (int i = 0; i < blockCount; i++)
    {
        if (controlFlowGraph[0][i] == 1)
        {
            if (visited[i] == 0)
            {
                switchTo(i,controlFlowGraph,blockCount,visited);
            }
        }
    }
}

void buildDominationGraph(const int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS], 
        int (&dominationGraph)[MAX_BLOCKS][MAX_BLOCKS],
        int blockCount)
{
    int visitedNodes[MAX_BLOCKS];
    int temp[MAX_BLOCKS][MAX_BLOCKS];
    for (int i = 0; i < blockCount; i++) 
    {
        for (int h = 0; h < blockCount; h++)
            for (int g = 0; g < blockCount; g++)
                temp[h][g] = controlFlowGraph[h][g];
        
        for (int h = 0; h < blockCount; h++)
            for (int g = 0; g < blockCount; g++)
                if (h == i || g == i)
                    temp[h][g] = 0;
        DFS(temp,visitedNodes,blockCount);
        for (int k = 0; k < blockCount; k++) 
            if (visitedNodes[k] == 0) dominationGraph[i][k] = 1;
            else if (visitedNodes[k] == 1) dominationGraph[i][k] = 0;;
    }
}

void buildDominanceFrontierGraph(int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS],
    int (&dominationGraph)[MAX_BLOCKS][MAX_BLOCKS],
    int (&strictDominationGraph)[MAX_BLOCKS][MAX_BLOCKS],
    int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS],int blockCount)
{
/*
    cout << "Strict DOmination: " << endl;
    for (int i = 0; i < blockCount; i++)
    {
        for (int j = 0; j < blockCount; j++)
            cout << strictDominationGraph[i][j] << " ";
        cout << endl;
    }
*/
    for(int i = 0; i < blockCount; i++)
    {
        for (int j = 0; j < blockCount; j++)
        if (dominationGraph[i][j])
        {
            for (int n = 0; n < blockCount; n++)
            {
                if (controlFlowGraph[j][n] == 1)
                {
                    if (strictDominationGraph[i][n] == 0)
                        dominanceFrontierGraph[i][n] = 1;
                }
            }
        }
    }
}

int findPreviousStackDepth(codeBlock (&codeBlocks)[MAX_BLOCKS], int blockCount,
    const int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS], int currentBlock)
{
    for (int i = 0; i < blockCount; i++)
        if (controlFlowGraph[i][currentBlock] == 1)
            return codeBlocks[i].resultingStackDepth;
    return 0;
}

int findBlockStartingWith(int offset,int currentLine, codeBlock (&codeBlocks)[MAX_BLOCKS],int blockCount)
{
    int16_t startLine = static_cast<int16_t>(offset) + static_cast<int16_t>(currentLine);
    for (int n = 0; n < blockCount; n++)
        if (static_cast<int16_t>(codeBlocks[n].start) == startLine)
            return n;
    return 0;
}

void addThreeAddress(codeBlock (&codeBlocks)[MAX_BLOCKS], int blockCount, 
    const int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS])
{
    int j = 0;
    int stackDepth = 0;
    for (int i = 0; i < blockCount; i++)
    {
        j = 0;

        if (i != 0)
            stackDepth = findPreviousStackDepth(codeBlocks, blockCount, controlFlowGraph, i);
        else stackDepth = 0;

        for (int n = 0; n < codeBlocks[i].numOfLines; n++)
        {
            switch(codeBlocks[i].codes[n].op_code)
            {
                case CODE_GETSTATIC:
                    codeBlocks[i].threeAddress[j].op_code = "GET_STATIC";
                    ++j;
                break;
                case CODE_RETURN:
                case CODE_IRETURN:
                    codeBlocks[i].threeAddress[j].op_code = "Return";
                    ++j;
                    break;

                case CODE_ICONST_M1:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = -1;
                    ++j;
                    break;

                case CODE_ICONST_0:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 0;
                    ++j;
                    break;

                case CODE_ICONST_1:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 1; 
                    ++j;
                    break;  

                case CODE_ICONST_2:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 2;
                    ++j;
                    break;

                case CODE_ICONST_3:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 3;
                    ++j;
                    break;

                case CODE_ICONST_4:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 4;
                    ++j;
                    break;
                case CODE_ICONST_5:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 5;
                    ++j;
                    break;

                case CODE_ILOAD_0:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 0;
                    ++j;
                    break;

                case CODE_ILOAD_1:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 1;
                    ++j;
                    break;

                case CODE_ILOAD_2:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 2;
                    ++j;
                    break;

                case CODE_ILOAD_3:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 3;
                    ++j;
                    break;

                case CODE_ISTORE_0:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 0;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    ++j;
                    break;

                case CODE_ISTORE_1:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 1;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    ++j;
                    break;

                case CODE_ISTORE_2:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 2;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    ++j;
                    break;

                case CODE_ISTORE_3:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 3;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    ++j;
                    break;

                case CODE_IADD:
                    codeBlocks[i].threeAddress[j].op_code = "ADD";
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = stackDepth;
                    ++stackDepth;
                    ++j;
                    break;

                case CODE_IMUL:
                    codeBlocks[i].threeAddress[j].op_code = "MUL";
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = stackDepth;
                    ++stackDepth;
                    ++j;
                    break;

                case CODE_ISUB:
                    codeBlocks[i].threeAddress[j].op_code = "SUB";
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = stackDepth;
                    ++stackDepth;
                    ++j;
                    break;

                case CODE_ISHL:
                    codeBlocks[i].threeAddress[j].op_code = "SHL";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = stackDepth;
                    ++stackDepth;  
                    ++j;
                    break;

                case CODE_ISHR:
                    codeBlocks[i].threeAddress[j].op_code = "SHR";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = stackDepth;
                    ++stackDepth;
                    ++j;
                    break;

                case CODE_BIPUSH:
                    codeBlocks[i].threeAddress[j].op_code = "MOV";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth;
                    ++stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = codeBlocks[i].codes[n].para;
                    ++j;
                    break;

                case CODE_IINC:
                    codeBlocks[i].threeAddress[j].op_code = "ADD";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[0].numbering =
                        codeBlocks[i].codes[n].para >> 8;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 
                        (int16_t)codeBlocks[i].codes[n].para & 0xFF;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myVar;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = 
                        codeBlocks[i].codes[n].para >> 8;
                    ++j;
                    break;

                case CODE_ICMPEQ:
                case CODE_ICMPNE:
                case CODE_ICMPLT:
                case CODE_ICMPGE:
                case CODE_ICMPGT:
                case CODE_ICMPLE:
                    codeBlocks[i].threeAddress[j].op_code = "CMP";
                    codeBlocks[i].threeAddress[j].operands[0].mType = temp;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = stackDepth;
                    ++j;
                    codeBlocks[i].threeAddress[j].op_code = "BRCond";
                    codeBlocks[i].threeAddress[j].operands[0].mType = temp;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 0;

                    codeBlocks[i].threeAddress[j].operands[1].mType = blockNum;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 
                        findBlockStartingWith(codeBlocks[i].start 
                            + codeBlocks[i].codes[n].para,
                            n,codeBlocks,blockCount);
                    
                    codeBlocks[i].threeAddress[j].operands[2].mType = blockNum;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = i + 1;
                    ++j;
                    break;

                case CODE_IFEQ:
                case CODE_IFNE:
                case CODE_IFLT:
                case CODE_IFGE:
                case CODE_IFGT:
                case CODE_IFLE:
                    codeBlocks[i].threeAddress[j].op_code = "CMP";
                    codeBlocks[i].threeAddress[j].operands[0].mType = temp;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    --stackDepth;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth;
                    codeBlocks[i].threeAddress[j].operands[2].mType = myConst;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = 0;
                    ++j;
                    codeBlocks[i].threeAddress[j].op_code = "BRCond";
                    codeBlocks[i].threeAddress[j].operands[0].mType = temp;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 0;
                    
                    codeBlocks[i].threeAddress[j].operands[1].mType = blockNum;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = 
                        findBlockStartingWith(codeBlocks[i].start 
                            + codeBlocks[i].codes[n].para,
                            n,codeBlocks,blockCount);
                    
                    codeBlocks[i].threeAddress[j].operands[2].mType = blockNum;
                    codeBlocks[i].threeAddress[j].operands[2].numbering = i + 1;
                    ++j;   
                    break;

                case CODE_GOTO:
                    codeBlocks[i].threeAddress[j].op_code = "JMP";
                    codeBlocks[i].threeAddress[j].operands[0].mType = blockNum;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 
                        findBlockStartingWith(codeBlocks[i].start 
                            + codeBlocks[i].codes[n].para,
                            n,codeBlocks,blockCount);
                    ++j;
                    break;
                case CODE_INVOKESTATIC:
                    codeBlocks[i].threeAddress[j].op_code = "CALL";
                    codeBlocks[i].threeAddress[j].operands[0].mType = mindex;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = 
                        codeBlocks[i].codes[n].para;
                    codeBlocks[i].threeAddress[j].operands[1].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[1].numbering = stackDepth - 1;
                    ++j;
                    break; 
                case CODE_INVOKEVIRTUAL:
                    codeBlocks[i].threeAddress[j].op_code = "INVOKE_VIRTUAL";
                    codeBlocks[i].threeAddress[j].operands[0].mType = myStack;
                    codeBlocks[i].threeAddress[j].operands[0].numbering = stackDepth - 1;
                    ++j;
                    break;
                default: break;
            }
        }
        codeBlocks[i].resultingStackDepth = stackDepth;
        codeBlocks[i].numOfThreeAddresses = j;
    }
}

void buildCodeBlocks(codeBlock (&codeBlocks)[MAX_BLOCKS], 
    const code_line (&codes)[MAX_LINES], 
    int numOfLines, int &blockCount, 
    int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS],
    int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS])
{
    int n = 0;
    int j = 1;

    int leaderMarker[MAX_LINES];
    for(int i = 0;i< MAX_LINES; i++)
        leaderMarker[i] = 0;

    findLeaderLines(leaderMarker, codes, blockCount, numOfLines);

    codeBlocks[ZERO].codes[ZERO].op_code = codes[ZERO].op_code;
    codeBlocks[ZERO].codes[ZERO].para = codes[ZERO].para;
    codeBlocks[ZERO].numOfLines = 1;
    codeBlocks[ZERO].start = 0;
    for (int i = 1; i < numOfLines; i++)
    {
        if (leaderMarker[i] == 1)
        {
            int h = 1;
            while(codes[i-h].op_code == 0) ++h;
            codeBlocks[n].end = i - h;

            ++n;
            j = 0;
            codeBlocks[n].codes[j].op_code = codes[i].op_code;
            codeBlocks[n].codes[j].para = codes[i].para;
            codeBlocks[n].start = i;
            codeBlocks[n].numOfLines = 1;
            ++j;
        }
        else
        {
            codeBlocks[n].numOfLines++;
            codeBlocks[n].codes[j].op_code = codes[i].op_code;
            codeBlocks[n].codes[j].para = codes[i].para;
            ++j;
        }
    }

    buildControlFlowGraph(controlFlowGraph, codeBlocks, blockCount);
    /*
    for (int i = 0; i < blockCount; i++)
    {
        for (int j = 0; j < blockCount; j++)
            cout << controlFlowGraph[i][j] << " ";
        cout << endl;
    }
    */
    int dominationGraph[MAX_BLOCKS][MAX_BLOCKS];
    buildDominationGraph(controlFlowGraph,dominationGraph, blockCount);
    dominationGraph[ZERO][ZERO] = 1;
    int strictDominationGraph[MAX_BLOCKS][MAX_BLOCKS];

    for (int i = 0; i < blockCount; i++)
    {
        for (int j = 0; j < blockCount; j++)
            if (i == j) strictDominationGraph[i][j] = 0;
            else strictDominationGraph[i][j] = dominationGraph[i][j];

    }
/*    
    cout << "Domination Graph: " << endl;
    for (int i = 0; i < blockCount; i++)
    {
        for (int j = 0; j < blockCount; j++)
        {
            cout << dominationGraph[i][j] << " ";
        }
        cout << endl;
    }
*/
    for (int i = 0; i < MAX_BLOCKS; i++)
        for (int j = 0; j < MAX_BLOCKS; j++)
            dominanceFrontierGraph[i][j] = 0;

    buildDominanceFrontierGraph(controlFlowGraph, dominationGraph,
        strictDominationGraph, dominanceFrontierGraph,blockCount);
/*
    cout << "DOMINANCE FRONTIER GRAPH: " << endl;
    for (int i = 0; i < blockCount; i++)
    {
        for (int j = 0; j < blockCount; j++)
            cout << dominanceFrontierGraph[i][j] << " ";
        cout << endl;
    }
*/
    addThreeAddress(codeBlocks, blockCount, controlFlowGraph);
}

bool alreadyHasPhiFor(int var_index, codeBlock (&codeBlocks)[MAX_BLOCKS],int thisBlock)
{
    for (int i = 0; i < codeBlocks[thisBlock].numOfThreeAddresses; i++)
    {
        if (codeBlocks[thisBlock].threeAddress[i].op_code == "PHI"
            && codeBlocks[thisBlock].threeAddress[i].operands[0].mType == myVar
            && codeBlocks[thisBlock].threeAddress[i].operands[0].numbering == var_index)
            return true;
    }
    return false;
}

void insertPhiNodeFor(int var_index, codeBlock (&codeBlocks)[MAX_BLOCKS], int blockCount,
    const int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS], int currentBlock)
{
    for (int dI = 0; dI < blockCount; dI++)
    {
        if (dominanceFrontierGraph[currentBlock][dI] == 1 
            && !alreadyHasPhiFor(var_index,codeBlocks,dI))
        {
            int a = codeBlocks[dI].numOfThreeAddresses;
            codeBlocks[dI].threeAddress[a].op_code = "PHI";
            codeBlocks[dI].threeAddress[a].operands[0].mType = myVar;
            codeBlocks[dI].threeAddress[a].operands[0].numbering = var_index;
            ++(codeBlocks[dI].numOfThreeAddresses);
        }
    }
}

void insertPhiNodes(codeBlock (&codeBlocks)[MAX_BLOCKS],int blockCount, 
    const int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS])
{
    for (int vI = 0; vI < MAX_NUM_OF_VAR; vI++)
    {
        for (int bI = 0; bI < blockCount; bI++)
        {
            for (int lI = 0; lI < codeBlocks[bI].numOfThreeAddresses; lI++)
            {
                if ((codeBlocks[bI].threeAddress[lI].op_code == "MOV"
                        && codeBlocks[bI].threeAddress[lI].operands[0].mType == myVar
                        && codeBlocks[bI].threeAddress[lI].operands[0].numbering == vI)
                        || (codeBlocks[bI].threeAddress[lI].op_code == "ADD"
                                && codeBlocks[bI].threeAddress[lI].operands[2].mType == myVar
                                && codeBlocks[bI].threeAddress[lI].operands[2].numbering == vI))
                {
                    insertPhiNodeFor(vI,codeBlocks,blockCount,dominanceFrontierGraph,bI);
                }
            }
        }
    }

    for (int bI = 0; bI <blockCount; bI++)
    {
        int numOfPhi = 0;
        for (int lI = 0; lI < codeBlocks[bI].numOfThreeAddresses; lI++)
        {
            if (codeBlocks[bI].threeAddress[lI].op_code == "PHI")
                ++numOfPhi;
        }
        codeBlocks[bI].numOfThreeAddresses -= numOfPhi;
        codeBlocks[bI].numOfPhiNodes = numOfPhi;
    }
}

void renameVariables(codeBlock (&codeBlocks)[MAX_BLOCKS],int blockCount,
    const int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS])
{
    int currentPhi = 0;

    int stackSSA[MAX_NUM_OF_VAR];
    int variableSSA[MAX_NUM_OF_VAR];
    int tempNumber = 0;
    for (int i = 0; i < MAX_NUM_OF_VAR; i++)
    {
        variableSSA[i] = 0;
        stackSSA[i] = 0;
    }

    for (int bI = 0; bI < blockCount; bI++)
    {
        for (int phiI = codeBlocks[bI].numOfPhiNodes - 1; phiI >= 0; phiI--)
        {
            currentPhi = phiI + codeBlocks[bI].numOfThreeAddresses;
            if (codeBlocks[bI].threeAddress[currentPhi].op_code == "PHI"
                && codeBlocks[bI].threeAddress[currentPhi].operands[0].mType == myVar)
            {
                ++variableSSA[codeBlocks[bI].threeAddress[currentPhi].operands[0].numbering];
                codeBlocks[bI].threeAddress[currentPhi].operands[1].mType = myVar;
                codeBlocks[bI].threeAddress[currentPhi].operands[1].numbering = 
                    codeBlocks[bI].threeAddress[currentPhi].operands[0].numbering; 

                codeBlocks[bI].threeAddress[currentPhi].operands[2].mType = myVar;
                codeBlocks[bI].threeAddress[currentPhi].operands[2].numbering = 
                    codeBlocks[bI].threeAddress[currentPhi].operands[0].numbering;
                
                codeBlocks[bI].threeAddress[currentPhi].operands[0].ssaVal = 
                    variableSSA[codeBlocks[bI].threeAddress[currentPhi].operands[0].numbering];    
            }
        }
        for (int lI = 0; lI < codeBlocks[bI].numOfThreeAddresses; lI++)
        {
            if (codeBlocks[bI].threeAddress[lI].op_code == "MOV")
            {
                if (codeBlocks[bI].threeAddress[lI].operands[0].mType == myVar)
                {
                    ++variableSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                    codeBlocks[bI].threeAddress[lI].operands[0].ssaVal = 
                        variableSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                }
                else if (codeBlocks[bI].threeAddress[lI].operands[0].mType == myStack)
                {
                    ++stackSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                    codeBlocks[bI].threeAddress[lI].operands[0].ssaVal = 
                        stackSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                }

                if (codeBlocks[bI].threeAddress[lI].operands[1].mType == myVar)
                {
                    codeBlocks[bI].threeAddress[lI].operands[1].ssaVal = 
                        variableSSA[codeBlocks[bI].threeAddress[lI].operands[1].numbering];
                }
                else if (codeBlocks[bI].threeAddress[lI].operands[1].mType == myStack)
                {
                    codeBlocks[bI].threeAddress[lI].operands[1].ssaVal = 
                        stackSSA[codeBlocks[bI].threeAddress[lI].operands[1].numbering];
                }
            }
            else if (codeBlocks[bI].threeAddress[lI].op_code == "ADD")
            {
                if (codeBlocks[bI].threeAddress[lI].operands[0].mType == myVar)
                {
                    ++variableSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                    codeBlocks[bI].threeAddress[lI].operands[0].ssaVal = 
                        variableSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                    codeBlocks[bI].threeAddress[lI].operands[2].ssaVal = 
                        variableSSA[codeBlocks[bI].threeAddress[lI].operands[2].numbering] - 1;  
                }
                else if (codeBlocks[bI].threeAddress[lI].operands[0].mType == myStack)
                {
                    ++stackSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                    codeBlocks[bI].threeAddress[lI].operands[0].ssaVal = 
                        stackSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
                    codeBlocks[bI].threeAddress[lI].operands[2].ssaVal = 
                        stackSSA[codeBlocks[bI].threeAddress[lI].operands[2].numbering] - 1;
                }
            }
            else if (codeBlocks[bI].threeAddress[lI].op_code == "CMP")
            {
                codeBlocks[bI].threeAddress[lI].operands[0].numbering = ++tempNumber;
                codeBlocks[bI].threeAddress[lI + 1].operands[0].numbering = tempNumber;

                if (codeBlocks[bI].threeAddress[lI].operands[1].mType == myVar)
                {
                    codeBlocks[bI].threeAddress[lI].operands[1].ssaVal = 
                        variableSSA[codeBlocks[bI].threeAddress[lI].operands[1].numbering];
                }
                else if (codeBlocks[bI].threeAddress[lI].operands[1].mType == myStack)
                {
                    codeBlocks[bI].threeAddress[lI].operands[1].ssaVal = 
                        stackSSA[codeBlocks[bI].threeAddress[lI].operands[1].numbering];
                }
                if (codeBlocks[bI].threeAddress[lI].operands[3].mType == myVar)
                {
                    codeBlocks[bI].threeAddress[lI].operands[2].ssaVal = 
                        variableSSA[codeBlocks[bI].threeAddress[lI].operands[2].numbering];
                }
                else if (codeBlocks[bI].threeAddress[lI].operands[2].mType == myStack)
                {
                    codeBlocks[bI].threeAddress[lI].operands[2].ssaVal = 
                        stackSSA[codeBlocks[bI].threeAddress[lI].operands[2].numbering];
                }
            }
            else if (codeBlocks[bI].threeAddress[lI].op_code == "CALL")
            {
                codeBlocks[bI].threeAddress[lI].operands[1].ssaVal = 
                    stackSSA[codeBlocks[bI].threeAddress[lI].operands[1].numbering];
            }
            else if (codeBlocks[bI].threeAddress[lI].op_code == "INVOKE_VIRTUAL")
            {
                codeBlocks[bI].threeAddress[lI].operands[0].ssaVal = 
                    stackSSA[codeBlocks[bI].threeAddress[lI].operands[0].numbering];
            }
        }
    }
}

int countFrontier(const int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS], int blockCount, int i)
{
    int count = 0;
    for (int k = 0; k < blockCount; k++)
    {
        if (dominanceFrontierGraph[k][i] == 1)
            ++count;
    }
    return count;
}

int findSSAOf(int myNumbering, int myBlock, codeBlock(&codeBlocks)[MAX_BLOCKS],int blockCount)
{
    for (int i = codeBlocks[myBlock].numOfThreeAddresses - 1; i >=0; i--)
    {
        if (codeBlocks[myBlock].threeAddress[i].op_code == "ADD"
            || codeBlocks[myBlock].threeAddress[i].op_code == "MOV")
        {
            if (codeBlocks[myBlock].threeAddress[i].operands[0].mType == myVar
                && codeBlocks[myBlock].threeAddress[i].operands[0].numbering == myNumbering)
                    return codeBlocks[myBlock].threeAddress[i].operands[0].ssaVal;
        }
    }
    return 0;
}

void finishingPhiNodes(codeBlock (&codeBlocks)[MAX_BLOCKS], int blockCount, 
    const int (&dominanceFrontierGraph)[MAX_BLOCKS][MAX_BLOCKS],
    const int (&controlFlowGraph)[MAX_BLOCKS][MAX_BLOCKS])
{
    int numVar = 0;
    int frontierLeft = 0;
    int frontierRight = 0;
    int ssaLeft = 0;
    int ssaRight = 0;

    for (int i = 0; i < blockCount; i++)
    {
        if (codeBlocks[i].numOfPhiNodes > 0)
        {
            for (int k = codeBlocks[i].numOfPhiNodes - 1; k >= 0; k--)
            {
                int m = codeBlocks[i].numOfThreeAddresses + k;
                numVar = codeBlocks[i].threeAddress[m].operands[0].numbering;
                if (countFrontier(dominanceFrontierGraph, blockCount,i) == 2)
                {
                    int n = 0;
                    while (n < blockCount)
                    {
                        if (dominanceFrontierGraph[n][i] == 1)
                        {
                            frontierLeft = n;
                            break;
                        }
                        ++n;
                    }
                    ++n;
                    while (n < blockCount)
                    {
                        if (dominanceFrontierGraph[n][i] == 1)
                        {
                            frontierRight = n;
                            break;
                        }
                        ++n;
                    }
                    ssaLeft = findSSAOf(numVar,frontierLeft,codeBlocks,blockCount);
                    ssaRight = findSSAOf(numVar, frontierRight,codeBlocks,blockCount);
                    
                    //cout << m <<codeBlocks[i].threeAddress[m].op_code << "block " << i << ": " << frontierLeft << " " << frontierRight << endl;
                    codeBlocks[i].threeAddress[m].operands[1].ssaVal = ssaLeft;
                    codeBlocks[i].threeAddress[m].operands[2].ssaVal = ssaRight;
                    
                }

                else if (countFrontier(dominanceFrontierGraph, blockCount,i) == 1)
                {
                    int n = 0;
                    while (n < blockCount)
                    {
                        if (dominanceFrontierGraph[n][i] == 1)
                        {
                            frontierLeft = n;
                            break;
                        }
                        ++n;
                    }
                    n = 0;
                    while (n < blockCount)
                    {
                        if (controlFlowGraph[n][i] == 1 && dominanceFrontierGraph[n][i] != 1)
                        {
                            frontierRight = n;
                            break;
                        }
                        ++n;
                    }
                    ssaLeft = findSSAOf(numVar,frontierLeft,codeBlocks,blockCount);
                    ssaRight = findSSAOf(numVar, frontierRight,codeBlocks,blockCount);

                    codeBlocks[i].threeAddress[m].operands[1].ssaVal = ssaLeft;
                    codeBlocks[i].threeAddress[m].operands[2].ssaVal = ssaRight;
                }

                else if (countFrontier(dominanceFrontierGraph, blockCount,i) > 2)
                {
                    int n = 0;
                    while (n < blockCount)
                    {
                        if (dominanceFrontierGraph[n][i] == 1)
                        {
                            frontierLeft = n;
                            if (findSSAOf(numVar,frontierLeft,codeBlocks,blockCount) != 0)
                            {
                                ssaLeft = findSSAOf(numVar,frontierLeft,codeBlocks,blockCount);
                                break;
                            }
                        }
                        ++n;
                    }
                    
                    while (n < blockCount)
                    {
                        if (controlFlowGraph[n][i] == 1 && dominanceFrontierGraph[n][i] != 1)
                        {
                            frontierRight = n;
                            if (findSSAOf(numVar,frontierRight,codeBlocks,blockCount) != 0)
                            {
                                ssaRight = findSSAOf(numVar,frontierRight,codeBlocks,blockCount);
                                break;
                            }
                        }
                        ++n;
                    }
                    if (ssaRight == 0)
                    {
                        n = 0;
                        while (n < blockCount)
                        {
                            if (controlFlowGraph[n][i] == 1 && dominanceFrontierGraph[n][i] != 1)
                            {
                                frontierRight = n;
                                if (findSSAOf(numVar,frontierRight,codeBlocks,blockCount) != 0)
                                {
                                    ssaRight = findSSAOf(numVar,frontierRight,codeBlocks,blockCount);
                                    break;
                                }
                            }
                            ++n;
                        }    
                    }

                    codeBlocks[i].threeAddress[m].operands[1].ssaVal = ssaLeft;
                    codeBlocks[i].threeAddress[m].operands[2].ssaVal = ssaRight;
                }
            }
        }
    }
}

void myPrint(int blockCount, codeBlock (&codeBlocks)[MAX_BLOCKS])
{
    for (int i = 0; i < blockCount;i++)
    {
        cout << "block_" << i << ":" << endl;
        for (int k = codeBlocks[i].numOfPhiNodes - 1; k >= 0; k--)
        {
            int m = codeBlocks[i].numOfThreeAddresses + k;
            cout << codeBlocks[i].threeAddress[m].op_code << " ";
            for (int h = 0; h < 4; h++)
            {
                if (codeBlocks[i].threeAddress[m].operands[h].mType == myStack
                || codeBlocks[i].threeAddress[m].operands[h].mType == myVar
                || codeBlocks[i].threeAddress[m].operands[h].mType == temp
                || codeBlocks[i].threeAddress[m].operands[h].mType == myConst
                || codeBlocks[i].threeAddress[m].operands[h].mType == mindex
                || codeBlocks[i].threeAddress[m].operands[h].mType == blockNum)
                {
                    cout << codeBlocks[i].threeAddress[m].operands[h] << " ";
                }
            }
            cout << endl;
        }

        for (int n = 0; n < codeBlocks[i].numOfThreeAddresses; n++)
        {
            cout << codeBlocks[i].threeAddress[n].op_code << " ";
            for (int h = 0; h < 4; h++)
            {
                if (codeBlocks[i].threeAddress[n].operands[h].mType == myStack
                || codeBlocks[i].threeAddress[n].operands[h].mType == myVar
                || codeBlocks[i].threeAddress[n].operands[h].mType == temp
                || codeBlocks[i].threeAddress[n].operands[h].mType == myConst
                //|| codeBlocks[i].threeAddress[n].operands[h].mType == mindex
                || codeBlocks[i].threeAddress[n].operands[h].mType == blockNum)
                {
                    cout << codeBlocks[i].threeAddress[n].operands[h] << " ";
                }
                else if (codeBlocks[i].threeAddress[n].operands[h].mType == mindex)
                {
                    cout << nameOfMethodAt(codeBlocks[i].threeAddress[n].operands[h].numbering) << " ";
                }
            }
            cout << endl;
        }
    }   
}

void genThisSSA(method_info* myMethod)
{
    int numOfLines = 0;
    int blockCount = 0;

    //method_info* mainMethod = findMethod(MAIN_METHODSSA);
    code_attribute* mainCode = (code_attribute*)findCodeOfMethod(myMethod);
    code_line codes[MAX_LINES];

    parseMyCode(codes, mainCode, numOfLines);

    //display graph with controlFlowGraph[FROM][TO]
    /*
         to    0 1 2 3 4
    from 
    0          0 1 0 0 1
    1          1 0 1 0 0
    2          1 0 1 1 0
    3          0 0 1 0 0
    4          1 0 0 0 0
    */

    int controlFlowGraph[MAX_BLOCKS][MAX_BLOCKS];
    for (int i = 0; i < MAX_BLOCKS; i++)
        for(int n = 0; n < MAX_BLOCKS; n++)
            controlFlowGraph[i][n] = 0;

    codeBlock codeBlocks[MAX_BLOCKS];

    int dominanceFrontierGraph[MAX_BLOCKS][MAX_BLOCKS];
    for (int i = 0; i < MAX_BLOCKS; i++)
        for(int n = 0; n < MAX_BLOCKS; n++)
            dominanceFrontierGraph[i][n] = 0;

	buildCodeBlocks(codeBlocks, codes, numOfLines, blockCount, controlFlowGraph, dominanceFrontierGraph);

    insertPhiNodes(codeBlocks,blockCount,dominanceFrontierGraph);
    
    renameVariables(codeBlocks ,blockCount, dominanceFrontierGraph);

    finishingPhiNodes(codeBlocks, blockCount, dominanceFrontierGraph, controlFlowGraph);

    myPrint(blockCount,codeBlocks);

}

void genSSA(parsed_class_info* &parsedClass)
{
    //genThisSSA();
    cout << endl;

    class_infoSSA = parsedClass;
    constant_poolSSA = parsedClass->constant_pool;
    methodsSSA = parsedClass->methods;
    
    method_info* methodTemp;
    cp_utf8_info* constTemp;

    for (int i = 0; i < class_infoSSA->num_of_methods; i++)
    {
        constTemp = (cp_utf8_info*)constant_poolSSA[methodsSSA[i]->name_index];
        if(INIT.compare(constTemp->myString) != 0)
        {
            cout << "Function: "  << constTemp->myString << endl;
            genThisSSA(methodsSSA[i]);
            cout << endl;
        }  
    }
  


/*
    method_info* mainMethod = findMethod(MAIN_METHODSSA);
    code_attribute* mainCode = (code_attribute*)findCodeOfMethod(mainMethod);
    code_line codes[MAX_LINES];

    parseMyCode(codes, mainCode, numOfLines);

    //display graph with controlFlowGraph[FROM][TO]
    /*
         to    0 1 2 3 4
    from 
    0          0 1 0 0 1
    1          1 0 1 0 0
    2          1 0 1 1 0
    3          0 0 1 0 0
    4          1 0 0 0 0
    

    int controlFlowGraph[MAX_BLOCKS][MAX_BLOCKS];
    for (int i = 0; i < MAX_BLOCKS; i++)
        for(int n = 0; n < MAX_BLOCKS; n++)
            controlFlowGraph[i][n] = 0;

    codeBlock codeBlocks[MAX_BLOCKS];

    int dominanceFrontierGraph[MAX_BLOCKS][MAX_BLOCKS];
    for (int i = 0; i < MAX_BLOCKS; i++)
        for(int n = 0; n < MAX_BLOCKS; n++)
            dominanceFrontierGraph[i][n] = 0;

	buildCodeBlocks(codeBlocks, codes, numOfLines, blockCount, controlFlowGraph, dominanceFrontierGraph);

    insertPhiNodes(codeBlocks,blockCount,dominanceFrontierGraph);
    
    renameVariables(codeBlocks ,blockCount, dominanceFrontierGraph);

    finishingPhiNodes(codeBlocks, blockCount, dominanceFrontierGraph, controlFlowGraph);

    myPrint(blockCount,codeBlocks);

    for (int i = 0; i < blockCount;i++)
    {
        cout << "block_" << i << ":" << endl;
        for (int k = codeBlocks[i].numOfPhiNodes - 1; k >= 0; k--)
        {
            int m = codeBlocks[i].numOfThreeAddresses + k;
            cout << codeBlocks[i].threeAddress[m].op_code << " ";
            for (int h = 0; h < 4; h++)
            {
                if (codeBlocks[i].threeAddress[m].operands[h].mType == myStack
                || codeBlocks[i].threeAddress[m].operands[h].mType == myVar
                || codeBlocks[i].threeAddress[m].operands[h].mType == temp
                || codeBlocks[i].threeAddress[m].operands[h].mType == myConst
                || codeBlocks[i].threeAddress[m].operands[h].mType == mindex
                || codeBlocks[i].threeAddress[m].operands[h].mType == blockNum)
                {
                    cout << codeBlocks[i].threeAddress[m].operands[h] << " ";
                }
            }
            cout << endl;
        }
        for (int n = 0; n < codeBlocks[i].numOfThreeAddresses; n++)
        {
            cout << codeBlocks[i].threeAddress[n].op_code << " ";
            for (int h = 0; h < 4; h++)
            {
                if (codeBlocks[i].threeAddress[n].operands[h].mType == myStack
                || codeBlocks[i].threeAddress[n].operands[h].mType == myVar
                || codeBlocks[i].threeAddress[n].operands[h].mType == temp
                || codeBlocks[i].threeAddress[n].operands[h].mType == myConst
                || codeBlocks[i].threeAddress[n].operands[h].mType == mindex
                || codeBlocks[i].threeAddress[n].operands[h].mType == blockNum)
                {
                    cout << codeBlocks[i].threeAddress[n].operands[h] << " ";
                }
            }
            cout << endl;
        }
    }
    */
    
}
