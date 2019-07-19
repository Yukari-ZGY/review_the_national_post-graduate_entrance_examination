#define IMP_VOID __declspec(dllimport) VOID __stdcall
#define IMP_SYSCALL __declspec(dllimport) NTSTATUS __stdcall

#define PAGE_SIZE 0x1000

#define SE_DEBUG_PRIVILEGE 20

#define OBJ_CASE_INSENSITIVE 0x00000040
#define DUPLICATE_SAME_ATTRIBUTES 0x00000004

#define NtCurrentProcess() ((HANDLE)0xFFFFFFFF)
#define NtCurrentThread() ((HANDLE)0xFFFFFFFE)

#define OBJECT_NAME_LENGTH sizeof(UNICODE_STRING) + (MAX_PATH + 1) * sizeof(WCHAR)

#define KERNEL_NAME_LENGTH 0x0D

#define STATUS_SUCCESS 0x00000000
#define STATUS_OBJECT_PATH_INVALID 0xC0000039

#define FileNameInformation 0x09

typedef ULONG NTSTATUS;

typedef struct ANSI_STRING
{
    /* 0x00 */ USHORT Length;
    /* 0x02 */ USHORT MaximumLength;
    /* 0x04 */ PCHAR Buffer;
    /* 0x08 */
}
    ANSI_STRING,
  *PANSI_STRING,
**PPANSI_STRING;

typedef struct _UNICODE_STRING 
{
    /* 0x00 */ USHORT Length;
    /* 0x02 */ USHORT MaximumLength;
    /* 0x04 */ PWSTR Buffer;
    /* 0x08 */
}
    UNICODE_STRING,
  *PUNICODE_STRING,
**PPUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES
{
    /* 0x00 */ ULONG Length;
    /* 0x04 */ HANDLE RootDirectory;
    /* 0x08 */ PUNICODE_STRING ObjectName;
    /* 0x0C */ ULONG Attributes;
    /* 0x10 */ PSECURITY_DESCRIPTOR SecurityDescriptor;
    /* 0x14 */ PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    /* 0x18 */
}
    OBJECT_ATTRIBUTES,
  *POBJECT_ATTRIBUTES,
**PPOBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK
{ 
    union
    { 
        /* 0x00 */ NTSTATUS Status; 
        /* 0x00 */ PVOID Pointer; 
    }; 

    /* 0x04 */ ULONG Information;
    /* 0x08 */
}
    IO_STATUS_BLOCK,
  *PIO_STATUS_BLOCK,
**PPIO_STATUS_BLOCK;

typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
}
    CLIENT_ID,
  *PCLIENT_ID,
**PPCLIENT_ID;

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemNotImplemented1,
    SystemProcessesAndThreadsInformation,
    SystemCallCounts,
    SystemConfigurationInformation,
    SystemProcessorTimes,
    SystemGlobalFlag,
    SystemNotImplemented2,
    SystemModuleInformation,
    SystemLockInformation,
    SystemNotImplemented3,
    SystemNotImplemented4,
    SystemNotImplemented5,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPagefileInformation,
    SystemInstructionEmulationCounts,
    SystemInvalidInfoClass1,
    SystemCacheInformation,
    SystemPoolTagInformation,
    SystemProcessorStatistics,
    SystemDpcInformation,
    SystemNotImplemented6,
    SystemLoadImage,
    SystemUnloadImage,
    SystemTimeAdjustment,
    SystemNotImplemented7,
    SystemNotImplemented8,
    SystemNotImplemented9,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemLoadAndCallImage,
    SystemPrioritySeparation,
    SystemNotImplemented10,
    SystemNotImplemented11,
    SystemInvalidInfoClass2,
    SystemInvalidInfoClass3,
    SystemTimeZoneInformation,
    SystemLookasideInformation,
    SystemSetTimeSlipEvent,
    SystemCreateSession,
    SystemDeleteSession,
    SystemInvalidInfoClass4,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemAddVerifier,
    SystemSessionProcessesInformation
} SYSTEM_INFORMATION_CLASS;

typedef LONG KPRIORITY;

