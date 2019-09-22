#include"interpreter.h"

#include<stack>

using namespace std;

parsed_class_info* class_info;
cp_info** constant_pool;
method_info** methods;
stack<int> myStack;

string MAIN_METHOD = "main";

method_info* findMethod(const string &methodName)
{
    method_info* methodTemp;
    cp_utf8_info* constTemp;

    for (int i = 0; i < class_info->num_of_methods; i++)
    {
        constTemp = (cp_utf8_info*)constant_pool[methods[i]->name_index];
        if(methodName.compare(constTemp->myString) == 0)
        {
            methodTemp = methods[i];
            break;
        }
    }
    return methodTemp;
}

attribute_info* findCodeOfMethod(method_info* method)
{

    attribute_info* attributeTemp;
    cp_utf8_info* constTemp;
    string codeString = "Code";

    for (int i = 0; i < method->attributes_count; i++)
    {
        constTemp = (cp_utf8_info*)constant_pool[method->attributes[i]->attribute_name_index];
        if (codeString.compare(constTemp->myString) == 0)
        {
            attributeTemp = method->attributes[i];
            break;
        }
    }

    return attributeTemp;

}

void parseCode(code_line* codes, code_attribute* codeAttr)
{
    int n = 0;
    while(n < codeAttr->get_code_length())
    {
        codes[n].op_code += (*(codeAttr->get_code() + n));
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
                codes[n].para += (*(codeAttr->get_code() + n + 1));
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
                codes[n].para += (((*(codeAttr->get_code() + n + 1)) << 8)
                    | (*(codeAttr->get_code() + n + 2)));
                n += 2;
            break;
        }
        ++n;
    }
}


void invokeMethod(code_line (&codes)[MAX_LINES], int i)
{
    //find the method to be invoked
    cp_methodref_info* invokedMethodRef = (cp_methodref_info*)constant_pool[codes[i].para];
    cp_nameandtype_info* namePool = 
        (cp_nameandtype_info*)constant_pool[invokedMethodRef->get_name_and_type_index()];
    cp_utf8_info* utfPool = (cp_utf8_info*)constant_pool[namePool->get_name_index()];
    method_info* invokedMed = findMethod(utfPool->myString);

    //Find the code
    code_attribute* codeAttr = (code_attribute*)findCodeOfMethod(invokedMed);
    code_line newCodes[MAX_LINES];
	for (int i = 0; i < MAX_LINES; i++)
	{
		newCodes[i].op_code = 0;
		newCodes[i].para = 0;
	}

    parseCode(newCodes,codeAttr);

    //Interprete code
    interpreteCode(newCodes);
}

string nameOfMethodAt(int index)
{
    cp_methodref_info* invokedMethodRef = (cp_methodref_info*)constant_pool[index];
    cp_nameandtype_info* namePool = 
        (cp_nameandtype_info*)constant_pool[invokedMethodRef->get_name_and_type_index()];
    cp_utf8_info* utfPool = (cp_utf8_info*)constant_pool[namePool->get_name_index()];
    return utfPool->myString;
}

