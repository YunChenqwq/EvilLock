#include <Windows.h>
#include <WinIoCtl.h>
#include <Ntddscsi.h>
#include <iostream>

#define SCSIOP_WRITE 0x2A
#define SCSIOP_READ 0x28
#define SCSI_R FALSE
#define SCSI_W TRUE
#define PHYSICAL_SECTOR_SIZE 512
BOOL SCSIBuild10CDB(PSCSI_PASS_THROUGH_DIRECT srb, ULONGLONG offset, ULONG length, BOOLEAN write);
BOOL SCSISectorIO(HANDLE hDrive, ULONGLONG offset, LPBYTE buffer, UINT buffSize, BOOLEAN write);
BOOL GetPhysicalDriveHandle(HANDLE& handle, int driveNumber);
BOOL SCSI_WriteDiskSector(HANDLE hFile, int Id, unsigned char* buffer);
BOOL SCSI_ReadDiskSector(HANDLE hFile, int Id, unsigned char* buffer);
int GetPhysicalDriveNumber();