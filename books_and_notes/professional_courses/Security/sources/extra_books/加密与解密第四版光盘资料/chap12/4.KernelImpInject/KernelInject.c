/*

  KernelInject.C

  Author: achillis
  Last Updated: 2016-03-11

  This framework is generated by EasySYS 0.3.0
  This template file is copying from QuickSYS 0.3.0 written by Chunhua Liu
	
  Tested On : (x86) WinXP / Win7 / Win10
*/

#include "dbghelp.h"
#include "KernelInject.h"
#include "ntifs.h"
#include "Common.h"
#include <ntimage.h>
// Device driver routine declarations.
//

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	);

NTSTATUS
KernelInjectDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
KernelInjectDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

NTSTATUS
KernelInjectDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	);

VOID
KernelInjectUnload(
	IN PDRIVER_OBJECT		DriverObject
	);

typedef NTSTATUS
(*PFN_ZwProtectVirtualMemory)(
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	PSIZE_T RegionSize,
	ULONG NewProtectWin32,
	PULONG OldProtect
    );

ULONG GetZwProtectMemoryAddr();

VOID
MyLoadImageRoutine(
	IN PUNICODE_STRING  FullImageName,
	IN HANDLE  ProcessId, // where image is mapped
	IN PIMAGE_INFO  ImageInfo
	);

NTSTATUS InjectDllByImportTable(ULONG ImageBase, char *szDllPath , char *szDllExportFun);
BOOL InitGlobalVars();
VOID InstallImageNotify();
VOID UnInstallImageNotify();
BOOL IsTagetProcess(PEPROCESS Eprocess);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, KernelInjectDispatchCreate)
#pragma alloc_text(PAGE, KernelInjectDispatchClose)
#pragma alloc_text(PAGE, KernelInjectDispatchDeviceControl)
#pragma alloc_text(PAGE, KernelInjectUnload)
#endif // ALLOC_PRAGMA

//Notify
BOOL g_IsImageNotifyInstalled=FALSE;
PFN_ZwProtectVirtualMemory pfnZwProtectVirtualMemory;
//尽量短一些
char g_szDllToInject[256]="C:\\MsgDll.dll";
char g_szDllExportFun[256]="Msg";


NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT		DriverObject,
	IN PUNICODE_STRING		RegistryPath
	)
{
	//
    // Create dispatch points for device control, create, close.
    //

    DriverObject->MajorFunction[IRP_MJ_CREATE]         = KernelInjectDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = KernelInjectDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = KernelInjectDispatchDeviceControl;
    DriverObject->DriverUnload                         = KernelInjectUnload;
	
	InitGlobalVars();
	//注册LoadImageNotfiy
	InstallImageNotify();
	DbgPrint("[KernelInject] Loaded!\n");
    return STATUS_SUCCESS;
}

