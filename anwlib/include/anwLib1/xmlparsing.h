/*/////////////////////////////////////////////////////////////////////////////

Copyright (c) 2003 Advanced & Wise Technology Corp.  All Rights Reserved.
File Name:		xmlparsing.h
Author:			max huang
Description:	xml parsing
Platform:		Win32
Compiler:		MS Visual C++ 6.0 in sp5 with Platform SDK Nov 2001.
Data:			2001/08/28 v0.1			max huang start develop
				2003/01/30						  add to anwclass

/////////////////////////////////////////////////////////////////////////////*/

#ifndef __XMLPARSING_H_
#define __XMLPARSING_H_

///////////////////////////////////////////////////////////////////////////////
//#pragma once

#include "StdString.h"
#pragma warning( disable : 4786 )
#import "msxml.dll"
using namespace MSXML;

class ANWLIB1_EXT CXmlParsing
{
	public:
		CXmlParsing();
		~CXmlParsing();

	public:
		BOOL LoadFile( LPCTSTR file );

		void Initialize();				
		void Uninitialize()
				{	m_docPtr.Release();	
					CoUninitialize();	
				}

		inline MSXML::IXMLDOMElementPtr GetElementPtr();
		inline MSXML::IXMLDOMNodeListPtr GetChildNode( MSXML::IXMLDOMElementPtr pElement );
		inline MSXML::IXMLDOMNodePtr GetParentNode( MSXML::IXMLDOMNodePtr pNode );
		inline MSXML::IXMLDOMNodePtr GetNode( MSXML::IXMLDOMNodeListPtr pNodeList, long index=0 );
		MSXML::IXMLDOMNodePtr GetNode( MSXML::IXMLDOMElementPtr pElement, TCHAR* name, long index=0 );

		CStdString& GetNodeName( MSXML::IXMLDOMNodePtr pNode );
		CStdString& GetNodeValue( MSXML::IXMLDOMNodePtr pNode );
		CStdString& GetAttribute( MSXML::IXMLDOMElementPtr pElement, TCHAR* name );
		CStdString& GetNodeData( MSXML::IXMLDOMElementPtr pElement, TCHAR* name );

	//private:
		MSXML::IXMLDOMDocumentPtr	m_docPtr;
		CStdString			m_str;
};

inline
MSXML::IXMLDOMElementPtr CXmlParsing::GetElementPtr()
{	return m_docPtr->documentElement;
}

inline
MSXML::IXMLDOMNodeListPtr CXmlParsing::GetChildNode( MSXML::IXMLDOMElementPtr	pElement )
{	return pElement->GetchildNodes();
}

inline
MSXML::IXMLDOMNodePtr CXmlParsing::GetNode( MSXML::IXMLDOMNodeListPtr pNodeList, long index )
{	return pNodeList->Getitem(index);
}

inline
MSXML::IXMLDOMNodePtr GetParentNode( MSXML::IXMLDOMNodePtr pNode )
{	return ( pNode )? pNode->parentNode : NULL;
}
/*
inline IXMLDOMNodePtr CXmlParsing::GetNode( TCHAR* id )
{	return m_docPtr->nodeFromID(_bstr_t(id));
}
*/
#pragma warning( default : 4786 )

///////////////////////////////////////////////////////////////////////////////
#endif //__XMLPARSING_H_
