// CSVHandle.cpp: implementation of the CCSVHandle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\phonebook.h"
#include "CSVHandle.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCSVHandle::CCSVHandle()
{

}

CCSVHandle::~CCSVHandle()
{

}

CCSVHandle::CCSVHandle(LPCTSTR lpszFile)
{
	m_sFile = lpszFile;
}

void CCSVHandle::SetFilePath(LPCTSTR lpszFile)
{
	m_sFile = lpszFile;
}


bool CCSVHandle::GetHandle(deque<Tstring> &deqHandle)
{
	//check the file exist
	if(m_sFile.empty())
		return false;

	//open the file with file stream
#ifdef _UNICODE
	USES_CONVERSION;
	ifstream file(W2A(m_sFile.c_str()));
#else
	ifstream file(m_sFile.c_str());
#endif
	//check open or not
	if(file.is_open()){
		file.seekg(0);
		//get every item
		char c;
		string str;
		bool bStart = false;
		while(file.get(c)){
			if( c == '\n')
				break;
			if(c == '\"')
				bStart = ( bStart == true ) ? false : true;
			if(bStart && c != '\"'){
				str += c;
			}
			else if(bStart == false && c != ','){
#ifdef _UNICODE
				USES_CONVERSION;
				Tstring Tstr;
				Tstr = A2W(str.c_str());
				deqHandle.push_back(Tstr);
#else
				deqHandle.push_back(str);
#endif
				str = "";
			}
		}
		//close this file after read
		file.close();
		return true;
	}
	return false;
}

bool CCSVHandle::WriteHandle(deque<Tstring> &deqHandle)
{
	//check the file exist
	if(m_sFile.empty())
		return false;

	//check the handle deque empty
	if(deqHandle.empty())
		return false;

	//open the file with file stream
//	ofstream file(m_sFile.c_str());
#ifdef _UNICODE
	USES_CONVERSION;
	ofstream file(W2A(m_sFile.c_str()));
#else
	ofstream file(m_sFile.c_str());
#endif
	//check open or not
	if(file.is_open()){
		//Get every item in the deque
		for(deque<Tstring>::iterator iter = deqHandle.begin() ; iter != deqHandle.end() ; iter ++){
#ifdef _UNICODE
			file << '\"' << W2A((*iter).c_str()) << '\"';
			if(iter != deqHandle.end() - 1)
				file << ',';
#else
			file << '\"' << (*iter).c_str() << '\"';
			if(iter != deqHandle.end() - 1)
				file << ',';
#endif
		}
		file << endl;
		//close this file after read
		file.close();
		return true;
	}
	return false;
	
}

