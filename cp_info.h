#ifndef CP_INFO_H
#define CP_INFO_H

#include<cstdint>
#include<iostream>
#include"string.h"
using namespace std;

struct cp_info
{
	string myString;
    cp_info(uint8_t newTag, uint8_t* newInfo)
    {
        tag = newTag;
        info = newInfo;
		myString = "";
    }
    uint8_t tag;
    uint8_t* info;

    uint8_t getInfo(int i)  { return info[i]; }

	~cp_info()
	{
		delete[] info;
	}
};

struct cp_methodref_info : cp_info
{
    cp_methodref_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_class_index() { return ((getInfo(0) << 8) | getInfo(1));}
    uint16_t get_name_and_type_index() { return ((getInfo(2) << 8) | getInfo(3)); }
};

struct cp_fieldref_info : cp_info
{
    cp_fieldref_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_class_index() { return ((getInfo(0) << 8) | getInfo(1));}
    uint16_t get_name_and_type_index() { return ((getInfo(2) << 8) | getInfo(3)); }
};

struct cp_interfacemethodref_info : cp_info
{
    cp_interfacemethodref_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_class_index() { return ((getInfo(0) << 8) | getInfo(1));}
    uint16_t get_name_and_type_index() { return ((getInfo(2) << 8) | getInfo(3)); }
};

struct cp_string_info : cp_info
{
    cp_string_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_string_index() { return ((getInfo(0) << 8) | getInfo(1));}
};

struct cp_integer_info : cp_info
{
    cp_integer_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint32_t get_bytes() { return ((((getInfo(0) << 24) 
        | (getInfo(1) << 16)) | (getInfo(2) << 8)) | getInfo(3)); }
};

struct cp_float_info : cp_info
{
    cp_float_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint32_t get_bytes() { return ((((getInfo(0) << 24) 
        | (getInfo(1) << 16)) | (getInfo(2) << 8)) | getInfo(3)); }
};

struct cp_long_info : cp_info
{
    cp_long_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint32_t get_high_bytes() { return ((((getInfo(0) << 24) 
        | (getInfo(1) << 16)) | (getInfo(2) << 8)) | getInfo(3)); }
    uint32_t get_low_bytes() { return ((((getInfo(4) << 24) 
        | (getInfo(5) << 16)) | (getInfo(6) << 8)) | getInfo(7)); }
    uint64_t get_whole() {return (static_cast<uint64_t>(get_high_bytes()) << 32) | get_low_bytes(); }
};

struct cp_double_info : cp_info
{
    cp_double_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint32_t get_high_bytes() { return ((((getInfo(0) << 24) 
        | (getInfo(1) << 16)) | (getInfo(2) << 8)) | getInfo(3)); }
    uint32_t get_low_bytes() { return ((((getInfo(4) << 24) 
        | (getInfo(5) << 16)) | (getInfo(6) << 8)) | getInfo(7)); }
    uint64_t get_whole() {return (static_cast<uint64_t>(get_high_bytes()) << 32) | get_low_bytes(); }
};

struct cp_nameandtype_info : cp_info
{
    cp_nameandtype_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_name_index() { return ((getInfo(0) << 8) | getInfo(1)); }
    uint16_t get_descriptor_index() { return ((getInfo(2) << 8) | getInfo(3));}
};

struct cp_utf8_info : cp_info
{
    cp_utf8_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {
        initializeString();
    }
    uint16_t get_length() { return ((getInfo(0) << 8) | getInfo(1)); }
    uint8_t* get_bytes() { return (info + 2); }
    
	void initializeString()
    {
		string newString = "";
        for (int i = 0; i < get_length(); i++)
        {
            newString.push_back(static_cast<char>(*(info+2+i)));
        }
		myString = newString;
    }

    void print() {
        cout << myString;
    }
};

struct cp_methodhandle_info : cp_info
{
    cp_methodhandle_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint8_t get_reference_kind() { return getInfo(0); }
    uint16_t get_reference_index() { return (getInfo(1) << 8) | getInfo(2);}
};

struct cp_methodtype_info : cp_info
{
    cp_methodtype_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_descriptor_index() { return ((getInfo(2) << 8) | getInfo(3));}
};

struct cp_invokedynamic_info : cp_info
{
    cp_invokedynamic_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t boostrap_method_attr_index() { return ((getInfo(0) << 8) | getInfo(1)); }
    uint16_t name_and_type_index() { return ((getInfo(2) << 8) | getInfo(3));}
};

struct cp_class_info : cp_info
{
    cp_class_info(uint8_t newTag, uint8_t* newInfo) : cp_info(newTag, newInfo) {}
    uint16_t get_name_index() { return ((getInfo(0) << 8) | getInfo(1)); }
};

#endif
