#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinIoCtl.h>
#include <Ntddscsi.h>
#include <iostream>
#include<string>
#include <cstdio>
#include <guiddef.h>
#include <shlobj_core.h>
#include<strsafe.h>
#define SCSIOP_WRITE 0x2A
#define SCSIOP_READ 0x28
#define SCSI_R FALSE
#define SCSI_W TRUE
#define PHYSICAL_SECTOR_SIZE 512
const GUID EFI_SYSTEM_PARTITION_GUID = { 0xC12A7328, 0xF81F, 0x11D2, {0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B} };
const GUID PARTITION_BASIC_DATA_GUID = { 0xEBD0A0A2, 0xB9E5, 0x4433, {0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7} };
BOOL SCSIBuild10CDB(PSCSI_PASS_THROUGH_DIRECT srb, ULONGLONG offset, ULONG length, BOOLEAN write);
BOOL SCSISectorIO(HANDLE hDrive, ULONGLONG offset, LPBYTE buffer, UINT buffSize, BOOLEAN write);
BOOL GetPhysicalDriveHandle(HANDLE& handle, int driveNumber);
BOOL SCSI_WriteDiskSector(HANDLE hFile, int Id, unsigned char* buffer);
BOOL SCSI_ReadDiskSector(HANDLE hFile, int Id, unsigned char* buffer);
int GetPhysicalDriveNumber();
std::string GetPartitiontype(HANDLE hDevice);
int GetSystemDiskPhysicalNumber();
int SetDriveLetterToEFI(const char* devicePath, char driveLetter);