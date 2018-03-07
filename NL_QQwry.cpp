#include "StdAfx.h"
#include "NL_QQwry.h"


IPwry::IPwry(void)
{
	m_bOpen=OpenQQwry("QQwry.dat");
	GetBE();
}


IPwry::~IPwry(void)
{
	CloseQQwry();
}

bool IPwry::OpenQQwry(CString szFileName)
{
	if(!m_file.Open(szFileName,CFile::modeRead|CFile::typeBinary))
		return false;
	else
		return true;
}

void IPwry::CloseQQwry()
{
	if(m_bOpen)m_file.Close();
}

bool IPwry::GetBE()
{
	if(!m_bOpen)return false;
	m_file.Seek(0,CFile::begin);
	if(m_file.Read(&m_be,sizeof(BE))>0)
		return true;
	else
		return false;
}

int IPwry::GetStartIPInfo(int iIndex)
{
	BYTE buf[MAXBUF];
	int ioff;
	if(!m_bOpen) return 0;
	ioff=m_be.uBOff+iIndex*7;
	if(ioff>m_be.uEOff) return 0;
	m_file.Seek(m_be.uBOff+iIndex*7,CFile::begin);
	m_file.Read(&m_ipoff,sizeof(IPOFF));
	ioff=(m_ipoff.off1+m_ipoff.off2*256+m_ipoff.off3*256*256);
	m_file.Seek(ioff,CFile::begin);
	m_file.Read(&m_ei,sizeof(EndInfo));

	if(m_ei.buf[0]!=1 &&  m_ei.buf[0]!=2)
	{
		m_ei.bMode=1;//没有跳
		for(int i=0;i<MAXBUF;i++)
		{
			if(m_ei.buf[i]==0)
			{
				if(m_ei.buf[i+1]==2)
				{
					m_ei.bMode=2;//Local 跳
					m_ei.offset1=m_ei.buf[i+2]+
						m_ei.buf[i+3]*256+
						m_ei.buf[i+4]*256*256;
				}
				break;
			}
		}
	}
	else if(m_ei.buf[0]==2)
	{
		m_ei.bMode=3;//Country 跳 local不跳
		m_ei.offset1=m_ei.buf[1]+m_ei.buf[2]*256+m_ei.buf[3]*256*256;
		if(m_ei.buf[4]!=2)
		{
			m_ei.bMode=3;
		}
		else
		{
			m_ei.bMode=4;//Country跳 local跳
			m_ei.offset2=m_ei.buf[5]+m_ei.buf[6]*256+m_ei.buf[7]*256*256;
		}
	}
	else if(m_ei.buf[0]==1)
	{
		m_ei.offset1=m_ei.buf[1]+m_ei.buf[2]*256+m_ei.buf[3]*256*256;
		m_file.Seek(m_ei.offset1,CFile::begin);
		m_file.Read(buf,MAXBUF);
		memcpy(m_ei.buf,buf,MAXBUF);
		m_ei.bMode=0;
		if(m_ei.buf[0]!=2)
		{
			for(int i=0;i<MAXBUF;i++)
			{
				if(m_ei.buf[i]==0)
				{
					if(m_ei.buf[i+1]!=2)
					{
						m_ei.bMode=5;//1 没有跳
					}
					else
					{
						m_ei.bMode=6;//1 Country不跳 Local 跳
						m_ei.offset2=m_ei.buf[i+2]+
							m_ei.buf[i+3]*256+
							m_ei.buf[i+4]*256*256;
					}
					break;
				}
			}
		}
		else
		{
			
			if(m_ei.buf[4]!=2)
			{
				m_ei.bMode=7;// 1 Country跳 Local不跳
				m_ei.offset2=m_ei.buf[1]+
							m_ei.buf[2]*256+
							m_ei.buf[3]*256*256;

			}
			else
			{
				m_ei.bMode=8;// 1 Country跳 Local跳
				m_ei.offset1=m_ei.buf[1]+
					m_ei.buf[2]*256+
					m_ei.buf[3]*256*256;
				m_ei.offset2=m_ei.buf[5]+
					m_ei.buf[6]*256+
					m_ei.buf[7]*256*256;
			}
		}
	}
	return ioff;
}

int IPwry::GetRecordCount(void)
{
	if(!m_bOpen) return 0;
	if((m_be.uEOff-m_be.uBOff)<0) return 0;
	return (m_be.uEOff-m_be.uBOff)/7+1;
}

