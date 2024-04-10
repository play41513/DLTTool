//---------------------------------------------------------------------------


#pragma hdrstop

#include "LOGFile.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
CLOGFile::CLOGFile(void)
{
	dwTestCount = 0;
}
void CLOGFile::FindLogFile(TImage* ImgDisk)
{
	//取得LOGFilePath、LOGDiskPath路徑
	LOGFilePath = "C:\\ASMP\\log\\"+astrWorkOrderNumber+"\\DLT-Tool";
	CheckDiskName();
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
void CLOGFile::NewFilePath(AnsiString Path)
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
bool CLOGFile::CheckDiskName()
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
				LOGDiskPath = DiskName+"\\ASMP\\log\\"+astrWorkOrderNumber+"\\DLT-Tool";
				if(!FileExists(LOGDiskPath.c_str())){
					_mkdir( DiskName.c_str());
					SS = DiskName + "\\ASMP";
					_mkdir( SS.c_str());
					SS = DiskName + "\\ASMP\\log";
					_mkdir( SS.c_str());
					SS = DiskName + "\\ASMP\\log\\"+astrWorkOrderNumber;
					_mkdir(SS.c_str());
					_mkdir(LOGDiskPath.c_str());
				}
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------
bool CLOGFile::writeLOG(AnsiString Msg,TImage* ImgDisk,TMemo* moDebug)
{
	bool bPASS[2] = {true,true};
	AnsiString SS;

	dwTestCount++;
	AnsiString strTemp;
	strTemp.sprintf("%04d",dwTestCount);

	if(Msg.Pos("PASS"))
	{
		SS = "\n"+strTemp+",[PASS],"+AnsiString(FormatDateTime("mm-dd-yyyy,hh:mm:ss", Now()))
			+",[NAME],"+logName
			+",[SN],"+logSN
			+astrLOGContent;
		SS+= ",[WorkOrder],"+astrWorkOrderNumber+",[EmployeeID],"+astrEmployeeID;
	}
	else
	{
		SS = "\n"+strTemp+",[FAIL],"+AnsiString(FormatDateTime("mm-dd-yyyy,hh:mm:ss", Now()))
			+",[NAME],"+logName
			+",[SN],"+logSN
			+astrLOGContent;
		SS+= ",[WorkOrder],"+astrWorkOrderNumber+",[EmployeeID],"+astrEmployeeID;
		SS+=",[ErrorMsg],"+astrErrorMsg;
	}
	bPASS[0] = SaveLogLine(LOGFilePath,SS);
	if(moDebug != NULL)
		moDebug->Lines->Add(SS);
	if(LOGDiskPath.Pos("csv") && ImgDisk->Visible )
	{
		bPASS[1] = SaveLogLine(LOGDiskPath,SS);
		if(!bPASS[1])
			ImgDisk->Visible = false;
	}
	for(int i = 0 ; i < 2 ; i++)
		if(!bPASS[i]) return false;

	return true;
}
bool  CLOGFile::SaveLogLine(AnsiString FileName,AnsiString FileLine)
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
void CLOGFile::ClearLOGContent()
{
	astrLOGContent = ",[MODE1],NULL,[MODE2],NULL,[MODE3],NULL,[MODE4],NULL,[MODE5],NULL";
}
void CLOGFile::AddLOGContent(AnsiString strClass,AnsiString strContent)
{
	DWORD dwPos = astrLOGContent.Pos(strClass);
	astrLOGContent = astrLOGContent.SubString(1,dwPos+6)
					+strContent
					+astrLOGContent.SubString(dwPos+11,astrLOGContent.Length()-dwPos+11);

}
