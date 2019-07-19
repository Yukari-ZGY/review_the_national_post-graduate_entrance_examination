/*-----------------------------------------------------------------------
第12章  注入技术
《加密与解密（第四版）》
(c)  看雪学院 www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/

/*

  ProcessNotifyInject.C

  Author: achillis
  Last Updated: 2006-03-23

  This framework is generated by EasySYS 0.3.0
  This template file is copying from QuickSYS 0.3.0 written by Chunhua Liu
	
  Tested On: (x86) WinXP / Win7 / Win10 
             (x64) Win7  (可注入64位及Wow64程序)
*/

#include "dbghelp.h"
#include "ProcessNotifyInject.h"
#include "ntifs.h"
#include "Common.h"


//ForWow64,使用下面的数据类型来定义确保不管是32位系统还是64位系统，各项元素的偏移均不变
//当驱动在32位系统运行时，两个结构定义相同
typedef struct _INJECT_DATA_WOW64 
{
	BYTE ShellCode[0xC0];
	/*Off=0xC0*/ULONG ModuleHandle; //Dll句柄
	/*Off=0xC4*/ULONG pDllPath;//PUNICODE_STRING DllPath
	/*Off=0xC8*/ULONG DllCharacteristics;
	/*Off=0xCC*/ULONG AddrOfLdrLoadDll;//LdrLoadDll地址
	/*Off=0xD0*/ULONG ProtectBase; //用于VirtualMemory
	/*Off=0xD4*/ULONG OldProtect; //用于VirtualMemory
	/*Off=0xD8*/ULONG ProtectSize;
	/*Off=0xDC*/ULONG FuncEntry;//跳转点的地址
	/*Off=0xE0*/ULONG AddrOfZwProtectVirtualMemory;
	/*Off=0xE4*/BYTE  SavedEntryCode[16];//保存跳转点的前16字节
	/*Off=0xF4*/UNICODE_STRING32 usDllPath;//Dll路径
	/*Off=0xEC*/WCHAR wDllPath[256];//Dll路径，也就是usDllPath中的Buffer
}INJECT_DATA_WOW64;

typedef struct _INJECT_DATA_NATIVE 
{
	BYTE ShellCode[0xC0];
	/*Off=0x0C0*/HANDLE ModuleHandle; //Dll句柄
	/*Off=0x0C8*/PUNICODE_STRING pDllPath;//PUNICODE_STRING DllPath
	/*Off=0x0D0*/ULONG DllCharacteristics;
	/*Off=0x0D8*/ULONG_PTR AddrOfLdrLoadDll;//LdrLoadDll地址
	/*Off=0x0E0*/ULONG_PTR ProtectBase; //用于VirtualMemory
	/*Off=0x0E8*/ULONG OldProtect; //用于VirtualMemory
	/*Off=0x0F0*/SIZE_T ProtectSize;
	/*Off=0x0F8*/ULONG_PTR FuncEntry;//跳转点的地址
	/*Off=0x100*/ULONG_PTR AddrOfZwProtectVirtualMemory;
	/*Off=0x108*/BYTE  SavedEntryCode[16];//保存跳转点的前8/16字节
	/*Off=0x118*/UNICODE_STRING usDllPath;//Dll路径
	/*Off=0x128*/WCHAR wDllPath[256];//Dll路径，也就是usDllPath中的Buffer
}INJECT_DATA_NATIVE;

typedef NTSTATUS
(*PFN_ZwProtectVirtualMemory)(
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	PSIZE_T RegionSize,
	ULONG NewProtectWin32,
	PULONG OldProtect
    );

// Device driver routine declarations.
//

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	);

NTSTATUS
ProcessnotifyinjectDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
ProcessnotifyinjectDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
ProcessnotifyinjectDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

VOID
ProcessnotifyinjectUnload(
	IN PDRIVER_OBJECT		DriverObject
	);

VOID
MyCreateProcessNotifyRoutine(
	IN HANDLE  ParentId,
	IN HANDLE  ProcessId,
	IN BOOLEAN  Create
	);