CString IPwry::GetStr(int ioffset)
{
	if(ioffset>m_be.uEOff) return "";
	BYTE ch;
	CString buf="";
	m_file.Seek(ioffset,CFile::begin);
	int i=0;
	while(1)
	{
		m_file.Read(&ch,1);
		if(ch==0)
			break;
		buf+=ch;
		i++;
		if(i>50)break;
	}
	return buf;
}
CString IPwry::GetCountryLocal(int index)
{
	if(index<0 || index>GetRecordCount()-1)
		return "未知IP";
	return GetCountryLocal(m_ei.bMode,GetStartIPInfo(index)+4);
}
CString IPwry::GetCountryLocal(BYTE bMode,int ioffset)
{
	CString buf="";
	if(bMode==1)//X 没有跳
	{
		buf=GetStr(ioffset);
		buf+=" ";
		buf+=GetStr();
	}
	if(bMode==2)//X Country不跳 Local 跳
	{
		buf=GetStr(ioffset);
		buf+=" ";
		buf+=GetStr(m_ei.offset1);
	}
	if(bMode==3)//2 Country跳 local不跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(ioffset+4);
	}

	if(bMode==4)//2 Country跳 local跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(m_ei.offset2);
	}

	if(bMode==5)//1 没有跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr();
	}

	if(bMode==6)//1 Country不跳 Local 跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(m_ei.offset2);
	}
	if(bMode==7)//1 Country跳 Local 不跳
	{
		buf=GetStr(m_ei.offset2);
		buf+=" ";
		buf+=GetStr(m_ei.offset1+4);
	}
	if(bMode==8)//1 Country跳 Local跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(m_ei.offset2);
	}
	return buf;
}

CString IPwry::GetStr()
{
	BYTE ch;
	CString buf="";
	int i=0;
	while(1)
	{ 
		m_file.Read(&ch,1);
		if(ch==0)
			break;
		buf+=ch;
		i++;
		if(i>50)break;
	}
	return buf;
}

void IPwry::SaveToFile()
{
	//
	//在内存中操作可能速度更快一些
	//利用缓存，就这样了，提高速度就自己解决了
	//
	FILE *out;
	CString str1,str2;
	out=fopen("out.txt","wb");
	int ioff;
	//m_buf.Format("%08X %08X",m_be.uBOff,m_be.uEOff);
	m_buf.Format("Total %d\r\n",GetRecordCount());
	fwrite(m_buf,1,m_buf.GetLength(),out);
	for(m_i=0;m_i<GetRecordCount();m_i++)
	//for(m_i=0;m_i<2000;m_i++)
	{
		ioff=GetStartIPInfo(m_i);
		str1.Format("%d.%d.%d.%d",m_ipoff.b3,m_ipoff.b2,m_ipoff.b1,m_ipoff.b0);
		str2.Format("%d.%d.%d.%d",m_ei.b3,m_ei.b2,m_ei.b1,m_ei.b0);
		m_buf.Format("%-15s %-15s %s\r\n",
			str1,str2,GetCountryLocal(m_ei.bMode,ioff+4));
		fwrite(m_buf,1,m_buf.GetLength(),out);
	}
	fclose(out);
}
CString IPwry::IP2Add(CString szIP)
{
	if(szIP=="")
		return "请输入IP地址";
	if(szIP=="127.0.0.1")
		return "本机IP";
	return GetCountryLocal(GetIndex(szIP));
}

int IPwry::GetIndex(CString szIP)
{
	int index=-1;
	DWORD dwInputIP;
	DWORD dwStartIP;
	dwInputIP=IP2DWORD(szIP);
	//顺序查找速度肯定慢
	/*
	
	if(dwInputIP<=0x7FFFFFFF)
	for (int i=0;i<GetRecordCount();i++)
	{
		dwStartIP=GetSIP(i);
		if(dwStartIP<=dwInputIP && dwInputIP<=m_dwLastIP)
		{index=i;break;}
	}
	else
	for (int i=GetRecordCount()-1;i>=0;i--)
	{
		dwStartIP=GetSIP(i);
		if(dwStartIP<=dwInputIP && dwInputIP<=m_dwLastIP)
		{index=i;break;}
	}
	*/

	//
	//利用半跳方法速度快一些
	//
	int iT;
	int iB,iE;
	iB=0;
	iE=GetRecordCount()-1;
	iT=iE/2;
	
	while(iB<iE)
	{
		dwStartIP=GetSIP(iT);
		if(dwInputIP==dwStartIP)
		{
			index =iT;
			break;
		}
		if((iE-iB)<=1)
		{
			/*
			CString s;
			s.Format("%d %d %d %d in:%08X s:%08X",iE-iB,iB,iT,iE,dwInputIP,dwStartIP);
			if(MessageBox(0,s,0,MB_YESNO)==IDNO);
			//*/
			for(int i=iB;i<=iE;i++)
			{
				dwStartIP=GetSIP(i);
				if(dwStartIP<=dwInputIP && dwInputIP<=m_dwLastIP)
				{
					index=i;
					break;
				}
			}
			break;
		}

		if(dwInputIP>dwStartIP)
		{
			iB=iT;
		}
		else
		{
			iE=iT;
		}
		iT=iB+(iE-iB)/2;
	}
	return index;
}

