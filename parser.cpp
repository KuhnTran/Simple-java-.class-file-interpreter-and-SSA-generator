#include "parsed_class_info.h"
#include "method_info.h"
#include "cp_info.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <iomanip>
#include <stdio.h>

using namespace std;

#define BUFF_MAX 16
#define TWO_BYTES 2
#define THREE_BYTES 3
#define FOUR_BYTES 4
#define EIGHT_BYTES 8

ifstream in;

void skipBytes(int bytes)
{
   char c;
   for (int i = 0; i < bytes; i++)
   {
      in.get(c);
   }
}

int getFourBytesSize()
{
   char c, d, e,f;
   in.get(c);
   in.get(d);
   in.get(e);
   in.get(f);
   return (((int)c << 24) | ((int)d << 16) | ((int)e << 8) | (int)f);
}

int getTwoBytesSize()
{
   char c='\0', d='\0';
   in.get(c);
   in.get(d);
   return ((static_cast<uint>(c) << 8) | static_cast<uint>(d));
}

int getOneByteSize()
{
   char c='\0';
   in.get(c);
   return static_cast<uint>(c);
}

uint8_t* getExtraBytes(int numOfBytes)
{
   char c='\0';
   uint8_t* extraBytes = new uint8_t[numOfBytes];
   for (int i = 0; i < numOfBytes; i++)
   {
      in.get(c);
      extraBytes[i] = static_cast<uint8_t>(c);
   }
   return extraBytes;
}

void addToPool(uint8_t tag, uint8_t* extraBytes, 
                  cp_info** &pool, int i)
{
   switch(tag)
   {
      case 7:
         pool[i] = new cp_class_info(tag, extraBytes);
      break;

      case 9:
         pool[i] = new cp_fieldref_info(tag, extraBytes);
      break;

      case 10: 
         pool[i] = new cp_methodref_info(tag, extraBytes);
      break;

      case 11:
         pool[i] = new cp_integer_info(tag, extraBytes);
      break;

      case 8:
         pool[i] = new cp_string_info(tag, extraBytes);
      break;

      case 3: 
         pool[i] = new cp_integer_info(tag, extraBytes);
      break;

      case 4: 
         pool[i] = new cp_float_info(tag, extraBytes);
      break;

      case 5:
         pool[i] = new cp_long_info(tag, extraBytes);
      break;

      case 6:
         pool[i] = new cp_double_info(tag, extraBytes);
      break;

      case 12:
         pool[i] = new cp_nameandtype_info(tag, extraBytes);
      break;

      case 15:
         pool[i] = new cp_methodhandle_info(tag,extraBytes);
      break;

      case 16: 
         pool[i] = new cp_methodtype_info(tag, extraBytes);
      break;

      case 18:
         pool[i] = new cp_invokedynamic_info(tag, extraBytes);
      break;

      case 1:
         pool[i] = new cp_utf8_info(tag, extraBytes);
   }
}