BOOL InitGlobalVars();
NTSTATUS InjectShellCodeToProcess();
VOID BuildShellCode(BYTE *pOutCode , BOOL Is64Bit);
VOID BuildInjectDataForNativeUse(PVOID pData64 , ULONG_PTR uNtdll , WCHAR *pDllPath , ULONG OldProtect);
VOID BuildInjectDataForWow64Use(PVOID pData64 , ULONG_PTR uNtdll , WCHAR *pDllPath , ULONG OldProtect);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, ProcessnotifyinjectDispatchCreate)
#pragma alloc_text(PAGE, ProcessnotifyinjectDispatchClose)
#pragma alloc_text(PAGE, ProcessnotifyinjectDispatchDeviceControl)
#pragma alloc_text(PAGE, ProcessnotifyinjectUnload)
#endif // ALLOC_PRAGMA

char g_szProcNameToInject[16]="notepad.exe"; //准备注入的进程的名字
WCHAR g_64BitDllPathToInject[256] = L"C:\\MsgDll64.dll";
WCHAR g_32BitDllPathToInject[256] = L"C:\\MsgDll.dll";
PFN_ZwProtectVirtualMemory pfnZwProtectVirtualMemory;

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	)
{
	//
    // Create dispatch points for device control, create, close.
    //

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = ProcessnotifyinjectDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = ProcessnotifyinjectDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcessnotifyinjectDispatchDeviceControl;
    DriverObject->DriverUnload                         = ProcessnotifyinjectUnload;
	
	InitGlobalVars();
	PsSetCreateProcessNotifyRoutine(MyCreateProcessNotifyRoutine,FALSE);
	DbgPrint("[ProcessNotifyInject] Loaded!\n");
    return STATUS_SUCCESS;
}

