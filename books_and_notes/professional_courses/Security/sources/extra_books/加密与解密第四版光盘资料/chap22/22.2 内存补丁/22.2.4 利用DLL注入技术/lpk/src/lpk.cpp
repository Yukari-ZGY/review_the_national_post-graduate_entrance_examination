/*--------------------------------------------------------------
   lpk.cpp -- DLL劫持
   Modified by kanxue ,
   Thanks CoDe_Inject!
   Thanks yonsm( AheadLib) !
   Thanks Backer!

第22章  补丁技术
《加密与解密（第四版）》
(c)  看雪学院 www.kanxue.com 2000-2018
  --------------------------------------------------------------*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXTERNC extern "C"
#define NAKED __declspec(naked)
#define EXPORT __declspec(dllexport)

#define ALCPP EXPORT NAKED
#define ALSTD EXTERNC EXPORT NAKED void __stdcall
#define ALCFAST EXTERNC EXPORT NAKED void __fastcall
#define ALCDECL EXTERNC EXPORT NAKED void __cdecl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <TlHelp32.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 补丁程序 
/*
去除这个CrackMe网络验证方法参考第5章，将相关补丁代码存放到函数PatchProcess( )里，需要补丁的地址汇总如下。
1）将401496h改成：
00401496   EB 29    jmp     short 004014C1
补丁编程实现就是：
	unsigned char p401496[2] = {0xEB, 0x29};
	WriteProcessMemory(hProcess,(LPVOID)0x401496, p401496, 2, NULL);
2）将40163Ch改成
0040163C   E8 67E40000   call    0040FAA8
补丁编程实现就是：
	unsigned char p40163C[5] = {
		0xE8, 0x67, 0xE4, 0x00, 0x00
	};
	WriteProcessMemory(hProcess, (LPVOID)0x40163C, p40163C, 5, NULL);
3）将00401655h改成：
00401655   EB 67         jmp     short 004016BE
补丁编程实现就是：
unsigned char p401655[2] = {0xEB, 0x67};
	WriteProcessMemory(hProcess, (LPVOID)0x401655, p401655, 2, NULL);
4）将40FAA8h 改成：
0040FAA8  50            	push   	eax
0040FAA9  8A85 ACFDFFFF	mov   	al, byte ptr [ebp-254]
0040FAAF  A2 76AE4100  	mov    	byte ptr [41AE76], al
0040FAB4  58             	pop   	eax
0040FAB5  C2 1000      	retn  	10
补丁编程实现就是：
unsigned char p40FAA8[16] = {
	0x50, 0x8A, 0x85, 0xAC, 0xFD, 0xFF, 0xFF, 0xA2, 0x76, 0xAE, 0x41, 0x00,
 0x58, 0xC2, 0x10, 0x00
	};
	WriteProcessMemory(hProcess, (LPVOID)0x40FAA8, p40FAA8, 16, NULL);
5）0041AE68改成：
0041AE68  14 15 00 00 D5 07 09 00 01 00 13 00 03 00 6D 00    ..?.. . . .m.
0041AE78  11 00 BB 00 91 53 01 00 21 61 00 00 1E 00 C5 0B   .?慡 .!a.. .?
0041AE88  C9 0B 30 BD 97 88 8E 00 BE 19 00 00 D4 12 00 00  ?0綏垘.?..?..
0041AE98  6F 35 E1 52 51 A4 B7 07 76 E7 D4 A1 43 98 88 D6  o5酭Qしv缭槇 
0041AEA8  45 FF C6 B1 43 66 77 98 77 67 54 66 77 53 64 58  E�票Cfw榳gTfwSdX
0041AEB8  6C 66 05 08 60 16 30 B4 AA 54 00 00 00 00 00 00  lf  ` 0椽T......
补丁编程实现就是：
unsigned char p41AE68[90] =
{
0x14, 0x15, 0x00, 0x00, 0xD5, 0x07, 0x09, 0x00, 0x01, 0x00, 0x13, 0x00, 0x03, 
0x00, 0x6D, 0x00, 0x11, 0x00, 0xBB, 0x00, 0x91, 0x53, 0x01, 0x00, 0x21, 0x61,
0x00, 0x00, 0x1E, 0x00, 0xC5, 0x0B, 0xC9, 0x0B, 0x30, 0xBD, 0x97, 0x88, 0x8E, 
0x00, 0xBE, 0x19, 0x00, 0x00, 0xD4, 0x12, 0x00, 0x00, 0x6F, 0x35, 0xE1, 0x52, 
0x51, 0xA4, 0xB7, 0x07, 0x76, 0xE7, 0xD4, 0xA1, 0x43, 0x98, 0x88, 0xD6, 0x45,
 0xFF, 0xC6, 0xB1, 0x43, 0x66, 0x77, 0x98, 0x77, 0x67, 0x54, 0x66, 0x77, 0x53,
 0x64, 0x58, 0x6C, 0x66, 0x05, 0x08, 0x60, 0x16, 0x30, 0xB4, 0xAA, 0x54, 
} ;
WriteProcessMemory(hProcess, (LPVOID)0x41AE68, p41AE68, 90, NULL);
如上代码格式，可以用OllyDBG插件获得，或十六进制工具转换。例如Hex Workshop打开文件，执行菜单Edit/Copy As/Source即可得到相应的代码格式。

*/
void PatchProcess(HANDLE hProcess)
{
	DWORD Oldpp;
	
	/************************************************************************/
	/* 补丁1：00401496     EB 29         jmp     short 004014C1             */
	/************************************************************************/
	unsigned char p401496[2] = {  
		0xEB, 0x29
	};
	VirtualProtectEx(hProcess, (LPVOID)0x401496, 2, PAGE_EXECUTE_READWRITE, &Oldpp);
	WriteProcessMemory(hProcess, (LPVOID)0x401496, p401496, 2, NULL);
	
	
	/************************************************************************/
	/* 补丁2：0040163C   E8 67E40000   call    0040FAA8                     */
	/************************************************************************/
	unsigned char p40163C[5] = {
		0xE8, 0x67, 0xE4, 0x00, 0x00
	};
	VirtualProtectEx(hProcess, (LPVOID)0x40163, 5, PAGE_EXECUTE_READWRITE, &Oldpp);
	WriteProcessMemory(hProcess, (LPVOID)0x40163C, p40163C, 5, NULL);
	
	
	/************************************************************************/
	/* 补丁3：00401655  EB 67         jmp     short 004016BE                */
	/************************************************************************/
	unsigned char p401655[2] = {
		0xEB, 0x67
	};
	VirtualProtectEx(hProcess, (LPVOID)0x401655, 2, PAGE_EXECUTE_READWRITE, &Oldpp);
	WriteProcessMemory(hProcess, (LPVOID)0x401655, p401655, 2, NULL);
	
	/************************************************************************/
	/*补丁4：                                                               */
	/*0040FAA8  50            push    eax                                   */
	/*0040FAA9  8A85 ACFDFFFF mov     al, byte ptr [ebp-254]                */
	/*0040FAAF  A2 76AE4100   mov     byte ptr [41AE76], al                 */
	/*0040FAB4  58            pop     eax                                   */
	/*0040FAB5  C2 1000       retn    10	                                */
	/************************************************************************/
	
	unsigned char p40FAA8[16] = {
		0x50, 0x8A, 0x85, 0xAC, 0xFD, 0xFF, 0xFF, 0xA2, 0x76, 0xAE, 0x41, 0x00, 0x58, 0xC2, 0x10, 0x00
	};
	VirtualProtectEx(hProcess, (LPVOID)0x40FAA8, 16, PAGE_EXECUTE_READWRITE, &Oldpp);
	WriteProcessMemory(hProcess, (LPVOID)0x40FAA8, p40FAA8, 16, NULL);
	
	/*****************************************************************************/
	/* 补丁5：                                                                   */
	/*0041AE68  14 15 00 00 D5 07 09 00 01 00 13 00 03 00 6D 00  ....?........m. */
	/*0041AE78  11 00 BB 00 91 53 01 00 21 61 00 00 1E 00 C5 0B  ..?慡..!a....?  */
	/*0041AE88  C9 0B 30 BD 97 88 8E 00 BE 19 00 00 D4 12 00 00  ?0綏垘.?..?..   */
	/*0041AE98  6F 35 E1 52 51 A4 B7 07 76 E7 D4 A1 43 98 88 D6  o5酭Qし.v缭槇.*/
	/*0041AEA8  45 FF C6 B1 43 66 77 98 77 67 54 66 77 53 64 58  E�票Cfw榳gTfwSdX*/
	/*0041AEB8  6C 66 05 08 60 16 30 B4 AA 54 00 00 00 00 00 00  lf..`.0椽T......*/
	/*****************************************************************************/
	unsigned char p41AE68[90] =
	{
		0x14, 0x15, 0x00, 0x00, 0xD5, 0x07, 0x09, 0x00, 0x01, 0x00, 0x13, 0x00, 0x03, 0x00, 0x6D, 0x00, 
		0x11, 0x00, 0xBB, 0x00, 0x91, 0x53, 0x01, 0x00, 0x21, 0x61, 0x00, 0x00, 0x1E, 0x00, 0xC5, 0x0B, 
		0xC9, 0x0B, 0x30, 0xBD, 0x97, 0x88, 0x8E, 0x00, 0xBE, 0x19, 0x00, 0x00, 0xD4, 0x12, 0x00, 0x00, 
		0x6F, 0x35, 0xE1, 0x52, 0x51, 0xA4, 0xB7, 0x07, 0x76, 0xE7, 0xD4, 0xA1, 0x43, 0x98, 0x88, 0xD6, 
		0x45, 0xFF, 0xC6, 0xB1, 0x43, 0x66, 0x77, 0x98, 0x77, 0x67, 0x54, 0x66, 0x77, 0x53, 0x64, 0x58, 
		0x6C, 0x66, 0x05, 0x08, 0x60, 0x16, 0x30, 0xB4, 0xAA, 0x54
	} ;
	VirtualProtectEx(hProcess, (LPVOID)0x41AE68, 90, PAGE_EXECUTE_READWRITE, &Oldpp);	
	WriteProcessMemory(hProcess, (LPVOID)0x41AE68, p41AE68, 90, NULL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 判断是否是目标程序   
BOOL isTarget(HANDLE hProcess)
{
	DWORD Targetcode = NULL;
	if (ReadProcessMemory(hProcess, (LPVOID)0x401484, &Targetcode, 4, NULL))
	{		
		if (Targetcode == 0x000543e8)//从目标程序随机取个点，本例为00401484  E8 43050000  call <jmp.&WS2_32.#4>
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void hijack()
{
	if (isTarget(GetCurrentProcess()))
	{
		PatchProcess(GetCurrentProcess());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//过滤处理ws2_32.dll各输出函数



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MemCode 命名空间
namespace MemCode
{
	HMODULE m_hModule = NULL;		// 原始模块句柄
	DWORD m_dwReturn[500] = {0};	// 原始函数返回地址
	
	// 加载原始模块
	inline BOOL WINAPI Load()
	{
		TCHAR tzPath[MAX_PATH]={0};
		TCHAR tzTemp[MAX_PATH]={0};
		
		GetSystemDirectory(tzPath, sizeof(tzPath));
		strcat(tzPath,"\\lpk.dll");
		m_hModule = LoadLibrary(tzPath);
		if (m_hModule == NULL)
		{
			wsprintf(tzTemp, TEXT("无法加载 %s，程序无法正常运行。"), tzPath);
			MessageBox(NULL, tzTemp, TEXT("MemCode"), MB_ICONSTOP);
		}

		return (m_hModule != NULL);	
	}
	
	// 释放原始模块
	inline VOID WINAPI Free()
	{
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
		}
	}
	
	// 获取原始函数地址
	FARPROC WINAPI GetAddress(PCSTR pszProcName)
	{
		FARPROC fpAddress;
		TCHAR szProcName[16]={0};
		TCHAR tzTemp[MAX_PATH]={0};
		
		if (m_hModule == NULL)
		{
			if (Load() == FALSE)
			{
				ExitProcess(-1);
			}
		}
		
		fpAddress = GetProcAddress(m_hModule, pszProcName);
		if (fpAddress == NULL)
		{
			if (HIWORD(pszProcName) == 0)
			{
				wsprintf(szProcName, "%d", pszProcName);
				pszProcName = szProcName;
			}
			
			wsprintf(tzTemp, TEXT("无法找到函数 %hs，程序无法正常运行。"), pszProcName);
			MessageBox(NULL, tzTemp, TEXT("MemCode"), MB_ICONSTOP);
			ExitProcess(-2);
		}
		
		return fpAddress;
	}
}
using namespace MemCode;
////////////////////////////////////////////////////////////////////////////////////////////////
//LpkEditControl导出的是数组，不是单一的函数 （by Backer）

EXTERNC EXPORT void __cdecl MemCode_LpkEditControl(void);
EXTERNC __declspec(dllexport) void (*LpkEditControl[14])() = {MemCode_LpkEditControl};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 入口函数

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			m_dwReturn[i] = TlsAlloc();
		}
		//LpkEditControl这个数组有14个成员，必须将其复制过来
		memcpy(LpkEditControl+1, (int*)GetAddress("LpkEditControl") + 1,sizeof(LpkEditControl) - 1);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			TlsFree(m_dwReturn[i]);
		}

		Free();
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
#pragma comment(linker, "/EXPORT:LpkInitialize=_MemCode_LpkInitialize,@1")
#pragma comment(linker, "/EXPORT:LpkTabbedTextOut=_MemCode_LpkTabbedTextOut,@2")
#pragma comment(linker, "/EXPORT:LpkDllInitialize=_MemCode_LpkDllInitialize,@3")
#pragma comment(linker, "/EXPORT:LpkDrawTextEx=_MemCode_LpkDrawTextEx,@4")
//#pragma comment(linker, "/EXPORT:LpkEditControl=_MemCode_LpkEditControl,@5")
#pragma comment(linker, "/EXPORT:LpkExtTextOut=_MemCode_LpkExtTextOut,@6")
#pragma comment(linker, "/EXPORT:LpkGetCharacterPlacement=_MemCode_LpkGetCharacterPlacement,@7")
#pragma comment(linker, "/EXPORT:LpkGetTextExtentExPoint=_MemCode_LpkGetTextExtentExPoint,@8")
#pragma comment(linker, "/EXPORT:LpkPSMTextOut=_MemCode_LpkPSMTextOut,@9")
#pragma comment(linker, "/EXPORT:LpkUseGDIWidthCache=_MemCode_LpkUseGDIWidthCache,@10")
#pragma comment(linker, "/EXPORT:ftsWordBreak=_MemCode_ftsWordBreak,@11")
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkInitialize(void)
{

	GetAddress("LpkInitialize");
	__asm JMP EAX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkTabbedTextOut(void)
{

	GetAddress("LpkTabbedTextOut");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkDllInitialize(void)
{

	GetAddress("LpkDllInitialize");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkDrawTextEx(void)
{

	GetAddress("LpkDrawTextEx");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkEditControl(void)
{

	GetAddress("LpkEditControl");
	__asm JMP DWORD ptr [EAX];
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkExtTextOut(void)
{

	GetAddress("LpkExtTextOut");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkGetCharacterPlacement(void)
{

	GetAddress("LpkGetCharacterPlacement");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkGetTextExtentExPoint(void)
{	

    hijack();
//	__asm{int 3};
	GetAddress("LpkGetTextExtentExPoint");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkPSMTextOut(void)
{	

	GetAddress("LpkPSMTextOut");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_LpkUseGDIWidthCache(void)
{	

	GetAddress("LpkUseGDIWidthCache");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ALCDECL MemCode_ftsWordBreak(void)
{	

	GetAddress("ftsWordBreak");
	__asm JMP EAX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

