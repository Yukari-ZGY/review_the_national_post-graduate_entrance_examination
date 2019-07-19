/*-----------------------------------------------------------------------
第13章  Hook技术
《加密与解密（第四版）》
(c)  看雪学院 www.kanxue.com 2000-2018
-----------------------------------------------------------------------*/
/*

  ProcProt.C

  Author: achillis
  Last Updated: 2006-03-23

  This framework is generated by EasySYS 0.3.0
  This template file is copying from QuickSYS 0.3.0 written by Chunhua Liu

*/

#include "dbghelp.h"
#include "ProcProt.h"
#include "mykernel.h"
#include "ntifs.h"
#include <ntimage.h>

typedef NTSTATUS
(NTAPI *PFN_NTTerminateProcess)(
    HANDLE ProcessHandle,
    NTSTATUS ExitStatus
    );

NTSTATUS
NTAPI
DetourNtTerminateProcess (
	HANDLE ProcessHandle,
    NTSTATUS ExitStatus
	);

char* OpenFileAndMap(char *szFilePath);
DWORD myGetProcAddress(DWORD hModule,char *FuncName);
VOID FreePE(ULONG MappedBase);
ULONG GetServiceIndex(char *ServiceName);
BOOL InitGlobalVars();
VOID UnhookSSDTServiceByIndex(ULONG ServiceIndex,ULONG OriginalFunAddr );
ULONG HookSSDTServiceByIndex(ULONG ServiceIndex,ULONG FakeFunAddr );
PMDL MakeAddrWritable (ULONG ulOldAddress, ULONG ulSize, PULONG pulNewAddress) ;

//用于映射文件操作
HANDLE g_hMap=NULL,g_hFile=NULL;
ULONG g_IndexNtTerminateProcess = 0;
char g_ServiceName[32]="NtTerminateProcess";
char g_szProcNameToProtect[16]="notepad.exe"; //被保护进程的名字
PFN_NTTerminateProcess OriginalNtTerminateProcess;

// Device driver routine declarations.
//

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	);

NTSTATUS
ProcprotDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
ProcprotDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
ProcprotDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

VOID
ProcprotUnload(
	IN PDRIVER_OBJECT		DriverObject
	);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, ProcprotDispatchCreate)
