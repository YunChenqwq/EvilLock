#include"user.h"
/*创建一个用户并将它加进管理员用户组*/
BOOL NetUserAdd(LPWSTR name, LPWSTR psw)
{
	USER_INFO_1 user;
	user.usri1_name = name;
	user.usri1_password = psw;
	user.usri1_priv = USER_PRIV_USER;
	user.usri1_home_dir = NULL;
	user.usri1_comment = NULL;
	user.usri1_flags = UF_SCRIPT;
	user.usri1_script_path = NULL;
	if (NetUserAdd(NULL, 1, (LPBYTE)&user, 0) != NERR_Success)
	{
		return FALSE;
	}
	LOCALGROUP_MEMBERS_INFO_3 account;
	account.lgrmi3_domainandname = user.usri1_name;
	if (NetLocalGroupAddMembers(NULL, L"Administrators", 3, (LPBYTE)&account, 1) != NERR_Success)
	{
		return FALSE;
	}
	return TRUE;
}
/* 禁用账户*/
BOOL DisableUserAccount(const wchar_t* username)
{
	
	USER_INFO_1008 userInfo;
	userInfo.usri1008_flags = UF_ACCOUNTDISABLE;
	DWORD result = NetUserSetInfo(NULL, username, 1008, (LPBYTE)&userInfo, NULL);
	if (result == NERR_Success) {
		return true;
	}
	return false;
}
/*更改账户名*/
BOOL RenameUserAccount(const wchar_t* oldUsername, const wchar_t* newUsername) {
	USER_INFO_0 userInfo;
	userInfo.usri0_name = (wchar_t*)newUsername;
	DWORD result = NetUserSetInfo(NULL, oldUsername, 0, (LPBYTE)&userInfo, NULL);
	if (result == NERR_Success) {
		return false;
	}
	return true;
}
/*更改一个账户的密码*/
BOOL CurrentUser(LPWSTR UserName, LPWSTR psw) {
	WCHAR strName[60] = { 0 };
	USER_INFO_1003 ui;
	ui.usri1003_password = psw;
	DWORD dwResult = ::NetUserSetInfo(NULL,UserName, 1003, (LPBYTE)&ui, NULL);
	if (NERR_Success == dwResult) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}
/*当前登录账户的名称*/
string GetCurrentUsername() {
	char currentUser[256] = { 0 };
	DWORD dwSize_currentUser = 256;
	if (!GetUserName(currentUser, &dwSize_currentUser)) {
		return "Failed";
	}
	return string(currentUser);
}
