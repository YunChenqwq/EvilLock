#include"DiskOperation.h"
using namespace std;
BOOL SCSIBuild10CDB(PSCSI_PASS_THROUGH_DIRECT srb, ULONGLONG offset, ULONG length, BOOLEAN Write) {
	if (!srb || offset >= 0x20000000000 || length < 1)
		return FALSE;
	LPBYTE cdb = srb->Cdb;
	if (Write == FALSE) {
		cdb[0] = SCSIOP_READ;
		cdb[1] = 0;
	}
	else {
		cdb[0] = SCSIOP_WRITE;
		cdb[1] = 0;
	}
	DWORD LBA = (DWORD)(offset / PHYSICAL_SECTOR_SIZE);
	cdb[2] = ((LPBYTE)&LBA)[3];
	cdb[3] = ((LPBYTE)&LBA)[2];
	cdb[4] = ((LPBYTE)&LBA)[1];
	cdb[5] = ((LPBYTE)&LBA)[0];
	cdb[6] = 0x00;

	WORD CDBTLen = (WORD)(length / PHYSICAL_SECTOR_SIZE);
	cdb[7] = ((LPBYTE)&CDBTLen)[1];
	cdb[8] = ((LPBYTE)&CDBTLen)[0];
	cdb[9] = 0x00;

	return TRUE;
}
BOOL SCSISectorIO(HANDLE hDrive, ULONGLONG offset, LPBYTE buffer, UINT buffSize, BOOLEAN write) {
	SCSI_PASS_THROUGH_DIRECT srb = { 0 };
	DWORD bytesReturned = 0;
	IO_SCSI_CAPABILITIES scap = { 0 };
	DWORD maxTransfLen = 8192;
	DWORD curSize = buffSize;
	LPBYTE tempBuff = NULL;
	static bool OneShotLog = false;

	BOOL retVal = 0;
	DWORD lastErr = 0;
	if (!buffer || !buffSize) return FALSE;

	retVal = DeviceIoControl(hDrive, IOCTL_SCSI_GET_CAPABILITIES, NULL, 0, &scap, sizeof(scap), &bytesReturned, NULL);
	lastErr = GetLastError();
	if (retVal)
		maxTransfLen = scap.MaximumTransferLength;

	RtlZeroMemory(&srb, sizeof(SCSI_PASS_THROUGH_DIRECT));
	srb.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	srb.CdbLength = 0xa;
	srb.SenseInfoLength = 0;
	srb.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);
	if (write)
		srb.DataIn = SCSI_IOCTL_DATA_OUT;
	else
		srb.DataIn = SCSI_IOCTL_DATA_IN;
	srb.TimeOutValue = 0x100;

	while (curSize) {
		if (curSize > maxTransfLen)
			srb.DataTransferLength = maxTransfLen;
		else {

			if ((curSize % PHYSICAL_SECTOR_SIZE) != 0)

				curSize = curSize + (PHYSICAL_SECTOR_SIZE - (curSize % PHYSICAL_SECTOR_SIZE));
			srb.DataTransferLength = curSize;
		}

		srb.DataBuffer = buffer;
		retVal = SCSIBuild10CDB(&srb, offset, srb.DataTransferLength, write);
		retVal = DeviceIoControl(hDrive, IOCTL_SCSI_PASS_THROUGH_DIRECT, (LPVOID)&srb, sizeof(SCSI_PASS_THROUGH_DIRECT),
			NULL, 0, &bytesReturned, NULL);
		lastErr = GetLastError();

		if (!retVal) break;
		else lastErr = 0;
		buffer += srb.DataTransferLength;
		curSize -= srb.DataTransferLength;
		offset += srb.DataTransferLength;
	}

	if (lastErr != ERROR_SUCCESS) {
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
BOOL GetPhysicalDriveHandle(HANDLE& handle, int DriveNumber)
{
	char drivestr[512];
	_snprintf_s(drivestr, 512, _TRUNCATE,"\\\\.\\PhysicalDrive%d", DriveNumber);
	//open it
	handle = CreateFileA(drivestr, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	return true;
}
/*SCSI写指定扇区*/
BOOL SCSI_WriteDiskSector(HANDLE hFile, int Id, unsigned char* buffer) {
	int offset = 0;
	int WirteSize = 0;
	DWORD Writed = 0;
	offset = Id * 512;
	if (buffer == NULL) {
		return FALSE;
	}
	//string own = "本功能来源于叶落模块,作者QQ393925220";
	SetFilePointer(hFile, offset, 0, 0);
	return SCSISectorIO(hFile, offset, buffer, PHYSICAL_SECTOR_SIZE, SCSI_W);
}
/*SCSI读指定扇区*/
BOOL SCSI_ReadDiskSector(HANDLE hFile, int Id, unsigned char* buffer) {
	int offset = 0;
	int WirteSize = 0;
	DWORD Writed = 0;
	offset = Id * 512;
	if (buffer == NULL) {
		return FALSE;
	}
	SetFilePointer(hFile, offset, 0, 0);
	return SCSISectorIO(hFile, offset, buffer, PHYSICAL_SECTOR_SIZE, SCSI_R);
}
/*获取物理驱动器数量*/
int GetPhysicalDriveNumber()
{
	int drivenumber;
	HANDLE hFile = NULL;
	char drivestr[512] = { 0 };
	for (drivenumber = 0;; drivenumber++)
	{
		_snprintf_s(drivestr, 512, _TRUNCATE, "\\\\.\\PhysicalDrive%d", drivenumber);
		hFile = CreateFileA(drivestr, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_RANDOM_ACCESS, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return drivenumber;
			break;
		}
		CloseHandle(hFile);
	}
	return drivenumber;
}