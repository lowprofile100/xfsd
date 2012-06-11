//关闭
//需要将相关数据结构修改成xfs系统的数据结构

#include "XProcs.h"

//
//  The Bug check file id for this module
//

#define BugCheckFileId                   (XFS_BUG_CHECK_CLOSE)

//
//  Local support routines
//

BOOLEAN
XCommonClosePrivate (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN FromFsd
    );

VOID
XQueueClose (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN DelayedClose
    );

PIRP_CONTEXT
XRemoveClose (
    IN PVCB Vcb OPTIONAL
    );

VOID
XCloseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, XCommonClose)
#pragma alloc_text(PAGE, XCommonClosePrivate)
#pragma alloc_text(PAGE, XQueueClose)
#pragma alloc_text(PAGE, XRemoveClose)
#pragma alloc_text(PAGE, XCloseWorker)
#endif


VOID
XFspClose (
    IN PVCB Vcb OPTIONAL
    )

/*++

Routine Description:

    This routine is called to process the close queues in the XData.  If the
    Vcb is passed then we want to remove all of the closes for this Vcb.
    Otherwise we will do as many of the delayed closes as we need to do.

Arguments:

    Vcb - If specified then we are looking for all of the closes for the
        given Vcb.

Return Value:

    None

--*/

{
    PIRP_CONTEXT IrpContext;
    IRP_CONTEXT StackIrpContext;

    THREAD_CONTEXT ThreadContext;

    PFCB Fcb;
    ULONG UserReference;

    ULONG VcbHoldCount = 0;
    PVCB CurrentVcb = NULL;

    BOOLEAN PotentialVcbTeardown = FALSE;

    PAGED_CODE();

    FsRtlEnterFileSystem();

    //
    //  Continue processing until there are no more closes to process.
    //

    while (IrpContext = XRemoveClose( Vcb )) {

        //
        //  If we don't have an IrpContext then use the one on the stack.
        //  Initialize it for this request.
        //

        if (SafeNodeType( IrpContext ) != XFS_NTC_IRP_CONTEXT ) {

            //
            //  Update the local values from the IrpContextLite.
            //

            Fcb = ((PIRP_CONTEXT_LITE) IrpContext)->Fcb;
            UserReference = ((PIRP_CONTEXT_LITE) IrpContext)->UserReference;

            //
            //  Update the stack irp context with the values from the
            //  IrpContextLite.
            //

            XInitializeStackIrpContext( &StackIrpContext,
                                         (PIRP_CONTEXT_LITE) IrpContext );

            //
            //  Free the IrpContextLite.
            //

            XFreeIrpContextLite( (PIRP_CONTEXT_LITE) IrpContext );

            //
            //  Remember we have the IrpContext from the stack.
            //

            IrpContext = &StackIrpContext;

        //
        //  Otherwise cleanup the existing IrpContext.
        //

        } else {

            //
            //  Remember the Fcb and user reference count.
            //

            Fcb = (PFCB) IrpContext->Irp;
            IrpContext->Irp = NULL;

            UserReference = (ULONG) IrpContext->ExceptionStatus;
            IrpContext->ExceptionStatus = STATUS_SUCCESS;
        }

        //
        //  We have an IrpContext.  Now we need to set the top level thread
        //  context.
        //

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FSP_FLAGS );

        //
        //  If we were given a Vcb then there is a request on top of this.
        //

        if (ARGUMENT_PRESENT( Vcb )) {

            ClearFlag( IrpContext->Flags,
                       IRP_CONTEXT_FLAG_TOP_LEVEL | IRP_CONTEXT_FLAG_TOP_LEVEL_XFS );
        }

        XSetThreadContext( IrpContext, &ThreadContext );

        //
        //  If we have hit the maximum number of requests to process without
        //  releasing the Vcb then release the Vcb now.  If we are holding
        //  a different Vcb to this one then release the previous Vcb.
        //
        //  In either case acquire the current Vcb.
        //
        //  We use the MinDelayedCloseCount from the XData since it is
        //  a convenient value based on the system size.  Only thing we are trying
        //  to do here is prevent this routine starving other threads which
        //  may need this Vcb exclusively.
        //
        //  Note that the check for potential teardown below is unsafe.  We'll 
        //  repeat later within the Xdata lock.
        //

        PotentialVcbTeardown = !ARGUMENT_PRESENT( Vcb ) &&
                               (Fcb->Vcb->VcbCondition != VcbMounted) &&
                               (Fcb->Vcb->VcbCondition != VcbMountInProgress) &&
                               (Fcb->Vcb->VcbCleanup == 0);

        if (PotentialVcbTeardown ||
            (VcbHoldCount > XData.MinDelayedCloseCount) ||
            (Fcb->Vcb != CurrentVcb)) {

            if (CurrentVcb != NULL) {

                XReleaseVcb( IrpContext, CurrentVcb );
            }

            if (PotentialVcbTeardown) {

                XAcquireXData( IrpContext );

                //
                //  Repeat the checks with global lock held.  The volume could have
                //  been remounted while we didn't hold the lock.
                //

                PotentialVcbTeardown = !ARGUMENT_PRESENT( Vcb ) &&
                                       (Fcb->Vcb->VcbCondition != VcbMounted) &&
                                       (Fcb->Vcb->VcbCondition != VcbMountInProgress) &&
                                       (Fcb->Vcb->VcbCleanup == 0);
                                
                if (!PotentialVcbTeardown)  {

                    XReleaseXData( IrpContext);
                }
            }

            CurrentVcb = Fcb->Vcb;
            XAcquireVcbShared( IrpContext, CurrentVcb, FALSE );

            VcbHoldCount = 0;

        } else {

            VcbHoldCount += 1;
        }

        //
        //  Call our worker routine to perform the close operation.
        //

        XCommonClosePrivate( IrpContext, CurrentVcb, Fcb, UserReference, FALSE );

        //
        //  If the reference count on this Vcb is below our residual reference
        //  then check if we should dismount the volume.
        //

        if (PotentialVcbTeardown) {

            XReleaseVcb( IrpContext, CurrentVcb );
            XCheckForDismount( IrpContext, CurrentVcb, FALSE );

            CurrentVcb = NULL;

            XReleaseXData( IrpContext );
            PotentialVcbTeardown = FALSE;
        }

        //
        //  Complete the current request to cleanup the IrpContext.
        //

        XCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
    }

    //
    //  Release any Vcb we may still hold.
    //

    if (CurrentVcb != NULL) {

        XReleaseVcb( IrpContext, CurrentVcb );

    }

    FsRtlExitFileSystem();
}


