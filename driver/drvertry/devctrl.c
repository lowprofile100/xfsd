//设备控制
//需要修改！！！！！将原从程序关于cdfs相关的数据结构修改成相关的xfs数据结构
//杨神加油！！！！

#include "XProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (XFS_BUG_CHECK_DEVCTRL)

//
//  Local support routines
//

NTSTATUS
XDevCtrlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, XCommonDevControl)
#endif


NTSTATUS
XCommonDevControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )



{
    NTSTATUS Status;

    TYPE_OF_OPEN TypeOfOpen;
// 需要修改成xfs相关的数据结构！！！！！！
    PFCB Fcb;
    PCCB Ccb;

    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;

    PVOID TargetBuffer = NULL;

    PAGED_CODE();

    //
    //  Extract and decode the file object.
    //

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = XDecodeFileObject( IrpContext,
                                     IrpSp->FileObject,
                                     &Fcb,
                                     &Ccb );

    //
    //  The only type of opens we accept are user volume opens.
    // 我们只接受用户卷打开

    if (TypeOfOpen != UserVolumeOpen) {

        XCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_XROM_READ_TOC) {

        //
        // ！！！！！！！！！！需修改！！！！！！！
        //  核实相关数据结构，检测卷是否改变

        XVerifyVcb( IrpContext, Fcb->Vcb );

    //
    //  Handle the case of the disk type ourselves.
    //

    } else if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_XROM_DISK_TYPE) {

        //
        // ！！！！！！！！！！需修改！！！！！！！
        //  核实相关数据结构，检测卷是否改变


        XVerifyVcb( IrpContext, Fcb->Vcb );

        //
        //  Check the size of the output buffer.
        //

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof( XROM_DISK_DATA )) {

            XCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
            return STATUS_BUFFER_TOO_SMALL;
        }

        //
        //  从相关数据结构上复制信息
        //

        ((PXROM_DISK_DATA) Irp->AssociatedIrp.SystemBuffer)->DiskData = Fcb->Vcb->DiskFlags;

        Irp->IoStatus.Information = sizeof( XROM_DISK_DATA );
        XCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    //
    //  得到栈位置，复制栈元素信息
    //

    NextIrpSp = IoGetNextIrpStackLocation( Irp );

    *NextIrpSp = *IrpSp;

    //
    //  Set up the completion routine
    //

    IoSetCompletionRoutine( Irp,
                            XDevCtrlCompletionRoutine,
                            NULL,
                            TRUE,
                            TRUE,
                            TRUE );

    //
    //  Send the request.
    //

    Status = IoCallDriver( IrpContext->Vcb->TargetDeviceObject, Irp );

    //
    //  清理IRP环境，驱动已经完成IRP
    //

    XCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


//
//  Local support routine
//

NTSTATUS
XDevCtrlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
    //
    //  Add the hack-o-ramma to fix formats.
    //

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Contxt );
}


