#include <ntddk.h>

VOID DriverUnload(PDRIVER_OBJECT driver)
{
    driver;
    DbgPrint("hello world is unloading\r\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{
    reg_path;
    DbgPrint("hello world!");
    driver->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}