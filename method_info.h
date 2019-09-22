#ifndef METHOD_INFO_H
#define METHOD_INFO_H

#include<cstdint>


struct attribute_info
{
    uint16_t attribute_name_index;
    uint32_t attribute_length;
    uint8_t* info;

    uint8_t getInfo(int i)  { return info[i]; }
	~attribute_info()
	{
		delete[] info;
	}
};

struct code_attribute : attribute_info
{
    /*
    uint16_t attribute_name_index;
    uint32_t attribute_length;
    uint16_t max_stack;
    uint16_t max_locals;
    uint32_t code_length;
    uint8_t* code;
    */
    uint16_t get_max_stack() { return ((getInfo(0) << 8) | getInfo(1));}
    
    uint16_t get_max_locals() { return ((getInfo(2) << 8) | getInfo(3));}

    uint32_t get_code_length() { return ((((getInfo(4) << 24) 
        | (getInfo(5) << 16)) | (getInfo(6) << 8)) | getInfo(7)); }
        
    uint8_t* get_code() { return (info + 8); }
};

struct method_info
{
    uint16_t acess_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    uint16_t attributes_count;
    attribute_info** attributes;
	
	~method_info()
	{
		for (int i = 0; i < attributes_count; i ++)
			delete attributes[i];
		delete[] attributes;
	}
};

struct field_info
{
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    uint16_t attributes_count;
};
#endif
