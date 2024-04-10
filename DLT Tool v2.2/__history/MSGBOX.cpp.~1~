//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "MSGBOX.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMsg *frmMsg;
//---------------------------------------------------------------------------
__fastcall TfrmMsg::TfrmMsg(TComponent* Owner)
	: TForm(Owner)
{
	CheckDiskStatus();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMsg::btnID_okClick(TObject *Sender)
{
	if(edtSetWorkOrderNumber->Text=="")
		MessageBoxA(Handle,"請輸入完整資訊","WARNING", MB_ICONEXCLAMATION);
	else if(edtSetEmployeeID->Text=="")
		MessageBoxA(Handle,"請輸入完整資訊","WARNING", MB_ICONEXCLAMATION);
	else if(ImgDisk->Visible == false)
		MessageBoxA(Handle,"請確認LOG隨身碟狀態","WARNING", MB_ICONEXCLAMATION);
	else
		ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMsg::plSwitchClick(TObject *Sender)
{
	if(plSwitch->Caption=="V")
	{
		plSwitch->Caption = "^";
		frmMsg->Height = 452;
	}
	else
	{
		plSwitch->Caption = "V";
		frmMsg->Height = 269;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMsg::edtSetEmployeeIDKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	if(Key==13)
		btnID_ok->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMsg::edtSetWorkOrderNumberKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift)
{
	if(Key==13)
		edtSetEmployeeID->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMsg::FormShow(TObject *Sender)
{
	/*if(frmMsg->Tag)
	{
		frmMsg->plWOInfo->Enabled = false;
		frmMsg->plWOInfo->Height  = 0;
		frmMsg->plSwitch->Enabled = false;
		frmMsg->plSwitch->Height  = 0;
		frmMsg->TouchKeyboard->Top = 182;
		frmMsg->Height = 160;
	}*/
}
//---------------------------------------------------------------------------


void __fastcall TfrmMsg::Timer1Timer(TObject *Sender)
{
	CheckDiskStatus();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMsg::LOG1Click(TObject *Sender)
{
	if(edtSetWorkOrderNumber->Text=="")
		MessageBoxA(Handle,"請輸入完整資訊","WARNING", MB_ICONEXCLAMATION);
	else if(edtSetEmployeeID->Text=="")
		MessageBoxA(Handle,"請輸入完整資訊","WARNING", MB_ICONEXCLAMATION);
	else
		ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void TfrmMsg::CheckDiskStatus()
{
	DWORD DiskInfo = GetLogicalDrives();
	AnsiString DiskName,DiskVerifty = "DEVICE_LOG",SS;
	char Disk;
	ImgDisk->Visible = false;
	for(int i =3;i<=25;i++)
	{
		if((DiskInfo&(0x01<<i))!=0)
		{
			char Volumelabel[20];
			DWORD SerialNumber;
			DWORD MaxCLength;
			DWORD FileSysFlag;
			char FileSysName[10];
			Disk = 0x41+i;
			DiskName = AnsiString(Disk)+":\\";
			GetVolumeInformation(DiskName.c_str(),Volumelabel,255,&SerialNumber,&MaxCLength,&FileSysFlag,FileSysName,255);
			if(!strcmp(Volumelabel,DiskVerifty.c_str()))
			{
				ImgDisk->Visible = true;
				break;
			}
		}
	}
}

