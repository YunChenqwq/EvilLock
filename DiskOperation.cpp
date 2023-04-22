#include"DiskOperation.h"
using namespace std;
struct MBR {
	unsigned char codeArea[446]; // 引导代码区域
	struct Partition {
		unsigned char status;    // 分区状态
		unsigned char start_head; // 起始磁头号
		unsigned char start_sec;  // 起始扇区号
		unsigned char start_cyl;  // 起始柱面号
		unsigned char type;       // 分区类型
		unsigned char end_head;   // 结束磁头号
		unsigned char end_sec;    // 结束扇区号
		unsigned char end_cyl;    // 结束柱面号
		unsigned int start_lba;   // 分区起始地址的LBA
		unsigned int size;        // 分区大小，以扇区数表示
	} partition[4];
	unsigned short signature; // MBR签名（0xAA55）
};
struct GPTHeader {
	char signature[8];          // GPT头部签名（"EFI PART"）
	uint32_t revision;          // GPT版本号
	uint32_t header_size;       // GPT头部大小
	uint32_t header_crc32;      // GPT头部CRC32校验码
	uint32_t reserved1;         // 保留字段（必须为0）
	uint64_t current_lba;       // GPT头部所在LBA地址
	uint64_t backup_lba;        // GPT备份头部所在LBA地址
	uint64_t first_lba;         // 第一个分区条目所在LBA地址
	uint64_t last_lba;          // 最后一个分区条目所在LBA地址
	uint8_t disk_guid[16];      // 磁盘GUID
	uint64_t partition_array_lba; // 分区表项数组所在LBA地址
	uint32_t partition_count;   // 分区数量
	uint32_t partition_entry_size; // 分区表项大小
	uint32_t partition_array_crc32; // 分区表项数组CRC32校验码
};

// GPT分区表项结构体
struct GPTPartitionEntry {
	uint8_t type_guid[16];      // 分区类型GUID
	uint8_t partition_guid[16]; // 分区GUID
	uint64_t start_lba;         // 分区起始LBA地址
	uint64_t end_lba;           // 分区结束LBA地址
	uint64_t attributes;        // 分区属性
	unsigned char name[72];     // 分区名称
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
/*获取驱动器句柄*/
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
/*获取硬盘分区类型*/
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

	// 获取系统文件夹路径
	DWORD ret = GetSystemDirectory(sysPath, sizeof(sysPath));
	if (ret == 0)
	{
		// fprintf(stderr, "GetSystemDirectory() Error: %ld\n", GetLastError());
		return -1;
	}

	// 提取系统文件夹中的盘符
	strncpy(drive, sysPath, 3);
	drive[3] = '\0';

	// 获取系统盘对应的挂载点
	if (!GetVolumeNameForVolumeMountPoint(drive, volName, sizeof(volName)))
	{
		// fprintf(stderr, "GetVolumeNameForVolumeMountPoint() Error: %ld\n", GetLastError());
		return -1;
	}

	// 获取挂载点对应的卷标信息
	if (!GetVolumeInformation(volName, NULL, 0, reinterpret_cast<LPDWORD>(&number), &len, &flags, NULL, 0))
	{
		//fprintf(stderr, "GetVolumeInformation() Error: %ld\n", GetLastError());
		return -1;
	}

	return number;
}
