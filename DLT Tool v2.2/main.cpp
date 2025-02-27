﻿//---------------------------------------------------------------------------
#include <windows.h>    // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <SetupAPI.h> // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <cfgmgr32.h> // 安全移除USB裝置用 *要比 vcl.h 早編譯
#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "MSGBOX.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFrmMain *FrmMain;

DWORD dwTimeStart = 0;
bool bTesting = false;
DWORD dwTimerStep = STEP_TIMER_STOP;
DWORD dwResultTimeOut=0,dwTestCount=0,dwNoLoadTime=0;
AnsiString ERROR_MSG,DEV_SN;
//---------------------------------------------------------------------------
__fastcall TFrmMain::TFrmMain(TComponent* Owner)
	: TForm(Owner)
{
	CL_DEV_CONTROL = new cCOM();
	if(FindIniFile())
	{
		ReadInISet();
		FrmMain->Caption = APP_TITLE;
		ShowSettingForm();
		dwTestCount = 0;
	}
	else
	{
		MessageBoxA(this->Handle,"(!)讀取設定檔失敗", "Error", MB_ICONWARNING);
		Application->Terminate();
	}
}
//---------------------------------------------------------------------------
void TFrmMain::Voltage_Test()
{
	//初始化
	int dwStep = READ_NO_LOAD_VOLTAGE_VALUE;
	bool bl = true;
	g_bError = false;
	plResult->Caption = "Testing...";
	ERROR_MSG = "";
	strNoLoad = "";
	strFullLoad = "";
	strOverLoad = "";
	strBackNoLoad = "";
	TPanel *plNoLoad 	 	= (TPanel *)FindComponent("plReadNoLoadVol");
	TPanel *plFullLoad   	= (TPanel *)FindComponent("plReadFullLoadVol");
	TPanel *plOverLoad   	= (TPanel *)FindComponent("plReadOverLoadVol");
	TEdit  *edtNoLoadMin 	= (TEdit *)FrmMain->FindComponent("edt_min");
	TEdit  *edtNoLoadMax 	= (TEdit *)FrmMain->FindComponent("edt_max");
	TEdit  *edtFullLoadMin  = (TEdit *)FrmMain->FindComponent("edt_load_min");
	TEdit  *edtFullLoadMax  = (TEdit *)FrmMain->FindComponent("edt_load_max");
	TEdit  *edtOverLoadMin  = (TEdit *)FrmMain->FindComponent("edt_overload_min");
	TEdit  *edtOverLoadMax  = (TEdit *)FrmMain->FindComponent("edt_overload_max");
	TEdit  *edtSetCurrent 	= (TEdit *)FrmMain->FindComponent("edtSetCurrent");
	//
	CL_DEV_CONTROL->dbLoadCurrent = edtSetCurrent->Text.ToDouble();
	CL_DEV_CONTROL->dbLossVol	  = edtLossVol->Text.ToDouble();
	CL_DEV_CONTROL->maxNoLoadVol  = edtNoLoadMax->Text.ToDouble();
	CL_DEV_CONTROL->minNoLoadVol  = edtNoLoadMin->Text.ToDouble();
	CL_DEV_CONTROL->maxFullLoadVol  = edtFullLoadMax->Text.ToDouble();
	CL_DEV_CONTROL->minFullLoadVol  = edtFullLoadMin->Text.ToDouble();

	while(bl)
	{
		switch(dwStep)
		{
			case READ_NO_LOAD_VOLTAGE_VALUE:
				plNoLoad->Caption = "Testing...";
				dwNoLoadTime = GetTickCount();
				CL_DEV_CONTROL->dwININoLoadTimeOut = dwNoLoad_TestTime;
				dwStep = DL_CMD("LOAD OFF\n");
				dwNoLoadTime = 0;
				if(dwStep == TEST_PASS)
				{
					plNoLoad->Caption	  = CL_DEV_CONTROL->astrVoltage;
					strNoLoad			  = CL_DEV_CONTROL->astrVoltage;
					plNoLoad->Font->Color = clGreen;

					dwStep = READ_FULL_LOAD_VOLTAGE_VALUE;
				}
				else if(dwStep == TEST_FAIL)
				{
					plNoLoad->Caption	  = CL_DEV_CONTROL->astrVoltage;
					strNoLoad			  = CL_DEV_CONTROL->astrVoltage;
					plNoLoad->Font->Color = clRed;
					g_bError = true;
					ERROR_MSG += "[NO-LOAD:"+CL_DEV_CONTROL->astrVoltage+"]";
					//dwStep = TEST_VOLTAGE_END;
					if(CL_DEV_CONTROL->astrVoltage == "0")
						dwStep = TEST_VOLTAGE_END;
					else dwStep = READ_FULL_LOAD_VOLTAGE_VALUE;
				}
				else if(dwStep == DL_NOT_FIND)
				{
					plNoLoad->Font->Color = clRed;
					plNoLoad->Caption = "FAIL";
					ERROR_MSG += "[LOADER_NO_RESPOND]";
					g_bError = true;
				}
				break;
			case READ_FULL_LOAD_VOLTAGE_VALUE:
				plFullLoad->Caption = "Testing...";
				FrmMain->Refresh();
				CL_DEV_CONTROL->dwINIFullLoadTimeOut = dwFullLoad_TestTime;
				dwStep = DL_CMD("LOAD ON\n");
				plFullLoad->Caption	  = CL_DEV_CONTROL->astrVoltage;
				strFullLoad			  = CL_DEV_CONTROL->astrVoltage;
				if(dwStep == TEST_PASS)
				{
					plFullLoad->Font->Color = clGreen;
					if(ckbOverLoad->Checked)
						dwStep = READ_OVER_LOAD_VOLTAGE_VALUE;
					else
						dwStep = BACK_NO_LOAD_VOLTAGE;
				}
				else if(dwStep == TEST_FAIL)
				{
					plFullLoad->Font->Color = clRed;
					g_bError = true;
					ERROR_MSG += "[FULL-LOAD:"+CL_DEV_CONTROL->astrVoltage+"]";
					if(ckbOverLoad->Checked)
						dwStep = READ_OVER_LOAD_VOLTAGE_VALUE;
					else
						dwStep = TEST_VOLTAGE_END;
				}
				else if(dwStep == DL_NOT_FIND)
				{
					plFullLoad->Font->Color = clRed;
					plFullLoad->Caption = "FAIL";
				}
				break;
			case READ_OVER_LOAD_VOLTAGE_VALUE:
				plOverLoad->Caption = "Testing...";
				FrmMain->Refresh();
				CL_DEV_CONTROL->dwINIFullLoadTimeOut = dwOverLoad_TestTime;
				CL_DEV_CONTROL->dbLoadCurrent = edtSetOverLoadCurrent->Text.ToDouble();
				CL_DEV_CONTROL->maxFullLoadVol  = edtOverLoadMax->Text.ToDouble();
				CL_DEV_CONTROL->minFullLoadVol  = edtOverLoadMin->Text.ToDouble();
				dwStep = DL_CMD("LOAD ON\n");
				plOverLoad->Caption	  = CL_DEV_CONTROL->astrVoltage;
				strOverLoad			  = CL_DEV_CONTROL->astrVoltage;
				if(dwStep == TEST_PASS)
				{
					plOverLoad->Font->Color = clGreen;
					dwStep = BACK_NO_LOAD_VOLTAGE;
				}
				else if(dwStep == TEST_FAIL)
				{
					plOverLoad->Font->Color = clRed;
					dwStep = TEST_VOLTAGE_END;
					g_bError = true;
					ERROR_MSG += "[OVERLOAD:"+CL_DEV_CONTROL->astrVoltage+"]";
				}
				else if(dwStep == DL_NOT_FIND)
				{
					plOverLoad->Font->Color = clRed;
					plOverLoad->Caption = "FAIL";
				}
				break;
			case BACK_NO_LOAD_VOLTAGE:
				if(g_bError)
				{
					dwStep = TEST_VOLTAGE_END;
					break;
                }
				plNoLoad->Font->Color = clBlue;
				plNoLoad->Caption = "Testing...";
				CL_DEV_CONTROL->dwININoLoadTimeOut = dwBackNoLoad_TestTime;
				FrmMain->Refresh();
				dwStep = DL_CMD("LOAD OFF\n");
				if(dwStep == TEST_PASS)
				{
					plNoLoad->Caption	  = CL_DEV_CONTROL->astrVoltage;
					strBackNoLoad		  = CL_DEV_CONTROL->astrVoltage;
					plNoLoad->Font->Color = clGreen;

					dwStep = TEST_VOLTAGE_END;
				}
				else if(dwStep == TEST_FAIL)
				{
					plNoLoad->Caption	  = CL_DEV_CONTROL->astrVoltage;
					plNoLoad->Font->Color = clRed;
					strBackNoLoad		  = CL_DEV_CONTROL->astrVoltage;
					g_bError = true;
					ERROR_MSG += "[BACK-NOLOAD:"+CL_DEV_CONTROL->astrVoltage+"]";
					dwStep = TEST_VOLTAGE_END;
				}
				else if(dwStep == DL_NOT_FIND)
				{
					plNoLoad->Font->Color = clRed;
					plNoLoad->Caption = "FAIL";
				}
				break;
			case TEST_VOLTAGE_END:
				bl =false;
				break;
			case HID_NOT_FIND:
				plNoLoad->Font->Color = clRed;
				plNoLoad->Caption = "FAIL";
				g_bError = true;
				ERROR_MSG += "[PDBOARD_NO_RESPOND]";
				bl =false;
				break;
			case DL_NOT_FIND:
				g_bError = true;
				ERROR_MSG += "[LOADER_NO_RESPOND]";
				bl =false;
				break;
		}
	}
	plResult->Color = g_bError ? clRed:clGreen;
	plResult->Caption = g_bError ? "FAIL":"PASS";
	if(g_bError)
	{
		if(!writeLOG(ERROR_MSG))
		{
			MessageBoxA(this->Handle,"(!)寫檔失敗", "Error", MB_ICONWARNING);
			ShowSettingForm();
		}
	}
	else
	{
		if(!writeLOG("PASS"))
		{
			MessageBoxA(this->Handle,"(!)寫檔失敗", "Error", MB_ICONWARNING);
			ShowSettingForm();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFrmMain::btnStartClick(TObject *Sender)
{
	if(ckbBarcode->Checked)
	{
		if(btnStart->Caption.Pos("Start"))
		{
			btnStart->Caption = "Stop";
			btnSet->Enabled   = false;
			plTitle->PopupMenu = NULL;
			if(dwTimerStep == STEP_TIMER_STOP)
				dwTimerStep = CHECK_DEV_ONLINE;
		}
		else
		{
			btnStart->Enabled = false;
			btnStart->Caption = "Start";
        }
	}
}
void __fastcall TFrmMain::ckbAutoClick(TObject *Sender)
{
	btnStart->Caption = ckbAuto->Checked ? "Auto Test":"Start";
	plVPID->Visible   = ckbAuto->Checked ? true:false;
}
//---------------------------------------------------------------------------
bool TFrmMain::CheckVPIDSET(TEdit * edt){
	TCHAR HexChar[8];
	if(edt->Text.Length() == 0){}
	else if(edt->Text.Length() <= 4 ){
		edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
		AnsiString pvid = edt->Text;
		_tcscpy_s(HexChar, 8, WideString(pvid).c_bstr());
		for(int j=0;j<pvid.Length();j++)
		{
			if(HexChar[j] < 0x30 || HexChar[j] > 0x39){
				if(HexChar[j] < 0x41 || HexChar[j] > 0x5A){
					edt->Font->Color = clRed;
					return false;
				}
			}
		}
	}
	else
	{
		int bb = edt->Text.Length();
		edt->Font->Color = clRed;
		return false;
	}
	if(edt->Text.Length() == 4) edt->Font->Color = clBlue;
	else edt->Font->Color = clWindowText;

	edt-> SelStart=edt-> Text.Length();
	return true;
}
void __fastcall TFrmMain::edtVIDChange(TObject *Sender)
{
	CheckVPIDSET((TEdit*)Sender);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::moDebugChange(TObject *Sender)
{
	if(moDebug->Lines->Count >500) moDebug->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TFrmMain::edtPassWordEnter(TObject *Sender)
{
	if(edtPassWord->Font->Color == clRed)
	{
		edtPassWord->Font->Color = clBlue;
		edtPassWord->Text 	= "";
	}
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::btnSetClick(TObject *Sender)
{
	if(btnSet->Caption == "ENTER")
	{
		if(edtPassWord->Text.UpperCase() == PASSWORD)
		{
			btnSet->Caption		 = "END";
			SetValueEnabled(true);
		}
		else edtPassWord->Font->Color = clRed;
	}
	else
	{
		if(SaveVoltageRangeSetting())
		{
			btnSet->Caption	  	= "ENTER";
			edtPassWord->Text 	= "";
			SetValueEnabled(false);
//---------------------------------
			if(btnStart->Enabled)
				SetInIVal(ckbAuto->Checked,"vid_"+edtVID->Text+"&pid_"+edtPID->Text);
			ReadInISet();
			dwTimerStep = STEP_TIMER_STOP;
			plTitle->PopupMenu = popBackPage;
		}
	}
}
//---------------------------------------------------------------------------
void  TFrmMain::SetValueEnabled(bool Enabled)
{
	edtPassWord->Enabled = !Enabled;
	edtVID->Enabled = Enabled;
	edtPID->Enabled = Enabled;
	ckbAuto->Enabled = Enabled;
	ckbOverLoad->Enabled = Enabled;

	pl_SetRange->Enabled	  = Enabled;
	edt_min->Enabled = Enabled;
	edt_max->Enabled = Enabled;
	edt_load_min->Enabled = Enabled;
	edt_load_max->Enabled = Enabled;
	edt_overload_min->Enabled = Enabled;
	edt_overload_max->Enabled = Enabled;

	edtSetCurrent->Enabled = Enabled;
	edtSetOverLoadCurrent->Enabled = Enabled;
	edtLossVol->Enabled = Enabled;
	btnStart->Enabled = !Enabled;
	pl_Set_Switch->Enabled = !Enabled;
}


void __fastcall TFrmMain::pl_ckb1_1Click(TObject *Sender)
{
	TPanel* pl=(TPanel*)Sender;
	pl->Caption = pl->Caption == "+" ? "-":"+";
	pl->Tag = pl->Caption=="+" ? 1:0;
}
//---------------------------------------------------------------------------
void __fastcall TFrmMain::edt_minChange(TObject *Sender)
{
	TEdit* edt=(TEdit*)Sender;
	if(edt->Text.Length()>5)
		edt->Text = edt->Text.SubString(1,5);
	TCHAR HexChar[8];
	bool bPASS = true;
	bool bPoint = false;
	if(edt->Text.Length() == 0){
		bPASS =false;
	}
	else
	{
		edt->Text = edt->Text.UpperCase();// 把字串轉成全大寫
		_tcscpy_s(HexChar, 8, edt->Text.c_str());
		for(int j=0;j<edt->Text.Length();j++)
		{
			if(HexChar[j] < 0x30 || HexChar[j] > 0x39){
				if(HexChar[j] == 0x2E)
				{
					if(j == edt->Text.Length()-1 ||bPoint)
					{
						edt->Font->Color = clRed;
						edt-> SelStart=edt-> Text.Length();
						bPASS = false;
					}
					if(!bPoint) bPoint = true;
				}
				else
				{
					edt->Font->Color = clRed;
					edt-> SelStart=edt-> Text.Length();
					bPASS = false;
				}
			}
		}
	}
	if(bPASS)
	{
		edt->Font->Color = edt->Text.ToDouble() <= 99 ? clBlue : clRed;
	}

	edt-> SelStart=edt-> Text.Length();
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::edt_minExit(TObject *Sender)
{
	TEdit* edt=(TEdit*)Sender;
	if(edt->Text.Length() == 0 || edt->Font->Color == clRed){
		edt->Text = AnsiString(edt->Tag)+".00";
		edt->Font->Color = clBlue;
	}
	else
	{
		AnsiString edtnum = AnsiString(edt->Text.ToDouble());
		edtnum.printf("%.2f",atof(edtnum.c_str()));
		edt->Text =  edtnum;
	}

}
//---------------------------------------------------------------------------
bool TFrmMain::SaveVoltageRangeSetting()
{

	if(edt_min->Text.ToDouble()>edt_max->Text.ToDouble())
	{
		AnsiString STRERROR_MSG = "最小電壓不應大於最大電壓";
		MessageBoxA(NULL,STRERROR_MSG.c_str(),"ERROR", MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

void __fastcall TFrmMain::pl_Set_SwitchClick(TObject *Sender)
{
	if(pl_Set->Height)
	{
	   pl_Set->Height = 0;
	   pl_Set_Switch ->Caption ="v";
	   FrmMain->Height -=180;
	}
	else
	{
	   pl_Set->Height = 180;
	   FrmMain->Height +=180;
	   pl_Set_Switch ->Caption ="^";
	}
}
//---------------------------------------------------------------------------
void TFrmMain::Delay(ULONG iMilliSeconds) // 原版delay time 用在thread裡面
{
	ULONG iStart;
	iStart = GetTickCount();
	while (GetTickCount() - iStart <= iMilliSeconds)
		Application->ProcessMessages();
}
//---------------------------------------------------------------------------
// PSU Test
int TFrmMain::DL_CMD(AnsiString CMD)
{
	DWORD dwResult;
	dwResult = CL_DEV_CONTROL->open_dev_com();
	if(dwResult == LOADER_OK)
	{
		CL_DEV_CONTROL->que_cmd.c.clear();
		CL_DEV_CONTROL->que_cmd.push("*remote;chan 1;mode cc;pres on;curr:high "+AnsiString(CL_DEV_CONTROL->dbLoadCurrent)+"\n");
		CL_DEV_CONTROL->que_cmd.push(CMD);
		CL_DEV_CONTROL->que_cmd.push("meas:curr?;meas:volt?\n");
		//測試
		dwResult = CL_DEV_CONTROL->Dev_CMD_Test() ? TEST_PASS:TEST_FAIL;
		if(CL_DEV_CONTROL->bNoResponse)
		{
			CL_DEV_CONTROL->bCOM_PORT_OPEN = false;
			MessageBox(this->Handle, _T("動態負載機通訊失敗!請檢查線路!"), Application->Title.t_str(), MB_ICONWARNING);
            CL_DEV_CONTROL->que_Msg.push("ERROR : Not Find Device COM-PORT");
			dwResult = DL_NOT_FIND;
		}
	}
	else if(dwResult == LOADER_NOT_FIND )
	{
		MessageBox(this->Handle, _T("動態負載機通訊失敗!請檢查線路!(Not Find COM-PORT)"), Application->Title.t_str(), MB_ICONWARNING);
		CL_DEV_CONTROL->que_Msg.push("ERROR : Not Find Device COM-PORT");
		CL_DEV_CONTROL->bCOM_PORT_OPEN = false;
		dwResult = DL_NOT_FIND;
	}
	else if(dwResult == LOADER_NO_SUPPORT )
	{
		MessageBox(this->Handle, _T("動態負載機瓦特數不足!"), Application->Title.t_str(), MB_ICONWARNING);
		CL_DEV_CONTROL->que_Msg.push("ERROR : LODER NO SUPPORT");
		dwResult = DL_NOT_FIND;
	}
	return dwResult;
}
void __fastcall TFrmMain::plResultDblClick(TObject *Sender)
{
	moDebug->Height = moDebug->Height ? 0 : 300;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::edtPassWordKeyPress(TObject *Sender, wchar_t &Key)
{
	if(Key == 13)
		btnSet->Click();
	else
		edtPassWord->Font->Color = clBlue;
}
//---------------------------------------------------------------------------
bool TFrmMain::FindIniFile()
{
	TSearchRec Sr;
	TStringList*FileList=new TStringList();
	if(FindFirst("*.ini",faAnyFile,Sr)==0)
	{
		do
		{
			FileList->Add(Sr.Name);
		}
		while(FindNext(Sr)==0);
		FindClose(Sr);
	}
	if(FileList->Count>1||FileList->Count==0)
	{
		delete FileList;
		plIniName->Font->Color = clRed;
		return false;
	}
	else
	{
		FILE_DUT_SET_INI = FileList->Strings[0];
		if(FILE_DUT_SET_INI.Length()>4)
			plIniName->Caption = FILE_DUT_SET_INI.SubString(1,FILE_DUT_SET_INI.Length()-4);
		delete FileList;
		return true;
	}
}
bool TFrmMain::ReadInISet()
{
	//
	AnsiString value = Findfilemsg(FILE_DUT_SET_INI,"[LossVoltage]",1);
	edtLossVol->Text = value;
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[DUT-PluginAutoTest]",1);
	ckbAuto->Checked = value == 1 ? true:false;
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[DUT-PidVid]",1);
	edtVID->Text = value.SubString(5,4);
	edtPID->Text = value.SubString(14,4);
	//
	edtSetCurrent->Text = Findfilemsg(FILE_DUT_SET_INI,"[SetCurrent]",1);
	edtSetOverLoadCurrent->Text = Findfilemsg(FILE_DUT_SET_INI,"[SetOverLoadCurrent]",1);
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[VoltageToleranceRange]",1);
	edt_min->Text	  = value.SubString(1,value.Pos("、")-1);
	edt_max->Text	  = value.SubString(value.Pos("、")+2,value.Pos("&")-value.Pos("、")-2);
	value = value.Delete(1,value.Pos("&"));
	edt_load_min->Text	  = value.SubString(1,value.Pos("、")-1);
	edt_load_max->Text	  = value.SubString(value.Pos("、")+2,value.Pos("-")-value.Pos("、")-2);
	value = value.Delete(1,value.Pos("-"));
	edt_overload_min->Text	  = value.SubString(1,value.Pos("、")-1);
	edt_overload_max->Text	  = value.SubString(value.Pos("、")+2,value.Length()-value.Pos("、")+2);
	//
	if(edt_load_max->Text.ToDouble() * edtSetCurrent->Text.ToDouble() >75)
		CL_DEV_CONTROL->SetMAX_WATT(LOADER_3310F);
	if(edt_load_max->Text.ToDouble() * edtSetCurrent->Text.ToDouble() >150)
		CL_DEV_CONTROL->SetMAX_WATT(LOADER_3311F);
	if(edt_load_max->Text.ToDouble() * edtSetCurrent->Text.ToDouble() <= 75)
		CL_DEV_CONTROL->SetMAX_WATT(LOADER_3315F);
	//
	value = Findfilemsg(FILE_DUT_SET_INI,"[OverLoadTest]",1);
	ckbOverLoad->Checked = value == 1 ? true:false;
	if(ckbOverLoad->Checked)
	{
		if(edt_overload_max->Text.ToDouble() * edtSetOverLoadCurrent->Text.ToDouble() >75)
			CL_DEV_CONTROL->SetMAX_WATT(LOADER_3310F);
		if(edt_overload_max->Text.ToDouble() * edtSetOverLoadCurrent->Text.ToDouble() >150)
			CL_DEV_CONTROL->SetMAX_WATT(LOADER_3311F);
	}
	//
	dwNoLoad_TestTime   				 = Findfilemsg(FILE_DUT_SET_INI,"[NoLoad-TestTime]",1).ToInt();
	dwFullLoad_TestTime 				 = Findfilemsg(FILE_DUT_SET_INI,"[FullLoad-TestTime]",1).ToInt();
	dwOverLoad_TestTime					 = Findfilemsg(FILE_DUT_SET_INI,"[OverLoad-TestTime]",1).ToInt();
	dwBackNoLoad_TestTime   			 = Findfilemsg(FILE_DUT_SET_INI,"[BackNoLoad-TestTime]",1).ToInt();
	dwResultUI_Time						 = Findfilemsg(FILE_DUT_SET_INI,"[ResultUI-Time]",1).ToInt();
	plVPID->Visible   = ckbAuto->Checked ? true:false;

	return true;
}
bool TFrmMain::SetInIVal(bool AutoTest,AnsiString DutVPID){
	AnsiString VoltageRange =
		AnsiString(edt_min->Text)
		+ "、"
		+ AnsiString(edt_max->Text)
		+ "&"
		+ AnsiString(edt_load_min->Text)
		+ "、"
		+ AnsiString(edt_load_max->Text)
		+ "-"
		+ AnsiString(edt_overload_min->Text)
		+ "、"
		+ AnsiString(edt_overload_max->Text);

	fstream fp;
	fp.open(FILE_DUT_SET_INI.c_str(), ios::out); // 開啟檔案

	fp << "[DUT-PluginAutoTest]" << endl;
	if(AutoTest) fp << "1" << endl;
	else fp << "0" << endl;
	fp << "[DUT-PidVid]" << endl;
	if(edtVID->Font->Color==clBlue&&edtPID->Font->Color==clBlue)
		fp << DutVPID.c_str() << endl;
	fp << "[OverLoadTest]" << endl;
	if(ckbOverLoad->Checked) fp << "1" << endl;
	else fp << "0" << endl;
	fp << "[SetOverLoadCurrent]" << endl;
	fp << AnsiString(edtSetOverLoadCurrent->Text).c_str() << endl;
	fp << "[SetCurrent]" << endl;
	fp << AnsiString(edtSetCurrent->Text).c_str() << endl;
	fp << "[VoltageToleranceRange]" << endl;
	fp << VoltageRange.c_str() << endl;
	fp << "[LossVoltage]" << endl;
	fp << AnsiString(edtLossVol->Text).c_str() << endl;
	fp << "[NoLoad-TestTime]" << endl;
	fp << AnsiString(dwNoLoad_TestTime).c_str() << endl;
	fp << "[FullLoad-TestTime]" << endl;
	fp << AnsiString(dwFullLoad_TestTime).c_str() << endl;
	fp << "[OverLoad-TestTime]" << endl;
	fp << AnsiString(dwOverLoad_TestTime).c_str() << endl;     //
	fp << "[BackNoLoad-TestTime]" << endl;
	fp << AnsiString(dwBackNoLoad_TestTime).c_str() << endl;
	fp << "[ResultUI-Time]" << endl;
	fp << AnsiString(dwResultUI_Time).c_str() << endl;

	fp.close(); // 關閉檔案
	return true;
}
AnsiString TFrmMain::Findfilemsg(AnsiString filename, AnsiString findmsg,
	int rownum) { // 找檔案找到字串行回傳幾行後的字串
	ifstream lanfile(filename.c_str());
	std::string filemsg;
	if (lanfile.is_open()) {
		while (!lanfile.eof()) {
			getline(lanfile, filemsg);
			if (strstr(filemsg.c_str(), findmsg.c_str())) {
				for (int i = 0; i < rownum; i++)
					getline(lanfile, filemsg);
				lanfile.close();
				return(AnsiString)filemsg.c_str();
			}
		}
		lanfile.close();
		return NULL;
	}
	else
		return NULL;
}
void __fastcall TFrmMain::ckbOverLoadClick(TObject *Sender)
{
	pl_OverloadTitle->Visible = ckbOverLoad->Checked ? true:false;
	plOverCur->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoadRange->Visible = ckbOverLoad->Checked ? true:false;
	plOverLoadRangeTitle->Visible = ckbOverLoad->Checked ? true:false;
	plReadOverLoadVol->Visible =  ckbOverLoad->Checked ? true:false;

	plNoLoadTitle->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	plFullLoadTitle->Width = ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	plSpace->Width = ckbOverLoad->Checked ? 18:0;
	plNoLoad->Width 	= ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	plFullLoad->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	edt_min->Width 	 =(plNoLoad->Width-30)/2;
	edt_load_min->Width =(plFullLoad->Width-30)/2;
	edt_overload_min->Width	 =(plOverLoadRangeTitle->Width-30)/2;
	pl_NoloadTitle->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
	plReadNoLoadVol->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
	if(ckbOverLoad->Checked)
	{
		pl_OverloadTitle->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
		plReadOverLoadVol->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::FormResize(TObject *Sender)
{

	plNoLoadTitle->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	plFullLoadTitle->Width = ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	plSpace->Width = ckbOverLoad->Checked ? 18:0;
	plNoLoad->Width 	= ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	plFullLoad->Width   = ckbOverLoad->Checked ? (pl_set_title->Width-36)/3:(pl_set_title->Width-18)/2;
	edt_min->Width 	 =(plNoLoad->Width-30)/2;
	edt_load_min->Width =(plFullLoad->Width-30)/2;
	edt_overload_min->Width	 =(plOverLoadRangeTitle->Width-30)/2;
	edt_overload_max->Width	 =(plOverLoadRangeTitle->Width-30)/2;
	plvotage->Width = Panel1->Width-120;
	plTitle2->Left = edt_min->Width;
	if(ckbOverLoad->Checked)
	{
		pl_OverloadTitle->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
		plReadOverLoadVol->Height = ckbOverLoad->Checked ? plVoltageBox->Height/3:plVoltageBox->Height/2;
		plLossVol->Width = (plSet2->Width-36)/3;
		plFullCur->Width = (plSet2->Width-36)/3;
	}
	else
	{
		plLossVol->Width = (plSet2->Width-18)/2;
		plFullCur->Width = (plSet2->Width-18)/2;
	}
	ckbOverLoadClick(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::Timer1Timer(TObject *Sender)
{
	Timer1->Enabled = false;
	switch(dwTimerStep)
	{
		case CHECK_DEV_ONLINE:
			if(ckbBarcode->Checked)
			{
				numBarcodeResult = 0;
				edtBarcodeMAC->Text = "";
				plBarcode->Height = 138;
				edtBarcodeMAC->Enabled = true;
				plBarcode->Top = (FrmMain->Height/2)-(plBarcode->Height/2);
				plBarcode->Left = (FrmMain->Width/2)-(plBarcode->Width/2);
				edtBarcodeMAC->SetFocus();
				SetForegroundWindowInternal(Handle);
				ClearUI();
				while(true)
				{
					if(numBarcodeResult == BARCODE_FINISH)
					{
						//初始化Barcode畫面
						lbBarcode->Font->Size = 32;
						lbBarcode->Caption = "請掃描Barcode";
						lbBarcode->Color = clTeal;
						plBarcode->Height = 0;
						FrmMain->Refresh();
						dwTimerStep = START_TEST;
						edtBarcodeMAC->Enabled = false;
						break;
					}
					if(numBarcodeResult == BARCODE_CHANEL)
					{
						plBarcode->Height = 0;
						FrmMain->Refresh();
						btnStart->Caption = "Start";
						dwTimerStep = STEP_TIMER_STOP;
						edtBarcodeMAC->Enabled = false;
						plTitle->PopupMenu = popBackPage;
						break;
					}
				   Delay(100);
				}
			}
			else if(ckbAuto)
			{
				if(CL_USB->VPIDCheckUSBDev(edtVID->Text,edtPID->Text)
					||CL_USB->VPIDCheckHUBDev(edtVID->Text,edtPID->Text))
				{
					if(btnSet->Caption!= "END")
					{
						btnSet->Enabled = false;
						dwTimerStep = START_TEST;
					}
				}
			}
			break;
		case START_TEST:
			dwTimeStart = GetTickCount();
			Voltage_Test();
			btnSet->Enabled = true;
			dwTimerStep = CHECK_DEV_OFFLINE;
			CL_DEV_CONTROL->que_Msg.push(float(GetTickCount() - dwTimeStart)/1000);
			if(ckbBarcode->Checked && !g_bError)
			{
				dwResultTimeOut = GetTickCount()+dwResultUI_Time;
			}
			break;
		case CHECK_DEV_OFFLINE:
			if(g_bError)
			{
				lbTime->Caption="";
				btnStart->Caption = "Start";
				btnStart->Enabled = true;
				dwTimerStep = STEP_TIMER_STOP;
				plTitle->PopupMenu = popBackPage;
				btnSet->Enabled   = true;
			}
			else if(ckbBarcode->Checked)
			{
				lbTime->Caption = (float(GetTickCount()-dwResultTimeOut))/1000;
				if(GetTickCount() > dwResultTimeOut)
				{
					dwResultTimeOut = 0;
					ClearUI();
					dwTimerStep = CHECK_DEV_ONLINE;
					if(btnStart->Caption.Pos("Start"))
					{
						btnStart->Enabled = true;
						dwTimerStep = STEP_TIMER_STOP;
						btnSet->Enabled   = true;
						plTitle->PopupMenu = popBackPage;
					}
				}
            }
			else if(ckbAuto)
			{
				if(!CL_USB->VPIDCheckUSBDev(edtVID->Text,edtPID->Text)
					&&!CL_USB->VPIDCheckHUBDev(edtVID->Text,edtPID->Text))
				{
					ClearUI();
					dwTimerStep = CHECK_DEV_ONLINE;
				}
			}
	}
	Timer1->Enabled = true;
}
void TFrmMain::ClearUI()
{
	plReadNoLoadVol->Font->Color = clBlue;
	plReadNoLoadVol->Caption = "";
	plReadFullLoadVol->Font->Color = clBlue;
	plReadFullLoadVol->Caption = "";
	plReadOverLoadVol->Font->Color = clBlue;
	plReadOverLoadVol->Caption = "";
	plResult->Caption = "Wait...";
	plResult->Color = clCream;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::btnBarcodeChanelClick(TObject *Sender)
{
	btnStart->Enabled = true;
	btnSet->Enabled	  = true;
	numBarcodeResult = BARCODE_CHANEL;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::edtBarcodeMACKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(Key==13)
	{
		if(edtBarcodeMAC->Text.Length())
		{
			DEV_SN = edtBarcodeMAC->Text;
			plDUT_SN->Caption = DEV_SN;
			numBarcodeResult = BARCODE_FINISH;
		}
	}
	else if(Key == 8)
		edtBarcodeMAC->Text = "";
}
//---------------------------------------------------------------------------
void TFrmMain::SetForegroundWindowInternal(HWND hWnd)
{
    if(!::IsWindow(hWnd)) return;

	//relation time of SetForegroundWindow lock
    DWORD lockTimeOut = 0;
    HWND  hCurrWnd = ::GetForegroundWindow();
	DWORD dwThisTID = ::GetCurrentThreadId(),
		  dwCurrTID = ::GetWindowThreadProcessId(hCurrWnd,0);

    //we need to bypass some limitations from Microsoft :)
    if(dwThisTID != dwCurrTID)
    {
        ::AttachThreadInput(dwThisTID, dwCurrTID, TRUE);

        ::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&lockTimeOut,0);
        ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,0,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);

        ::AllowSetForegroundWindow(ASFW_ANY);
	}

    ::SetForegroundWindow(hWnd);

    if(dwThisTID != dwCurrTID)
    {
        ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,(PVOID)lockTimeOut,SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE);
        ::AttachThreadInput(dwThisTID, dwCurrTID, FALSE);
    }
}
void TFrmMain::ShowSettingForm()
{
	if(frmMsg==NULL)  frmMsg = new TfrmMsg(Application);

	frmMsg->edtSetWorkOrderNumber->Text = edtSetWorkOrderNumber->Caption;
	frmMsg->edtSetEmployeeID->Text = edtSetEmployeeID->Caption.Trim();


	if(frmMsg->ShowModal()== mrOk)
	{
		edtSetWorkOrderNumber->Caption  = frmMsg->edtSetWorkOrderNumber->Text;
		edtSetEmployeeID->Caption = frmMsg->edtSetEmployeeID->Text;
		//取得LOGFilePath、LOGDiskPath路徑
		LOGFilePath = "C:\\ASMP\\log\\"+edtSetWorkOrderNumber->Caption+"\\DLT-Tool";
		CheckDiskName();
		//
		FindLogFile();
		//
		delete frmMsg;
		frmMsg = NULL;
	}
	else
	{
		delete frmMsg;
		frmMsg = NULL;
		Close();
	}
}
bool TFrmMain::CheckDiskName()
{
	DWORD DiskInfo = GetLogicalDrives();
	AnsiString DiskName,DiskVerifty = "DEVICE_LOG",SS;
	char Disk;
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
			GetVolumeInformationA(DiskName.c_str(),Volumelabel,255,&SerialNumber,&MaxCLength,&FileSysFlag,FileSysName,255);
			if(!strcmp(Volumelabel,DiskVerifty.c_str()))
			{
				LOGDiskPath = DiskName+"\\ASMP\\log\\"+edtSetWorkOrderNumber->Caption+"\\DLT-Tool";
				if(!FileExists(LOGDiskPath.c_str())){
					_mkdir( DiskName.c_str());
					SS = DiskName + "\\ASMP";
					_mkdir( SS.c_str());
					SS = DiskName + "\\ASMP\\log";
					_mkdir( SS.c_str());
					SS = DiskName + "\\ASMP\\log\\"+edtSetWorkOrderNumber->Caption;
					_mkdir(SS.c_str());
					_mkdir(LOGDiskPath.c_str());
				}
				return true;
			}
		}
	}
	return false;
}
void TFrmMain::FindLogFile()
{
	//
	NewFilePath(LOGFilePath);
	NewFilePath(LOGDiskPath);
	//
	TSearchRec Sr;
	AnsiString time = FormatDateTime("yyyymmdd-hhnnss", Now());
	if(DirectoryExists(LOGDiskPath.c_str()))
	{
		ImgDisk->Visible = true;
		if(FindFirst(LOGDiskPath+"\\*.csv",faAnyFile,Sr)==0)
		{
			do
			{
				if(Sr.Name.SubString(1,8) == time.SubString(1,8))
				{
					LOGDiskPath += "\\"+Sr.Name;
					//
					LOGFilePath += "\\"+Sr.Name;
					break;
				}
			}
			while(FindNext(Sr)==0);
			FindClose(Sr);
		}

		if(!LOGDiskPath.Pos(".csv"))
		{
			LOGDiskPath += "\\"+time+".csv";
			LOGFilePath += "\\"+time+".csv";
		}
		else//檔案已存在於USBDisk
		{
			if(!FileExists(LOGFilePath))
			{
				CopyFileA(LOGDiskPath.c_str(),LOGFilePath.c_str(),true);
			}
		}
	}
	else
	{
		ImgDisk->Visible = false;
		if(DirectoryExists(LOGFilePath.c_str()))
		{
			if(FindFirst(LOGFilePath+"\\*.csv",faAnyFile,Sr)==0)
			{
				do
				{
					if(Sr.Name.SubString(1,8) == time.SubString(1,8))
					{
						LOGFilePath += "\\"+Sr.Name;
						break;
					}
				}
				while(FindNext(Sr)==0);
				FindClose(Sr);
			}
		}
		if(!LOGFilePath.Pos(".csv"))
		{
			LOGFilePath += "\\"+time+".csv";
		}
	}
}
void TFrmMain::NewFilePath(AnsiString Path)
{
	Path+="\\";
	if(!DirectoryExists(Path.c_str()))
	{
		AnsiString SS,Temp = "";
		while(true)
		{
			SS = Path.SubString(1,Path.Pos("\\"));
			Path.Delete(1,Path.Pos("\\"));
			Temp+=SS;
			_mkdir( Temp.c_str());
			if(Path == "")
				break;
		}
	}
}
bool TFrmMain::writeLOG(AnsiString Msg)
{
	bool bPASS[2] = {true,true};
	AnsiString SS;
	if(Msg=="PASS")
		dwTestCount++;
	AnsiString strTemp;
	AnsiString strLoader;
	switch(CL_DEV_CONTROL->GetLoaderModel())
	{
		case LOADER_3315F:
			strLoader = "3315F";
			break;
		case LOADER_3310F:
			strLoader = "3310F";
			break;
		case LOADER_3311F:
			strLoader = "3311F";
			break;
		default:
			strLoader = "";
			break;
	}
	strTemp.sprintf("%04d",dwTestCount);
	if(Msg.Pos("PASS"))
	{
		SS = "\n"+strTemp+",[PASS],"+AnsiString(FormatDateTime("mm-dd-yyyy,hh:mm:ss", Now()))
			+",[NAME],"+plIniName->Caption+",[SN],"+DEV_SN+",[NoLoad],"+strNoLoad+",[FullLoad],"+strFullLoad;
		if(ckbOverLoad->Checked)
			SS+= ",[OverLoad],"+strOverLoad;
		SS += ",[BackNoLoad],"+strBackNoLoad;
		SS += ",[Loader-Model],"+strLoader;
		SS+= ",[WorkOrder],"+edtSetWorkOrderNumber->Caption+",[EmployeeID],"+edtSetEmployeeID->Caption;
	}
	else
	{
		SS = "\n"+strTemp+",[FAIL],"+AnsiString(FormatDateTime("mm-dd-yyyy,hh:mm:ss", Now()))
			+",[NAME],"+plIniName->Caption+",[SN],"+DEV_SN+",[NoLoad],"+strNoLoad+",[FullLoad],"+strFullLoad;
		if(ckbOverLoad->Checked)
			SS+= ",[OverLoad],"+strOverLoad;
		SS += ",[BackNoLoad],"+strBackNoLoad;
		SS += ",[Loader-Model],"+strLoader;
		SS+= ",[WorkOrder],"+edtSetWorkOrderNumber->Caption+",[EmployeeID],"+edtSetEmployeeID->Caption;
		SS+=",[ErrorMsg],"+Msg;
	}
	bPASS[0] = SaveLogLine(LOGFilePath,SS);
	if(LOGDiskPath.Pos("csv") && ImgDisk->Visible )
	{
		bPASS[1] = SaveLogLine(LOGDiskPath,SS);
		if(!bPASS[1]) ImgDisk->Visible = false;
	}
	for(int i = 0 ; i < 2 ; i++)
		if(!bPASS[i]) return false;
	return true;
}
bool  TFrmMain::SaveLogLine(AnsiString FileName,AnsiString FileLine)
{
	FILE * fp;
	fp = fopen(FileName.c_str(),"a+");
	if(NULL == fp)
		return false;
	fseek( fp, 0, SEEK_END);
	fwrite(FileLine.c_str(),FileLine.Length(),1,fp); //寫入一筆資料
	fclose(fp);
	return true;
}
void __fastcall TFrmMain::Timer2Timer(TObject *Sender)
{
	if(!CL_DEV_CONTROL->que_Msg.empty())
	{
		DEBUG(CL_DEV_CONTROL->que_Msg.front());
		CL_DEV_CONTROL->que_Msg.pop();
	}
	if(dwResultTimeOut != 0)
	{
		float fTime =	float(dwResultTimeOut - GetTickCount())/1000;
		if( fTime <= (dwResultUI_Time/1000) && fTime >=0)
			lbTime->Caption = AnsiString(fTime).SubString(1,3);
	}
	else if(dwNoLoadTime != 0)
	{
		float fTime =	float(GetTickCount()-dwNoLoadTime)/1000;
		if((fTime*1000) >= CL_DEV_CONTROL->dwININoLoadTimeOut)
			lbTime->Caption = "";
		else if( fTime < 10 && fTime >=0)
			lbTime->Caption = AnsiString(fTime).SubString(1,3);
		else if( fTime >= 10 && fTime >=0)
			lbTime->Caption = AnsiString(fTime).SubString(1,4);
    }
	else
		lbTime->Caption = "";
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::btnBarcodeClearClick(TObject *Sender)
{
	edtBarcodeMAC->Text = "";
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::popItemClick(TObject *Sender)
{
	ShowSettingForm();
}
//---------------------------------------------------------------------------

