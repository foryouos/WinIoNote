#include <ntddk.h>
#include "OpenLibSys.h"

#define IOPM_SIZE 0x2000
typedef char IOPM[IOPM_SIZE];

// Function definition section
// -----------------------------------------------------------------
NTSTATUS WinIoDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
void WinIoUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS UnmapPhysicalMemory(HANDLE PhysicalMemoryHandle, PVOID pPhysMemLin, PVOID PhysSection);
//将物理内存映射到线性(虚拟)地址空间，以便驱动程序可以直接访问该内存
NTSTATUS MapPhysicalMemoryToLinearSpace(PVOID pPhysAddress,
	SIZE_T PhysMemSizeInBytes,
	PVOID* ppPhysMemLin,
	HANDLE* pPhysicalMemoryHandle,
	PVOID* ppPhysSection);

#ifndef AMD64
void Ke386SetIoAccessMap(int, IOPM*);
void Ke386QueryIoAccessMap(int, IOPM*);
void Ke386IoSetAccessProcess(PEPROCESS, int);
#endif

// -----------------------------------------------------------------

// Installable driver initialization entry point. 入口函数
// This entry point is called directly by the I/O system.

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING  DeviceNameUnicodeString;
	UNICODE_STRING  DeviceLinkUnicodeString;
	NTSTATUS        ntStatus;
	PDEVICE_OBJECT  DeviceObject = NULL;

	KdPrint(("Entering DriverEntry"));

	RtlInitUnicodeString(&DeviceNameUnicodeString, L"\\Device\\WinIoNote");

	// Create a device object  在新系统中，如果出现IO被拦截的问题，可以修改上诉Object的名称，重新签名避免被拦截

	ntStatus = IoCreateDevice(DriverObject,
		0,
		&DeviceNameUnicodeString,
		FILE_DEVICE_WINIO,
		0,
		FALSE,
		&DeviceObject);

	if (NT_SUCCESS(ntStatus))
	{
		// 为设备控制，创建，关闭操作设置分发函数
		// Create dispatch points for device control, create, close.
		//设备I/O 请求包(IRP)的不同主要功能，分别表示打开设备，关闭设备和设备控制请求
		DriverObject->MajorFunction[IRP_MJ_CREATE] =
			DriverObject->MajorFunction[IRP_MJ_CLOSE] =
			DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WinIoDispatch;
		DriverObject->DriverUnload = WinIoUnload; //设置驱动程序的卸载程序

		// Create a symbolic link, e.g. a name that a Win32 app can specify
		// to open the device.

		RtlInitUnicodeString(&DeviceLinkUnicodeString, L"\\DosDevices\\WinIoNote");
		// 在设备对象名称和设备的用户可见名称之间设置符号连接
		ntStatus = IoCreateSymbolicLink(&DeviceLinkUnicodeString,
			&DeviceNameUnicodeString);

		if (!NT_SUCCESS(ntStatus))
		{
			// Symbolic link creation failed- note this & then delete the
			// device object (it's useless if a Win32 app can't get at it).

			KdPrint(("ERROR: IoCreateSymbolicLink failed"));
			// 从系统中删除设备对象
			IoDeleteDevice(DeviceObject);
		}

	}
	else
	{
		KdPrint(("ERROR: IoCreateDevice failed"));
	}

	KdPrint(("Leaving DriverEntry"));

	return ntStatus;
}


// Process the IRPs sent to this device  分发驱动执行的不同的函数，实现不同的功能