NTSTATUS
XCommonClose (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is the Fsd entry for the close operation.  We decode the file
    object to find the XFS structures and type of open.  We call our internal
    worker routine to perform the actual work.  If the work wasn't completed
    then we post to one of our worker queues.  The Ccb isn't needed after this
    point so we delete the Ccb and return STATUS_SUCCESS to our caller in all
    cases.

Arguments:

    Irp - Supplies the Irp to process

Return Value:

    STATUS_SUCCESS

--*/

{
    TYPE_OF_OPEN TypeOfOpen;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    ULONG UserReference = 0;

    BOOLEAN PotentialVcbTeardown = FALSE;
    BOOLEAN ForceDismount = FALSE;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

    //
    //  If we were called with our file system device object instead of a
    //  volume device object, just complete this request with STATUS_SUCCESS.
    //

    if (IrpContext->Vcb == NULL) {

        XCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    //
    //  Decode the file object to get the type of open and Fcb/Ccb.
    //

    TypeOfOpen = XDecodeFileObject( IrpContext,
                                     IoGetCurrentIrpStackLocation( Irp )->FileObject,
                                     &Fcb,
                                     &Ccb );

    //
    //  No work to do for unopened file objects.
    //

    if (TypeOfOpen == UnopenedFileObject) {

        XCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

        return STATUS_SUCCESS;
    }

    Vcb = Fcb->Vcb;

    //
    //  Clean up any CCB associated with this open.
    //
    
    if (Ccb != NULL) {

        UserReference = 1;

        //
        //  Was a FSCTL_DISMOUNT issued on this handle?  If so,  we need to
        //  force a dismount of the volume now.
        //
        
        ForceDismount = BooleanFlagOn( Ccb->Flags, CCB_FLAG_DISMOUNT_ON_CLOSE);

        //
        //  We can always deallocate the Ccb if present.
        //

        XDeleteCcb( IrpContext, Ccb );
    }

    //
    //  If this is the last reference to a user file or directory on a 
    //  currently mounted volume, then post it to the delayed close queue.  Note
    //  that the VcbCondition check is unsafe,  but it doesn't really matter -
    //  we just might delay the volume teardown a little by posting this close.
    //

    if ((Vcb->VcbCondition == VcbMounted) &&
        (Fcb->FcbReference == 1) &&
        ((TypeOfOpen == UserFileOpen) ||
         (TypeOfOpen == UserDirectoryOpen))) {

        XQueueClose( IrpContext, Fcb, UserReference, TRUE );
        IrpContext = NULL;

    //
    //  Otherwise try to process this close.  Post to the async close queue
    //  if we can't acquire all of the resources.
    //

    } else {

        //
        //  If we may be dismounting this volume then acquire the XData
        //  resource.
        //
        //  Since we now must make volumes go away as soon as reasonable after
        //  the last user handles closes, key off of the cleanup count.  It is
        //  OK to do this more than neccesary.  Since this Fcb could be holding
        //  a number of other Fcbs (and thus their references), a simple check
        //  on reference count is not appropriate.
        //
        //  Do an unsafe check first to avoid taking the (global) Xdata lock in the 
        //  common case.
        //

        if (((Vcb->VcbCleanup == 0) || ForceDismount) &&
            (Vcb->VcbCondition != VcbMounted))  {

            //
            //  Possible.  Acquire XData to synchronise with the remount path,  and
            //  then repeat the tests.
            //
            //  Note that we must send the notification outside of any locks,  since 
            //  the worker that processes the notify could also be calling into our 
            //  pnp path which wants both XData and VcbResource.  For a force dismount
            //  the volume will be marked invalid (no going back),  so we will definitely
            //  go ahead and dismount below.
            //

            if (ForceDismount)  {
            
                //
                //  Send notification.
                //
                
                FsRtlNotifyVolumeEvent( IoGetCurrentIrpStackLocation( Irp )->FileObject, 
                                        FSRTL_VOLUME_DISMOUNT );
            }
            
            XAcquireXData( IrpContext );

            if (((Vcb->VcbCleanup == 0) || ForceDismount) &&
                (Vcb->VcbCondition != VcbMounted) &&
                (Vcb->VcbCondition != VcbMountInProgress) &&
                FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TOP_LEVEL_XFS ))  {

                PotentialVcbTeardown = TRUE;
            }
            else {

                //
                //  We can't dismount this volume now,  there are other references or
                //  it's just been remounted.
                //

                XReleaseXData( IrpContext);
            }
        }

        if (ForceDismount)  {
        
            //
            //  Physically disconnect this Vcb from the device so a new mount can
            //  occur.  Vcb deletion cannot happen at this time since there is
            //  a handle on it associated with this very request,  but we'll call
            //  check for dismount again later anyway.
            //

            XCheckForDismount( IrpContext, Vcb, TRUE );
        }
        
        //
        //  Call the worker routine to perform the actual work.  This routine
        //  should never raise except for a fatal error.
        //

        if (!XCommonClosePrivate( IrpContext, Vcb, Fcb, UserReference, TRUE )) {

            //
            //  If we didn't complete the request then post the request as needed.
            //

            XQueueClose( IrpContext, Fcb, UserReference, FALSE );
            IrpContext = NULL;

        //
        //  Check whether we should be dismounting the volume and then complete
        //  the request.
        //

        } else if (PotentialVcbTeardown) {

            XCheckForDismount( IrpContext, Vcb, FALSE );
        }
    }

    //
    //  Always complete this request with STATUS_SUCCESS.
    //

    XCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    if (PotentialVcbTeardown) {

        XReleaseXData( IrpContext );
    }

    //
    //  Always return STATUS_SUCCESS for closes.
    //

    return STATUS_SUCCESS;
}


//
//  Local support routine
//

BOOLEAN
XCommonClosePrivate (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN FromFsd
    )

/*++

Routine Description:

    This is the worker routine for the close operation.  We can be called in
    an Fsd thread or from a worker Fsp thread.  If called from the Fsd thread
    then we acquire the resources without waiting.  Otherwise we know it is
    safe to wait.

    We check to see whether we should post this request to the delayed close
    queue.  If we are to process the close here then we acquire the Vcb and
    Fcb.  We will adjust the counts and call our teardown routine to see
    if any of the structures should go away.

Arguments:

    Vcb - Vcb for this volume.

    Fcb - Fcb for this request.

    UserReference - Number of user references for this file object.  This is
        zero for an internal stream.

    FromFsd - This request was called from an Fsd thread.  Indicates whether
        we should wait to acquire resources.

    DelayedClose - Address to store whether we should try to put this on
        the delayed close queue.  Ignored if this routine can process this
        close.

Return Value:

    BOOLEAN - TRUE if this thread processed the close, FALSE otherwise.

--*/

{
    BOOLEAN RemovedFcb;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    //
    //  Try to acquire the Vcb and Fcb.  If we can't acquire them then return
    //  and let our caller know he should post the request to the async
    //  queue.
    //

    if (XAcquireVcbShared( IrpContext, Vcb, FromFsd )) {

        if (!XAcquireFcbExclusive( IrpContext, Fcb, FromFsd )) {

            //
            //  We couldn't get the Fcb.  Release the Vcb and let our caller
            //  know to post this request.
            //

            XReleaseVcb( IrpContext, Vcb );
            return FALSE;
        }

    //
    //  We didn't get the Vcb.  Let our caller know to post this request.
    //

    } else {

        return FALSE;
    }

    //
    //  Lock the Vcb and decrement the reference counts.
    //

    XLockVcb( IrpContext, Vcb );
    XDecrementReferenceCounts( IrpContext, Fcb, 1, UserReference );
    XUnlockVcb( IrpContext, Vcb );

    //
    //  Call our teardown routine to see if this object can go away.
    //  If we don't remove the Fcb then release it.
    //

    XTeardownStructures( IrpContext, Fcb, &RemovedFcb );

    if (!RemovedFcb) {

        XReleaseFcb( IrpContext, Fcb );
    }

    //
    //  Release the Vcb and return to our caller.  Let him know we completed
    //  this request.
    //

    XReleaseVcb( IrpContext, Vcb );

    return TRUE;
}

VOID
XCloseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
/*++

Routine Description:

    Worker routine to call CsFspClose.

Arguments:

    DeviceObject - Filesystem registration device object

    Context - Callers context

Return Value:

    None

--*/

{
    XFspClose (NULL);
}


VOID
XQueueClose (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG UserReference,
    IN BOOLEAN DelayedClose
    )

/*++

Routine Description:

    This routine is called to queue a request to either the async or delayed
    close queue.  For the delayed queue we need to allocate a smaller
    structure to contain the information about the file object.  We do
    that so we don't put the larger IrpContext structures into this long
    lived queue.  If we can allocate this structure then we put this
    on the async queue instead.

Arguments:

    Fcb - Fcb for this file object.

    UserReference - Number of user references for this file object.  This is
        zero for an internal stream.

    DelayedClose - Indicates whether this should go on the async or delayed
        close queue.

Return Value:

    None

--*/

{
    PIRP_CONTEXT_LITE IrpContextLite = NULL;
    BOOLEAN StartWorker = FALSE;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB( Fcb );

    //
    //  Start with the delayed queue request.  We can move this to the async
    //  queue if there is an allocation failure.
    //

    if (DelayedClose) {

        //
        //  Try to allocate non-paged pool for the IRP_CONTEXT_LITE.
        //

        IrpContextLite = XCreateIrpContextLite( IrpContext );
    }

    //
    //  We want to clear the top level context in this thread if
    //  necessary.  Call our cleanup routine to do the work.
    //

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_MORE_PROCESSING );
    XCleanupIrpContext( IrpContext, TRUE );

    //
    //  Synchronize with the XData lock.
    //

    XLockXData();

    //
    //  If we have an IrpContext then put the request on the delayed close queue.
    //

    if (IrpContextLite != NULL) {

        //
        //  Initialize the IrpContextLite.
        //

        IrpContextLite->NodeTypeCode = XFS_NTC_IRP_CONTEXT_LITE;
        IrpContextLite->NodeByteSize = sizeof( IRP_CONTEXT_LITE );
        IrpContextLite->Fcb = Fcb;
        IrpContextLite->UserReference = UserReference;
        IrpContextLite->RealDevice = IrpContext->RealDevice;

        //
        //  Add this to the delayed close list and increment
        //  the count.
        //

        InsertTailList( &XData.DelayedCloseQueue,
                        &IrpContextLite->DelayedCloseLinks );

        XData.DelayedCloseCount += 1;

        //
        //  If we are above our threshold then start the delayed
        //  close operation.
        //

        if (XData.DelayedCloseCount > XData.MaxDelayedCloseCount) {

            XData.ReduceDelayedClose = TRUE;

            if (!XData.FspCloseActive) {

                XData.FspCloseActive = TRUE;
                StartWorker = TRUE;
            }
        }

        //
        //  Unlock the XData.
        //

        XUnlockXData();

        //
        //  Cleanup the IrpContext.
        //

        XCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    //
    //  Otherwise drop into the async case below.
    //

    } else {

        //
        //  Store the information about the file object into the IrpContext.
        //

        IrpContext->Irp = (PIRP) Fcb;
        IrpContext->ExceptionStatus = (NTSTATUS) UserReference;

        //
        //  Add this to the async close list and increment the count.
        //

        InsertTailList( &XData.AsyncCloseQueue,
                        &IrpContext->WorkQueueItem.List );

        XData.AsyncCloseCount += 1;

        //
        //  Remember to start the Fsp close thread if not currently started.
        //

        if (!XData.FspCloseActive) {

            XData.FspCloseActive = TRUE;

            StartWorker = TRUE;
        }

        //
        //  Unlock the XData.
        //

        XUnlockXData();
    }

    //
    //  Start the FspClose thread if we need to.
    //

    if (StartWorker) {

        IoQueueWorkItem( XData.CloseItem, XCloseWorker, CriticalWorkQueue, NULL );
    }

    //
    //  Return to our caller.
    //

    return;
}


