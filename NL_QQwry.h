#pragma once

#define MAXBUF 50

typedef struct _tagEndInfo
{
	BYTE b0;
	BYTE b1;
	BYTE b2;
	BYTE b3;
	BYTE buf[MAXBUF];
	BYTE bMode;
	int offset1;
	int offset2;
}EndInfo,PEndInfo;

typedef struct _tagIPOFF
{
	BYTE b0;
	BYTE b1;
	BYTE b2;
	BYTE b3;
	BYTE off1;
	BYTE off2;
	BYTE off3;
}IPOFF,*PIPOFF;

typedef struct _tagBE
{
	int uBOff;
	int uEOff;
}BE,*PBE;

class IPwry
{
public:
	DWORD m_dwLastIP;
	CString IP2Add(CString szIP);
	CString GetCountryLocal(int index);
	DWORD GetSIP(int index);
	DWORD IP2DWORD(CString szIP);
	int GetIndex(CString szIP);
	void SaveToFile();
	CString GetStr(void);
	CString GetCountryLocal(BYTE bMode,int ioffset);
	CString GetStr(int ioffset);
	int GetRecordCount(void);
	int m_i;
	int GetStartIPInfo(int iIndex);
	CString m_buf;
	CString Test(void);
	bool GetBE(void);
	bool OpenQQwry(CString szFileName);
	void CloseQQwry(void);
	BE m_be;
	IPOFF m_ipoff;
	EndInfo m_ei;
	IPwry(void);
	~IPwry(void);
private:
	bool m_bOpen;
	CFile m_file;

};

