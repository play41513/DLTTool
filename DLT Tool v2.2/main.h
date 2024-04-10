//---------------------------------------------------------------------------

#ifndef mainH
#define mainH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include <dbt.h> //DBT_常數 註冊要取得的裝置消息
#include "IO_Request.h"
#include <Buttons.hpp>
#include <Registry.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
#include <Menus.hpp>
#include <Graphics.hpp>

#include "COM_control.h"
#include "USBDevConnectionInfo.h"
#include <pngimage.hpp>
#include <queue>
#include <fstream.h>
#include <string>
#include <winuser.h>
#include <direct.h>

const AnsiString APP_TITLE = "DLT Tool ver.2.2 (Dynamic load Test Tool, Action Star ,2021)";
const AnsiString PASSWORD  = "SET";

const float UNIT = 0.0195; //單位元電壓值

//GUID
const char GUID_USB_HUB[] = "{F18A0E88-C30C-11D0-8815-00A0C906BED8}";
const char GUID_USB_DEVICE[] = "{A5DCBF10-6530-11D2-901F-00C04FB951ED}";
const char GUID_HID[] = "{4d1e55b2-f16f-11cf-88cb-001111000030}";

//
#define GET_VALUE_TIMEOUT_MS 500
#define TIME_INTERVAL 200
#define HARDWARE_VOLTAGE_COMPENSATION 13

#define HID_IS_ONLINE		0
#define HID_TURN_ON			1
#define SET_VOLTAGE			2
#define EXECUTE_SETTING 	3
#define TEST_VOLTAGE_END	6
#define HID_NOT_FIND		7
#define DL_NOT_FIND		    8
#define TEST_PASS		    9
#define TEST_FAIL		    10
#define READ_NO_LOAD_VOLTAGE_VALUE		4
#define READ_FULL_LOAD_VOLTAGE_VALUE	5
#define READ_OVER_LOAD_VOLTAGE_VALUE	11
#define BACK_NO_LOAD_VOLTAGE 			12

#define CHECK_DEV_ONLINE	1
#define START_TEST			2
#define CHECK_DEV_OFFLINE	3

#define REBACK_NO_LOAD		    12

#define BARCODE_FINISH	1
#define BARCODE_CHANEL  2
#define STEP_TIMER_STOP	99


#define DEBUG(String)    FrmMain->moDebug->Lines->Add(String)

//---------------------------------------------------------------------------

class TFrmMain : public TForm
{
__published:	// IDE-managed Components
	TPanel *pl_Set_Switch;
	TPanel *Panel1;
	TBitBtn *btnStart;
	TPanel *Panel7;
	TPanel *pl_Set;
	TPanel *Panel71;
	TEdit *edtPassWord;
	TBitBtn *btnSet;
	TPanel *pl_Debug;
	TMemo *moDebug;
	TPanel *plIniName;
	TPanel *plResult;
	TTimer *Timer1;
	TPanel *Panel6;
	TPanel *pl_set_title;
	TPanel *plNoLoadTitle;
	TPanel *plFullLoadTitle;
	TPanel *Panel55;
	TPanel *pl_SetRange;
	TPanel *plNoLoad;
	TEdit *edt_max;
	TPanel *plTitle2;
	TEdit *edt_min;
	TPanel *plFullLoad;
	TEdit *edt_load_min;
	TPanel *Panel50;
	TEdit *edt_load_max;
	TPanel *Panel52;
	TPanel *plCKB;
	TCheckBox *ckbAuto;
	TPanel *Panel10;
	TPanel *Panel11;
	TEdit *edtVID;
	TPanel *Panel9;
	TPanel *Panel12;
	TEdit *edtPID;
	TPanel *plSpace;
	TPanel *plOverLoadRangeTitle;
	TPanel *plOverLoadRange;
	TEdit *edt_overload_min;
	TPanel *Panel53;
	TEdit *edt_overload_max;
	TPanel *plTitle;
	TPanel *Panel5;
	TPanel *plVoltageBox;
	TPanel *pl_FullloadTitle;
	TPanel *pl_NoloadTitle;
	TPanel *pl_OverloadTitle;
	TPanel *plvotage;
	TPanel *plReadNoLoadVol;
	TPanel *plReadFullLoadVol;
	TPanel *plReadOverLoadVol;
	TPanel *plVPID;
	TCheckBox *ckbBarcode;
	TPanel *plBarcode;
	TLabel *lbBarcode;
	TPanel *Panel13;
	TBitBtn *btnBarcodeChanel;
	TBitBtn *btnBarcodeClear;
	TEdit *edtBarcodeMAC;
	TPanel *Panel15;
	TPanel *Panel16;
	TPanel *Panel17;
	TPanel *edtSetEmployeeID;
	TPanel *edtSetWorkOrderNumber;
	TImage *ImgDisk;
	TPanel *plSet2;
	TPanel *Panel75;
	TEdit *edtLossVol;
	TPanel *Panel4;
	TPanel *plLossVol;
	TImage *Image1;
	TPanel *plFullCur;
	TPanel *Panel72;
	TEdit *edtSetCurrent;
	TPanel *Panel73;
	TPanel *plOverCur;
	TPanel *Panel21;
	TEdit *edtSetOverLoadCurrent;
	TPanel *Panel23;
	TPanel *Panel8;
	TCheckBox *ckbOverLoad;
	TPanel *Panel2;
	TPanel *Panel22;
	TPanel *Panel18;
	TPanel *Panel14;
	TTimer *Timer2;
	TLabel *lbTime;
	TPanel *plDUT_SNTitle;
	TPanel *Panel19;
	TPanel *plDUT_SN;
	TPopupMenu *popBackPage;
	TMenuItem *popItem;
	void __fastcall btnStartClick(TObject *Sender);
	void __fastcall ckbAutoClick(TObject *Sender);
	void __fastcall edtVIDChange(TObject *Sender);