//
//  Local support routine
//

PIRP_CONTEXT
XRemoveClose (
    IN PVCB Vcb OPTIONAL
    )

/*++

Routine Description:

Arguments:

    This routine is called to scan the async and delayed close queues looking
    for a suitable entry.  If the Vcb is specified then we scan both queues
    looking for an entry with the same Vcb.  Otherwise we will look in the
    async queue first for any close item.  If none found there then we look
    in the delayed close queue provided that we have triggered the delayed
    close operation.

Return Value:

    PIRP_CONTEXT - NULL if no work item found.  Otherwise it is the pointer to
        either the IrpContext or IrpContextLite for this request.

--*/

{
    PIRP_CONTEXT IrpContext = NULL;
    PIRP_CONTEXT NextIrpContext;
    PIRP_CONTEXT_LITE NextIrpContextLite;

    PLIST_ENTRY Entry;

    PAGED_CODE();

    ASSERT_OPTIONAL_VCB( Vcb );

    //
    //  Lock the XData to perform the scan.
    //

    XLockXData();

    //
    //  First check the list of async closes.
    //

    Entry = XData.AsyncCloseQueue.Flink;

    while (Entry != &XData.AsyncCloseQueue) {

        //
        //  Extract the IrpContext.
        //

        NextIrpContext = CONTAINING_RECORD( Entry,
                                            IRP_CONTEXT,
                                            WorkQueueItem.List );

        //
        //  If no Vcb was specified or this Vcb is for our volume
        //  then perform the close.
        //

        if (!ARGUMENT_PRESENT( Vcb ) || (NextIrpContext->Vcb == Vcb)) {

            RemoveEntryList( Entry );
            XData.AsyncCloseCount -= 1;

            IrpContext = NextIrpContext;
            break;
        }

        //
        //  Move to the next entry.
        //

        Entry = Entry->Flink;
    }

    //
    //  If we didn't find anything look through the delayed close
    //  queue.
    //
    //  We will only check the delayed close queue if we were given
    //  a Vcb or the delayed close operation is active.
    //

    if ((IrpContext == NULL) &&
        (ARGUMENT_PRESENT( Vcb ) ||
         (XData.ReduceDelayedClose &&
          (XData.DelayedCloseCount > XData.MinDelayedCloseCount)))) {

        Entry = XData.DelayedCloseQueue.Flink;

        while (Entry != &XData.DelayedCloseQueue) {

            //
            //  Extract the IrpContext.
            //

            NextIrpContextLite = CONTAINING_RECORD( Entry,
                                                    IRP_CONTEXT_LITE,
                                                    DelayedCloseLinks );

            //
            //  If no Vcb was specified or this Vcb is for our volume
            //  then perform the close.
            //

            if (!ARGUMENT_PRESENT( Vcb ) || (NextIrpContextLite->Fcb->Vcb == Vcb)) {

                RemoveEntryList( Entry );
                XData.DelayedCloseCount -= 1;

                IrpContext = (PIRP_CONTEXT) NextIrpContextLite;
                break;
            }

            //
            //  Move to the next entry.
            //

            Entry = Entry->Flink;
        }
    }

    //
    //  If the Vcb wasn't specified and we couldn't find an entry
    //  then turn off the Fsp thread.
    //

    if (!ARGUMENT_PRESENT( Vcb ) && (IrpContext == NULL)) {

        XData.FspCloseActive = FALSE;
        XData.ReduceDelayedClose = FALSE;
    }

    //
    //  Unlock the XData.
    //

    XUnlockXData();

    return IrpContext;
}



