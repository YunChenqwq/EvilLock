#include "Lock.h"

int DiskLock(string ShowMsg, string password)
{
	HANDLE DiskHandle = NULL;
	GetPhysicalDriveHandle(DiskHandle, 0);
	string boot = GetPartitiontype(DiskHandle);
	if (boot == "GPT")
	{
		ofstream script("Esp.txt"); // ����һ���ļ����ڴ洢�ű�
		script << "select disk " << GetSystemDiskPhysicalNumber() << endl;
		script << "select partition 1\n"; // ѡ��ESP���� û��Ҫ�ٻ�ȡһ��ϵͳ�����ţ�һ���˶���1
		script << "assign letter=X\n"; // ��ESP�������ص��̷�X
		script << "exit\n"; // �˳�diskpart
		script.close(); // �ر��ļ�
		system("diskpart /s Esp.txt"); // ���нű�

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
