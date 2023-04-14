#include <windows.h>
#include <lm.h> 
#include<stdlib.h>
#include <iostream>
#include<string>

using namespace std;
BOOL NetUserAdd(LPWSTR name, LPWSTR psw);
BOOL DisableUserAccount(const wchar_t* username);
BOOL RenameUserAccount(const wchar_t* oldUsername, const wchar_t* newUsername);
BOOL CurrentUser(LPWSTR UserName, LPWSTR psw);
string GetCurrentUsername();