DWORD IPwry::GetSIP(int index)
{
	DWORD ip;
	BYTE b[3];
	int ioff;
	if(!m_bOpen)return -1;
	if(index>GetRecordCount()-1)return -1;
	if(index<0)return -1;
	ioff=m_be.uBOff+index*7;
	m_file.Seek(ioff,CFile::begin);
	m_file.Read(&ip,4);
	m_file.Read(b,3);
	ioff=b[0]+b[1]*256+b[2]*256*256;
	m_file.Seek(ioff,CFile::begin);
	m_file.Read(&m_dwLastIP,4);
	return ip;
}

DWORD IPwry::IP2DWORD(CString szIP)
{
	DWORD iIP;
	BYTE b[4];
	CString szTemp;
	char ch;
	int iLen;
	int iXB;
	szIP+=".";
	memset(b,0,4);
	iLen=szIP.GetLength();
	iXB=0;
	iIP=0;
	for(int i=0;i<iLen;i++)
	{
		ch=szIP.GetAt(i);
		szTemp+=ch;
		if(ch=='.')
		{
			b[iXB]=atoi(szTemp);
			szTemp="";
			iXB++;
		}
	}
	iIP=b[0]*256*256*256+b[1]*256*256+b[2]*256+b[3];
	return iIP;
}



CString IPwry::Test()
{
	int ioff;
	CString str;
	//m_buf.Format("%08X %08X",m_be.uBOff,m_be.uEOff);
	m_buf.Format("Total %d\r\n",GetRecordCount());
	
	//for(m_i=GetRecordCount()-200;m_i<GetRecordCount();m_i++)
	for(m_i=0;m_i<1000;m_i++)
	{
		ioff=GetStartIPInfo(m_i);

		if(m_ei.bMode>=1 && m_ei.bMode<=8)
		{
			str.Format("%6d %03d.%03d.%03d.%03d "
				"%03d.%03d.%03d.%03d  %d  %06X ",
			m_i,
			m_ipoff.b3,
			m_ipoff.b2,
			m_ipoff.b1,
			m_ipoff.b0,
			m_ei.b3,
			m_ei.b2,
			m_ei.b1,
			m_ei.b0,
			m_ei.bMode,
			ioff);
		m_buf+=str;

			str=GetCountryLocal(m_ei.bMode,ioff+4);
			str+="\r\n";
		}
		else
		{
		str.Format("%6d %03d.%03d.%03d.%03d "
			"%03d.%03d.%03d.%03d (%d) %06X ",
			m_i,
			m_ipoff.b3,
			m_ipoff.b2,
			m_ipoff.b1,
			m_ipoff.b0,
			m_ei.b3,
			m_ei.b2,
			m_ei.b1,
			m_ei.b0,
			m_ei.bMode,
			ioff);
		m_buf+=str;

		str.Format("%02X-%02X-%02X-%02X-%02X "
			"%02X-%02X-%02X-%02X-%02X "
			"%02X-%02X-%02X-%02X-%02X "
			"%02X-%02X-%02X-%02X-%02X\r\n",

			m_ei.buf[0],
			m_ei.buf[1],
			m_ei.buf[2],
			m_ei.buf[3],
			m_ei.buf[4],
			m_ei.buf[5],
			m_ei.buf[6],
			m_ei.buf[7],
			m_ei.buf[8],
			m_ei.buf[9],
			m_ei.buf[10],
			m_ei.buf[11],
			m_ei.buf[12],
			m_ei.buf[13],
			m_ei.buf[14],
			m_ei.buf[15],
			m_ei.buf[16],
			m_ei.buf[17],
			m_ei.buf[18],
			m_ei.buf[19]);
		}
		m_buf+=str;
	}
	return m_buf;
}
