#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <guiddef.h>
#include <shlobj_core.h>

using namespace std;
//本方法留存备用，实际使用会报毒，所以暂时调用diskpart脚本qwq
const GUID EFI_SYSTEM_PARTITION_GUID = { 0xC12A7328, 0xF81F, 0x11D2, {0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B} };
const GUID PARTITION_BASIC_DATA_GUID = { 0xEBD0A0A2, 0xB9E5, 0x4433, {0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7} };
int main()
{
    // 遍历硬盘上的EFI分区
    for (int i = 1; i < 5; i++)
    {
        // 构造设备路径
       char szDevicePath[MAX_PATH];
        sprintf_s(szDevicePath, "\\\\.\\Harddisk0Partition%d", i);
        puts(szDevicePath);
        // 打开设备句柄
        HANDLE hDisk = NULL;
        hDisk=CreateFileA(szDevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDisk == INVALID_HANDLE_VALUE)
        {
            cout << "无法打开设备句柄：" << i << endl;
            continue;
        }

        // 获取分区信息
        PARTITION_INFORMATION_EX partInfo;
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, &partInfo, sizeof(partInfo), &bytesReturned, NULL))
        {
            cout << "无法获取分区信息：" << i << endl;
            CloseHandle(hDisk);
            continue;
        }

        // 判断是否为EFI分区
        if (IsEqualGUID(partInfo.Gpt.PartitionType, PARTITION_BASIC_DATA_GUID) && IsEqualGUID(partInfo.Gpt.PartitionId, EFI_SYSTEM_PARTITION_GUID))
        {
            // 解除只读保护
            DWORD dwAttr = partInfo.Gpt.Attributes;
            dwAttr &= ~GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY;
            partInfo.Gpt.Attributes = dwAttr;
            if (!DeviceIoControl(hDisk, IOCTL_DISK_SET_PARTITION_INFO_EX, &partInfo, sizeof(partInfo), NULL, 0, &bytesReturned, NULL)) { wcout << L"无法解除只读保护：" << i << endl; CloseHandle(hDisk); continue; }

            // 更新磁盘属性
            if (!DeviceIoControl(hDisk, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &bytesReturned, NULL))
            {
                cout << "无法更新磁盘属性：" << i << endl;
                CloseHandle(hDisk);
                continue;
            }

            // 关闭磁盘句柄
            CloseHandle(hDisk);
        }
            // 分配盘符
            WCHAR szVolumePath[MAX_PATH];
            swprintf_s(szVolumePath, L"\\\\?\\GLOBALROOT\\Device\\Harddisk%dPartition1", i);
            if (!DefineDosDeviceW(DDD_RAW_TARGET_PATH, L"X:", szVolumePath))
            {
                cout << "无法分配盘符：" << i << endl;
                continue;
            }

            // 更新资源管理器的路径
            SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, L"X:\\", NULL);
            cout << "ESP分区已解除只读保护并分配盘符X" << endl;
    }
    while (1);
    return 0;
}