NTSTATUS WinIoDispatch(IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	PIO_STACK_LOCATION IrpStack;
	ULONG              dwInputBufferLength;
	ULONG              dwOutputBufferLength;
	ULONG              dwIoControlCode;
	PVOID              pvIOBuffer;
	NTSTATUS           ntStatus;
	IOPM* pIOPM = NULL;
	struct             tagPhysStruct PhysStruct;
	struct             tagPortStruct PortStruct;
	struct             tagPhysStruct32* pPhysStruct32 = NULL;

	KdPrint(("Entering WinIoDispatch"));

	// Init to default settings

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	// 返回指定IRP中调用方I/O堆栈位置的指针
	IrpStack = IoGetCurrentIrpStackLocation(Irp);

	// Get the pointer to the input/output buffer and it's length

	pvIOBuffer = Irp->AssociatedIrp.SystemBuffer;
	dwInputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
	dwOutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	// 函数功能
	switch (IrpStack->MajorFunction)
	{
	case IRP_MJ_CREATE:

		KdPrint(("IRP_MJ_CREATE"));

		break;

	case IRP_MJ_CLOSE:

		KdPrint(("IRP_MJ_CLOSE"));

		break;

	case IRP_MJ_DEVICE_CONTROL:

		KdPrint(("IRP_MJ_DEVICE_CONTROL"));
		// 获取控制项
		dwIoControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;

		switch (dwIoControlCode)
		{
			// The Ke386 calls are no longer available on x64 Windows

#ifndef AMD64
		case IOCTL_WINIO_ENABLEDIRECTIO:

			KdPrint(("IOCTL_WINIO_ENABLEDIRECTIO"));

			pIOPM = MmAllocateNonCachedMemory(sizeof(IOPM));

			if (pIOPM)
			{
				RtlZeroMemory(pIOPM, sizeof(IOPM));

				Ke386IoSetAccessProcess(PsGetCurrentProcess(), 1);
				Ke386SetIoAccessMap(1, pIOPM);
				IrpStack->FileObject->FsContext2 = pIOPM;
			}
			else
				Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

			break;

		case IOCTL_WINIO_DISABLEDIRECTIO:

			KdPrint(("IOCTL_WINIO_DISABLEDIRECTIO"));

			pIOPM = (IOPM*)IrpStack->FileObject->FsContext2;

			if (pIOPM)
			{
				Ke386IoSetAccessProcess(PsGetCurrentProcess(), 0);
				Ke386SetIoAccessMap(1, pIOPM);
				MmFreeNonCachedMemory(pIOPM, sizeof(IOPM));
			}

			break;
#endif

		case IOCTL_WINIO_WRITEPORT: // IO读写访问

			KdPrint(("IOCTL_WINIO_WRITEPORT"));

			if (dwInputBufferLength)
			{
				memcpy(&PortStruct, pvIOBuffer, dwInputBufferLength);
				// 字节大小 对应的 8 16 和 32 位
				switch (PortStruct.bSize)
				{
				case 1:
					// 将字节写入指定的端口地址
					WRITE_PORT_UCHAR((PUCHAR)(USHORT)PortStruct.wPortAddr, (UCHAR)PortStruct.dwPortVal);
					break;

				case 2:
					WRITE_PORT_USHORT((PUSHORT)(USHORT)PortStruct.wPortAddr, (USHORT)PortStruct.dwPortVal);
					break;

				case 4:
					WRITE_PORT_ULONG((PULONG)(USHORT)PortStruct.wPortAddr, PortStruct.dwPortVal);
					break;
				}
			}
			else
				Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

			break;

		case IOCTL_WINIO_READPORT://从指定的端口地址读取字节

			KdPrint(("IOCTL_WINIO_READPORT"));

			if (dwInputBufferLength)
			{
				memcpy(&PortStruct, pvIOBuffer, dwInputBufferLength);

				switch (PortStruct.bSize)
				{
				case 1:
					PortStruct.dwPortVal = (ULONG)READ_PORT_UCHAR((PUCHAR)(USHORT)PortStruct.wPortAddr);
					break;

				case 2:
					PortStruct.dwPortVal = (ULONG)READ_PORT_USHORT((PUSHORT)(USHORT)PortStruct.wPortAddr);
					break;

				case 4:
					PortStruct.dwPortVal = READ_PORT_ULONG((PULONG)(USHORT)PortStruct.wPortAddr);
					break;
				}

				memcpy(pvIOBuffer, &PortStruct.dwPortVal, sizeof(PortStruct.dwPortVal));
				Irp->IoStatus.Information = sizeof(PortStruct.dwPortVal);
			}
			else
				Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

			break;

		case IOCTL_WINIO_MAPPHYSTOLIN:

			KdPrint(("IOCTL_WINIO_MAPPHYSTOLIN"));

			if (dwInputBufferLength)
			{
				memcpy(&PhysStruct, pvIOBuffer, dwInputBufferLength);
				//内存访问
				ntStatus = MapPhysicalMemoryToLinearSpace((PVOID)PhysStruct.pvPhysAddress,
					(SIZE_T)PhysStruct.dwPhysMemSizeInBytes,
					(PVOID*)&PhysStruct.pvPhysMemLin,
					(HANDLE*)&PhysStruct.PhysicalMemoryHandle,
					(PVOID*)&PhysStruct.pvPhysSection);

				if (NT_SUCCESS(ntStatus))
				{
					memcpy(pvIOBuffer, &PhysStruct, dwInputBufferLength);
					Irp->IoStatus.Information = dwInputBufferLength;
				}

				Irp->IoStatus.Status = ntStatus;
			}
			else
				Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

			break;

		case IOCTL_WINIO_UNMAPPHYSADDR:

			KdPrint(("IOCTL_WINIO_UNMAPPHYSADDR"));

			if (dwInputBufferLength)
			{
				memcpy(&PhysStruct, pvIOBuffer, dwInputBufferLength);

				ntStatus = UnmapPhysicalMemory((HANDLE)PhysStruct.PhysicalMemoryHandle, (PVOID)PhysStruct.pvPhysMemLin, (PVOID)PhysStruct.pvPhysSection);

				Irp->IoStatus.Status = ntStatus;
			}
			else
				Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

			break;

		default:

			KdPrint(("ERROR: Unknown IRP_MJ_DEVICE_CONTROL"));

			Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

			break;
		}

		break;
	}

	// DON'T get cute and try to use the status field of the irp in the
	// return status.  That IRP IS GONE as soon as you call IoCompleteRequest.

	ntStatus = Irp->IoStatus.Status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	// We never have pending operation so always return the status code.

	KdPrint(("Leaving WinIoDispatch"));

	return ntStatus;
}

