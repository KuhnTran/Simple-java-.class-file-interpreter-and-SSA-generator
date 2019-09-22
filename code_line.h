#ifndef CODE_LINE_H
#define CODE_LINE_H

#define CODE_RETURN 0xb1
#define CODE_IRETURN 0xac
    
#define CODE_ICONST_M1 0x2
#define CODE_ICONST_0 0x3
#define CODE_ICONST_1 0x4
#define CODE_ICONST_2 0x5
#define CODE_ICONST_3 0x6
#define CODE_ICONST_4 0x7
#define CODE_ICONST_5 0x8

#define CODE_ILOAD_0 0x1a
#define CODE_ILOAD_1 0x1b
#define CODE_ILOAD_2 0x1c
#define CODE_ILOAD_3 0x1d

#define CODE_ISTORE_0 0x3b
#define CODE_ISTORE_1 0x3c
#define CODE_ISTORE_2 0x3d
#define CODE_ISTORE_3 0x3e

#define CODE_IADD 0x60
#define CODE_IMUL 0x68
#define CODE_IINC 0x84
#define CODE_ISUB 0x64
#define CODE_ISHL 0x78
#define CODE_ISHR 0x7a

#define CODE_ICMPEQ 0x9f
#define CODE_ICMPNE 0xa0
#define CODE_ICMPLT 0xa1
#define CODE_ICMPGE 0xa2
#define CODE_ICMPGT 0xa3
#define CODE_ICMPLE 0xa4

#define CODE_IFEQ 0x99
#define CODE_IFNE 0x9a
#define CODE_IFLT 0x9b
#define CODE_IFGE 0x9c
#define CODE_IFGT 0x9d
#define CODE_IFLE 0x9e


#define CODE_GOTO 0xa7
#define CODE_BIPUSH 0x10
#define CODE_INVOKESTATIC 0xb8
#define CODE_INVOKEVIRTUAL 0xb6


#define CODE_GETSTATIC 0xb2
struct code_line
{
    int op_code;
    int para;
};

#endif