NTSTATUS
ProcessnotifyinjectDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

	dprintf("[ProcessNotifyInject] IRP_MJ_CREATE\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
ProcessnotifyinjectDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

	dprintf("[ProcessNotifyInject] IRP_MJ_CLOSE\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
ProcessnotifyinjectDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	dprintf("[ProcessNotifyInject] IRP_MJ_DEVICE_CONTROL\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

VOID
ProcessnotifyinjectUnload(
	IN PDRIVER_OBJECT		DriverObject
	)
{
	PsSetCreateProcessNotifyRoutine(MyCreateProcessNotifyRoutine,TRUE);
    DbgPrint("[ProcessNotifyInject] Unloaded\n");
}

BOOL InitGlobalVars()
{
	NTSTATUS status;
	UNICODE_STRING usDrvName;
	HANDLE CsrssPid;
	//初始化
	
	pfnZwProtectVirtualMemory=(PFN_ZwProtectVirtualMemory)GetZwProtectVirtualMemoryAddr();
	if (pfnZwProtectVirtualMemory==NULL)
	{
		DbgPrint("无法获取ZwProtectVirtualMemory的地址!\n");
		return FALSE;
	}
	else
	{
		DbgPrint("ZwProtectVirtualMemory Addr=0x%p\n",pfnZwProtectVirtualMemory);
	}
	return TRUE;
}


//此时进程的状态是，第一个线程已经创建但尚未返回
//exe和ntdll已经加载，但是PEB->Ldr尚未初始化
VOID
MyCreateProcessNotifyRoutine(
	IN HANDLE  ParentId,
	IN HANDLE  ProcessId,
	IN BOOLEAN  Create
	)
{
	char *szCurProc = NULL;
	char *szTargetProcName = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pTargetProc = NULL ;
	PETHREAD pTargetThread = NULL ;
	HANDLE hThreadID = 0 ;
	SIZE_T MemSize = 0x1000;
	PCONTEXT pContext = NULL ;
	
	dprintf("ParentId = %d  ProcessId = %d 当前动作：%s\n",
		ParentId,ProcessId,Create?"进程创建":"进程退出");
	
	if (Create)
	{
		status = PsLookupProcessByProcessId(ProcessId,&pTargetProc);
		if (NT_SUCCESS(status))
		{
			szTargetProcName = PsGetProcessImageFileName(pTargetProc);
			if (_stricmp(szTargetProcName,g_szProcNameToInject) == 0)
			{
				DbgPrint("目标进程正在创建，准备注入!\n");
				KeAttachProcess(pTargetProc);
				status = InjectShellCodeToProcess();
				if (NT_SUCCESS(status))
				{
					DbgPrint("向进程 %s 插入ShellCode成功!\n",szTargetProcName);
				}
				KeDetachProcess();
			}


			ObDereferenceObject(pTargetProc);
		}
	}
	
	
}

//本驱动相当于x86/x64混合编程，所以需要考虑不同系统下面指针长度的问题
NTSTATUS InjectShellCodeToProcess()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG_PTR uNtdllBase = 0 ;
	ULONG_PTR AddrOfLdrLoadDll = 0;
	ULONG_PTR AddrofNtTestAlert = 0 ;
	PVOID  pData = NULL;
	SIZE_T MemSize = 0x1000;
	SIZE_T ProtectSize = 0 ;
	ULONG_PTR ProtectBase = 0 ;
	ULONG uShellCodeSize = 0 ;
	ULONG OldProtect = 0 ;
	BYTE *pFunStart = NULL,*pFunEnd = NULL ,*pTemp = NULL;
	SIZE_T JmpCodeSize = 0 ;
	BYTE JmpCode[16] ={0};
	BOOL bWow64Process = FALSE ;
	PWCHAR pDllToInject = NULL ;
	char *szImageFileName = PsGetProcessImageFileName(NtCurrentProcess());

	DbgPrint("[InjectShellCodeToProcess] CurrentProcess : %s\n",szImageFileName);

#ifdef _AMD64_
	//64位系统中，需要判断目标进程是32位还是64位，从而决定注入哪个dll
	status = IsWow64Process(NtCurrentProcess(),&bWow64Process);
	if (!NT_SUCCESS(status))
	{
		//如果查询失败，可能是系统不支持wow64，也就是说，肯定不是 wow64进程了
		bWow64Process = FALSE;
	}

	//根据是否wow64进程进行区分处理
	if (bWow64Process)
	{
		dprintf("目标进程是 Wow64 进程!\n");

		pDllToInject = g_32BitDllPathToInject ;
		pData = (INJECT_DATA_WOW64*)pData;
		//取得ntdll的基址,如果是Wow64进程，此时32位的ntdll也已经加载
		uNtdllBase = FindImageBase(NtCurrentProcess(),L"SysWow64\\ntdll.dll");
		AddrofNtTestAlert = KeWow64GetProcAddress((ULONG)uNtdllBase,"ZwTestAlert");
	}
	else
	{
		dprintf("目标进程不是 Wow64 进程!\n");
		pDllToInject = g_64BitDllPathToInject ;
		pData = (INJECT_DATA_NATIVE*)pData;
		uNtdllBase = FindImageBase(NtCurrentProcess(),L"System32\\ntdll.dll");
		AddrofNtTestAlert = KeGetProcAddress(uNtdllBase,"ZwTestAlert");
	}
#else
	pDllToInject = g_32BitDllPathToInject ;
	uNtdllBase = FindImageBase(NtCurrentProcess(),L"ntdll.dll");  //32位系统进程中只有一个ntdll,所以不需要区分
	AddrofNtTestAlert = KeGetProcAddress(uNtdllBase,"ZwTestAlert");
#endif
	
	if (uNtdllBase != 0)
	{
		//先申请内存
		status = ZwAllocateVirtualMemory(NtCurrentProcess(),
			&pData,0,&MemSize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		
		if (NT_SUCCESS(status))//AllocMem
		{
			dprintf("Allocated Mem = 0x%p\n",pData);
			dprintf("ntdll.dll Base = 0x%p \n",uNtdllBase);
			dprintf("ZwTestAlert = 0x%p \n",AddrofNtTestAlert);

			//修改ZwTestAlert函数的页属性
			ProtectBase = AddrofNtTestAlert ;
			ProtectSize = sizeof(ULONG_PTR)*2 ;
			status = pfnZwProtectVirtualMemory(NtCurrentProcess(),(PVOID*)&ProtectBase,
				&ProtectSize,PAGE_EXECUTE_READWRITE,&OldProtect);
			
			if (NT_SUCCESS(status)) //ProtectMem
			{
				dprintf("ZwTestAlert 页属性修改成功! OldProtect = 0x%X\n",OldProtect);
#ifdef _AMD64_
				if (bWow64Process)
				{
					BuildInjectDataForWow64Use(pData,uNtdllBase,pDllToInject,OldProtect);
				}
				else
				{
					BuildInjectDataForNativeUse(pData,uNtdllBase,pDllToInject,OldProtect);
				}
#else
				BuildInjectDataForNativeUse(pData,uNtdllBase,pDllToInject,OldProtect);
#endif
				
				//构造call指令以转移到ShellCode
				ZeroMemory(JmpCode,16);
#ifdef _AMD64_
				if (!bWow64Process)
				{
					JmpCodeSize = 14 ;
					JmpCode[0] = 0xFF;
					JmpCode[1] = 0x15;	//"\xFF\x15\x00\x00\x00\x00"; //call [next]
					*(ULONG_PTR*)(JmpCode + 6 ) = (ULONG_PTR)pData; //ShellCode地址，就在申请的内存开头
				}
				else
				{
					JmpCodeSize = 5 ;
					JmpCode[0] = 0xE8;	//"\xE8\x11\x11\x11\x11"; //call shellcode
					*(ULONG*)(JmpCode + 1 ) = (ULONG)((ULONG_PTR)pData - (ULONG_PTR)AddrofNtTestAlert - 5); //ShellCode地址，就在申请的内存开头
				}
				
#else
				JmpCodeSize = 5 ;
				JmpCode[0] = 0xE8;	//"\xE8\x11\x11\x11\x11"; //call shellcode
				*(ULONG_PTR*)(JmpCode + 1 ) = (ULONG_PTR)pData - AddrofNtTestAlert - 5 ; //ShellCode地址，就在申请的内存开头
#endif
				dprintf("[*] Before WriteCode , AddrOfNtTestAlert = %p\n",AddrofNtTestAlert);
				//写入函数开头
				memcpy((PVOID)AddrofNtTestAlert,JmpCode,JmpCodeSize);
				dprintf("[*] Write Code To NtTestAlert OK.\n");
				status = STATUS_SUCCESS ;

			}
			else
			{
				DbgPrint("无法修改ntdll!ZwTestAlert的页属性!\n");
			}
			
		}
		else
		{
			DbgPrint("申请内存失败! status = 0x%08X\n",status);
		}
		
	}
	
	return status;
}

//仅Wow64时使用
#ifdef _AMD64_
VOID BuildInjectDataForWow64Use(PVOID pData32 , ULONG_PTR uNtdll , WCHAR *pDllPathToInject , ULONG OldProtect)
{
	ULONG uNtdllBase = (ULONG)uNtdll;
	INJECT_DATA_WOW64 *pData = (INJECT_DATA_WOW64*)pData32;

	//获取并保存函数地址
	pData->AddrOfLdrLoadDll = KeWow64GetProcAddress(uNtdllBase,"LdrLoadDll");
	pData->AddrOfZwProtectVirtualMemory = KeWow64GetProcAddress(uNtdllBase,"ZwProtectVirtualMemory");
	pData->FuncEntry = KeWow64GetProcAddress(uNtdllBase,"ZwTestAlert");
	
	//保存ShellCode,32位
	BuildShellCode(pData->ShellCode,FALSE);
				
	//初始化DllCharacteristics
	pData->DllCharacteristics = 0 ;

	//初始化UNICODE_STRING
	wcscpy(pData->wDllPath,pDllPathToInject);
	pData->usDllPath.Buffer = (ULONG)(ULONG_PTR)pData->wDllPath;
	pData->usDllPath.MaximumLength = 256 * sizeof(WCHAR);
	pData->usDllPath.Length = wcslen(pData->wDllPath) * sizeof(WCHAR);
	
	//设置PUNICODE_STRING
	pData->pDllPath = (ULONG)(ULONG_PTR)&pData->usDllPath;

	//保存函数入口
	memcpy(pData->SavedEntryCode,(PVOID)(ULONG_PTR)pData->FuncEntry,sizeof(ULONG)*2);
	//保存原来的属性
	pData->OldProtect = OldProtect;
	pData->ProtectSize = sizeof(ULONG)*2 ;
	pData->ProtectBase = pData->FuncEntry;
}
#endif


//对原生进程使用，32位就是32,64位就是64
VOID BuildInjectDataForNativeUse(PVOID pData64 , ULONG_PTR uNtdll , WCHAR *pDllPath , ULONG OldProtect)
{
	ULONG_PTR uNtdllBase = (ULONG_PTR)uNtdll;
	INJECT_DATA_NATIVE *pData = (INJECT_DATA_NATIVE*)pData64;
	
	//获取并保存函数地址
	pData->AddrOfLdrLoadDll = KeGetProcAddress(uNtdllBase,"LdrLoadDll");
	pData->AddrOfZwProtectVirtualMemory = KeGetProcAddress(uNtdllBase,"ZwProtectVirtualMemory");
	pData->FuncEntry = KeGetProcAddress(uNtdllBase,"ZwTestAlert");
	
	//保存ShellCode
#ifdef _AMD64_
	BuildShellCode(pData->ShellCode,TRUE);
#else
	BuildShellCode(pData->ShellCode,FALSE);
#endif
	
				
	//初始化DllCharacteristics
	pData->DllCharacteristics = 0 ;
	//初始化UNICODE_STRING
	wcscpy(pData->wDllPath,pDllPath);
	pData->usDllPath.Buffer = pData->wDllPath;
	pData->usDllPath.MaximumLength = 256 * sizeof(WCHAR);
	pData->usDllPath.Length = wcslen(pData->wDllPath) * sizeof(WCHAR);
	
	//设置PUNICODE_STRING
	pData->pDllPath = &pData->usDllPath;

	//保存函数入口
	memcpy(pData->SavedEntryCode,(PVOID)pData->FuncEntry,sizeof(ULONG_PTR)*2);

	//保存原来的属性
	pData->OldProtect = OldProtect;
	pData->ProtectSize = sizeof(ULONG_PTR)*2 ;
	pData->ProtectBase = pData->FuncEntry;
}

VOID BuildShellCode(BYTE *pOutCode , BOOL Is64Bit)
{
	BYTE *pShellcodeStart = NULL ;
	BYTE *pShellcodeEnd = 0 ;
	SIZE_T ShellCodeSize = 0 ;

	BYTE ShellCode32[128]=
		"\x60"							//pushad
		"\x9C"							//pushfd
		"\xE8\x00\x00\x00\x00"			//call 00000005
		"\x5B"							//pop ebx
		"\x66\x83\xE3\x00"				//and bx,0x0
		"\x3E\x8B\xBB\xDC\x00\x00\x00"	//mov edi,[ebx+0xDC]
		"\x3E\x8D\xB3\xE4\x00\x00\x00"	//lea esi,[ebx+0xE4]
		"\xB9\x02\x00\x00\x00"			//mov ecx,0x2
		"\xF3\xA5"						//rep movs dword ptr es:[edi],dword ptr [esi]
		"\x3E\x8D\x83\xD4\x00\x00\x00"	//lea eax,[ebx+0xD4]
		"\x50"							//push eax
		"\x3E\xFF\xB3\xD4\x00\x00\x00"	//push dword ptr [ebx+0xD4]
		"\x3E\x8D\x83\xD8\x00\x00\x00"	//lea eax,[ebx+0xD8]
		"\x50"							//push eax
		"\x3E\x8D\x83\xD0\x00\x00\x00"	//lea eax,[ebx+0xD0]
		"\x50"							//push eax
		"\x6A\xFF"						//push -0x1
		"\x3E\xFF\x93\xE0\x00\x00\x00"	//call [ebx+0xE0]
		"\x3E\x8D\x83\xC0\x00\x00\x00"	//lea eax,[ebx+0xC0]
		"\x50"							//push eax
		"\x3E\xFF\xB3\xC4\x00\x00\x00"	//push dword ptr [ebx+0xC4]
		"\x3E\x8D\x83\xC8\x00\x00\x00"	//lea eax,[ebx+0xC8]
		"\x50"							//push eax
		"\x33\xC0"						//xor eax,eax
		"\x50"							//push eax
		"\x3E\xFF\x93\xCC\x00\x00\x00"	//call [ebx+0xCC]
		"\x9D"							//popfd
		"\x61"							//popad
		"\x3E\x83\x2C\x24\x05"			//sub dword ptr ds:[esp],0x5
		"\xC3"							//retn
		"\x90"							//nop
		"\x90"							//nop
		"\x90"							//nop
		"\x90"							//nop
		"\x90"							//nop
		;
	
	BYTE ShellCode64[256] = 
		//x64没有pushad，所以要自己保存寄存器
		//并且因为call时栈指针必须按16字节对齐的原因,以下push个数必须为偶数
		//原因是call ZwTestAlert -> call ShellCodeFun ,连call两次，栈中两个返回地址，已经按16字节对齐了
		"\x50"							//push    rax
		"\x51"							//push    rcx
		"\x52"							//push    rdx
		"\x53"							//push    rbx
		"\x55"							//push    rbp
		"\x56"							//push    rsi
		"\x57"							//push    rdi
		"\x41\x50"						//push    r8
		"\x41\x51"			            //push    r9
		"\x9c"							//pushfq
		"\xe8\x00\x00\x00\x00"			//call  next
		"\x5b"							//pop     rbx
		"\x66\x83\xe3\x00"				//and     bx,0
		"\x48\x8b\xcb"					//mov     rcx,rbx
		"\xE8\x12\x00\x00\x00"			//call	  LoadDllAndRestoreExeEntry //如果修改了这里到子函数之间的指令，注意修正call偏移
		"\x9d"							//popfq
		"\x41\x59"						//pop     r9
		"\x41\x58"						//pop     r8
		"\x5f"							//pop     rdi
		"\x5e"							//pop     rsi
		"\x5d"							//pop     rbp
		"\x5b"							//pop     rbx
		"\x5a"							//pop     rdx
		"\x59"							//pop     rcx
		"\x58"							//pop     rax
		"\x3e\x83\x2c\x24\x06"			//sub     dword ptr ds:[rsp],6 
		"\xc3"							//ret
		//LoadDllAndRestoreFunEntry
		"\x48\x89\x5C\x24\x08"			//mov        qword ptr ss:[rsp+08h], rbx
		"\x57"							//push       rdi
		"\x48\x83\xEC\x30"				//sub        rsp, 30h
		"\x48\x8B\x91\xF8\x00\x00\x00"	//mov        rdx, qword ptr ds:[rcx+000000F8h]
		"\x48\x8B\x81\x08\x01\x00\x00"	//mov        rax, qword ptr ds:[rcx+00000108h]
		"\x48\x8B\xB9\xD8\x00\x00\x00"  //mov        rdi, qword ptr ds:[rcx+000000D8h]
		"\x4C\x8B\x91\x00\x01\x00\x00"  //mov        r10, qword ptr ds:[rcx+00000100h]
		"\x48\x89\x02"					//mov        qword ptr ds:[rdx], rax
		"\x48\x8B\x81\x10\x01\x00\x00"  //mov        rax, qword ptr ds:[rcx+00000110h]
		"\x48\x89\x42\x08"				//mov        qword ptr ds:[rdx+08h], rax
		"\x48\x8D\x81\xE8\x00\x00\x00"  //lea        rax, qword ptr ds:[rcx+000000E8h]
		"\x4C\x8D\x81\xF0\x00\x00\x00"  //lea        r8, qword ptr ds:[rcx+000000F0h]
		"\x44\x8B\x08"					//mov        r9d, dword ptr ds:[rax]
		"\x48\x8D\x91\xE0\x00\x00\x00"	//lea        rdx, qword ptr ds:[rcx+000000E0h]
		"\x48\x8B\xD9"					//mov        rbx, rcx
		"\x48\x83\xC9\xFF"				//or         rcx, FFFFFFFFFFFFFFFFh
		"\x48\x89\x44\x24\x20"			//mov        qword ptr ss:[rsp+20h], rax
		"\x41\xFF\xD2"					//call       r10
		"\x4C\x8B\x83\xC8\x00\x00\x00"	//mov        r8, qword ptr ds:[rbx+000000C8h]
		"\x4C\x8D\x8B\xC0\x00\x00\x00"  //lea        r9, qword ptr ds:[rbx+000000C0h]
		"\x48\x8D\x93\xD0\x00\x00\x00"  //lea        rdx, qword ptr ds:[rbx+000000D0h]
		"\x33\xC9"						//xor        ecx, ecx
		"\x48\x8B\xC7"					//mov        rax, rdi
		"\x48\x8B\x5C\x24\x40"			//mov        rbx, qword ptr ss:[rsp+40h]
		"\x48\x83\xC4\x30"				//add        rsp, 30h
		"\x5F"							//pop        rdi
		"\x48\xFF\xE0"					//jmp        rax
		"\x90\x90\x90\x90\x90"
		;

	

	if (Is64Bit)
	{
		pShellcodeStart = ShellCode64;
	}
	else
	{
		pShellcodeStart = ShellCode32;
	}
	
	
	//搜索Shellcode结束标志
	pShellcodeEnd = pShellcodeStart + 100;
	while (memcmp(pShellcodeEnd,"\x90\x90\x90\x90\x90",5) != 0)
	{
		pShellcodeEnd++;
	}
	
	ShellCodeSize = pShellcodeEnd - pShellcodeStart;
	dprintf("[*] Shellcode Len = %d\n",ShellCodeSize);
	memcpy(pOutCode,pShellcodeStart,ShellCodeSize);
}