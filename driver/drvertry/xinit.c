/*++

Module Name:

    XInit.c

Abstract:

    This module implements the DRIVER_INITIALIZATION routine for Xfs
启动驱动实体程序

--*/

#include "XProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (XFS_BUG_CHECK_XINIT)

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
XUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
XInitializeGlobalData (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FileSystemDeviceObject
    );

NTSTATUS
XShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, XUnload)
#pragma alloc_text(PAGE, XShutdown)
#pragma alloc_text(INIT, XInitializeGlobalData)
#endif


//
//  Local support routine
//

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

   创建驱动实体，启动驱动

Arguments:

    DriverObject - Pointer to driver object created by the system.

Return Value:

    NTSTATUS - The function value is the final status from the initialization
        operation.

--*/

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PDEVICE_OBJECT XfsFileSystemDeviceObject;

    //
    // Create the device object.
    //

    RtlInitUnicodeString( &UnicodeString, L"\\Xfs" );

    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_X_ROM_FILE_SYSTEM,
                             0,
                             FALSE,
                             &XfsFileSystemDeviceObject );

    if (!NT_SUCCESS( Status )) {
        return Status;
    }
    DriverObject->DriverUnload = XUnload;
  
    DriverObject->MajorFunction[IRP_MJ_CREATE]                  =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   =
    DriverObject->MajorFunction[IRP_MJ_READ]                    =
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]       =
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]         =
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION]=
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]       =
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]     =
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          =
    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]            =
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 =
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = (PDRIVER_DISPATCH) XFsdDispatch;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]                = XShutdown;

    DriverObject->FastIoDispatch = &XFastIoDispatch;

    Status = IoRegisterShutdownNotification (XfsFileSystemDeviceObject);
    if (!NT_SUCCESS (Status)) {
        IoDeleteDevice (XfsFileSystemDeviceObject);
        return Status;
    }

    //
    //  全局数据结构
    //

    Status = XInitializeGlobalData( DriverObject, XfsFileSystemDeviceObject );
    if (!NT_SUCCESS (Status)) {
        IoDeleteDevice (XfsFileSystemDeviceObject);
        return Status;
    }



    XfsFileSystemDeviceObject->Flags |= DO_LOW_PRIORITY_FILESYSTEM;

    IoRegisterFileSystem( XfsFileSystemDeviceObject );
    ObReferenceObject (XfsFileSystemDeviceObject);

    //
    //  And return to our caller
    //

    return( STATUS_SUCCESS );
}

NTSTATUS
XShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

//关闭管理器
   {
    IoUnregisterFileSystem (DeviceObject);
    IoDeleteDevice (XData.FileSystemDeviceObject);

    XCompleteRequest( NULL, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


VOID
XUnload(
    IN PDRIVER_OBJECT DriverObject
    )
//卸载

{
    PIRP_CONTEXT IrpContext;

    //
    // 释放IRP环境
    //
    while (1) {
        IrpContext = (PIRP_CONTEXT) PopEntryList( &XData.IrpContextList) ;
        if (IrpContext == NULL) {
            break;
        }
        ExFreePool (IrpContext);
    }

    IoFreeWorkItem (XData.CloseItem);
    ExDeleteResourceLite( &XData.DataResource );
    ObDereferenceObject (XData.FileSystemDeviceObject);
}

//
//  Local support routine
//

NTSTATUS
XInitializeGlobalData (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FileSystemDeviceObject
    )

//！！！！！注意！！！！！
//初始化全局数据结构

{
    //  分发历程。
    //  Start by initializing the FastIoDispatch Table.
    //

    RtlZeroMemory( &XFastIoDispatch, sizeof( FAST_IO_DISPATCH ));

    XFastIoDispatch.SizeOfFastIoDispatch =    sizeof(FAST_IO_DISPATCH);
    XFastIoDispatch.FastIoCheckIfPossible =   XFastIoCheckIfPossible;  //  CheckForFastIo
    XFastIoDispatch.FastIoRead =              FsRtlCopyRead;            //  Read
    XFastIoDispatch.FastIoQueryBasicInfo =    XFastQueryBasicInfo;     //  QueryBasicInfo
    XFastIoDispatch.FastIoQueryStandardInfo = XFastQueryStdInfo;       //  QueryStandardInfo
    XFastIoDispatch.FastIoLock =              XFastLock;               //  Lock
    XFastIoDispatch.FastIoUnlockSingle =      XFastUnlockSingle;       //  UnlockSingle
    XFastIoDispatch.FastIoUnlockAll =         XFastUnlockAll;          //  UnlockAll
    XFastIoDispatch.FastIoUnlockAllByKey =    XFastUnlockAllByKey;     //  UnlockAllByKey
    XFastIoDispatch.AcquireFileForNtCreateSection =  XAcquireForCreateSection;
    XFastIoDispatch.ReleaseFileForNtCreateSection =  XReleaseForCreateSection;
    XFastIoDispatch.FastIoQueryNetworkOpenInfo =     XFastQueryNetworkInfo;   //  QueryNetworkInfo

    //
    //  Initialize the XData structure.
    //

    RtlZeroMemory( &XData, sizeof( X_DATA ));

    XData.NodeTypeCode = XFS_NTC_DATA_HEADER;
    XData.NodeByteSize = sizeof( X_DATA );

    XData.DriverObject = DriverObject;
    XData.FileSystemDeviceObject = FileSystemDeviceObject;

    InitializeListHead( &XData.VcbQueue );

    ExInitializeResourceLite( &XData.DataResource );

    //
    //  Initialize the cache manager callback routines
    //   

    XData.CacheManagerCallbacks.AcquireForLazyWrite  = &XAcquireForCache;
    XData.CacheManagerCallbacks.ReleaseFromLazyWrite = &XReleaseFromCache;
    XData.CacheManagerCallbacks.AcquireForReadAhead  = &XAcquireForCache;
    XData.CacheManagerCallbacks.ReleaseFromReadAhead = &XReleaseFromCache;

    XData.CacheManagerVolumeCallbacks.AcquireForLazyWrite  = &XNoopAcquire;
    XData.CacheManagerVolumeCallbacks.ReleaseFromLazyWrite = &XNoopRelease;
    XData.CacheManagerVolumeCallbacks.AcquireForReadAhead  = &XNoopAcquire;
    XData.CacheManagerVolumeCallbacks.ReleaseFromReadAhead = &XNoopRelease;

    //
    //  Initialize the lock mutex and the async and delay close queues.
    //

    ExInitializeFastMutex( &XData.XDataMutex );
    InitializeListHead( &XData.AsyncCloseQueue );
    InitializeListHead( &XData.DelayedCloseQueue );

    XData.CloseItem = IoAllocateWorkItem (FileSystemDeviceObject);
    if (XData.CloseItem == NULL) {
        
        ExDeleteResourceLite( &XData.DataResource );
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    //
    //  Do the initialization based on the system size.
    //

    switch (MmQuerySystemSize()) {

    case MmSmallSystem:

        XData.IrpContextMaxDepth = 4;
        XData.MaxDelayedCloseCount = 8;
        XData.MinDelayedCloseCount = 2;
        break;

    case MmMediumSystem:

        XData.IrpContextMaxDepth = 8;
        XData.MaxDelayedCloseCount = 24;
        XData.MinDelayedCloseCount = 6;
        break;

    case MmLargeSystem:

        XData.IrpContextMaxDepth = 32;
        XData.MaxDelayedCloseCount = 72;
        XData.MinDelayedCloseCount = 18;
        break;
    }
    return STATUS_SUCCESS;
}

