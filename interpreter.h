#ifndef INTERPRETER_H
#define INTERPRETER_H

#include"parsed_class_info.h"
#include"code_line.h"
#include<stack>
#define MAX_LINES 1024
#define MAX_NUM_OF_VAR 4


void interprete(parsed_class_info* &);

method_info* findMethod(const string &methodName);

attribute_info* findCodeOfMethod(method_info* method);

void interpreteCode(code_line (&codes)[MAX_LINES]);

string nameOfMethodAt(int index);

#endif