NTSTATUS
KernelInjectDispatchCreate(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

	dprintf("[KernelInject] IRP_MJ_CREATE\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
KernelInjectDispatchClose(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

	dprintf("[KernelInject] IRP_MJ_CLOSE\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
KernelInjectDispatchDeviceControl(
	IN PDEVICE_OBJECT		DeviceObject,
	IN PIRP					Irp
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	dprintf("[KernelInject] IRP_MJ_DEVICE_CONTROL\n");

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

VOID
KernelInjectUnload(
	IN PDRIVER_OBJECT		DriverObject
	)
{
	UnInstallImageNotify();
    DbgPrint("[KernelInject] Unloaded\n");
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
		DbgPrint("ZwProtectVirtualMemory Addr=0x%08X\n",pfnZwProtectVirtualMemory);
	}
	return TRUE;
}

VOID InstallImageNotify()
{
	NTSTATUS status;
	if (g_IsImageNotifyInstalled==FALSE)
	{
		status=PsSetLoadImageNotifyRoutine(MyLoadImageRoutine);
		if (NT_SUCCESS(status))
		{
			DbgPrint("ImageNotify 安装成功!\n");
			g_IsImageNotifyInstalled = TRUE;
		}
		else
		{
			DbgPrint("安装ImageNotify失败！ status=0x%08X\n",status);
		}
	}
}

VOID UnInstallImageNotify()
{
	if (g_IsImageNotifyInstalled)
	{
		PsRemoveLoadImageNotifyRoutine(MyLoadImageRoutine);
		g_IsImageNotifyInstalled = FALSE;
		DbgPrint("ImageNotify 卸载成功!\n");
	}
}

VOID
MyLoadImageRoutine(
	IN PUNICODE_STRING  FullImageName,
	IN HANDLE  ProcessId, // where image is mapped
	IN PIMAGE_INFO  ImageInfo
	)
{
	PROCESS_BASIC_INFORMATION ProcessInfo;
	NTSTATUS status;
	ULONG returnLen=0;
	ULONG PebAddr;
	ULONG ExeImageBase;
	PEPROCESS CurProcess = PsGetCurrentProcess();

	if (ProcessId!=NULL)//用户进程
	{
		//检查当前进程
		if (IsTagetProcess(CurProcess))
		{
			//检查当前进程的基本信息以取得Peb
			status=ZwQueryInformationProcess(
				NtCurrentProcess(),
				ProcessBasicInformation,
				&ProcessInfo,
				sizeof(PROCESS_BASIC_INFORMATION),
				&returnLen
				);
			if (NT_SUCCESS(status))
			{
				PebAddr = (ULONG)ProcessInfo.PebBaseAddress ;
				ExeImageBase = *(ULONG*)(PebAddr+8) ;
				//如果模块基址与Peb->ImageBase相等，表明正在加载的是Exe文件
				//也可以通过其它方式判断
				if (ExeImageBase == (ULONG)ImageInfo->ImageBase)
				{
					DbgPrint("准备注入Dll!\n");
					status=InjectDllByImportTable(ExeImageBase, g_szDllToInject , g_szDllExportFun);
				}
			}
		}
	}
}

//检查目标进程是不是notepad.exe
BOOL IsTagetProcess(PEPROCESS Eprocess)
{
	char *szProcname = PsGetProcessImageFileName(Eprocess);
	if (_stricmp(szProcname,"notepad.exe") == 0)
	{
		return TRUE;
	}
	return FALSE;
	
}


#define ALIGN_SIZE_UP(Size,Alignment)  (((ULONG_PTR)(Size) + Alignment - 1) & ~(Alignment - 1))

//需要把dll放在system32等可以搜索到的目录下
NTSTATUS InjectDllByImportTable(ULONG ImageBase , char *szDllPath, char *szDllExportFun)
{
	
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader;
	PIMAGE_FILE_HEADER pFileHeader;
	PIMAGE_OPTIONAL_HEADER pOptHeader;
	PIMAGE_IMPORT_DESCRIPTOR pImportDir,pTmpImportDir;
	PIMAGE_IMPORT_BY_NAME     pImportByName;
	PIMAGE_IMPORT_DESCRIPTOR pOriginalImportDescriptor,pNewImportDesp,pTmpImportDesp;
	ULONG ImportTableVA,ImportTableSize;
	ULONG newImportTableVA,newImportTableSize;
	BYTE *pBuf=NULL ,*pData = NULL;
	ULONG OldProtect;
	NTSTATUS status;
	ULONG i=0;
	ULONG AddresstoChangeProtect=0,SizeToChangeProtect=0;
	ULONG TotalImageSize = 0 ;
	ULONG AlignedHeaderSize = 0 ;
	SIZE_T MemSize = 0x1000;

	//检查参数
	if (ImageBase==0)
	{
		return STATUS_INVALID_PARAMETER;
	}
	//检查映像
	pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	if (pDosHeader->e_magic  != 0x5A4D)	//MZ
	{
		return STATUS_INVALID_IMAGE_NOT_MZ;
	}
	pNtHeader=(PIMAGE_NT_HEADERS)(ImageBase+pDosHeader->e_lfanew);
	if (pNtHeader->Signature != 0x00004550)	//PE
	{
		return STATUS_INVALID_IMAGE_FORMAT;
	}
	__try
	{
		pFileHeader=(PIMAGE_FILE_HEADER)(ImageBase+pDosHeader->e_lfanew+4);
		pOptHeader=(PIMAGE_OPTIONAL_HEADER)((BYTE*)pFileHeader+sizeof(IMAGE_FILE_HEADER));
		TotalImageSize = pOptHeader->SizeOfImage ;
		
		//获取输入表地址
		ImportTableVA = pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		ImportTableSize =  pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		DbgPrint("ImportTable VirtualAddress=0x%08X  Size=0x%X\n",ImportTableVA,ImportTableSize);
		
		pOriginalImportDescriptor=(PIMAGE_IMPORT_DESCRIPTOR)(ImageBase+ImportTableVA);
		DbgPrint("原始的导入表开始于 0x%08X\n",pOriginalImportDescriptor);
		newImportTableSize = ImportTableSize + sizeof(IMAGE_IMPORT_DESCRIPTOR) ;

		//利用PE头后面的一部分空间,Thunk数据有0x40大小就够了
		AlignedHeaderSize = ALIGN_SIZE_UP(pOptHeader->SizeOfHeaders , pOptHeader->SectionAlignment) ;
		DbgPrint("PE Header Size = 0x%X AlignedSize = 0x%X\n",pOptHeader->SizeOfHeaders,AlignedHeaderSize);

		pBuf = (BYTE*)ImageBase + AlignedHeaderSize - newImportTableSize - 0x40 ;

		DbgPrint("Address Of New ImportTable pBuf=0x%p\n",pBuf);
		
		//修改PE头的页属性
		AddresstoChangeProtect = ImageBase ;
		SizeToChangeProtect = AlignedHeaderSize;
		DbgPrint("修改PE头的页保护属性，Addr=0x%08X  Size=0x%X\n",AddresstoChangeProtect,SizeToChangeProtect);

		status=pfnZwProtectVirtualMemory(NtCurrentProcess(),
			(PVOID*)&AddresstoChangeProtect,
			&SizeToChangeProtect,PAGE_EXECUTE_READWRITE,&OldProtect);
		if (NT_SUCCESS(status))
		{
			DbgPrint("PE头的内存属性修改成功!\n");
			//保存原始导入表
			memcpy(pBuf,pOriginalImportDescriptor,ImportTableSize);
			//新的偏移位置， 稍后填充
			pNewImportDesp = (PIMAGE_IMPORT_DESCRIPTOR)(pBuf + ImportTableSize - sizeof(IMAGE_IMPORT_DESCRIPTOR));
			DbgPrint("新的导入表项开始于 0x%08X\n",pNewImportDesp);
			
			//构造数据
			pData = pBuf + newImportTableSize ;
			DbgPrint("pData = 0x%p\n",pData);
			//0x00开始是dll名称
			strcpy(pData+0x00,szDllPath);
			//0x20构造FunName
			pImportByName=(PIMAGE_IMPORT_BY_NAME)(pData + 0x20);
			pImportByName->Hint=0;//按名称导入，这里直接填0即可
			strcpy(pImportByName->Name,szDllExportFun);
			//0x30构造OriginalFirstTHunk,指向0x20处的IMAGE_IMPORT_BY_NAME
			*(ULONG*)(pData+0x30)=(ULONG)pData + 0x20 - ImageBase;
			//0x38作为FirstThunk
			*(ULONG*)(pData+0x38)=(ULONG)pData + 0x20 - ImageBase;
			
			//第一项填充为我们自己的dll
			pNewImportDesp->OriginalFirstThunk = (ULONG)pData + 0x30 - ImageBase;
			pNewImportDesp->TimeDateStamp = 0;
			pNewImportDesp->ForwarderChain = 0;
			pNewImportDesp->Name = (ULONG)pData - ImageBase ;
			pNewImportDesp->FirstThunk = (ULONG)pData + 0x38 - ImageBase;
			
			//计算新的导入表偏移
			newImportTableVA = (ULONG)pBuf - ImageBase ;

			//修改数据
			DbgPrint("开始修改PE头...\n");
			pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = newImportTableVA;
			pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = newImportTableSize ;
			//禁止绑定导入表
			pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
			pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0 ;

			DbgPrint("导入表感染完成!\n");
			return STATUS_SUCCESS;
		}
		else
		{
			DbgPrint("无法修改PE头的页面属性，感染失败! status=0x%08X\n",status);
			return status;
		}
	}__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("发生内存读写错误!\n");
		return GetExceptionCode();
	}
	
	
}
