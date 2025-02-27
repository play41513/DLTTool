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
DWORD dwTimerStep = STEP_TIMER_STOP;
DWORD dwResultTimeOut = 0,dwTestCount = 0,dwLoadTime = 0;
DWORD dwStartTestDelay = 0,dwTime = 0;
bool  bRelayControl;
//---------------------------------------------------------------------------
__fastcall TFrmMain::TFrmMain(TComponent* Owner)
	: TForm(Owner)
{
	CL_DEV_CONTROL = new cCOM();
	if(FindIniFile())
	{
		LOGFile = new CLOGFile();
		CL_RELAY = new cRelayControl();
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
	DWORD dwStep = STEP_READ_MODE1_VOLTAGE_VALUE;
	bool bl = true;
	TPanel* plReadLoadVolMode;
	while(bl)
	{
		switch(dwStep)
		{
			case STEP_READ_MODE1_VOLTAGE_VALUE:
				g_bError = false;
				plResult->Caption = "Testing...";
				LOGFile->ClearLOGContent();
				dwLoadTime = GetTickCount();
				CL_DEV_CONTROL->SetReadValueElement(plReadLoadVolMode1);
				//plReadLoadVolMode1->Caption = "Testing...";
				dwModeIndex = 1;
				//
				if(plLoopBack->Visible)
					plLoopBackNowCount->Caption = plLoopBackNowCount->Caption.ToInt()+1;
				//
				dwStep = DL_CMD(dwModeIndex);
				if(dwStep == TEST_PASS)
				{
					plReadLoadVolMode1->Font->Color = clGreen;
					LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),CL_DEV_CONTROL->astrVoltage);
					dwStep = STEP_WAITTING_AC_ON;
				}
				else if(dwStep == TEST_FAIL)
				{
					plReadLoadVolMode1->Font->Color = clRed;
					g_bError = true;
					LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),CL_DEV_CONTROL->astrVoltage);
					LOGFile->astrErrorMsg = "(!)Mode1-LOAD:"+CL_DEV_CONTROL->astrVoltage;
					//
					dwStep = STEP_TEST_VOLTAGE_END;
				}
				else if(dwStep == STEP_DL_NOT_FIND)
				{
					plReadLoadVolMode1->Font->Color = clRed;
					plReadLoadVolMode1->Caption 	= "FAIL";
				}
				dwModeIndex++;
				dwLoadTime = 0;
				break;
			case STEP_WAITTING_AC_ON:
				if(!bRelayControl)
				{
					dwStep = STEP_READ_OTHER_MODE_VOLTAGE_VALUE;
					break;
				}
				if(CL_RELAY->CONTROL_RELAY_POWER(true))
					dwStep = STEP_READ_OTHER_MODE_VOLTAGE_VALUE;
				else
					dwStep = STEP_RELAY_NOT_FIND;
				break;
			case STEP_READ_OTHER_MODE_VOLTAGE_VALUE:
			{
				dwLoadTime = GetTickCount();
				plReadLoadVolMode = (TPanel *)FrmMain->FindComponent("plReadLoadVolMode"+AnsiString(dwModeIndex));
				CL_DEV_CONTROL->SetReadValueElement(plReadLoadVolMode);
				//plReadLoadVolMode->Caption = "Testing...";

				//
				dwStep = DL_CMD(dwModeIndex);
				if(dwStep == TEST_PASS)
				{
					plReadLoadVolMode->Font->Color = clGreen;
					LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),CL_DEV_CONTROL->astrVoltage);
					if(bModePowerReset[dwModeIndex-1])
					{
						CL_DEV_CONTROL->CloseDLLoad();
						CL_RELAY->RELAY_POWER_RESET(dwModePowerResetDelay[dwModeIndex-1]);
					}
					dwModeIndex++;
					if(dwModeIndex <= 5 && bModeEnable[dwModeIndex-1])
					{
						dwStep = STEP_READ_OTHER_MODE_VOLTAGE_VALUE;
					}
					else
						dwStep = STEP_WAITTING_AC_OFF;
				}
				else if(dwStep == TEST_FAIL)
				{
					plReadLoadVolMode->Font->Color  = clRed;
					g_bError = true;
					LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),CL_DEV_CONTROL->astrVoltage);
					LOGFile->astrErrorMsg = "(!)Mode"+AnsiString(dwModeIndex)+"-LOAD:"+CL_DEV_CONTROL->astrVoltage;
					//
					dwStep = STEP_WAITTING_AC_OFF;
				}
				else if(dwStep == STEP_DL_NOT_FIND)
				{
					plReadLoadVolMode->Font->Color = clRed;
					plReadLoadVolMode->Caption 	= "FAIL";
				}
				dwLoadTime = 0;
				break;
			}
			case STEP_WAITTING_AC_OFF:
				CL_DEV_CONTROL->CloseDLLoad();
				if(!bRelayControl)
				{
					dwStep = STEP_TEST_VOLTAGE_END;
					break;
				}
				if(CL_RELAY->CONTROL_RELAY_POWER(false))
				{
					dwStep = STEP_TEST_VOLTAGE_END;
				}
				else
					dwStep = STEP_RELAY_NOT_FIND;
				break;
			case STEP_TEST_VOLTAGE_END:
				plResult->Color = g_bError ? clRed:clGreen;
				plResult->Caption = g_bError ? "FAIL":"PASS";
				FrmMain->Refresh();
				if(g_bError)
				{
					if(!LOGFile->writeLOG("FAIL",ImgDisk,moDebug))
					{
						if(!plLoopBack->Visible)
						{
							MessageBoxA(this->Handle,"(!)寫檔失敗", "Error", MB_ICONWARNING);
							ShowSettingForm();
						}
					}
				}
				else
				{
					if(!LOGFile->writeLOG("PASS",ImgDisk,moDebug))
					{
						if(!plLoopBack->Visible)
						{
							MessageBoxA(this->Handle,"(!)寫檔失敗", "Error", MB_ICONWARNING);
							ShowSettingForm();
						}
					}
				}
				if(plLoopBack->Visible)
				{
					if(plLoopBackNowCount->Caption.ToInt()< plLoopBackTotalCount->Text.ToInt())
					{
						dwStep = STEP_READ_MODE1_VOLTAGE_VALUE;
						if(plStart->Caption.Pos("Start"))
							bl =false;
						if(LOGFile->astrErrorMsg.Pos("(!)LOADER_NOT_FIND"))
							bl =false;
						if(LOGFile->astrErrorMsg.Pos("(!)RELAY_NOT_FIND"))
							bl =false;
						if(bl)
							ClearUI();
						break;
					}
				}
				bl =false;
				break;
			case STEP_DL_NOT_FIND:
				g_bError = true;
				LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),"FAIL");
				LOGFile->astrErrorMsg = "(!)LOADER_NOT_FIND";
				dwStep = STEP_TEST_VOLTAGE_END;
				break;
			case STEP_RELAY_NOT_FIND:
				g_bError = true;
				LOGFile->AddLOGContent("MODE"+AnsiString(dwModeIndex),"FAIL");
				LOGFile->astrErrorMsg = "(!)RELAY_NOT_FIND";
				MessageBoxA(this->Handle,"(!)繼電器異常", "Error", MB_ICONWARNING);
				dwStep = STEP_TEST_VOLTAGE_END;
				break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TFrmMain::moDebugChange(TObject *Sender)
{
	if(moDebug->Lines->Count >500) moDebug->Clear();
}
//---------------------------------------------------------------------------