void parseConstantPool(cp_info** &pool, int size)
{
   char c;
   int index = 1;
   uint8_t tagByte;
   uint8_t* extraBytes;

   while(index < size)
   {
      //cout << "Index " << index << ": ";

      in.get(c);
      tagByte = static_cast<uint8_t>(c);
      switch (tagByte)
      {
         case 7: case 8: case 16:
            //cout << hex << (int)c << " Case 1" << endl;
            extraBytes = getExtraBytes(TWO_BYTES);
         break;

         case 15:
            //cout << hex << (int)c << " Case 2"  << endl;;
            extraBytes = getExtraBytes(THREE_BYTES);
         break; 

         case 9: case 10: case 11: case 3: case 4: case 12: case 18:
            //cout << hex << (int)c << " Case 3"  << endl;
            extraBytes = getExtraBytes(FOUR_BYTES);
         break;

         case 5: case 6:
            //cout << hex << (int)c << " Case 4" << endl;
            extraBytes = getExtraBytes(EIGHT_BYTES);
         break;
         
         case 1:
            //cout << hex << (int)c << " Case 5 ";

            uint16_t length = 0;
            uint8_t temp1 = 0, temp2 = 0;

            in.get(c);
            length += static_cast<uint8_t>(c);
            temp1 = static_cast<uint8_t>(c);

            in.get(c);
            length = length << 8;
            length += static_cast<uint8_t>(c);
            temp2 = static_cast<uint8_t>(c);

            extraBytes = new uint8_t[length+2];
            extraBytes[0] = temp1;
            extraBytes[1] = temp2;

            //cout << (int)temp1 << " " << (int)temp2 << endl;

            for (int i = 0; i < length; i++)
            {
               in.get(c);
               extraBytes[i+2]=static_cast<uint8_t>(c);
               //cout << c;
            }
            //cout << endl;
         break;
      }
      addToPool(tagByte, extraBytes, pool, index);
      ++index;
   }   
}

attribute_info** parseAttribute(int AttrSize)
{
   attribute_info** attributes = new attribute_info*[AttrSize];
   int i = 0;
   while (i < AttrSize)
   {
      attributes[i] = new attribute_info();
      attributes[i]->attribute_name_index = getTwoBytesSize();
      attributes[i]->attribute_length = getFourBytesSize();
      attributes[i]->info = new uint8_t[attributes[i]->attribute_length];
      for (int n = 0; n < attributes[i]->attribute_length; n++)
         attributes[i]->info[n] = getOneByteSize();
      ++i;
   }
   return attributes;
}

void parseMethod(int numOfMethods, method_info** &methods)
{
   int i = 0;
   while(i < numOfMethods)
   {
      methods[i] = new method_info();
      methods[i]->acess_flags = getTwoBytesSize();
      methods[i]->name_index = getTwoBytesSize();
      methods[i]->descriptor_index = getTwoBytesSize();
      methods[i]->attributes_count = getTwoBytesSize();

      //cout << "Method " << i << " " << methods[i]->attributes_count << endl;

      methods[i]->attributes = parseAttribute(methods[i]->attributes_count);
      ++i;
   }
}

parsed_class_info* parseFile(const string &fileName)
{
   in.open(fileName, ios::in | ios::binary);
   if(!in)
   {
      //cout << "Cannot open input file." << endl;
      exit (1);
   }

   char c;

   //skip magic numbers and versions:
   skipBytes(8);
   
   //get number of constant and parse constant pool
   int numOfConst = getTwoBytesSize();
   cp_info** constantPool = new cp_info*[numOfConst]();
   parseConstantPool(constantPool, numOfConst);

   //get additional information
   int access_flags = getTwoBytesSize();
   int this_class = getTwoBytesSize();
   int super_class = getTwoBytesSize();
   int interfaces_count = getTwoBytesSize();
   uint16_t interfaces;
   for (int i = 0; i < interfaces_count; i++) interfaces = getTwoBytesSize();
   int numOfFields = 0;
	numOfFields = getTwoBytesSize(); 
	
   /*
   cout << "ACCESS_FLAGS " << access_flags << endl;
   cout << "THIS_CLASS " << this_class << endl;
   cout << "SUPER CLASS: " << super_class << endl;
   cout << "Interfaces Counts: " << interfaces_count << endl;
   cout << "NUM OF FIELDS: " << numOfFields << endl;
   */

   //get number of methods and parse methods
  	int numOfMethods = 0;
	numOfMethods = getTwoBytesSize();
   
   //cout << "NUM OF METHODS:  " << numOfMethods << endl;

   method_info** methods = new method_info*[numOfMethods];
   parseMethod(numOfMethods, methods);

   parsed_class_info* parsed_class = new parsed_class_info(constantPool, methods, numOfConst, numOfMethods);
   
   in.close();
   return parsed_class;
}