// Delete the associated device and return

void WinIoUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING DeviceLinkUnicodeString;
	NTSTATUS ntStatus;

	KdPrint(("Entering WinIoUnload"));
	// 
	RtlInitUnicodeString(&DeviceLinkUnicodeString, L"\\DosDevices\\WinIoNote");

	ntStatus = IoDeleteSymbolicLink(&DeviceLinkUnicodeString);

	if (NT_SUCCESS(ntStatus))
	{
		IoDeleteDevice(DriverObject->DeviceObject);
	}
	else
	{
		KdPrint(("ERROR: IoDeleteSymbolicLink"));
	}

	KdPrint(("Leaving WinIoUnload"));
}


NTSTATUS MapPhysicalMemoryToLinearSpace(PVOID pPhysAddress,//要映射的内存地址
	SIZE_T PhysMemSizeInBytes,//映射物理内存大小
	PVOID* ppPhysMemLin,// 返回线性(虚拟)地址的指针
	HANDLE* pPhysicalMemoryHandle,//返回打开的物理内存句柄的指针
	PVOID* ppPhysSection)// 返回节(对象)的指针
{
	UNICODE_STRING     PhysicalMemoryUnicodeString; //用于存储物理内存设备的 Unicode 字符串表示。
	OBJECT_ATTRIBUTES  ObjectAttributes;//用于存储对象属性的结构体，指定打开句柄时的参数
	PHYSICAL_ADDRESS   ViewBase;//用于存储映射视图的基地址
	NTSTATUS           ntStatus;//用于存储函数的返回状态
	PHYSICAL_ADDRESS   pStartPhysAddress;//用于存储映射范围的起始和结束物理地址
	PHYSICAL_ADDRESS   pEndPhysAddress;
	BOOLEAN            Result1, Result2;//用于存储转换结果的布尔值。
	ULONG              IsIOSpace;//用于指示地址空间类型（内存或 I/O）
	unsigned char* pbPhysMemLin = NULL;// 指向映射到虚拟地址空间的物理内存的指针。

	KdPrint(("Entering MapPhysicalMemoryToLinearSpace"));
	//用于访问物理地址的设备对象
	RtlInitUnicodeString(&PhysicalMemoryUnicodeString, L"\\Device\\PhysicalMemory");
	// 打开句柄的例程的对象句柄的属性
	InitializeObjectAttributes(&ObjectAttributes,
		&PhysicalMemoryUnicodeString,
		OBJ_CASE_INSENSITIVE,
		(HANDLE)NULL,
		(PSECURITY_DESCRIPTOR)NULL);

	*pPhysicalMemoryHandle = NULL;
	*ppPhysSection = NULL;
	// 打开现有节对象的句柄。
	ntStatus = ZwOpenSection(pPhysicalMemoryHandle, SECTION_ALL_ACCESS, &ObjectAttributes);

	if (NT_SUCCESS(ntStatus))
	{
		// 对象句柄上提供访问验证
		ntStatus = ObReferenceObjectByHandle(*pPhysicalMemoryHandle,
			SECTION_ALL_ACCESS,
			(POBJECT_TYPE)NULL,
			KernelMode,
			ppPhysSection,
			(POBJECT_HANDLE_INFORMATION)NULL);

		if (NT_SUCCESS(ntStatus))
		{
			//检查对象引用是否成功
			pStartPhysAddress.QuadPart = (ULONGLONG)(ULONG_PTR)pPhysAddress;

			pEndPhysAddress.QuadPart = pStartPhysAddress.QuadPart + PhysMemSizeInBytes;

			IsIOSpace = 0; //表示内存地址 0x1表示I/O地址
			// This function translates a physical bus address to a physical system address.
			// 将物理总线地址转换为物理系统地址
			Result1 = HalTranslateBusAddress(1, 0, pStartPhysAddress, &IsIOSpace, &pStartPhysAddress);

			IsIOSpace = 0;

			Result2 = HalTranslateBusAddress(1, 0, pEndPhysAddress, &IsIOSpace, &pEndPhysAddress);

			if (Result1 && Result2)
			{
				// Let ZwMapViewOfSection pick a linear address

				PhysMemSizeInBytes = (SIZE_T)pEndPhysAddress.QuadPart - (SIZE_T)pStartPhysAddress.QuadPart;

				ViewBase = pStartPhysAddress;
				//计算映射所需的字节大小，并设置视图的基地址
				ntStatus = ZwMapViewOfSection(*pPhysicalMemoryHandle,
					(HANDLE)-1,
					&pbPhysMemLin,
					0L,
					PhysMemSizeInBytes,
					&ViewBase,
					&PhysMemSizeInBytes,
					ViewShare,
					0,
					PAGE_READWRITE | PAGE_NOCACHE);

				// If the physical memory is already mapped with a different caching attribute, try again
				//函数将物理内存映射到进程的虚拟地址空间。参数包括映射的大小、基地址和内存访问权限（读写和不缓存
				if (ntStatus == STATUS_CONFLICTING_ADDRESSES)
				{
					//将节的视图映射到主题进程的虚拟地址空间中
					ntStatus = ZwMapViewOfSection(*pPhysicalMemoryHandle,
						(HANDLE)-1,
						&pbPhysMemLin, //指向接受视图基址的变量的指针
						0L,
						PhysMemSizeInBytes, // 指定视图的初始提交区域的大小
						&ViewBase,
						&PhysMemSizeInBytes,
						ViewShare,
						0,
						PAGE_READWRITE); //指定要应用于映射视图的页面保护
				}

				//重新映射视图
				if (!NT_SUCCESS(ntStatus))
					KdPrint(("ERROR: ZwMapViewOfSection failed"));
				else
				{
					pbPhysMemLin += pStartPhysAddress.QuadPart - ViewBase.QuadPart;
					*ppPhysMemLin = pbPhysMemLin;
				}
			}
			else
				KdPrint(("ERROR: HalTranslateBusAddress failed"));
		}
		else
			KdPrint(("ERROR: ObReferenceObjectByHandle failed"));
	}
	else
		KdPrint(("ERROR: ZwOpenSection failed"));

	if (!NT_SUCCESS(ntStatus))
		ZwClose(*pPhysicalMemoryHandle); //关闭对象句柄

	KdPrint(("Leaving MapPhysicalMemoryToLinearSpace"));

	return ntStatus;
}


NTSTATUS UnmapPhysicalMemory(HANDLE PhysicalMemoryHandle, PVOID pPhysMemLin, PVOID pPhysSection)
{
	NTSTATUS ntStatus;

	KdPrint(("Entering UnmapPhysicalMemory"));
	// 从主题进程的虚拟地址空间中取消映射节的视图
	ntStatus = ZwUnmapViewOfSection((HANDLE)-1, pPhysMemLin);

	if (!NT_SUCCESS(ntStatus))
		KdPrint(("ERROR: UnmapViewOfSection failed"));

	if (pPhysSection)
		ObDereferenceObject(pPhysSection);//递减给定对象的引用次数并执行保留检查
	//例程关闭对象句柄
	ZwClose(PhysicalMemoryHandle);

	KdPrint(("Leaving UnmapPhysicalMemory"));

	return ntStatus;
}