typedef struct _VM_COUNTERS
{
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
}
    VM_COUNTERS,
  *PVM_COUNTERS,
**PPVM_COUNTERS;

typedef struct _IO_COUNTERSEX
{
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
}
    IO_COUNTERSEX,
  *PIO_COUNTERSEX,
**PPIO_COUNTERSEX;

typedef enum
{
    StateInitialized,
    StateReady,
    StateRunning,
    StateStandby,
    StateTerminated,
    StateWait,
    StateTransition,
    StateUnknown
} THREAD_STATE;

typedef enum _KWAIT_REASON
{
    Executive = 0,
    FreePage = 1,
    PageIn = 2,
    PoolAllocation = 3,
    DelayExecution = 4,
    Suspended = 5,
    UserRequest = 6,
    WrExecutive = 7,
    WrFreePage = 8,
    WrPageIn = 9,
    WrPoolAllocation = 10,
    WrDelayExecution = 11,
    WrSuspended = 12,
    WrUserRequest = 13,
    WrEventPair = 14,
    WrQueue = 15,
    WrLpcReceive = 16,
    WrLpcReply = 17,
    WrVirtualMemory = 18,
    WrPageOut = 19,
    WrRendezvous = 20,
    Spare2 = 21,
    Spare3 = 22,
    Spare4 = 23,
    Spare5 = 24,
    WrCalloutStack = 25,
    WrKernel = 26,
    WrResource = 27,
    WrPushLock = 28,
    WrMutex = 29,
    WrQuantumEnd = 30,
    WrDispatchInt = 31,
    WrPreempted = 32,
    WrYieldExecution = 33,
    WrFastMutex = 34,
    WrGuardedMutex = 35,
    WrRundown = 36,
    MaximumWaitReason = 37
} KWAIT_REASON;

typedef enum _PROCESSINFOCLASS
{
	ProcessBasicInformation,			// 0x00 Y N
	ProcessQuotaLimits,					// 0x01 Y Y
	ProcessIoCounters,					// 0x02 Y N
	ProcessVmCounters,					// 0x03 Y N
	ProcessTimes,						// 0x04 Y N
	ProcessBasePriority,				// 0x05 N Y
	ProcessRaisePriority,				// 0x06 N Y
	ProcessDebugPort,					// 0x07 Y Y
	ProcessExceptionPort,				// 0x08 N Y
	ProcessAccessToken,					// 0x09 N Y
	ProcessLdtInformation,				// 0x0A Y Y
	ProcessLdtSize,						// 0x0B N Y
	ProcessDefaultHardErrorMode,		// 0x0C Y Y
	ProcessIoPortHandlers,				// 0x0D N Y
	ProcessPooledUsageAndLimits,		// 0x0E Y N
	ProcessWorkingSetWatch,				// 0x0F Y Y
	ProcessUserModeIOPL,				// 0x10 N Y
	ProcessEnableAlignmentFaultFixup,	// 0x11 N Y
	ProcessPriorityClass,				// 0x12 N Y
	ProcessWx86Information,				// 0x13 Y N
	ProcessHandleCount,					// 0x14 Y N
	ProcessAffinityMask,				// 0x15 N Y
	ProcessPriorityBoost,				// 0x16 Y Y
	ProcessDeviceMap,					// 0x17 Y Y
	ProcessSessionInformation,			// 0x18 Y Y
	ProcessForegroundInformation,		// 0x19 N Y
	ProcessWow64Information				// 0x1A Y N
} PROCESSINFOCLASS;

typedef struct _PROCESS_BASIC_INFORMATION
{
	/* 0x00 */ NTSTATUS ExitStatus;
	/* 0x04 */ PVOID PebBaseAddress;
	/* 0x08 */ ULONG AffinityMask;
	/* 0x0C */ ULONG BasePriority;
	/* 0x10 */ ULONG UniqueProcessId;
	/* 0x14 */ ULONG InheritedFromUniqieProcessId;
	/* 0x18 */
}
	PROCESS_BASIC_INFORMATION,
  *PPROCESS_BASIC_INFORMATION,
**PPPROCESS_BASIC_INFORMATION;

