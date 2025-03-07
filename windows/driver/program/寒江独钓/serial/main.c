#include <ntddk.h>
#include <ntstrsafe.h>

#define CCP_MAX_COM_ID 32    //�ٶ����ں���ൽ32
#define DELAY_ONE_MICROSECOND (-10)
#define DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)

static PDEVICE_OBJECT s_fltobj[CCP_MAX_COM_ID] = { 0 };
static PDEVICE_OBJECT s_nextobj[CCP_MAX_COM_ID] = { 0 };

NTSTATUS ccpAttachDevice(
    PDRIVER_OBJECT driver,
    PDEVICE_OBJECT oldobj,
    PDEVICE_OBJECT* fltobj,
    PDEVICE_OBJECT* next)
{
    NTSTATUS status;
    PDEVICE_OBJECT topdev = NULL;

    status = IoCreateDevice(
        driver,
        0,            //�豸��չ 0
        NULL,        //�豸�� NULL
        oldobj->DeviceType,    //�豸���ͣ�Ҫ���豸������豸����
        0,
        FALSE,
        fltobj);
    if (status != STATUS_SUCCESS)
        return status;

    //=======������Ҫ����=======
    if (oldobj->Flags & DO_BUFFERED_IO)
        (*fltobj)->Flags |= DO_BUFFERED_IO;
    if (oldobj->Flags & DO_DIRECT_IO)
        (*fltobj)->Flags |= DO_DIRECT_IO;
    if (oldobj->Characteristics & FILE_DEVICE_SECURE_OPEN)
        (*fltobj)->Characteristics |= FILE_DEVICE_SECURE_OPEN;
    (*fltobj)->Flags |= DO_POWER_PAGABLE;

    status = IoAttachDeviceToDeviceStackSafe(*fltobj, oldobj, &topdev);
    if (status != STATUS_SUCCESS)
    {
        IoDeleteDevice(*fltobj);
        *fltobj = NULL;
        return status;
    }
    *next = topdev;

    (*fltobj)->Flags = (*fltobj)->Flags & ~DO_DEVICE_INITIALIZING;    //�����λ�������豸���Ϊ������
    return STATUS_SUCCESS;
}

PDEVICE_OBJECT ccpOpenCom(ULONG id, NTSTATUS* status)
{
    UNICODE_STRING name_str;
    static WCHAR name[32] = { 0 };
    PFILE_OBJECT fileobj = NULL;
    PDEVICE_OBJECT devobj = NULL;

    memset(name, 0, sizeof(WCHAR) * 32);
    RtlStringCchPrintfW(name, 32, L"\\Device\\Serial%d", id);
    RtlInitUnicodeString(&name_str, name);

    *status = IoGetDeviceObjectPointer(&name_str,
        FILE_ALL_ACCESS,        //Ȩ��
        &fileobj,
        &devobj);
    if (*status == STATUS_SUCCESS)
        ObDereferenceObject(fileobj);        //�����ļ��������com�������ã����������

    return devobj;
}

void ccpAttachAllComs(PDRIVER_OBJECT driver)
{
    ULONG i;
    PDEVICE_OBJECT com_ob;
    NTSTATUS status;
    for (i = 0; i < CCP_MAX_COM_ID; i++)
    {
        com_ob = ccpOpenCom(i, &status);
        if (com_ob == NULL)
            continue;
        status = ccpAttachDevice(driver,
            com_ob,
            &s_fltobj[i],
            &s_nextobj[i]);
    }
}

NTSTATUS ccpDispatch(PDEVICE_OBJECT device, PIRP irp)
{
    // NTSTATUS status;
    ULONG i, j;
    PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
    for (i = 0; i < CCP_MAX_COM_ID; i++)
    {
        if (s_fltobj[i] == device)
        {
            if (irpsp->MajorFunction == IRP_MJ_POWER)        //��Դ����
            {
                PoStartNextPowerIrp(irp);
                IoSkipCurrentIrpStackLocation(irp);
                return PoCallDriver(s_nextobj[i], irp);
            }

            if (irpsp->MajorFunction == IRP_MJ_WRITE)        //д����
            {
                ULONG len = irpsp->Parameters.Write.Length;
                PUCHAR buf = NULL;
                if (irp->MdlAddress != NULL)
                    buf = (PUCHAR)MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
                else if (irp->UserBuffer != NULL)
                    buf = (PUCHAR)irp->UserBuffer;
                else
                    buf = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
                for (j = 0; j < len; j++)
                    DbgPrint("comcap: send: %2x", buf[j]);
                IoSkipCurrentIrpStackLocation(irp);
                return IoCallDriver(s_nextobj[i], irp);
            }
        }
    }
    irp->IoStatus.Information = 0;
    irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

void ccpUnload(PDRIVER_OBJECT drv)
{
    ULONG i;
    LARGE_INTEGER interval;

    drv;
    for (i = 0; i < CCP_MAX_COM_ID; i++)
    {
        if (s_nextobj[i] != NULL)
            IoDetachDevice(s_nextobj[i]);
    }

    interval.QuadPart = (5 * DELAY_ONE_SECOND);
    KeDelayExecutionThread(KernelMode, FALSE, &interval);

    for (i = 0; i < CCP_MAX_COM_ID; i++)
    {
        if (s_fltobj[i] != NULL)
            IoDeleteDevice(s_fltobj[i]);
    }
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
    size_t i;

    reg_path;
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        driver->MajorFunction[i] = ccpDispatch;    //�����еķַ�����������ΪccpDispatch
    }
    driver->DriverUnload = ccpUnload;

    ccpAttachAllComs(driver);

    return STATUS_SUCCESS;
}