	void __fastcall moDebugChange(TObject *Sender);
	void __fastcall edtPassWordEnter(TObject *Sender);
	void __fastcall btnSetClick(TObject *Sender);
	void __fastcall pl_ckb1_1Click(TObject *Sender);
	void __fastcall edt_minChange(TObject *Sender);
	void __fastcall edt_minExit(TObject *Sender);
	void __fastcall pl_Set_SwitchClick(TObject *Sender);
	void __fastcall plResultDblClick(TObject *Sender);
	void __fastcall edtPassWordKeyPress(TObject *Sender, wchar_t &Key);
	void __fastcall ckbOverLoadClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall btnBarcodeChanelClick(TObject *Sender);
	void __fastcall edtBarcodeMACKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall btnBarcodeClearClick(TObject *Sender);
	void __fastcall popItemClick(TObject *Sender);

private:	// User declarations
	void Voltage_Test();
	void SetValueEnabled(bool Enabled);

	int  DL_CMD(AnsiString CMD);

	void ReadRegSet();
	void ClearUI();
	void ShowSettingForm();
	bool TFrmMain::CheckDiskName();
	bool TFrmMain::writeLOG(AnsiString Msg);
	void TFrmMain::NewFilePath(AnsiString Path);
	void TFrmMain::FindLogFile();
	void SetRegVal(AnsiString numTestItem,bool AutoTest,AnsiString DutVPID);
	bool CheckVPIDSET(TEdit * edt);
	bool SaveVoltageRangeSetting();
	void SetForegroundWindowInternal(HWND hWnd);
	bool FindIniFile();
	bool ReadInISet();
	bool SaveLogLine(AnsiString FileName,AnsiString FileLine);
	bool SetInIVal(bool AutoTest,AnsiString DutVPID);
	AnsiString Findfilemsg(AnsiString filename, AnsiString findmsg,
	int rownum);
	AnsiString FILE_DUT_SET_INI;
	AnsiString LOGFilePath,LOGDiskPath;
	AnsiString strNoLoad,strFullLoad,strOverLoad,strBackNoLoad;
	DWORD numBarcodeResult;
	DWORD dwOverLoad_TestTime,dwFullLoad_TestTime,dwNoLoad_TestTime,dwBackNoLoad_TestTime;
	DWORD dwResultUI_Time;
public:		// User declarations

	bool g_bError;
	cCOM *CL_DEV_CONTROL;
	USBDevConnectionInfo *CL_USB;
	void Delay(ULONG iMilliSeconds);
	__fastcall TFrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFrmMain *FrmMain;
//---------------------------------------------------------------------------
#endif