typedef struct _THREAD_BASIC_INFORMATION
{
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	ULONG AffinityMask;
	ULONG Priority;
	ULONG BasePriority;
}
    THREAD_BASIC_INFORMATION,
  *PTHREAD_BASIC_INFORMATION,
**PPTHREAD_BASIC_INFORMATION;

typedef struct _SYSTEM_THREADS
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
    ULONG ContextSwitchCount;
    THREAD_STATE State;
    KWAIT_REASON WaitReason;
}
    SYSTEM_THREADS,
  *PSYSTEM_THREADS,
**PPSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES
{
    ULONG NextEntryDelta;
    ULONG ThreadCount;
    ULONG Reserved1[6];
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ProcessName;
    KPRIORITY BasePriority;
    ULONG ProcessId;
    ULONG InheritedFromProcessId;
    ULONG HandleCount;
    ULONG Reserved2[2];
    VM_COUNTERS VmCounters;
    IO_COUNTERSEX IoCounters;
    SYSTEM_THREADS Threads[1];
}
    SYSTEM_PROCESSES,
  *PSYSTEM_PROCESSES,
**PPSYSTEM_PROCESSES;

typedef struct _SYSTEM_MODULE_INFORMATION
{
    /* 0x0000 */ ULONG Reserved[2];
    /* 0x0008 */ PVOID Base;
    /* 0x000C */ ULONG Size;
    /* 0x0010 */ ULONG Flags;
    /* 0x0014 */ USHORT Index;
    /* 0x0016 */ USHORT Unknown;
    /* 0x0018 */ USHORT LoadCount;
    /* 0x001A */ USHORT ModuleNameOffset;
    /* 0x001C */ UCHAR ImageName[256];
    /* 0x011C */
}
    SYSTEM_MODULE_INFORMATION,
  *PSYSTEM_MODULE_INFORMATION,
**PPSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_MODULE_INFORMATION_EX
{
    /* 0x00 */ ULONG ModulesCount;
    /* 0x04 */ SYSTEM_MODULE_INFORMATION Modules[0];
    /* 0xXX */
}
    SYSTEM_MODULE_INFORMATION_EX,
  *PSYSTEM_MODULE_INFORMATION_EX,
**PPSYSTEM_MODULE_INFORMATION_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    /* 0x00 */ ULONG ProcessId;
    /* 0x04 */ UCHAR ObjectTypeNumber;
    /* 0x05 */ UCHAR Flags;
    /* 0x06 */ USHORT Handle;
    /* 0x08 */ ULONG Object;
    /* 0x0C */ ACCESS_MASK GrandAccess;
    /* 0x10 */
}
    SYSTEM_HANDLE_INFORMATION,
  *PSYSTEM_HANDLE_INFORMATION,
**PPSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX
{
    /* 0x00 */ ULONG HandleCount;
    /* 0x04 */ SYSTEM_HANDLE_INFORMATION Handles[0];
    /* 0xXX */
}
    SYSTEM_HANDLE_INFORMATION_EX,
  *PSYSTEM_HANDLE_INFORMATION_EX,
**PPSYSTEM_HANDLE_INFORMATION_EX;

typedef enum _OBJECT_INFORMATION_CLASS
{
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectAllTypesInformation,
    ObjectHandleInformation
} OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_NAME_INFORMATION
{
    UNICODE_STRING Name;
}
    OBJECT_NAME_INFORMATION,
  *POBJECT_NAME_INFORMATION,
**PPOBJECT_NAME_INFORMATION;

typedef struct _OBJECT_BASIC_INFORMATION
{
    ULONG Attributes;
    ACCESS_MASK GrandAccess;
    ULONG HandleCount;
    ULONG PointerCount;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
    ULONG Reserved[3];
    ULONG NameInformationLength;
    ULONG TypeInformationLength;
    ULONG SecurityDescriptorLength;
    LARGE_INTEGER CreateTime;
}
    OBJECT_BASIC_INFORMATION,
  *POBJECT_BASIC_INFORMATION,
**PPOBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION
{
    /* 0x00 */ UNICODE_STRING Name;
    /* 0x08 */ ULONG ObjectCount;
    /* 0x0C */ ULONG HandleCount;
    /* 0x10 */ ULONG Reserved1[4];
    /* 0x20 */ ULONG PeakObjectCount;
    /* 0x24 */ ULONG PeakHandleCount;
    /* 0x28 */ ULONG Reserved2[4];
    /* 0x38 */ ULONG InvalidAttributes;
    /* 0x3C */ GENERIC_MAPPING GenericMapping;
    /* 0x40 */ ULONG ValidAccess;
    /* 0x44 */ UCHAR Unknown;
    /* 0x45 */ BOOLEAN MaintainHandleDatabase;
    /* 0x46 */ UCHAR Reserved3[2];
    /* 0x48 */ ULONG PoolType;
    /* 0x4C */ ULONG PagedPoolUsage;
    /* 0x50 */ ULONG NonPagedPoolUsage;
	/* 0x54 */
}
	OBJECT_TYPE_INFORMATION,
  *POBJECT_TYPE_INFORMATION,
**PPOBJECT_TYPE_INFORMATION;

typedef enum _KPROFILE_SOURCE
{
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE;

typedef VOID (NTAPI *PIO_APC_ROUTINE)
(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
);

IMP_VOID RtlInitAnsiString
(
    IN OUT PANSI_STRING  DestinationString,
    IN PUCHAR SourceString
);

IMP_VOID RtlInitUnicodeString
(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
);

IMP_VOID RtlCreateUnicodeStringFromAsciiz
(
    OUT PUNICODE_STRING DestinationString,  
    IN PUCHAR SourceString 
);

IMP_VOID RtlFreeUnicodeString
(
    IN PUNICODE_STRING UnicodeString
);

IMP_VOID RtlFreeAnsiString
(
    IN PANSI_STRING AnsiString
);

IMP_SYSCALL LdrLoadDll
(
    IN PWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
);

IMP_SYSCALL LdrUnloadDll
(
    IN PVOID DllHandle
);

IMP_SYSCALL LdrGetProcedureAddress
(
    IN PVOID DllHandle,
    IN PANSI_STRING ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress
);

IMP_SYSCALL NtOpenProcessToken
(
	IN HANDLE ProcessHandle,
	IN ACCESS_MASK DesiredAccess,
	OUT PHANDLE TokenHandle
);

IMP_SYSCALL NtAdjustPrivilegesToken
(
	IN HANDLE TokenHandle,
	IN BOOLEAN DisableAllPrivileges,
	IN PTOKEN_PRIVILEGES NewState,
	IN ULONG BufferLength,
	OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
	OUT PULONG ReturnLength
);

IMP_SYSCALL NtAllocateVirtualMemory
(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN OUT PULONG AllocationSize,
    IN ULONG AllocationType,
    IN ULONG Protect
);

IMP_SYSCALL NtFreeVirtualMemory
(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG FreeSize,
    IN ULONG FreeType
);

IMP_SYSCALL NtQuerySystemInformation
(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

IMP_SYSCALL NtQueryInformationFile
(
    IN HANDLE FileHandle,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
	IN ULONG FileInformationLength,
	IN ULONG FileInformationClass
);

IMP_SYSCALL NtDeviceIoControlFile
(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
);

IMP_SYSCALL NtQueryObject
(
    IN HANDLE ObjectHandle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN ULONG ObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

IMP_SYSCALL NtDuplicateObject
(
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Attributes,
    IN ULONG Options
);

IMP_SYSCALL NtOpenProcess
(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
);

IMP_SYSCALL NtQueryInformationThread
(
	IN HANDLE ThreadHandle,
	IN ULONG ThreadInformationClass,
	OUT PVOID ThreadInformation,
	IN ULONG ThreadInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

IMP_SYSCALL NtQueryInformationProcess
(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

IMP_SYSCALL NtDelayExecution
(
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Interval
);

IMP_SYSCALL NtOpenSection
(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
);

IMP_SYSCALL NtQueryIntervalProfile
(
    IN KPROFILE_SOURCE Source,
    OUT PULONG Interval
);

IMP_SYSCALL NtClose
(
    IN HANDLE Handle
);