#pragma alloc_text(PAGE, ProcprotDispatchClose)
#pragma alloc_text(PAGE, ProcprotDispatchDeviceControl)
#pragma alloc_text(PAGE, ProcprotUnload)
#endif // ALLOC_PRAGMA

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	)
{
	//
    // Create dispatch points for device control, create, close.
    //

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = ProcprotDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = ProcprotDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcprotDispatchDeviceControl;
    DriverObject->DriverUnload                         = ProcprotUnload;
	
	if (InitGlobalVars()==FALSE)
    {
		DbgPrint("[DriverEntry] InitGlobalVars Failed!\n");
		goto __failed;
    }
	OriginalNtTerminateProcess=(PFN_NTTerminateProcess)HookSSDTServiceByIndex(g_IndexNtTerminateProcess,(ULONG)DetourNtTerminateProcess);
	if (OriginalNtTerminateProcess==0)
	{
		DbgPrint("[DriverEntry] HookSSDTServiceByIndex Failed!\n");
		goto __failed;
	}
	dprintf("HOOK %s OK! FunAddr = 0x%X\n",g_ServiceName,OriginalNtTerminateProcess);

	DbgPrint("[ProcProt] Loaded!\n");
    return STATUS_SUCCESS;

__failed:
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS
ProcprotDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

	dprintf("[ProcProt] IRP_MJ_CREATE\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
ProcprotDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

	dprintf("[ProcProt] IRP_MJ_CLOSE\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
ProcprotDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	dprintf("[ProcProt] IRP_MJ_DEVICE_CONTROL\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

VOID
ProcprotUnload(
	IN PDRIVER_OBJECT		DriverObject
	)
{
	//驱动卸载时要取消安装的SSDT Hook
	UnhookSSDTServiceByIndex(g_IndexNtTerminateProcess,(ULONG)OriginalNtTerminateProcess);
    DbgPrint("[ProcProt] Unloaded\n");
}


//该函数只做到了简单防止外部进程通过TerminateProcess结束记事本
NTSTATUS
NTAPI
DetourNtTerminateProcess (
	IN HANDLE ProcessHandle,
	IN NTSTATUS ExitStatus
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	PEPROCESS Process = NULL ;
	char *szImageName = NULL ;
	char *szCurProcName = PsGetProcessImageFileName(PsGetCurrentProcess());

	//如果句柄是0或-1，表明是某个进程在调用ExitProcess自行退出，放行
	if (ProcessHandle == 0
		|| ProcessHandle == (HANDLE)-1)
	{
		DbgPrint("进程正在自行退出.\n");
		goto __PASSTHROUGH;
	}
	else
	{
		//判断目标进程是不是记事本
		status = ObReferenceObjectByHandle (ProcessHandle,
			0,
			PsProcessType,
			KernelMode,
			&Process,
			NULL);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("无法获取句柄对应的进程\n");
			goto __PASSTHROUGH;
		}

		szImageName = PsGetProcessImageFileName(Process);
		if (_stricmp(szImageName,g_szProcNameToProtect) == 0)
		{
			//进程名称匹配成功
			DbgPrint("进程 %s 试图结束notepad.exe,拒绝!\n",szCurProcName);
			return STATUS_ACCESS_DENIED;
		}
		
	}

__PASSTHROUGH:
	return OriginalNtTerminateProcess(ProcessHandle,ExitStatus);
}


char* OpenFileAndMap(char *szFilePath)
{
	char* MappedBase=0;
	OBJECT_ATTRIBUTES oba;
	IO_STATUS_BLOCK ioblk;
	char objFilePath[260];
	ANSI_STRING ansiStr;
	UNICODE_STRING uFilePath;
	NTSTATUS status;
	LARGE_INTEGER offset;
	SIZE_T viewsize=0;
	RtlInitAnsiString(&ansiStr,szFilePath);
	RtlAnsiStringToUnicodeString(&uFilePath,&ansiStr,TRUE);
	InitializeObjectAttributes(&oba ,
        &uFilePath,
        OBJ_CASE_INSENSITIVE |OBJ_KERNEL_HANDLE,
        NULL ,
        NULL
        );
	status=ZwCreateFile(&g_hFile,
		SYNCHRONIZE|GENERIC_READ,
		&oba,
		&ioblk,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
		);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateFile Failed! status=0x%08X\n",status);
		goto __failed;
	}
	RtlFreeUnicodeString(&uFilePath);
	InitializeObjectAttributes(&oba,
        NULL,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );
	status=ZwCreateSection(&g_hMap,
		SECTION_MAP_READ,
		0,
		0,
		PAGE_READONLY,
		SEC_IMAGE,
		g_hFile
		);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateSection Failed! status=0x%08X\n",status);
		goto __failed;
	}
	offset.QuadPart=0;
	status=ZwMapViewOfSection(g_hMap,
		NtCurrentProcess(),
		(PVOID*)&MappedBase,
		0,
		1000,
		NULL,
		&viewsize,
		ViewShare,
		MEM_TOP_DOWN,
		PAGE_READONLY);
	if (NT_SUCCESS(status))
	{
		ZwClose(g_hFile);
		return MappedBase;
	}
	
__failed:
	if (g_hFile)
		ZwClose(g_hFile);
	if (g_hMap)
		ZwClose(g_hMap);
	return NULL;
}

VOID FreePE(ULONG MappedBase)
{
	NTSTATUS status;
	status=ZwUnmapViewOfSection(NtCurrentProcess(),(PVOID)MappedBase);
	if (g_hMap)
	{
		ZwClose(g_hMap);
		g_hMap=NULL;
	}
	if (g_hFile)
	{
		ZwClose(g_hFile);
		g_hFile=NULL;
	}
	
}

DWORD myGetProcAddress(DWORD hModule,char *FuncName)
{
	//自己实现GetProcAddress
	DWORD retAddr=0;
	DWORD *namerav,*funrav;
	DWORD cnt=0;
	DWORD max,min,mid;
	WORD *nameOrdinal;
	WORD nIndex=0;
	int cmpresult=0;
	char *ModuleBase=(char*)hModule;
	char *szMidName = NULL ;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_FILE_HEADER pFileHeader;
	PIMAGE_OPTIONAL_HEADER pOptHeader;
	PIMAGE_EXPORT_DIRECTORY pExportDir;
	pDosHeader=(PIMAGE_DOS_HEADER)ModuleBase;
	pFileHeader=(PIMAGE_FILE_HEADER)(ModuleBase+pDosHeader->e_lfanew+4);
	pOptHeader=(PIMAGE_OPTIONAL_HEADER)((char*)pFileHeader+sizeof(IMAGE_FILE_HEADER));
	pExportDir=(PIMAGE_EXPORT_DIRECTORY)(ModuleBase+pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	namerav=(DWORD*)(ModuleBase+pExportDir->AddressOfNames);
	funrav=(DWORD*)(ModuleBase+pExportDir->AddressOfFunctions);
	nameOrdinal=(WORD*)(ModuleBase+pExportDir->AddressOfNameOrdinals);
	if ((DWORD)FuncName<0x0000FFFF)
	{
		retAddr=(DWORD)(ModuleBase+funrav[(WORD)FuncName]);
	}
	else
	{
		//二分法查找
		max = pExportDir->NumberOfNames ;
		min = 0;
		mid=(max+min)/2;
		while (min < max)
		{
			szMidName = ModuleBase+namerav[mid] ;
			cmpresult=strcmp(FuncName,szMidName);
			if (cmpresult < 0)
			{
				//比中值小,则取中值-1为最大值
				max = mid -1 ;
			}
			else if (cmpresult > 0)
			{
				//比中值大,则取中值+1为最小值
				min = mid + 1;
			}
			else
			{
				break;
			}
			mid=(max+min)/2;
			
		}
		
		if (strcmp(FuncName,ModuleBase+namerav[mid]) == 0)
		{
			nIndex=nameOrdinal[mid];
			retAddr=(DWORD)(ModuleBase+funrav[nIndex]);
		}
	}
	return retAddr;
}

/*
ntdll!ZwOpenProcess:
7c92dd7b b87a000000      mov     eax,7Ah
7c92dd80 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
7c92dd85 ff12            call    dword ptr [edx]
7c92dd87 c21000          ret     10h
*/
ULONG GetServiceIndex(char *ServiceName)
{
	ULONG ServiceIndex=0;
	ULONG FnAddr=0;
	ULONG MapedNtdllBase=0;
	char szNtdllPath[260]="\\SystemRoot\\system32\\ntdll.dll";
	if (ServiceName==NULL)
	{
		return 0;
	}
	MapedNtdllBase=(ULONG)OpenFileAndMap(szNtdllPath);
	if (MapedNtdllBase==0)
	{
		dprintf("[GetServiceIndex] Map Ntdll.dll Failed!\n");
		return 0;
	}
	FnAddr=(ULONG)myGetProcAddress(MapedNtdllBase,ServiceName);
	if (FnAddr==0)
	{
		dprintf("[GetServiceIndex] Get Function Address of %s Failed!\n",ServiceName);
		return 0;
	}
	if (*(BYTE*)FnAddr!=0xB8)
	{
		dprintf("[GetServiceIndex] Function  %s dispatch! Maybe Hooked...\n",ServiceName);
		return 0;
	}
	ServiceIndex= *(ULONG*)(FnAddr+1);
	dprintf("ServiceIndex of %s is %d\n",ServiceName,ServiceIndex);
	FreePE(MapedNtdllBase);
	return ServiceIndex;
}

BOOL InitGlobalVars()
{
	//通过查找ntdll的导出表，获取NtTerminateProcess的服务索引
	g_IndexNtTerminateProcess=GetServiceIndex(g_ServiceName);
	if (g_IndexNtTerminateProcess==0)
	{
		dprintf("[InitGlobalVars] Get ServiceIndex of %s Failed!\n",g_ServiceName);
		return FALSE;
	}
	return TRUE;
}


ULONG HookSSDTServiceByIndex(ULONG ServiceIndex,ULONG FakeFunAddr )
{
	ULONG OriginalFun=0;
	PMDL pMdl = NULL;
	PULONG g_newSSDT = NULL ;
	//通过Mdl重新映射SSDT,得到一个可写的地址
	pMdl = MakeAddrWritable((ULONG)KeServiceDescriptorTable->ServiceTable,KeServiceDescriptorTable->TableSize * sizeof(ULONG) ,(PULONG)&g_newSSDT);
	dprintf("Mapped new SSDT Address = 0x%X\n",g_newSSDT);
	if (g_newSSDT != NULL)
	{
		OriginalFun= g_newSSDT[ServiceIndex];
		g_newSSDT[ServiceIndex] = FakeFunAddr;
		MmUnlockPages(pMdl);
		IoFreeMdl(pMdl);
	}
	else
	{
		dprintf("Map new SSDT failed! \n");
	}
	return OriginalFun;
}

VOID UnhookSSDTServiceByIndex(ULONG ServiceIndex,ULONG OriginalFunAddr )
{
	PMDL pMdl = NULL;
	PULONG g_newSSDT = NULL ;
	pMdl = MakeAddrWritable((ULONG)KeServiceDescriptorTable->ServiceTable,KeServiceDescriptorTable->TableSize * sizeof(ULONG) ,(PULONG)&g_newSSDT);
	dprintf("Mapped new SSDT Address = 0x%X\n",g_newSSDT);
	if (g_newSSDT != NULL)
	{
		g_newSSDT[ServiceIndex] = OriginalFunAddr;
		MmUnlockPages(pMdl);
		IoFreeMdl(pMdl);
	}
	else
	{
		dprintf("Map new SSDT failed! \n");
	}
	
}

PMDL MakeAddrWritable (ULONG ulOldAddress, ULONG ulSize, PULONG pulNewAddress) 
{
	PVOID pNewAddr;
	PMDL pMdl = IoAllocateMdl((PVOID)ulOldAddress, ulSize, FALSE, TRUE, NULL);
	if (!pMdl)
		return NULL;
	
	__try {
		MmProbeAndLockPages(pMdl, KernelMode, IoWriteAccess);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		IoFreeMdl(pMdl);
		return NULL;
	}
	
	if ( pMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL ))
		pNewAddr = pMdl->MappedSystemVa;
	else                                            // Map a new VA!!!
		pNewAddr = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
	
	if ( !pNewAddr ) {
		MmUnlockPages(pMdl);
		IoFreeMdl(pMdl);
		return NULL;
	}
	
	if ( pulNewAddress )
		*pulNewAddress = (ULONG)pNewAddr;
	
	return pMdl;
}
