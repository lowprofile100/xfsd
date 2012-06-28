/************************************************************************
* 文件名称:Driver.cpp                                                 
* NT driver基本结构，包含driver实体，初始化函数，卸载函数等
*************************************************************************/

#include "Driver.h"
#include "tslib/read_file.h"

/************************************************************************
* 函数名称:DriverEntry
* 功能描述:初始化驱动程序，定位和申请硬件资源，创建内核对象
* 参数列表:
      pDriverObject:从I/O管理器中传进来的驱动对象
      pRegistryPath:驱动程序在注册表的中的路径
* 返回 值:返回初始化驱动状态
*************************************************************************/
#pragma INITCODE
extern "C" NTSTATUS DriverEntry (
			IN PDRIVER_OBJECT pDriverObject,
			IN PUNICODE_STRING pRegistryPath	) 
{
	NTSTATUS status;
	KdPrint(("Enter DriverEntry\n"));

	//注册其他驱动调用函数入口
	pDriverObject->DriverUnload = HelloDDKUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]       =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]         =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION]=HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]       =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]     =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]            =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_CLEANUP]                 =HelloDDKDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_PNP]                     =HelloDDKDispatchRoutine;
	//read函数！
	pDriverObject->MajorFunction[IRP_MJ_READ] = ReadRoutine;
	//write函数
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = WriteRoutine;

	//创建驱动设备对象
	status = CreateDevice(pDriverObject);

	KdPrint(("DriverEntry end\n"));
	return status;
}

/************************************************************************
* 函数名称:CreateDevice
* 功能描述:初始化设备对象
* 参数列表:
      pDriverObject:从I/O管理器中传进来的驱动对象
* 返回 值:返回初始化状态
*************************************************************************/
#pragma INITCODE
NTSTATUS CreateDevice (
		IN PDRIVER_OBJECT	pDriverObject) 
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
	
	//创建设备名称
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName,L"\\Device\\XFSDevice");
	
	//创建设备
	status = IoCreateDevice( pDriverObject,
						sizeof(DEVICE_EXTENSION),
						&(UNICODE_STRING)devName,
						FILE_DEVICE_FILE_SYSTEM,//驱动对象的类型！！！！？
						0, TRUE,
						&pDevObj );
	//判断设备是否成功
	if (!NT_SUCCESS(status))
		return status;
	//设备设置为缓冲区读写设备
	pDevObj->Flags |= DO_BUFFERED_IO;
	//得到设备扩展
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	//设备扩展的设备对象
	pDevExt->pDevice = pDevObj;
	//设备扩展中的设备名称
	pDevExt->ustrDeviceName = devName;
	//创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName,L"\\??\\XFSD");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink( &symLinkName,&devName );
	//判断符号链接是否成功
	if (!NT_SUCCESS(status)) 
	{
		IoDeleteDevice( pDevObj );
		return status;
	}
	return STATUS_SUCCESS;
}

/************************************************************************
* 函数名称:HelloDDKUnload
* 功能描述:负责驱动程序的卸载操作
* 参数列表:
      pDriverObject:驱动对象
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
VOID HelloDDKUnload (IN PDRIVER_OBJECT pDriverObject) 
{
	PDEVICE_OBJECT	pNextObj;
	KdPrint(("Enter DriverUnload\n"));
	//得到下一个设备
	pNextObj = pDriverObject->DeviceObject;
	//枚举所有设备对象
	while (pNextObj != NULL) 
	{
		//得到设备扩展
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;

		//删除符号链接
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;
		//删除设备
		IoDeleteDevice( pDevExt->pDevice );
	}
}

/************************************************************************
* 函数名称:HelloDDKDispatchRoutine
* 功能描述:对所有IRP进行直接返回处理
* 参数列表:
      pDevObj:功能设备对象
      pIrp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("No such fuction yet\n"));
	NTSTATUS status = STATUS_SUCCESS;
	// 完成IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDKDispatchRoutine\n"));
	return status;
}

/************************************************************************
* 函数名称:WriteRoutine
* 功能描述:对所有IRP进行直接返回处理
* 参数列表:
      pDevObj:功能设备对象
      pIrp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS WriteRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("No such fuction yet\n"));
	NTSTATUS status = STATUS_SUCCESS;
	// 完成IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDKDispatchRoutine\n"));
	return status;
}

/************************************************************************
* 函数名称:ReadRoutine
* 功能描述:对读IRP进行处理
* 参数列表:
      pDevObj:功能设备对象
      pIrp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS ReadRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("ReadRoutine on.\n"));
	//对一般IRP的简单操作，后面会介绍对IRP更复杂的操作

	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulReadLength = stack->Parameters.Read.Length;
	ULONG ulReadOffset = (ULONG)stack->Parameters.Read.ByteOffset.QuadPart;

char buff2[10240];
char *addr;
	//读文件啦
   init_read_file_from_disk();
   long b=read_file_from_disk( addr,buff2,sizeof(buff2));
	
		//将数据存储在AssociatedIrp.SystemBuffer，以便应用程序使用
		memcpy(pIrp->AssociatedIrp.SystemBuffer,buff2,ulReadLength);
		status = STATUS_SUCCESS;
	
	
	// 完成IRP
	pIrp->IoStatus.Status = status;
	//设置IRP操作了多少字节
	pIrp->IoStatus.Information = ulReadLength;	// bytes xfered

	

	//处理IRP
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	KdPrint(("Leave HelloDDKRead\n"));

	return status;

}



