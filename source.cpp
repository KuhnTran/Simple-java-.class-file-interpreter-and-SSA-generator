#include "parsed_class_info.h"
#include "parser.h"
#include "interpreter.h"
#include "genssa.h"
#include<iostream>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc < 2) 
    {
        cout << "ERROR: must include .class file path" << endl;
        exit(1);
    }
    parsed_class_info* myParsedClass = parseFile(argv[1]);
   	cout << "Interpretation: " << endl; 
    interprete(myParsedClass);

	cout << endl << "Generate SSA: " << endl;
    genSSA(myParsedClass);

	delete myParsedClass;	
	//clean up heap memory after operations
	/*
	for (int i = 0; i < myParsedClass->num_of_consts; i++)
		delete myParsedClass->constant_pool[i];
	delete[] myParsedClass->constant_pool;
*/
    return 0;
}