void interpreteCode(code_line (&codes)[MAX_LINES])
{
    int variables[MAX_NUM_OF_VAR];
    
    int i = 0;
    bool returnReached = false;
    string nameOfMethod;
    int temp = 0;
    int temp2 = 0;
    int temp3 = 0;
    int16_t offset = 0;

    while(i < MAX_LINES && !returnReached)
    {
        switch(codes[i].op_code)
        {
            case CODE_RETURN:
            case CODE_IRETURN:
                returnReached = true;
                break;

            case CODE_ICONST_M1:
                myStack.push(-1);
                break;
            case CODE_ICONST_0:
                myStack.push(0);
                break;
            case CODE_ICONST_1:
                myStack.push(1);            
                break;
            case CODE_ICONST_2:
                myStack.push(2);    
                break;
            case CODE_ICONST_3:
                myStack.push(3);
                break;
            case CODE_ICONST_4:
                myStack.push(4);
                break;
            case CODE_ICONST_5:
                myStack.push(5);
                break;

            case CODE_ILOAD_0:
                break;
            case CODE_ILOAD_1:
            case CODE_ILOAD_2:
            case CODE_ILOAD_3:
                myStack.push(variables[codes[i].op_code - CODE_ILOAD_0]);
                break;

            case CODE_ISTORE_0:
            case CODE_ISTORE_1:
            case CODE_ISTORE_2:
            case CODE_ISTORE_3:
                temp = myStack.top();
                myStack.pop();
                variables[codes[i].op_code - CODE_ISTORE_0] = temp;
                break;

            case CODE_IADD:
                temp = myStack.top();
                myStack.pop();
                temp += myStack.top();
                myStack.pop();
                myStack.push(temp);
                break;

            case CODE_IMUL:
                temp = myStack.top();
                myStack.pop();
                temp *= myStack.top();
                myStack.pop();
                myStack.push(temp);
                break;

            case CODE_ISUB:
                temp = myStack.top();
                myStack.pop();
                temp -= myStack.top();
                myStack.pop();
                myStack.push(0-temp);
                break;

            case CODE_ISHL:
                temp2 = myStack.top();
                myStack.pop();
                temp = myStack.top();
                myStack.pop();
                temp2 = temp2 & 0b1111;
                temp = temp << temp2;
                myStack.push(temp);
                break;

            case CODE_ISHR:
                temp2 = myStack.top();
                myStack.pop();
                temp = myStack.top();
                myStack.pop();
                temp2 = temp2 & 0b11111;
                temp = temp >> temp2;
                myStack.push(temp);
                break;

            case CODE_IINC:
                temp = (codes[i].para & 0b11111111);
                temp2 = (codes[i].para >> 8);
                variables[temp2] += temp;
                break;

            case CODE_BIPUSH:
                myStack.push(codes[i].para);
                break;

            case CODE_ICMPEQ:
            case CODE_ICMPNE:
            case CODE_ICMPLT:
            case CODE_ICMPGE:
            case CODE_ICMPGT:
            case CODE_ICMPLE:
                offset = (int16_t)codes[i].para;
                temp2 = myStack.top();
                myStack.pop();
                temp = myStack.top();
                myStack.pop();
                if (codes[i].op_code == CODE_ICMPEQ && temp == temp2) i += (offset - 1);
                else if (codes[i].op_code == CODE_ICMPNE && temp != temp2) i += (offset - 1);
                else if (codes[i].op_code == CODE_ICMPLT && temp < temp2) i += (offset - 1);
                else if (codes[i].op_code == CODE_ICMPGE && temp >= temp2) i += (offset - 1);
                else if (codes[i].op_code == CODE_ICMPGT && temp > temp2) i += (offset - 1);
                else if (codes[i].op_code == CODE_ICMPLE && temp <= temp2) i += (offset - 1);      
                break;

            case CODE_IFEQ:
            case CODE_IFNE:
            case CODE_IFLT:
            case CODE_IFGE:
            case CODE_IFGT:
            case CODE_IFLE:
                offset = (int16_t)codes[i].para;
                temp = myStack.top();
                myStack.pop();
                if (codes[i].op_code == CODE_IFEQ && temp == 0) i += (offset - 1);
                else if (codes[i].op_code == CODE_IFNE && temp != 0) i += (offset - 1);
                else if (codes[i].op_code == CODE_IFLT && temp < 0) i += (offset - 1);
                else if (codes[i].op_code == CODE_IFGE && temp >= 0) i += (offset - 1);
                else if (codes[i].op_code == CODE_IFGT && temp > 0) i += (offset - 1);
                else if (codes[i].op_code == CODE_IFLE && temp <= 0) i += (offset - 1);
                break;

            case CODE_GOTO:
                offset = (int16_t)codes[i].para;
                //cout << offset << " " << i << " ";
                i += offset - 1;
                //cout << i + 1 << endl;
                break;

            case CODE_INVOKESTATIC:
                invokeMethod(codes,i);
                break;

            case CODE_INVOKEVIRTUAL:
                cout << "INVOKED VIRTUAL: " << myStack.top() << endl;
                myStack.pop();
                break;
        }
        ++i;
        while (codes[i].op_code == 0 && i < MAX_LINES)
            ++i;
    }
}

void interprete(parsed_class_info* &parsedClass)
{

    class_info = parsedClass;
    constant_pool = parsedClass->constant_pool;
    methods = parsedClass->methods;

    method_info* mainMethod = findMethod(MAIN_METHOD);
    code_attribute* mainCode = (code_attribute*)findCodeOfMethod(mainMethod);

    code_line codes[MAX_LINES];
	for (int i = 0; i < MAX_LINES; i++)
	{
		codes[i].op_code = 0;	
		codes[i].para = 0;
	}	

    parseCode(codes, mainCode);

    interpreteCode(codes);
    
    /*
    cout << endl << "CODE: " << endl;
    
    for (int i = 0; i < mainCode->get_code_length(); i++)
    {
        cout << (int)(*(mainCode->get_code() + i)) << " ";
    }
    cout << endl << "MAX_LOCALS: " << mainCode->get_max_locals();
    cout << endl << "MAX_STACK: " << mainCode->get_max_stack() << endl;
    */

}
