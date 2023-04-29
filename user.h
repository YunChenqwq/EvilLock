#include <windows.h>
#include <lm.h> 
#include<stdlib.h>
#include <iostream>
#include<string>
using namespace std;
BOOL NetUserAddAdmin(LPWSTR name, LPWSTR psw);
BOOL DisableUserAccount(LPWSTR username);
BOOL RenameUserAccount(LPWSTR oldUsername, LPWSTR newUsername);
BOOL CurrentUser(LPWSTR UserName, LPWSTR psw);
string GetCurrentUsername();
LPWSTR W_GetCurrentUsername();