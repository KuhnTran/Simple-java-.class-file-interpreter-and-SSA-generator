#ifndef PARSED_CLASS_INFO_H
#define PARSED_CLASS_INFO_H

#include "cp_info.h"
#include "method_info.h"

struct parsed_class_info
{
    int num_of_consts;
    int num_of_methods;
    cp_info** constant_pool;
    method_info** methods;

    parsed_class_info(cp_info** &cp, method_info** &me, int constNum, int metNum)  
	{
        constant_pool = cp;
		methods = me;
		num_of_consts = constNum;
		num_of_methods = metNum; 
	
	}
	~parsed_class_info()
	{
		for (int i = 0; i < num_of_consts; i++)
			delete constant_pool[i];

		for (int i = 0; i < num_of_methods; i++)
			delete methods[i];

		delete[] constant_pool;
		delete[] methods;
	}
};

#endif