void  TFrmMain::SetValueEnabled(bool Enabled)
{
	for(int i = 1 ; i <= 5 ; i++)
	{
		((TEdit*)FrmMain->FindComponent("edtMINVol_Mode"+AnsiString(i)))->Enabled
			= Enabled;
		((TEdit*)FrmMain->FindComponent("edtMAXVol_Mode"+AnsiString(i)))->Enabled
			= Enabled;
		((TEdit*)FrmMain->FindComponent("edtMINCurr_Mode"+AnsiString(i)))->Enabled
			= Enabled;
		((TEdit*)FrmMain->FindComponent("edtMAXCurr_Mode"+AnsiString(i)))->Enabled
			= Enabled;
		((TEdit*)FrmMain->FindComponent("edtSetCurrent"+AnsiString(i)))->Enabled
			= Enabled;
	}

	edtLossVol->Enabled = Enabled;
	plStart->Enabled = !Enabled;
}
void TFrmMain::Delay(ULONG iMilliSeconds) // 原版delay time 用在thread裡面
{
	ULONG iStart;
	iStart = GetTickCount();
	while (GetTickCount() - iStart <= iMilliSeconds)
		Application->ProcessMessages();
}
//---------------------------------------------------------------------------
// PSU Test
int TFrmMain::DL_CMD(DWORD dwIndex)
{

	DWORD dwResult;
	dwResult = CL_DEV_CONTROL->open_dev_com();
	//
	CL_DEV_CONTROL->dbLoadCurrent	= 	((TEdit*)FrmMain->FindComponent("edtSetCurrent"+AnsiString(dwIndex)))->Text.ToDouble();
	CL_DEV_CONTROL->maxLoadVol	   	= 	((TEdit*)FrmMain->FindComponent("edtMAXVol_Mode"+AnsiString(dwIndex)))->Text.ToDouble();
	CL_DEV_CONTROL->minLoadVol	   	= 	((TEdit*)FrmMain->FindComponent("edtMINVol_Mode"+AnsiString(dwIndex)))->Text.ToDouble();
	CL_DEV_CONTROL->maxLoadCurr		= 	((TEdit*)FrmMain->FindComponent("edtMAXCurr_Mode"+AnsiString(dwIndex)))->Text.ToDouble();
	CL_DEV_CONTROL->minLoadCurr		= 	((TEdit*)FrmMain->FindComponent("edtMINCurr_Mode"+AnsiString(dwIndex)))->Text.ToDouble();
	CL_DEV_CONTROL->dwLoadTimeOut   =   dwLoadTimeOut[dwIndex-1];
	CL_DEV_CONTROL->bOCPPolling		=	bPollingOCP[dwIndex-1];

	CL_DEV_CONTROL->dwPollingOCPdenominator = dwPollingOCPdenominator[dwIndex-1];
	//
	if(dwResult == LOADER_OK)
	{
		CL_DEV_CONTROL->que_cmd.c.clear();
		CL_DEV_CONTROL->que_cmd.push("*remote;chan 1;mode cc;pres on;curr:high "+AnsiString(CL_DEV_CONTROL->dbLoadCurrent)+"\n");
		if(CL_DEV_CONTROL->dbLoadCurrent != 0)
		{
			CL_DEV_CONTROL->dbLossVol = edtLossVol->Text.ToDouble();
			CL_DEV_CONTROL->que_cmd.push("LOAD ON\n");
		}
		else
		{
			CL_DEV_CONTROL->dbLossVol = 0;
			CL_DEV_CONTROL->que_cmd.push("LOAD OFF\n");
		}
		CL_DEV_CONTROL->que_cmd.push("meas:curr?;meas:volt?\n");
		//測試
		dwResult = CL_DEV_CONTROL->Dev_CMD_Test() ? TEST_PASS:TEST_FAIL;
		if(CL_DEV_CONTROL->bNoResponse)
		{
			CL_DEV_CONTROL->bCOM_PORT_OPEN = false;
			MessageBox(this->Handle, _T("動態負載機通訊失敗!請檢查線路!"), Application->Title.t_str(), MB_ICONWARNING);
            CL_DEV_CONTROL->que_Msg.push("ERROR : Not Find Device COM-PORT");
			dwResult = STEP_DL_NOT_FIND;
		}
	}
	else if(dwResult == LOADER_NOT_FIND )
	{
		MessageBox(this->Handle, _T("動態負載機通訊失敗!請檢查線路!(Not Find COM-PORT)"), Application->Title.t_str(), MB_ICONWARNING);
		CL_DEV_CONTROL->que_Msg.push("ERROR : Not Find Device COM-PORT");
		CL_DEV_CONTROL->bCOM_PORT_OPEN = false;
		dwResult = STEP_DL_NOT_FIND;
	}
	else if(dwResult == LOADER_NO_SUPPORT )
	{
		MessageBox(this->Handle, _T("動態負載機瓦特數不足!"), Application->Title.t_str(), MB_ICONWARNING);
		CL_DEV_CONTROL->que_Msg.push("ERROR : LODER NO SUPPORT");
		dwResult = STEP_DL_NOT_FIND;
	}
	return dwResult;
}
void __fastcall TFrmMain::plResultDblClick(TObject *Sender)
{
	moDebug->Height = moDebug->Height ? 0 : 300;
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
		FILE_DUT_SET_INI = ExtractFilePath(Application->ExeName)+FILE_DUT_SET_INI;
		delete FileList;
		return true;
	}
}
bool TFrmMain::ReadInISet()
{
	//
	TIniFile *ini;
	String fn = ChangeFileExt(FILE_DUT_SET_INI, ".ini");
	AnsiString strTemp;

	if (!FileExists(fn)) {
		return false;
	}
	ini = new TIniFile(fn);
	float dwMaxWATT = 0,dwNowMaxWATT;
	for(int i = 1 ; i <= 5 ; i++)
	{
		strTemp = "MODE"+AnsiString(i);
		((TEdit*)FrmMain->FindComponent("edtMINVol_Mode"+AnsiString(i)))->Text
			= ini->ReadString(strTemp,"MinVol","20");
		((TEdit*)FrmMain->FindComponent("edtMAXVol_Mode"+AnsiString(i)))->Text
			= ini->ReadString(strTemp,"MaxVol","20");

		((TEdit*)FrmMain->FindComponent("edtMINCurr_Mode"+AnsiString(i)))->Text
			= ini->ReadString(strTemp,"MinCurr","0");
		((TEdit*)FrmMain->FindComponent("edtMAXCurr_Mode"+AnsiString(i)))->Text
			= ini->ReadString(strTemp,"MaxCurr","0");

		((TEdit*)FrmMain->FindComponent("edtSetCurrent"+AnsiString(i)))->Text
			= ini->ReadString(strTemp,"LoadCurrent","3");
		dwLoadTimeOut[i-1] = ini->ReadInteger(strTemp,"TIMEOUT",1000);
		bModeEnable[i-1]   = ini->ReadBool(strTemp,"ENABLE",false);

		dwNowMaxWATT = ini->ReadString(strTemp,"MaxVol","20").ToDouble()* ini->ReadString(strTemp,"LoadCurrent","0").ToDouble();
		if(bModeEnable[i-1] && dwMaxWATT < dwNowMaxWATT)
			dwMaxWATT =	dwNowMaxWATT;
		bModePowerReset[i-1]   = ini->ReadBool(strTemp,"POWER_RESET",false);
		dwModePowerResetDelay[i-1]  = ini->ReadInteger(strTemp,"POWER_RESET_DELAY",1000);
		bPollingOCP[i-1] = ini->ReadBool(strTemp,"OCP_POLLING",false);
		dwPollingOCPdenominator[i-1] = ini->ReadInteger(strTemp,"OCP_POLLING_DENOMINATOR",3);
	}
	//
	edtLossVol->Text = ini->ReadString("PARAMETER","LossVoltage","0");
	dwResultUI_Time  = ini->ReadInteger("PARAMETER","ResultUI-Time",3000);
	dwStartTestDelay = ini->ReadInteger("PARAMETER","StartTestDelay",0);
	bRelayControl    = ini->ReadBool("PARAMETER","RELAY_CONTROL",false);
	if(!bRelayControl)
	{
		plPowerOn->Caption = "";
		plPowerOff->Caption = "";
    }
	strTemp			 = ini->ReadString("PARAMETER","COMPORT","");
	if(strTemp != "")
		frmMsg->SetCOMPortIndex(strTemp);
	plLoopBack->Visible	 = ini->ReadBool("PARAMETER","LOOP_TEST",0);

	CL_DEV_CONTROL->SetMAX_WATT(LOADER_3311F);
	//
	if(dwMaxWATT >75)
		CL_DEV_CONTROL->SetMAX_WATT(LOADER_3310F);
	if(dwMaxWATT >150)
		CL_DEV_CONTROL->SetMAX_WATT(LOADER_3311F);
	if(dwMaxWATT <= 75)
		CL_DEV_CONTROL->SetMAX_WATT(LOADER_3315F);
	//
	delete ini;
	return true;
}
void TFrmMain::SetCOMPortINISetting(AnsiString astrCOMPort)
{
	TIniFile *ini;
	String fn = ChangeFileExt(FILE_DUT_SET_INI, ".ini");
	AnsiString strTemp;

	if (!FileExists(fn)) {
		return;
	}
	ini = new TIniFile(fn);
	//
	ini->WriteString("PARAMETER","COMPORT",astrCOMPort);
	//
	delete ini;
}
void __fastcall TFrmMain::Timer1Timer(TObject *Sender)
{
	Timer1->Enabled = false;
	switch(dwTimerStep)
	{
		case CHECK_DEV_ONLINE:
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
					dwTime 		= GetTickCount()+dwStartTestDelay;
					dwTimeStart = GetTickCount();
					break;
				}
				if(numBarcodeResult == BARCODE_CHANEL)
				{
					plBarcode->Height = 0;
					FrmMain->Refresh();
					plStart->Caption = "Start";
					dwTimerStep = STEP_TIMER_STOP;
					edtBarcodeMAC->Enabled = false;
					plTitle->PopupMenu = popBackPage;
					break;
				}
				if(plStart->Caption == "Start")
				{
					plStart->Enabled = true;
					plBarcode->Height = 0;
					FrmMain->Refresh();
					dwTimerStep = STEP_TIMER_STOP;
					edtBarcodeMAC->Enabled = false;
					plTitle->PopupMenu = popBackPage;
					break;
                }
			   Delay(100);
			}
			break;
		case START_TEST:
			if( GetTickCount() > dwTime)
			{
				dwTime = 0;
				LOGFile->astrErrorMsg = "";
				LOGFile->logName = plIniName->Caption;
				LOGFile->logSN   = plDUT_SN->Caption;
				Voltage_Test();
				dwTimerStep = CHECK_DEV_OFFLINE;
				CL_DEV_CONTROL->que_Msg.push("總測試時間(sec):");
				CL_DEV_CONTROL->que_Msg.push(float(GetTickCount() - dwTimeStart)/1000);
				if(!g_bError)
				{
					dwResultTimeOut = GetTickCount()+dwResultUI_Time;
				}
			}
			break;
		case CHECK_DEV_OFFLINE:
			if(g_bError)
			{
				lbTime->Caption="";
				plStart->Caption = "Start";
				plStart->Enabled = true;
				dwTimerStep = STEP_TIMER_STOP;
				plTitle->PopupMenu = popBackPage;
			}
			else
			{
				lbTime->Caption = (float(GetTickCount()-dwResultTimeOut))/1000;
				if(GetTickCount() > dwResultTimeOut)
				{
					dwResultTimeOut = 0;
					ClearUI();
					dwTimerStep = CHECK_DEV_ONLINE;
					if(plStart->Caption.Pos("Start"))
					{
						plStart->Enabled = true;
						dwTimerStep = STEP_TIMER_STOP;
						plTitle->PopupMenu = popBackPage;
					}
				}
			}
	}
	Timer1->Enabled = true;
}
void TFrmMain::ClearUI()
{
	plReadLoadVolMode1->Font->Color = clBlue;
	plReadLoadVolMode1->Caption = "";
	plReadLoadVolMode2->Font->Color = clBlue;
	plReadLoadVolMode2->Caption = "";
	plReadLoadVolMode3->Font->Color = clBlue;
	plReadLoadVolMode3->Caption = "";
	plReadLoadVolMode4->Font->Color = clBlue;
	plReadLoadVolMode4->Caption = "";
	plReadLoadVolMode5->Font->Color = clBlue;
	plReadLoadVolMode5->Caption = "";
	plResult->Caption = "Wait...";
	plResult->Color = clCream;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::btnBarcodeChanelClick(TObject *Sender)
{
	plStart->Enabled = true;
	numBarcodeResult = BARCODE_CHANEL;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::edtBarcodeMACKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if(Key==13)
	{
		if(edtBarcodeMAC->Text.Length())
		{
			plDUT_SN->Caption = edtBarcodeMAC->Text;
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
    ReadInISet();
	frmMsg->edtSetWorkOrderNumber->Text = edtSetWorkOrderNumber->Caption;
	frmMsg->edtSetEmployeeID->Text = edtSetEmployeeID->Caption.Trim();


	if(frmMsg->ShowModal()== mrOk)
	{
		edtSetWorkOrderNumber->Caption  = frmMsg->edtSetWorkOrderNumber->Text;
		edtSetEmployeeID->Caption = frmMsg->edtSetEmployeeID->Text;

		LOGFile->astrWorkOrderNumber = edtSetWorkOrderNumber->Caption;
		LOGFile->astrEmployeeID		 = edtSetEmployeeID->Caption;
		LOGFile->FindLogFile(ImgDisk);

		ImgDisk->Visible = frmMsg->ImgDisk->Visible;
		CL_RELAY->SerCOMPort(frmMsg->cbCOMPort->Text);
		SetCOMPortINISetting(frmMsg->cbCOMPort->Text);
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
		if( fTime <= float(dwResultUI_Time/1000) && fTime >=0)
			lbTime->Caption = AnsiString(fTime).SubString(1,3);
	}
	else if(dwLoadTime != 0)
	{
		float fTime =	float(GetTickCount()-dwLoadTime)/1000;
		if((fTime*1000) >= (float)CL_DEV_CONTROL->dwLoadTimeOut)
			lbTime->Caption = "";
		else if( fTime < 10 && fTime >=0)
			lbTime->Caption = AnsiString(fTime).SubString(1,3);
		else if( fTime >= 10 && fTime >=0)
			lbTime->Caption = AnsiString(fTime).SubString(1,4);
	}
	else if(dwTime != 0)
	{
		float fTime =	float(dwTime - GetTickCount())/1000;
		if((fTime*1000) >= (float)dwStartTestDelay)
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
void __fastcall TFrmMain::plStartClick(TObject *Sender)
{
	if(plStart->Caption.Pos("Start"))
	{
		plStart->Caption = "Stop";
		plTitle->PopupMenu = NULL;
		plLoopBackNowCount->Caption = "0";
		if(dwTimerStep == STEP_TIMER_STOP)
			dwTimerStep = CHECK_DEV_ONLINE;
	}
	else
	{
		plStart->Enabled = false;
		plStart->Caption = "Start";
	}
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::plStartMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
	plStart->Left++;
	plStart->Top++;
	plStart->Width--;
	plStart->Height--;
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::plStartMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
	plStart->Left--;
	plStart->Top--;
	plStart->Width++;
	plStart->Height++;
}
//---------------------------------------------------------------------------



void __fastcall TFrmMain::plPowerOnClick(TObject *Sender)
{
	//CL_RELAY->CONTROL_RELAY_POWER(true);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::plPowerOffClick(TObject *Sender)
{
	//CL_RELAY->CONTROL_RELAY_POWER(false);
}
//---------------------------------------------------------------------------

void __fastcall TFrmMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	CL_RELAY->CONTROL_RELAY_POWER(false);
}
//---------------------------------------------------------------------------

