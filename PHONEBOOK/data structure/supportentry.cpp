// SupportEntry.cpp: implementation of the CSupportEntry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\phonebook.h"
#include "SupportEntry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSupportEntry::CSupportEntry()
{

}

CSupportEntry::~CSupportEntry()
{

}

CSupportEntry::CSupportEntry(const CSupportEntry &se):m_iMemoryType(se.m_iMemoryType),m_deqBoundary(se.m_deqBoundary)
{

}

CSupportEntry::CSupportEntry(const GSM_Phone_Support_Entry &Entry):m_iMemoryType(Entry.MemoryType)
{
	for(int i = 0; i < Entry.EntriesNum ; i ++)
		m_deqBoundary.push_back(Entry.Entries[i]);
}

const CSupportEntry& CSupportEntry::operator =(const CSupportEntry &se)
{
	m_iMemoryType = se.m_iMemoryType;
	m_deqBoundary = se.m_deqBoundary;
	return *this;
}

const CSupportEntry& CSupportEntry::operator =(const GSM_Phone_Support_Entry &Entry)
{
	m_iMemoryType = Entry.MemoryType;
	for(int i = 0; i < Entry.EntriesNum ; i ++)
		m_deqBoundary.push_back(Entry.Entries[i]);
	return *this;
}