//////////////////////////////////////////////////////////////////////////
//
// Function : Write contrsct data to file
//////////////////////////////////////////////////////////////////////////
bool CCSVHandle::WriteContact(DATALIST &deqContact)
{
	//check the file exist
	if(m_sFile.empty())
		return false;

	//check the handle deque empty
	if(deqContact.empty())
		return false;

	//open the file with file stream
#ifdef _UNICODE
	USES_CONVERSION;
	ofstream file(W2A(m_sFile.c_str()),ios::out | ios::app );
#else
	ofstream file(m_sFile.c_str(),ios::out | ios::app);
#endif
	//check open or not
	if(file.is_open()){
		//Get every item in the deque
		bool bStart = true;
		for(DATALIST::iterator iter = deqContact.begin() ; iter != deqContact.end() ; iter ++){
			//add Recognition and mobile name column
			if(bStart){
				TCHAR szRecon[MAX_PATH];
				if(al_GetSettingString(_T("public"),_T("IDS_CAMAGICMOBILE"),theApp.m_szRes,szRecon))
#ifdef _UNICODE
					file << '\"' << W2A(szRecon) << '\"' << ',' << '\"' << W2A(theApp.m_szMobileName) << '\"' << ',';
#else
					file << '\"' << szRecon << '\"' << ',' << '\"' << theApp.m_szMobileName << '\"' << ',';
#endif
				bStart = false;	
			}
			else{
#ifdef _UNICODE
				file << "\"\",\"\",";
#else
				file << "\"\",\"\",";
#endif
			}

			//Get the data in the CCardPanel object
			deque<Tstring>	deqData;
			(*iter).GetDataDeque(deqData);
			//write all data of one object to file
			for(deque<Tstring>::iterator iter = deqData.begin() ; iter != deqData.end() ; iter ++)
#ifdef _UNICODE
				file << '\"' << W2A((*iter).c_str() )<< '\"' << ',';
#else
				file << '\"' << (*iter).c_str() << '\"' << ',';
#endif
			//end of line
			file << endl;
		}
		//close this file after read
		file.close();
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Function : Get contrsct data from file
//////////////////////////////////////////////////////////////////////////
bool CCSVHandle::GetContact(DATALIST &lsContact)
{
	//check the file exist
	if(m_sFile.empty())
		return false;

	//open the file with file stream
#ifdef _UNICODE
	USES_CONVERSION;
	ifstream file(W2A(m_sFile.c_str()),ios::in);
#else
	ifstream file(m_sFile.c_str(),ios::in);
#endif
	//check open or not
	if(file.is_open()){
		//Get the handle
		char c;
		vector<char> vecLine;
		bool bHandle = false;
		int iIndex = 1;
		//get one char 
		while(file.get(c)){
			if(c != '\n')
				vecLine.push_back(c);	//get a line
			else if(bHandle){
				//get one line
				string str;
				bool bStart = false;
				//get the data in the 
				deque<Tstring> deqData;
				int i = 0;
				for(vector<char>::iterator iter = vecLine.begin() ; iter != vecLine.end() ; iter ++){
					//check description
					if(iter == vecLine.begin() && (*iter) != '\"')
						break;
					//check the char
					if((*iter) == '\"')
						bStart = ( bStart == true ) ? false : true;
					if(bStart && (*iter) != '\"')
						str += (*iter);
					else if(bStart == false && (*iter) != ','){
						if( i > 1)
						{
#ifdef _UNICODE
							USES_CONVERSION;
							Tstring Tstr;
							Tstr = A2W(str.c_str());
							deqData.push_back(Tstr);
#else
							deqData.push_back(str);
#endif
						}
						i ++;
						str = "";
					}
				}			
				vecLine.clear();
				//set data in the CCardPanel object
				if(deqData.size() > 0){
					CCardPanel data;
					data.SetDataDeque(deqData);
					//set data index
					char sz[MAX_PATH];
					itoa(iIndex,sz,10);
					data.SetLocation(sz);
					iIndex ++;
					//set data memory type to be CSVFILE
					data.SetStorageType(CSVFILE_NAME);
					//push data to deque
					lsContact.push_back(data);
				}
			}
			else{
				//clear the line
				bHandle = true;
				vecLine.clear();
			}
		}
		//close this file after read
		file.close();
		return true;
	}
	return false;
}


bool CCSVHandle::CheckFormat()
{
	//check the file exist
	if(m_sFile.empty())
		return false;

	//open the file with file stream
#ifdef _UNICODE
	USES_CONVERSION;
	ifstream file(W2A(m_sFile.c_str()),ios::in);
#else
	ifstream file(m_sFile.c_str(),ios::in);
#endif
	//check open or not
	if(file.is_open()){
		//Get the handle
		char c;
		vector<char> vecLine;
		bool bHandle = false;
		int iIndex = 1;
		//get one char 
		while(file.get(c)){
			if(c != '\n')
				vecLine.push_back(c);	//get a line
			else if(bHandle){	//end of line and not handle
				//get one line
				string str;
				bool bStart = false;
				//get the data in the 
				deque<Tstring> deqData;
				for(vector<char>::iterator iter = vecLine.begin() ; iter != vecLine.end() ; iter ++){
					//check the char
					if((*iter) == '\"')
						bStart = ( bStart == true ) ? false : true;
					if(bStart && (*iter) != '\"')
						str += (*iter);
					else if(bStart == false && (*iter) != ',')
					{
#ifdef _UNICODE
							USES_CONVERSION;
							Tstring Tstr;
							Tstr = A2W(str.c_str());
							deqData.push_back(Tstr);
#else
							deqData.push_back(str);
#endif
						str = "";
					}
				}			
				//clear char vector 
				vecLine.clear();
				//check the key
				deque<Tstring>::iterator iterStr =  deqData.begin();
				TCHAR szKey[MAX_PATH];
				if(al_GetSettingString(_T("public"),_T("IDS_CAMAGICMOBILE"),theApp.m_szRes,szKey)){
					if((*iterStr) == szKey)
						return true;
					else
						return false;
				}

				break;
			}
			else{
				//clear the line
				bHandle = true;
				vecLine.clear();
			}
		}
		//close this file after read
		file.close();
	}
	return false;
	
}

bool CCSVHandle::WriterDescription()
{
	if(m_sFile.empty())
		return false;

	//open the file with file stream
#ifdef _UNICODE
	USES_CONVERSION;
	ofstream file(W2A(m_sFile.c_str()),ios::out | ios::app);
#else
	ofstream file(m_sFile.c_str(),ios::out | ios::app);
#endif
	//check open or not
	if(file.is_open()){
		TCHAR szTxt[MAX_PATH * 4];
		if(al_GetSettingString(_T("public"),_T("IDS_CSV_DESCRIPTION"),theApp.m_szRes,szTxt)){
#ifdef _UNICODE
			USES_CONVERSION;
			file << W2A(szTxt) << endl ;
#else
			file << szTxt << endl ;
#endif
		}
		file.close();
		return true;
	}
	return false;
}
