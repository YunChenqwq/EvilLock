#include "Lock.h"

int DiskLock(string ShowMsg, string password)
{
	HANDLE DiskHandle = NULL;
	GetPhysicalDriveHandle(DiskHandle, 0);
	string boot = GetPartitiontype(DiskHandle);
	if (boot == "GPT")
	{
		ofstream script("Esp.txt"); // 创建一个文件用于存储脚本
		script << "select disk " << GetSystemDiskPhysicalNumber() << endl;
		script << "select partition 1\n"; // 选择ESP分区 没必要再获取一次系统分区号，一般人都是1
		script << "assign letter=X\n"; // 将ESP分区挂载到盘符X
		script << "exit\n"; // 退出diskpart
		script.close(); // 关闭文件
		system("diskpart /s Esp.txt"); // 运行脚本

	}
	else
	{

	}
}

int UserLock(string NewAccount, string oldAccount, string password)
{
	return 0;
}

int FileLock(string password)
{
	return 0;
}

int LoginLock(string ShowMsg, string password)
{
	return 0;
}
