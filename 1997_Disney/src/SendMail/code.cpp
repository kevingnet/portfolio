// code.cpp : Implementation of CSimple

#include "stdafx.h"
#include "SendMail.h"
#include "code.h"

//SDK includes
#include <winsock.h>
#include <stdio.h>         /* for sprintf                           */
#include <string.h>        /* for strlen                            */
#include <memory.h>
#include <process.h>       /* for _beginthread                      */
#include "smtp.h"


/////////////////////////////////////////////////////////////////////////////
//

// Created by the ATL 1.1 COM Wizard
STDMETHODIMP CSendMail::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_ISendMail,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}


//Ctor
CSendMail::CSendMail()
{
    m_bstrHost		=	"Enter a Host Name (DSN or TCP/IP Address)";
    m_bstrFrom		=	"Enter Name of Sender";
    m_bstrSender	=	"Enter E-Mail Address of Sender";
    m_bstrTo		=	"Enter Name of Recipient";
    m_bstrRecipient =	"Enter E-Mail Address of Recipient";
    m_bstrSubject	=	"Enter Subject of Message";
    m_bstrMessage	=	"Enter Message Body";
    m_bstrResponse	=	"This will contail the return value";
	m_iPortNumber	=	25;

} 

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Method to send E-Mail

STDMETHODIMP CSendMail::Send()  
{
	//TODO

  //SOCKET sock;


//from app

	CSMTPConnection smtp ;

	//if ( !smtp.Connect( _T ( m_host ) ) )
	if ( !smtp.Connect( "mailhost.wds.disney.com" ) )
	{
		//DWORD dwError = ::GetLastError () ;
		//CString sResponse = smtp.GetLastCommandResponse () ;
		//m_response = "Failed to connect to SMTP server" ;
		//TRACE ( _T ( "Failed to connect to SMTP server\n" ) ) ;
		return -2 ;
	}

	long lResult = smtp.SendTestMessage () ;

	m_bstrResponse = "Fucked Up!!!";

	char szTemp[10];

	m_bstrResponse = _ltoa(lResult, szTemp, 10);

	smtp.Disconnect () ;

	//m_response = "Message may have been sent" ;


  //end from app
    return S_OK;
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

STDMETHODIMP CSendMail::get_Host(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrHost.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_Host(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrHost = newVal;

	return S_OK; 
}

STDMETHODIMP CSendMail::get_From(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrFrom.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_From(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrFrom = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_Sender(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrSender.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_Sender(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrSender = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_To(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrTo.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_To(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrTo = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_Recipient(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrRecipient.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_Recipient(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrRecipient = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_Subject(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrSubject.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_Subject(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrSubject = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_Message(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrMessage.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_Message(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrMessage = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_Response(BSTR * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_bstrResponse.Copy();

	return S_OK;
}

STDMETHODIMP CSendMail::put_Response(BSTR newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_bstrResponse = newVal;

	return S_OK;
}

STDMETHODIMP CSendMail::get_PortNumber(short * pVal)
{
	// TODO: Add your implementation code here
    if (pVal == NULL)
        return E_POINTER;

    // Get Value from Property
    *pVal = m_iPortNumber;

	return S_OK;
}

STDMETHODIMP CSendMail::put_PortNumber(short newVal)
{
	// TODO: Add your implementation code here
    if (newVal == NULL)
        return E_POINTER;

    m_iPortNumber = newVal;

	return S_OK;
}




