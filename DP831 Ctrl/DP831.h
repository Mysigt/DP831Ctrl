#pragma once
#ifndef DP831_H
#define DP831_H
#include <atlstr.h>
#include <string>

class DPSrc
{
public:
	CString m_strInstrAddr[2]; //instrument address
	CString m_strResult; //Return value
	
	void Connect(); //initialize connection with instrument
	void Send(CString m_strCommand, int srcNum); //send command to instrument
	void Read(int srcNum); //read from instrument

private:
	bool InstrWrite(CString strAddr, CString strContent); //used to implement connect/send/read
	bool InstrRead(CString strAddr, CString *pstrResult);
};
#endif
