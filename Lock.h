#include"DiskOperation.h"
#include"others.h"
#include"user.h"
using namespace std;
int DiskLock(string ShowMsg, string password);
int UserLock(string NewAccount,string oldAccount, string password);
int FileLock(string password);
int LoginLock(string ShowMsg, string password);

