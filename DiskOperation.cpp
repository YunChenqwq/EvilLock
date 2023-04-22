#include"DiskOperation.h"
using namespace std;
struct MBR {
	unsigned char codeArea[446]; // ������������
	struct Partition {
		unsigned char status;    // ����״̬
		unsigned char start_head; // ��ʼ��ͷ��
		unsigned char start_sec;  // ��ʼ������
		unsigned char start_cyl;  // ��ʼ�����
		unsigned char type;       // ��������
		unsigned char end_head;   // ������ͷ��
		unsigned char end_sec;    // ����������
		unsigned char end_cyl;    // ���������
		unsigned int start_lba;   // ������ʼ��ַ��LBA
		unsigned int size;        // ������С������������ʾ
	} partition[4];
	unsigned short signature; // MBRǩ����0xAA55��
};
struct GPTHeader {
	char signature[8];          // GPTͷ��ǩ����"EFI PART"��
	uint32_t revision;          // GPT�汾��
	uint32_t header_size;       // GPTͷ����С
	uint32_t header_crc32;      // GPTͷ��CRC32У����
	uint32_t reserved1;         // �����ֶΣ�����Ϊ0��
	uint64_t current_lba;       // GPTͷ������LBA��ַ
	uint64_t backup_lba;        // GPT����ͷ������LBA��ַ
	uint64_t first_lba;         // ��һ��������Ŀ����LBA��ַ
	uint64_t last_lba;          // ���һ��������Ŀ����LBA��ַ
	uint8_t disk_guid[16];      // ����GUID
	uint64_t partition_array_lba; // ����������������LBA��ַ
	uint32_t partition_count;   // ��������
	uint32_t partition_entry_size; // ���������С
	uint32_t partition_array_crc32; // ������������CRC32У����
};

// GPT��������ṹ��
struct GPTPartitionEntry {
	uint8_t type_guid[16];      // ��������GUID
	uint8_t partition_guid[16]; // ����GUID
	uint64_t start_lba;         // ������ʼLBA��ַ
	uint64_t end_lba;           // ��������LBA��ַ
	uint64_t attributes;        // ��������
	unsigned char name[72];     // ��������
};
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
/*��ȡ���������*/
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
/*SCSIдָ������*/
BOOL SCSI_WriteDiskSector(HANDLE hFile, int Id, unsigned char* buffer) {
	int offset = 0;
	int WirteSize = 0;
	DWORD Writed = 0;
	offset = Id * 512;
	if (buffer == NULL) {
		return FALSE;
	}
	//string own = "��������Դ��Ҷ��ģ��,����QQ393925220";
	SetFilePointer(hFile, offset, 0, 0);
	return SCSISectorIO(hFile, offset, buffer, PHYSICAL_SECTOR_SIZE, SCSI_W);
}
/*SCSI��ָ������*/
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
/*��ȡ��������������*/
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
/*��ȡӲ�̷�������*/
string GetPartitiontype(HANDLE hDevice)
{
		int nDiskBufferSize = sizeof(PARTITION_INFORMATION_EX);
		PARTITION_INFORMATION_EX* lpDiskPartinfo_ex = (PARTITION_INFORMATION_EX*)malloc(nDiskBufferSize);
		DWORD nDiskBytesRead = 0;
		memset(lpDiskPartinfo_ex, 0, sizeof(lpDiskPartinfo_ex));
		BOOL Ret = FALSE;
		Ret = DeviceIoControl(hDevice, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, lpDiskPartinfo_ex, nDiskBufferSize, &nDiskBytesRead, NULL);

		if (!Ret)
		{
			int e = GetLastError();
			return strerror(e);
		}
		if (lpDiskPartinfo_ex->PartitionStyle == PARTITION_STYLE_MBR)
		{
			return "MBR";
		}
		else if (lpDiskPartinfo_ex->PartitionStyle == PARTITION_STYLE_GPT)
		{
			return "GPT";
		}
		else if (lpDiskPartinfo_ex->PartitionStyle == PARTITION_STYLE_RAW)
		{
			return "RAW";
		}
		free(lpDiskPartinfo_ex);
		lpDiskPartinfo_ex = NULL;
}
int GetSystemDiskPhysicalNumber()
{
	char sysPath[128];
	char drive[8];
	char volName[128];
	DWORD len, flags;
	int number;

	// ��ȡϵͳ�ļ���·��
	DWORD ret = GetSystemDirectory(sysPath, sizeof(sysPath));
	if (ret == 0)
	{
		// fprintf(stderr, "GetSystemDirectory() Error: %ld\n", GetLastError());
		return -1;
	}

	// ��ȡϵͳ�ļ����е��̷�
	strncpy(drive, sysPath, 3);
	drive[3] = '\0';

	// ��ȡϵͳ�̶�Ӧ�Ĺ��ص�
	if (!GetVolumeNameForVolumeMountPoint(drive, volName, sizeof(volName)))
	{
		// fprintf(stderr, "GetVolumeNameForVolumeMountPoint() Error: %ld\n", GetLastError());
		return -1;
	}

	// ��ȡ���ص��Ӧ�ľ����Ϣ
	if (!GetVolumeInformation(volName, NULL, 0, reinterpret_cast<LPDWORD>(&number), &len, &flags, NULL, 0))
	{
		//fprintf(stderr, "GetVolumeInformation() Error: %ld\n", GetLastError());
		return -1;
	}

	return number;
}
