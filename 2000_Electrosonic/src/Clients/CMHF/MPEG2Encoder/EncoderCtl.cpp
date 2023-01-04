// EncoderCtl.cpp : Implementation of the CEncoderCtrl ActiveX Control class.

#include "stdafx.h"
#include "MPEG2Encoder.h"
#include "EncoderCtl.h"
#include "EncoderPpg.h"
#include "EncoderEventsSink.h"
#include "EncoderInterface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CEncoderCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CEncoderCtrl, COleControl)
	//{{AFX_MSG_MAP(CEncoderCtrl)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CEncoderCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CEncoderCtrl)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szMPEGFileName", m_szMPEGFileName, OnszMPEGFileNameChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szDuration", m_szDuration, OnszDurationChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngVerticalRes", m_lngVerticalRes, OnVerticalResChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngHorizRes", m_lngHorizRes, OnHorizResChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngChromaFormat", m_lngChromaFormat, OnChromaFormatChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngVideoBitRate", m_lngVideoBitRate, OnVideoBitRateChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngMPEGType", m_lngMPEGType, OnMPEGTypeChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngVideoFormat", m_lngVideoFormat, OnVideoFormatChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szVideoFormat", m_szVideoFormat, OnszVideoFormatChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szMPEGType", m_szMPEGType, OnszMPEGTypeChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngAudioBitRate", m_lngAudioBitRate, OnAudioBitRateChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szChromaFormat", m_szChromaFormat, OnszChromaFormatChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngVideoInputType", m_lngVideoInputType, OnVideoInputTypeChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szVideoInputType", m_szVideoInputType, OnszVideoInputTypeChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngAudioInputType", m_lngAudioInputType, OnAudioInputTypeChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "bFTPEnable", m_bFTPEnable, OnbFTPEnableChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szFTPServerName", m_szFTPServerName, OnszFTPServerNameChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szFTPLogin", m_szFTPLogin, OnszFTPLoginChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szFTPPassword", m_szFTPPassword, OnszFTPPasswordChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szMPEGFTPFileName", m_szMPEGFTPFileName, OnszMPEGFTPFileNameChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szAudioInputType", m_szAudioInputType, OnszAudioInputTypeChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "szTapeID", m_szTapeID, OnszTapeIDChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CEncoderCtrl, "lngNumberOfTapes", m_lngNumberOfTapes, OnlngNumberOfTapesChanged, VT_I4)
	DISP_PROPERTY_EX(CEncoderCtrl, "ControlPointer", GetControlPointer, SetControlPointer, VT_I4)
	DISP_FUNCTION(CEncoderCtrl, "Start", Start, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "Pause", Pause, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "Resume", Resume, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "Stop", Stop, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "Reset", Reset, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "DoAdvancedDialog", DoAdvancedDialog, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "Cue", Cue, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEncoderCtrl, "GetState", GetState, VT_I4, VTS_NONE)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CEncoderCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CEncoderCtrl, COleControl)
	//{{AFX_EVENT_MAP(CEncoderCtrl)
	EVENT_CUSTOM("Log", FireLog, VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("Finished", FireFinished, VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("Pause", FirePause, VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("EncoderError", FireEncoderError, VTS_I4  VTS_BSTR)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CEncoderCtrl, 1)
	PROPPAGEID(EncoderPropPage::guid)
END_PROPPAGEIDS(CEncoderCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CEncoderCtrl, "MPEG2ENCODER.MPEG2EncoderCtrl.1",
	0x485c71a6, 0xbfe, 0x11d5, 0x8b, 0xe8, 0, 0, 0, 0, 0, 0)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CEncoderCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DMPEG2Encoder =
		{ 0x485c71a4, 0xbfe, 0x11d5, { 0x8b, 0xe8, 0, 0, 0, 0, 0, 0 } };
const IID BASED_CODE IID_DMPEG2EncoderEvents =
		{ 0x485c71a5, 0xbfe, 0x11d5, { 0x8b, 0xe8, 0, 0, 0, 0, 0, 0 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwMPEG2EncoderOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CEncoderCtrl, IDS_MPEG2ENCODER, _dwMPEG2EncoderOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::CEncoderCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CEncoderCtrl

BOOL CEncoderCtrl::CEncoderCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_MPEG2ENCODER,
			IDB_MPEG2ENCODER,
			afxRegInsertable | afxRegApartmentThreading,
			_dwMPEG2EncoderOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

//////////////////////////////////////////
//Loads Video settings from the registry//
////////////////////////////////////////// 
void CEncoderCtrl::InitializeSettings()
{
	CRegistry Settings ;
	CRegistry Video ;

	GetSettingFromVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPENABLE );
	GetSettingFromVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPSERVER );
	GetSettingFromVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPLOGIN );
	GetSettingFromVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPPASSWORD );

    if( Settings.Open(HKEY_CURRENT_USER, ESI_KEY ) == TRUE )
	{
		if( Video.Open(Settings.hKey(), ESI_SUBKEY ) == TRUE )
		{
			Video.GetValue( _T("Advanced Settings App"), &m_szAdvancedSettingsApp, "RegCtrlPnl.exe" );
			Video.GetValue( _T("Vela Registry Key"), &m_szVelaRegistryKey, ARGUS_KEY );
			Video.GetValue( _T("Vela Registry Video SubKey"), &m_szVelaRegistryVideoSubKey, ARGUS_VIDEOSUBKEY );
			Video.GetValue( _T("Vela Registry Audio SubKey"), &m_szVelaRegistryAudioSubKey, ARGUS_AUDIOSUBKEY );
			Video.GetValue( _T("Vela Registry Mux SubKey"), &m_szVelaRegistryMuxSubKey, ARGUS_MUXSUBKEY );
			Video.GetValue( _T("Vela Registry Remote Storage SubKey"), &m_szVelaRegistryRemoteStorageSubKey, ARGUS_REMOTESTORAGESUBKEY );

			Video.GetValue( _T("MPEG File Name"), &m_szMPEGFileName, _T("D:\\MPEGFILES\\test.mpg") );
			Video.GetValue( _T("Duration"), &m_szDuration, _T("00:00:30:01") );
			Video.GetValue( _T("VideoBitRate"), &m_lngVideoBitRate, 8000000 );
			Video.GetValue( _T("AudioBitRate"), &m_lngVideoBitRate, AUDIO_BIT_RATE_192000 );

			Video.GetValue( _T("MPEG Type"), (BYTE*)&m_lngMPEGType, (BYTE)MPEG_TYPE_MPEG2 );
			Video.GetValue( _T("VideoFormat"), (BYTE*)&m_lngVideoFormat, (BYTE)VF_NTSC );
			
			Video.GetValue( _T("MPEG 2 NTCS VideoFormat"), &m_Mode_MPEG_2_NTSC.lngVideoFormat, (BYTE)VF_NTSC );
			Video.GetValue( _T("MPEG 2 NTCS VideoMode"), &m_Mode_MPEG_2_NTSC.lngVideoMode, (BYTE)VM_AFF );
			Video.GetValue( _T("MPEG 2 NTCS ChromaFormat"), &m_Mode_MPEG_2_NTSC.lngChroma, (BYTE)CF_4_2_0 );
			Video.GetValue( _T("MPEG 2 NTCS HorizRes"), &m_Mode_MPEG_2_NTSC.lngHorizontalRes, HRES_720 );
			Video.GetValue( _T("MPEG 2 NTCS VerticalRes"), &m_Mode_MPEG_2_NTSC.lngVerticalRes, VRES_480 );
			Video.GetValue( _T("MPEG 2 NTCS MUXMPEGStandard"), &m_Mode_MPEG_2_NTSC.lngMuxStandard, (BYTE)MST_TRANSPORT_STREAM );

			Video.GetValue( _T("MPEG 2 PAL VideoFormat"), &m_Mode_MPEG_2_PAL.lngVideoFormat, (BYTE)VF_PAL );
			Video.GetValue( _T("MPEG 2 PAL VideoMode"), &m_Mode_MPEG_2_PAL.lngVideoMode, (BYTE)VM_AFF );
			Video.GetValue( _T("MPEG 2 PAL ChromaFormat"), &m_Mode_MPEG_2_PAL.lngChroma, (BYTE)CF_4_2_0 );
			Video.GetValue( _T("MPEG 2 PAL HorizRes"), &m_Mode_MPEG_2_PAL.lngHorizontalRes, HRES_720 );
			Video.GetValue( _T("MPEG 2 PAL VerticalRes"), &m_Mode_MPEG_2_PAL.lngVerticalRes, VRES_576 );
			Video.GetValue( _T("MPEG 2 PAL MUXMPEGStandard"), &m_Mode_MPEG_2_PAL.lngMuxStandard, (BYTE)MST_TRANSPORT_STREAM );
			
			Video.GetValue( _T("MPEG 1 NTCS VideoFormat"), &m_Mode_MPEG_1_NTSC.lngVideoFormat, (BYTE)VF_NTSC );
			Video.GetValue( _T("MPEG 1 NTCS VideoMode"), &m_Mode_MPEG_1_NTSC.lngVideoMode, (BYTE)VM_SIF );
			Video.GetValue( _T("MPEG 1 NTCS ChromaFormat"), &m_Mode_MPEG_1_NTSC.lngChroma, (BYTE)CF_4_2_0 );
			Video.GetValue( _T("MPEG 1 NTCS HorizRes"), &m_Mode_MPEG_1_NTSC.lngHorizontalRes, HRES_352 );
			Video.GetValue( _T("MPEG 1 NTCS VerticalRes"), &m_Mode_MPEG_1_NTSC.lngVerticalRes, VRES_240 );
			Video.GetValue( _T("MPEG 1 NTCS MUXMPEGStandard"), &m_Mode_MPEG_1_NTSC.lngMuxStandard, (BYTE)MST_SYSTEM_STREAM );
			
			Video.GetValue( _T("MPEG 1 PAL VideoFormat"), &m_Mode_MPEG_1_PAL.lngVideoFormat, (BYTE)VF_PAL );
			Video.GetValue( _T("MPEG 1 PAL VideoMode"), &m_Mode_MPEG_1_PAL.lngVideoMode, (BYTE)VM_SIF );
			Video.GetValue( _T("MPEG 1 PAL ChromaFormat"), &m_Mode_MPEG_1_PAL.lngChroma, (BYTE)CF_4_2_0 );
			Video.GetValue( _T("MPEG 1 PAL HorizRes"), &m_Mode_MPEG_1_PAL.lngHorizontalRes, HRES_352 );
			Video.GetValue( _T("MPEG 1 PAL VerticalRes"), &m_Mode_MPEG_1_PAL.lngVerticalRes, VRES_288 );
			Video.GetValue( _T("MPEG 1 PAL MUXMPEGStandard"), &m_Mode_MPEG_1_PAL.lngMuxStandard, (BYTE)MST_SYSTEM_STREAM );

			switch( m_lngMPEGType )
			{
			case MPEG_TYPE_MPEG1:
				switch( m_lngVideoFormat )
				{
				case VF_NTSC:
					m_lngChromaFormat	= m_Mode_MPEG_1_NTSC.lngChroma;
					m_lngHorizRes		= m_Mode_MPEG_1_NTSC.lngHorizontalRes;
					m_lngVerticalRes	= m_Mode_MPEG_1_NTSC.lngVerticalRes;
					m_lngVideoMode		= m_Mode_MPEG_1_NTSC.lngVideoMode;
					m_lngMuxStandard	= m_Mode_MPEG_1_NTSC.lngMuxStandard;
					break;
				case VF_PAL:
					m_lngChromaFormat	= m_Mode_MPEG_1_PAL.lngChroma;
					m_lngHorizRes		= m_Mode_MPEG_1_PAL.lngHorizontalRes;
					m_lngVerticalRes	= m_Mode_MPEG_1_PAL.lngVerticalRes;
					m_lngVideoMode		= m_Mode_MPEG_1_PAL.lngVideoMode;
					m_lngMuxStandard	= m_Mode_MPEG_1_PAL.lngMuxStandard;
					break;
				}
				break;
			case MPEG_TYPE_MPEG2:
				switch( m_lngVideoFormat )
				{
				case VF_NTSC:
					m_lngChromaFormat	= m_Mode_MPEG_2_NTSC.lngChroma;
					m_lngHorizRes		= m_Mode_MPEG_2_NTSC.lngHorizontalRes;
					m_lngVerticalRes	= m_Mode_MPEG_2_NTSC.lngVerticalRes;
					m_lngVideoMode		= m_Mode_MPEG_2_NTSC.lngVideoMode;
					m_lngMuxStandard	= m_Mode_MPEG_2_NTSC.lngMuxStandard;
					break;
				case VF_PAL:
					m_lngChromaFormat	= m_Mode_MPEG_2_PAL.lngChroma;
					m_lngHorizRes		= m_Mode_MPEG_2_PAL.lngHorizontalRes;
					m_lngVerticalRes	= m_Mode_MPEG_2_PAL.lngVerticalRes;
					m_lngVideoMode		= m_Mode_MPEG_2_PAL.lngVideoMode;
					m_lngMuxStandard	= m_Mode_MPEG_2_PAL.lngMuxStandard;
					break;
				}
				break;
			}

			Video.Close();
		}
		Settings.Close();
	}
	ValidateSettings();
}//InitializeVideoSettings

////////////////////////////////////////
//Saves Video settings to the registry//
////////////////////////////////////////
void CEncoderCtrl::SaveSettingToVela( int iVelaKey, int iVelaValue )
{
	CRegistry Settings;
	CRegistry Key;

	if( Settings.Open(HKEY_CURRENT_USER, m_szVelaRegistryKey ) != TRUE )
	{
		FireEncoderError( 0, "Warning: Unable to save registry key to vela" );
		return;
	}
	CString szKey;
	switch( iVelaKey )
	{
	case VELA_KEY_VIDEO:
		szKey = m_szVelaRegistryVideoSubKey;
		break;
	case VELA_KEY_AUDIO:
		szKey = m_szVelaRegistryAudioSubKey;
		break;
	case VELA_KEY_MUX:
		szKey = m_szVelaRegistryMuxSubKey;
		break;
	case VELA_KEY_REMOTESTORE:
		szKey = m_szVelaRegistryRemoteStorageSubKey;
		break;
	}

	if( Key.Open(Settings.hKey(), szKey ) != TRUE )
	{
		FireEncoderError( 0, "Warning: Unable to save registry key to vela" );
		Settings.Close();
		return;
	}

	switch( iVelaValue )
	{
	case VELA_VALUE_VIDEOFORMAT:
		Key.SetValue( _T("VideoFormat"), (BYTE*)&m_lngVideoFormat );
		break;
	case VELA_VALUE_VIDEOMODE:
		Key.SetValue( _T("VideoMode"), (BYTE*)&m_lngVideoMode );
		break;
	case VELA_VALUE_VIDEOINPUTTYPE:
		Key.SetValue( _T("InputType"), (BYTE*)&m_lngVideoInputType );
		break;
	case VELA_VALUE_VIDEOBITRATE:
		Key.SetValue( _T("BitRate") , &m_lngVideoBitRate );
		break;
	case VELA_VALUE_CHROMAFORMAT:
		Key.SetValue( _T("ChromaFormat"), (BYTE*)&m_lngChromaFormat );
		break;
	case VELA_VALUE_HORZRES:
		Key.SetValue( _T("HorizRes"), &m_lngHorizRes );
		break;
	case VELA_VALUE_VERTRES:
		Key.SetValue( _T("VerticalRes"), &m_lngVerticalRes );
		break;
	case VELA_VALUE_AUDIOBITRATE:
		Key.SetValue( _T("BitRate0") , &m_lngAudioBitRate );
		Key.SetValue( _T("BitRate1") , &m_lngAudioBitRate );
		break;
	case VELA_VALUE_AUDIOINPUTTYPE:
		Key.SetValue(  "Input0" , (BYTE*)&m_lngAudioInputType );// 0-Analog, 1-Digital, 2-Inactive
		Key.SetValue(  "Input1" , (BYTE*)&m_lngAudioInputType );// 0-Analog, 1-Digital, 2-Inactive
		break;
	case VELA_VALUE_MUXSTANDARD:
		Key.SetValue( _T("MPEGStd"), &m_lngMuxStandard );
		break;
	case VELA_VALUE_MUXBITRATE:
		Key.SetValue( _T("MuxRate"), &m_lngMuxBitRate );
		break;
	case VELA_VALUE_FTPENABLE:
		Key.SetValue( _T("FtpEnable") , &m_bFTPEnable );
		break;
	case VELA_VALUE_FTPSERVER:
		Key.SetValue( _T("ServerName"), m_szFTPServerName );
		break;
	case VELA_VALUE_FTPLOGIN:
		Key.SetValue( _T("UserName"), m_szFTPLogin );
		break;
	case VELA_VALUE_FTPPASSWORD:
		Key.SetValue( _T("UserPass"), m_szFTPPassword );
		break;
	case VELA_VALUE_FTPFILENAME:
		Key.SetValue( _T("RemoteFilename"), m_szMPEGFTPFileName );
		break;
	}

	Key.Close();
	Settings.Close();
}//SaveVideoSettings

void CEncoderCtrl::GetSettingFromVela( int iVelaKey, int iVelaValue )
{
	CRegistry Settings;
	CRegistry Key;

	if( Settings.Open(HKEY_CURRENT_USER, m_szVelaRegistryKey ) != TRUE )
	{
		FireEncoderError( 0, "Warning: Unable to save registry key to vela" );
		return;
	}
	CString szKey;
	switch( iVelaKey )
	{
	case VELA_KEY_VIDEO:
		szKey = m_szVelaRegistryVideoSubKey;
		break;
	case VELA_KEY_AUDIO:
		szKey = m_szVelaRegistryAudioSubKey;
		break;
	case VELA_KEY_MUX:
		szKey = m_szVelaRegistryMuxSubKey;
		break;
	case VELA_KEY_REMOTESTORE:
		szKey = m_szVelaRegistryRemoteStorageSubKey;
		break;
	}

	if( Key.Open(Settings.hKey(), szKey ) != TRUE )
	{
		FireEncoderError( 0, "Warning: Unable to save registry key to vela" );
		Settings.Close();
		return;
	}

	switch( iVelaValue )
	{
	case VELA_VALUE_VIDEOFORMAT:
		Key.GetValue( _T("VideoFormat"), &m_lngVideoFormat, (BYTE)m_lngVideoFormat );
		break;
	case VELA_VALUE_VIDEOMODE:
		Key.GetValue( _T("VideoMode"), &m_lngVideoMode, (BYTE)m_lngVideoMode );
		break;
	case VELA_VALUE_VIDEOINPUTTYPE:
		Key.GetValue( _T("InputType"), &m_lngVideoInputType, (BYTE)m_lngVideoInputType );
		break;
	case VELA_VALUE_VIDEOBITRATE:
		Key.GetValue( _T("BitRate"), &m_lngVideoBitRate, m_lngVideoBitRate );
		break;
	case VELA_VALUE_CHROMAFORMAT:
		Key.GetValue( _T("ChromaFormat"), &m_lngChromaFormat, (BYTE)m_lngChromaFormat );
		break;
	case VELA_VALUE_HORZRES:
		Key.GetValue( _T("HorizRes"), &m_lngHorizRes, m_lngHorizRes );
		break;
	case VELA_VALUE_VERTRES:
		Key.GetValue( _T("VerticalRes"), &m_lngVerticalRes, m_lngVerticalRes );
		break;
	case VELA_VALUE_AUDIOBITRATE:
		Key.GetValue( _T("BitRate0"), &m_lngAudioBitRate, m_lngAudioBitRate );
		Key.GetValue( _T("BitRate1"), &m_lngAudioBitRate, m_lngAudioBitRate );
		break;
	case VELA_VALUE_AUDIOINPUTTYPE:
		Key.GetValue( _T("Input0"), &m_lngAudioInputType, m_lngAudioInputType );
		Key.GetValue( _T("Input1"), &m_lngAudioInputType, m_lngAudioInputType );
		break;
	case VELA_VALUE_MUXSTANDARD:
		Key.GetValue( _T("MPEGStd"), &m_lngMuxStandard, m_lngMuxStandard );
		break;
	case VELA_VALUE_MUXBITRATE:
		Key.GetValue( _T("MuxRate"), &m_lngMuxBitRate, m_lngMuxBitRate );
		break;
	case VELA_VALUE_FTPENABLE:
		Key.GetValue( _T("FtpEnable"), &m_bFTPEnable, m_bFTPEnable );
		break;
	case VELA_VALUE_FTPSERVER:
		Key.GetValue( _T("ServerName"), &m_szFTPServerName, "" );
		break;
	case VELA_VALUE_FTPLOGIN:
		Key.GetValue( _T("UserName"), &m_szFTPLogin, "" );
		break;
	case VELA_VALUE_FTPPASSWORD:
		Key.GetValue( _T("UserPass"), &m_szFTPPassword, "" );
		break;
	case VELA_VALUE_FTPFILENAME:
		Key.GetValue( _T("RemoteFilename"), &m_szMPEGFTPFileName, "" );
		break;
	}
	Key.Close();
	Settings.Close();
}

void CEncoderCtrl::SaveSetting( int iControlValue )
{
	CRegistry Settings ;
	CRegistry Video ;
	if( Settings.Open(HKEY_CURRENT_USER, ESI_KEY ) == TRUE )
	{
		if( Video.Open(Settings.hKey(), ESI_SUBKEY ) == TRUE )
		{
			switch( iControlValue )
			{
			case CONTROL_VALUE_ALL:
				Video.SetValue( _T("MPEG File Name"), m_szMPEGFileName );
				Video.SetValue( _T("Duration"), m_szDuration );
				Video.SetValue( _T("VideoFormat"), &m_lngVideoFormat );
				Video.SetValue( _T("MPEG Type"), &m_lngMPEGType );
				Video.SetValue( _T("VideoBitRate") , &m_lngVideoBitRate );
				Video.SetValue( _T("AudioBitRate") , &m_lngAudioBitRate );
				break;
			case CONTROL_VALUE_MPEGFILE:
				Video.SetValue( _T("MPEG File Name"), m_szMPEGFileName );
				break;
			case CONTROL_VALUE_DURATION:
				Video.SetValue( _T("Duration"), m_szDuration );
				break;
			case CONTROL_VALUE_VIDEOFORMAT:
				Video.SetValue( _T("VideoFormat"), &m_lngVideoFormat );
				break;
			case CONTROL_VALUE_MPEGTYPE:
				Video.SetValue( _T("MPEG Type"), &m_lngMPEGType );
				break;
			case CONTROL_VALUE_VIDEOBITRATE:
				Video.SetValue( _T("VideoBitRate") , &m_lngVideoBitRate );
				break;
			case CONTROL_VALUE_AUDIOBITRATE:
				Video.SetValue( _T("AudioBitRate") , &m_lngAudioBitRate );
				break;
			}
			Video.Close();
		}
		Settings.Close();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::CEncoderCtrl - Constructor

void CEncoderCtrl::SetDefaults()
{
	m_szAdvancedSettingsApp					= "RegCtrlPnl.exe";
	m_szVelaRegistryKey						= ARGUS_KEY;
	m_szVelaRegistryVideoSubKey				= ARGUS_VIDEOSUBKEY;
	m_szVelaRegistryAudioSubKey				= ARGUS_AUDIOSUBKEY;
	m_szVelaRegistryMuxSubKey				= ARGUS_MUXSUBKEY;
	m_szVelaRegistryRemoteStorageSubKey		= ARGUS_REMOTESTORAGESUBKEY;

	m_szMPEGFileName			= _T("D:\\MPEGFILES\\test.mpg");
	m_szDuration				= _T("02:10:30:01"); //30 seconds 1 frame

	m_szTapeID					= _T("1");
	m_lngNumberOfTapes			= 1;

	m_lngVideoBitRate			= VIDEO_420_DEFAULT_BITRATE; //8 mbps
	m_lngVideoInputType			= VIS_DIGITAL;
	m_szVideoInputType			= "Digital";
	m_lngMPEGType				= MPEG_TYPE_MPEG2;
	m_szMPEGType				= _T("MPEG 2");
	m_lngVideoFormat			= VF_NTSC;
	m_szVideoFormat				= _T("NTSC");
	m_lngChromaFormat			= CF_4_2_0;
	m_szChromaFormat			= _T("4:2:0");
	m_lngVideoMode				= VM_AFF;
	m_lngHorizRes				= HRES_720;
	m_lngVerticalRes			= VRES_480;

	m_lngAudioBitRate			= AUDIO_BIT_RATE_192000;
	m_lngAudioInputType			= AI_ANALOG;
	m_szAudioInputType			= _T("Analog");

	m_lngMuxBitRate				= m_lngVideoBitRate;
	m_lngMuxStandard			= MST_TRANSPORT_STREAM;


	m_bFTPEnable				= FALSE;
//	m_szFTPServerName			= ;
//	m_szFTPLogin				= ;
//	m_szFTPPassword				= ;
//	m_szMPEGFTPFileName			= ;
}

CEncoderCtrl::CEncoderCtrl()
{
	InitializeIIDs(&IID_DMPEG2Encoder, &IID_DMPEG2EncoderEvents);

	// TODO: Initialize your control's instance data here.
	m_pFM				= NULL;
	m_pEncoderInterface = NULL;

	m_bCreated		= FALSE;
	m_bInitialized	= FALSE;

	SetDefaults();

	InitializeSettings();
	ValidateSettings();
}


/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::~CEncoderCtrl - Destructor

CEncoderCtrl::~CEncoderCtrl()
{
	if( !this ) return;
	CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::OnResetState - Reset control to default state

void CEncoderCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
	SetDefaults();
	InitializeSettings();
	ValidateSettings();
}

//control properties

#define DURATION_LENGTH 11
void CEncoderCtrl::OnszDurationChanged() 
{
	//check for valid duration
	bool bDurationIsValid = false;
	char cFirst=0, cSecond=0;
	//"00:00:30:01"
	m_szDuration.TrimLeft();
	m_szDuration.TrimRight();
	if( m_szDuration.GetLength() == DURATION_LENGTH )
	{
		cFirst = m_szDuration[0];
		if( cFirst != '0' )
		{
			FireEncoderError( ERROR_INVALID_DATA, "Invalid parameter: szDuration. Too many hours. This encoder version does not support recording sessions that are longer than 9 hours" );
		}
		else
		{
			cSecond = m_szDuration[1];
			if( isdigit(m_szDuration[0]) )
				if( isdigit(m_szDuration[1]) )
					if( ':' == m_szDuration[2] )
						if( isdigit(m_szDuration[3]) )
							if( isdigit(m_szDuration[4]) )
								if( ':' == m_szDuration[5] )
									if( isdigit(m_szDuration[6]) )
										if( isdigit(m_szDuration[7]) )
											if( ':' == m_szDuration[8] )
												if( isdigit(m_szDuration[9]) )
													if( isdigit(m_szDuration[10]) )
														bDurationIsValid = true;
		}
	}
	
	if( bDurationIsValid == true )
	{
		if( cSecond == '0' )
		{
			FireEncoderError( 0, "Warning: szDuration appears to be too short (less than an hour)" );
		}
		SaveSetting( CONTROL_VALUE_DURATION );
		SetModifiedFlag();
	}
	else
	{
		FireEncoderError( ERROR_INVALID_DATA, "Invalid parameter: szDuration. Setting to default." );
		//set to default
		m_szDuration = _T("02:10:30:01");
	}
}

void CEncoderCtrl::OnszMPEGFileNameChanged() 
{
	//check for valid file
	bool bFileNameIsValid = false;
	DWORD dwDummy = GetFileAttributes( m_szMPEGFileName );
	if( -1 == dwDummy )
	{
		HANDLE hFile =  CreateFile( m_szMPEGFileName, GENERIC_WRITE, 0, NULL, 
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			//could not create file, assume invalid
		}
		else
		{
			CloseHandle( hFile );
			bFileNameIsValid = true;
		}
	}
	else
	{
		bFileNameIsValid = true;
	}
	if( bFileNameIsValid == true )
	{
		SaveSetting( CONTROL_VALUE_MPEGFILE );
		SetModifiedFlag();
	}
	else
	{
		FireEncoderError( ERROR_INVALID_DATA, "Invalid file name" );
		//setting to default
		m_szMPEGFileName = _T("D:\\MPEGFILES\\test.mpg");;
	}
}

//FTP properties

void CEncoderCtrl::OnszMPEGFTPFileNameChanged() 
{
	SaveSettingToVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPFILENAME );
	SetModifiedFlag();
}

void CEncoderCtrl::OnszFTPServerNameChanged() 
{
	SaveSettingToVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPSERVER );
	SetModifiedFlag();
}

void CEncoderCtrl::OnszFTPLoginChanged() 
{
	SaveSettingToVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPLOGIN );
	SetModifiedFlag();
}

void CEncoderCtrl::OnszFTPPasswordChanged() 
{
	SaveSettingToVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPPASSWORD );
	SetModifiedFlag();
}

void CEncoderCtrl::OnbFTPEnableChanged() 
{
	SaveSettingToVela( VELA_KEY_REMOTESTORE, VELA_VALUE_FTPENABLE );
	SetModifiedFlag();
}
	
//audio properties

void CEncoderCtrl::OnAudioBitRateChanged() 
{
	//look up in table

	long lngCurrentRate = 0;

	if( m_lngAudioBitRate >= AUDIO_BIT_RATE_384000 )
		lngCurrentRate = AUDIO_BIT_RATE_384000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_320000 )
		lngCurrentRate = AUDIO_BIT_RATE_320000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_256000 )
		lngCurrentRate = AUDIO_BIT_RATE_256000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_224000 )
		lngCurrentRate = AUDIO_BIT_RATE_224000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_192000 )
		lngCurrentRate = AUDIO_BIT_RATE_192000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_160000 )
		lngCurrentRate = AUDIO_BIT_RATE_160000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_128000 )
		lngCurrentRate = AUDIO_BIT_RATE_128000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_112000 )
		lngCurrentRate = AUDIO_BIT_RATE_112000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_96000 )
		lngCurrentRate = AUDIO_BIT_RATE_96000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_80000 )
		lngCurrentRate = AUDIO_BIT_RATE_80000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_64000 )
		lngCurrentRate = AUDIO_BIT_RATE_64000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_56000 )
		lngCurrentRate = AUDIO_BIT_RATE_56000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_48000 )
		lngCurrentRate = AUDIO_BIT_RATE_48000;
	else if( m_lngAudioBitRate >= AUDIO_BIT_RATE_32000 )
		lngCurrentRate = AUDIO_BIT_RATE_32000;
	else if( lngCurrentRate <= AUDIO_BIT_RATE_32000 )
		lngCurrentRate = AUDIO_BIT_RATE_32000;

	m_lngAudioBitRate = lngCurrentRate;

	SaveSetting( CONTROL_VALUE_AUDIOBITRATE );
	SaveSettingToVela( VELA_KEY_AUDIO, VELA_VALUE_AUDIOBITRATE );
	ValidateBitRateSettings();
	SaveSettingToVela( VELA_KEY_MUX, VELA_VALUE_MUXBITRATE );
	SetModifiedFlag();
}

void CEncoderCtrl::OnAudioInputTypeChanged() 
{
	switch( m_lngAudioInputType )
	{
	case AI_ANALOG:
		m_szAudioInputType = _T("Analog");
		break;
	case AI_DIGITAL:
		m_szAudioInputType = _T("Digital");
		break;
	case AI_INACTIVE:
		m_szAudioInputType = _T("Inactive");
		break;
	default:
		FireLog( 0, "Warning: Invalid Audio Input Type. Disabling Audio." );
		m_lngAudioInputType = AI_INACTIVE;
		m_szAudioInputType = _T("Inactive");
		break;
	}
	SaveSettingToVela( VELA_KEY_AUDIO, VELA_VALUE_AUDIOINPUTTYPE );
	SetModifiedFlag();
}

//video properties

void CEncoderCtrl::OnMPEGTypeChanged()
{
	switch( m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
	case MPEG_TYPE_MPEG2:
		break;
	default:
		m_lngMPEGType = MPEG_TYPE_MPEG2;
		FireLog( 0, "Warning: Invalid MPEG Type. Setting to default MPEG 2" );
		break;
	}
	SaveSetting( CONTROL_VALUE_MPEGTYPE );
	OnVideoFormatChanged(); //continues below...
}

void CEncoderCtrl::OnVideoFormatChanged()
{
	switch( m_lngVideoFormat )
	{
	case VF_NTSC:
	case VF_PAL:
		break;
	default:
		FireLog( 0, "Warning: Invalid Video Format. Setting to default NTSC" );
		m_lngVideoFormat = VF_NTSC;
		break;
	}
	switch( m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
		switch( m_lngVideoFormat )
		{
		case VF_NTSC:
			m_lngChromaFormat			= m_Mode_MPEG_1_NTSC.lngChroma;
			m_lngVideoMode				= m_Mode_MPEG_1_NTSC.lngVideoMode;
			m_lngHorizRes				= m_Mode_MPEG_1_NTSC.lngHorizontalRes;
			m_lngVerticalRes			= m_Mode_MPEG_1_NTSC.lngVerticalRes;
			break;
		case VF_PAL:
			m_lngChromaFormat			= m_Mode_MPEG_1_PAL.lngChroma;
			m_lngVideoMode				= m_Mode_MPEG_1_PAL.lngVideoMode;
			m_lngHorizRes				= m_Mode_MPEG_1_PAL.lngHorizontalRes;
			m_lngVerticalRes			= m_Mode_MPEG_1_PAL.lngVerticalRes;
			break;
		}
		break;
	case MPEG_TYPE_MPEG2:
		switch( m_lngVideoFormat )
		{
		case VF_NTSC:
			m_lngChromaFormat			= m_Mode_MPEG_2_NTSC.lngChroma;
			m_lngVideoMode				= m_Mode_MPEG_2_NTSC.lngVideoMode;
			m_lngHorizRes				= m_Mode_MPEG_2_NTSC.lngHorizontalRes;
			m_lngVerticalRes			= m_Mode_MPEG_2_NTSC.lngVerticalRes;
			break;
		case VF_PAL:
			m_lngChromaFormat			= m_Mode_MPEG_2_PAL.lngChroma;
			m_lngVideoMode				= m_Mode_MPEG_2_PAL.lngVideoMode;
			m_lngHorizRes				= m_Mode_MPEG_2_PAL.lngHorizontalRes;
			m_lngVerticalRes			= m_Mode_MPEG_2_PAL.lngVerticalRes;
			break;
		}
		break;
	}
	ValidateSettings();
	SaveSetting( CONTROL_VALUE_VIDEOFORMAT );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VIDEOFORMAT );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VIDEOMODE );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_CHROMAFORMAT );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_HORZRES );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VERTRES );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VIDEOBITRATE );
	SaveSettingToVela( VELA_KEY_MUX, VELA_VALUE_MUXSTANDARD );
	SaveSettingToVela( VELA_KEY_MUX, VELA_VALUE_MUXBITRATE );
	SetModifiedFlag();
}

void CEncoderCtrl::OnVideoInputTypeChanged() 
{
	switch( m_lngVideoInputType )
	{
	case VIS_DIGITAL:
		m_szVideoInputType = _T("Digital");
		break;
	case VIS_COMPOSITE:
		m_szVideoInputType = _T("Composite");
		break;
	default:
		FireLog( 0, "Warning: Invalid Input Type. Setting to default Digital" );
		m_lngVideoInputType = VIS_DIGITAL;
		m_szVideoInputType = _T("Digital");
		break;
	}
	ValidateSettings();
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VIDEOINPUTTYPE );
	SetModifiedFlag();
}

void CEncoderCtrl::OnVideoBitRateChanged() 
{
	ValidateBitRateSettings();
	SaveSetting( CONTROL_VALUE_VIDEOBITRATE );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VIDEOBITRATE );
	SaveSettingToVela( VELA_KEY_MUX, VELA_VALUE_MUXBITRATE );
	SetModifiedFlag();
}

void CEncoderCtrl::OnChromaFormatChanged()
{
	switch( m_lngChromaFormat )
	{
	case CF_4_2_0:
		break;
	case CF_4_2_2:
		switch( m_lngMPEGType )
		{
		case MPEG_TYPE_MPEG1:
			FireLog( 0, "Warning: Invalid Chroma Format. MPEG 2 only supports 4:2:0. Setting to 4:2:0. If you really want to use 4:2:2 change to MPEG 2. Setting to 4:2:0" );
			m_lngChromaFormat = CF_4_2_0;
			break;
		case MPEG_TYPE_MPEG2:
			break;
		}
		break;
	default:
		FireLog( 0, "Warning: Invalid Chroma Format. Setting to default of 4:2:0" );
		m_lngChromaFormat = CF_4_2_0;
		break;
	}
	ValidateSettings();
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_CHROMAFORMAT );
	SetModifiedFlag();
}

void CEncoderCtrl::OnHorizResChanged() 
{
	ValidateResolutionSettings();
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_HORZRES );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VERTRES );
	SetModifiedFlag();
}

void CEncoderCtrl::OnVerticalResChanged() 
{
	ValidateResolutionSettings();
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_HORZRES );
	SaveSettingToVela( VELA_KEY_VIDEO, VELA_VALUE_VERTRES );
	SetModifiedFlag();
}

void CEncoderCtrl::ValidateSettings()
{
	switch( m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
		m_szMPEGType = _T("MPEG 1");
		m_szChromaFormat = _T("4:2:0");
		if( m_lngChromaFormat != CF_4_2_0 )
		{
			FireLog( 0, "Warning: Invalid Chroma Format for MPEG1. Setting to 4:2:0" );
			m_lngChromaFormat = CF_4_2_0;
		}
		switch( m_lngVideoFormat )
		{
		case VF_NTSC:
			m_szVideoFormat	= _T("NTSC");
			m_lngVideoMode = m_Mode_MPEG_1_NTSC.lngVideoMode;
			m_lngMuxStandard = m_Mode_MPEG_1_NTSC.lngMuxStandard;
			break;
		case VF_PAL:
			m_szVideoFormat	= _T("PAL");
			m_lngVideoMode = m_Mode_MPEG_1_PAL.lngVideoMode;
			m_lngMuxStandard = m_Mode_MPEG_1_PAL.lngMuxStandard;
			break;
		}
		switch( m_lngVideoMode )
		{
		case VM_SIF:
			break;
		case VM_AFF:
		default:
			FireLog( 0, "Warning: Invalid Video Mode for MPEG 1. Setting to default SIF" );
			m_lngVideoMode = VM_SIF;
			break;
		}
		switch( m_lngMuxStandard )
		{
		case MST_SYSTEM_STREAM:
			break;
		case MST_PROGRAM_STREAM:
		case MST_TRANSPORT_STREAM:
		default:
			FireLog( 0, "Warning: Invalid MUX Standard for MPEG 1. Setting to default System" );
			m_lngMuxStandard = MST_SYSTEM_STREAM;
			break;
		}
		break;
	case MPEG_TYPE_MPEG2:
		m_szMPEGType = _T("MPEG 2");
		switch( m_lngVideoFormat )
		{
		case VF_NTSC:
			m_szVideoFormat	= _T("NTSC");
			m_lngVideoMode = m_Mode_MPEG_2_NTSC.lngVideoMode;
			m_lngMuxStandard = m_Mode_MPEG_2_NTSC.lngMuxStandard;
			break;
		case VF_PAL:
			m_szVideoFormat	= _T("PAL");
			m_lngVideoMode = m_Mode_MPEG_2_PAL.lngVideoMode;
			m_lngMuxStandard = m_Mode_MPEG_2_PAL.lngMuxStandard;
			break;
		}
		switch( m_lngVideoMode )
		{
		case VM_AFF:
			break;
		case VM_SIF:
		default:
			FireLog( 0, "Warning: Invalid Video Mode for MPEG 2. Setting to default AFF" );
			m_lngVideoMode = VM_AFF;
			break;
		}
		switch( m_lngMuxStandard )
		{
		case MST_PROGRAM_STREAM:
		case MST_TRANSPORT_STREAM:
			break;
		case MST_SYSTEM_STREAM:
		default:
			FireLog( 0, "Warning: Invalid MUX Standard for MPEG 2. Setting to default Transport" );
			m_lngMuxStandard = MST_TRANSPORT_STREAM;
			break;
		}
		switch( m_lngChromaFormat )
		{
		case CF_4_2_0:
		case CF_4_2_2:
			break;
		case CF_4_4_4:
		default:
			FireLog( 0, "Warning: Invalid Chroma Format for MPEG 2. Setting to default 4:2:0" );
			m_lngChromaFormat = CF_4_2_0;
			break;
		}
		break;
	}
	ValidateResolutionSettings();
	ValidateBitRateSettings();
}

void CEncoderCtrl::ValidateBitRateSettings()
{
	//check if the new video bit rate is valid based on the MPEG type and Chroma format
	switch( m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
		if( m_lngVideoBitRate > VIDEO_MPEG1_MAX_BITRATE )
		{
			FireLog( 0, "Warning: Invalid Video Bit Rate for MPEG1. Exceeds maximum allowed. Setting to maximum of 3,000,000" );
			m_lngVideoBitRate = VIDEO_MPEG1_MAX_BITRATE;
		}
		else if( m_lngVideoBitRate < VIDEO_420_MIN_BITRATE )
		{
			FireLog( 0, "Warning: Invalid Video Bit Rate for MPEG1. Below minimum allowed. Setting to minimum of 1,700,000" );
			m_lngVideoBitRate = VIDEO_420_MIN_BITRATE;
		}
		break;
	case MPEG_TYPE_MPEG2:
		switch( m_lngChromaFormat )
		{
		case CF_4_2_0:
			if( m_lngVideoBitRate > VIDEO_420_MAX_BITRATE )
			{
				FireLog( 0, "Warning: Invalid Video Bit Rate for MPEG2 Chroma 4:2:0. Exceeds maximum allowed. Setting to default of 8,000,000" );
				m_lngVideoBitRate = VIDEO_420_DEFAULT_BITRATE;
			}
			else if( m_lngVideoBitRate < VIDEO_420_MIN_BITRATE )
			{
				FireLog( 0, "Warning: Invalid Video Bit Rate for MPEG2 Chroma 4:2:0. Below minimum allowed. Setting to default of 8,000,000" );
				m_lngVideoBitRate = VIDEO_420_DEFAULT_BITRATE;
			}
			break;
		case CF_4_2_2:
			if( m_lngVideoBitRate > VIDEO_422_MAX_BITRATE )
			{
				FireLog( 0, "Warning: Invalid Video Bit Rate for MPEG2 Chroma 4:2:2. Exceeds maximum allowed. Setting to default of 8,000,000" );
				m_lngVideoBitRate = VIDEO_422_DEFAULT_BITRATE;
			}
			else if( m_lngVideoBitRate < VIDEO_422_MIN_BITRATE )
			{
				FireLog( 0, "Warning: Invalid Video Bit Rate for MPEG2 Chroma 4:2:2. Below minimum allowed. Setting to default of 8,000,000" );
				m_lngVideoBitRate = VIDEO_422_DEFAULT_BITRATE;
			}
			break;
		}
		break;
	}
	//Mux (multiplex) Bit Rate should be Video + Audio Bit Rates
	m_lngMuxBitRate = m_lngVideoBitRate + m_lngAudioBitRate;
	//, but not to exceed the parameters
	switch( m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
		if( m_lngMuxBitRate > VIDEO_MPEG1_MAX_BITRATE )
		{
			m_lngMuxBitRate = VIDEO_MPEG1_MAX_BITRATE;
		}
		else if( m_lngMuxBitRate < VIDEO_420_MIN_BITRATE )
		{
			m_lngMuxBitRate = VIDEO_420_MIN_BITRATE;
		}
		break;
	case MPEG_TYPE_MPEG2:
		switch( m_lngChromaFormat )
		{
		case CF_4_2_0:
			if( m_lngMuxBitRate > VIDEO_420_MAX_BITRATE )
			{
				m_lngMuxBitRate = VIDEO_420_DEFAULT_BITRATE;
			}
			else if( m_lngMuxBitRate < VIDEO_420_MIN_BITRATE )
			{
				m_lngMuxBitRate = VIDEO_420_DEFAULT_BITRATE;
			}
			break;
		case CF_4_2_2:
			if( m_lngMuxBitRate > VIDEO_422_MAX_BITRATE )
			{
				m_lngMuxBitRate = VIDEO_422_DEFAULT_BITRATE;
			}
			else if( m_lngMuxBitRate < VIDEO_422_MIN_BITRATE )
			{
				m_lngMuxBitRate = VIDEO_422_DEFAULT_BITRATE;
			}
			break;
		}
		break;
	}
}

void CEncoderCtrl::ValidateResolutionSettings()
{
	switch( m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
		if( m_lngHorizRes != HRES_352 )
		{
			FireLog( 0, "Warning: Invalid Horizontal Resolution for MPEG1. Setting to 352" );
			m_lngHorizRes = HRES_352;
		}
		switch( m_lngVideoFormat )
		{
		case VF_NTSC:
			if( m_lngVerticalRes != VRES_240 )
			{
				FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG1 NTSC. Setting to 240" );
				m_lngVerticalRes = VRES_240;
			}
			break;
		case VF_PAL:
			if( m_lngVerticalRes != VRES_288 )
			{
				FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG1 PAL. Setting to 288" );
				m_lngVerticalRes = VRES_288;
			}
			break;
		}
		break;
	case MPEG_TYPE_MPEG2:
		switch( m_lngChromaFormat )
		{
		case CF_4_2_2:
			if( m_lngHorizRes != HRES_720 )
			{
				FireLog( 0, "Warning: Invalid Horizontal Resolution for MPEG2 Chroma 4:2:2. Setting to default 720" );
				m_lngHorizRes = HRES_720;
			}
			switch( m_lngVideoFormat )
			{
			case VF_NTSC:
				if( m_lngVerticalRes != VRES_512 )
				{
					FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:2 NTSC. Setting to 512" );
					m_lngVerticalRes = VRES_512;
				}
				break;
			case VF_PAL:
				if( m_lngVerticalRes != VRES_608 )
				{
					FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:2 PAL. Setting to 608" );
					m_lngVerticalRes = VRES_608;
				}
				break;
			}
			break;
		case CF_4_2_0:
			switch( m_lngHorizRes )
			{
			case HRES_352:
			case HRES_704:
			case HRES_720:
				break;
			default:
				FireLog( 0, "Warning: Invalid Horizontal Resolution for MPEG2 Chroma 4:2:0. Setting to default 720" );
				m_lngHorizRes = HRES_720;
				break;
			}
			switch( m_lngVideoFormat )
			{
			case VF_NTSC:
				switch( m_lngHorizRes )
				{
				case HRES_352:
				case HRES_704:
					if( m_lngVerticalRes != VRES_480 )
					{
						FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:0 NTSC. Setting to 480" );
						m_lngVerticalRes = VRES_480;
					}
					break;
				case HRES_720:
					switch( m_lngVerticalRes )
					{
					case VRES_480:
					case VRES_512:
						break;
					default:
						FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:0 NTSC. Setting to 480" );
						m_lngVerticalRes = VRES_480;
						break;
					}
					break;
				}
				break;
			case VF_PAL:
				switch( m_lngHorizRes )
				{
				case HRES_352:
					if( m_lngVerticalRes != VRES_288 )
					{
						FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:0 PAL. Setting to 288" );
						m_lngVerticalRes = VRES_288;
					}
					break;
				case HRES_704:
					if( m_lngVerticalRes != VRES_576 )
					{
						FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:0 PAL. Setting to 576" );
						m_lngVerticalRes = VRES_576;
					}
					break;
				case HRES_720:
					switch( m_lngVerticalRes )
					{
					case VRES_576:
					case VRES_608:
						break;
					default:
						FireLog( 0, "Warning: Invalid Vertical Resolution for MPEG2 Chroma 4:2:0 PAL. Setting to 576" );
						m_lngVerticalRes = VRES_576;
						break;
					}
					break;
				}
				break;
			}
			break;
		}
		break;
	}
}

//string based properties, that invoke their long counter parts

void CEncoderCtrl::OnszVideoFormatChanged() 
{
	switch( m_szVideoFormat[0] )
	{
	case 'p':
	case 'P':
		m_lngVideoFormat = m_Mode_MPEG_2_PAL.lngVideoFormat;
		break;
	case 'n':
	case 'N':
		m_lngVideoFormat = m_Mode_MPEG_2_NTSC.lngVideoFormat;
		break;
	default:
		FireLog( 0, "Warning: Invalid Video Format" );
		break;
	}
	OnVideoFormatChanged();
}

void CEncoderCtrl::OnszMPEGTypeChanged() 
{
	switch( atoi( m_szMPEGType.SpanIncluding( "0123456789" ) ) )
	{
	case 1:
		m_lngMPEGType = MPEG_TYPE_MPEG1;
		break;
	case 2:
		m_lngMPEGType = MPEG_TYPE_MPEG2;
		break;
	default:
		m_lngMPEGType = MPEG_TYPE_MPEG2;
		FireLog( 0, "Warning: Invalid MPEG Type. Setting to default MPEG2" );
		break;
	}
	OnMPEGTypeChanged();
}

void CEncoderCtrl::OnszChromaFormatChanged() 
{
	switch( atoi( m_szChromaFormat.SpanIncluding( "0123456789" ) ) )
	{
	case 422:
		m_lngChromaFormat = CF_4_2_2;
		break;
	case 420:
		m_lngChromaFormat = CF_4_2_0;
		break;
	case 444:
		FireLog( 0, "Warning: Unsupported Chroma Format. Setting to default 4:2:0" );
		m_lngChromaFormat = CF_4_2_0;
		break;
	default:
		FireLog( 0, "Warning: Invalid Chroma Format. Setting to default 4:2:0" );
		m_lngChromaFormat = CF_4_2_0;
		break;
	}
	OnChromaFormatChanged();
}

void CEncoderCtrl::OnszVideoInputTypeChanged() 
{
	switch( m_szVideoInputType[0] )
	{
	case 'd':
	case 'D':
		m_lngVideoInputType = VIS_DIGITAL;
		break;
	case 'c':
	case 'C':
		//what is up with this! shouldn't it be VIS_COMPOSITE!!!
		m_lngVideoInputType = VIS_HARDWARE_DEFAULT;
		break;
	default:
		m_lngVideoInputType = VIS_DIGITAL;
		FireLog( 0, "Warning: Invalid Video Input Type. Setting to default Digital" );
		break;
	}
	OnVideoInputTypeChanged();
}

void CEncoderCtrl::OnszAudioInputTypeChanged() 
{
	switch( m_szAudioInputType[0] )
	{
	case 'a':
	case 'A':
		m_lngAudioInputType = AI_ANALOG;
		break;
	case 'd':
	case 'D':
		m_lngAudioInputType = AI_DIGITAL;
		break;
	case 'i':
	case 'I':
		m_lngAudioInputType = AI_INACTIVE;
		break;
	default:
		FireLog( 0, "Warning: Invalid Audio Input Type" );
		break;
	}
	OnAudioInputTypeChanged();
}

/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::OnDraw - Drawing function

void CEncoderCtrl::OnDraw(
			CDC* pdc, const CRect& /*rcBounds*/, const CRect& /*rcInvalid*/)
{
	pdc->DrawIcon( CPoint(0,0), AfxGetApp()->LoadIcon( IDI_ABOUTDLL ) );
}


/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::DoPropExchange - Persistence support

void CEncoderCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}

/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl::AboutBox - Display an "About" box to the user

void CEncoderCtrl::AboutBox()
{
	if( !this ) return;
	CDialog dlgAbout(IDD_ABOUTBOX_MPEG2ENCODER);
	dlgAbout.DoModal();
}

void CEncoderCtrl::DoAdvancedDialog() 
{
	if( !this ) return;
	WinExec( m_szAdvancedSettingsApp, SW_SHOW );
}

/////////////////////////////////////////////////////////////////////////////
// CEncoderCtrl message handlers

long CEncoderCtrl::GetState() 
{
	if( m_pEncoderInterface != NULL )
	{
		return (int)m_pEncoderInterface->GetEncoderState();
	}
	return 0;
}

void CEncoderCtrl::CleanUp()
{
	if( !this ) return;
	if( m_pEncoderInterface )
	{
		m_pEncoderInterface->CleanUp();
		delete m_pEncoderInterface;
		m_pEncoderInterface = NULL;
	}
	m_bCreated		= FALSE;
	m_bInitialized	= FALSE;
}

/////////////////////////////////////////////////////////////////
// CreateEncoderInterface()
//
// 1.   Create an object of type CEncoderInterface, a class which handles
//              establishing an interface to the FilterManager component.
//
// 2.   Call Create() to initialize COM, create an instance of the
//              Filter Manager COM Interface as well as an instance of the
//              Filter Manager Events interface.
//
// 3.   Get a pointer to the FilterManager interface, for all future
//              calls to FilterManager methods.
//
// 4.   Call SetPlaybackEnabled(TRUE) to instruct the Filter Manager
//              interface to activate the decoder playback component.
//
// 5.   Call Initialize(), to instruct Filter Manager to initialize
//              all of the encoder COM components.
////////////////////////////////////////////////////////////////////
BOOL CEncoderCtrl::CreateEncoderInterface()
{
	if( m_bCreated == TRUE )
		return TRUE;

	COMMON_VARIABLES
	try
	{
		// Create an instance of the class responsible for setting up an interface 
		// to the FilterManager COM component.
		if( m_pEncoderInterface == NULL )
		{
			FireLog( 0, "Creating encoder interface" );

			m_pEncoderInterface = new CEncoderInterface;

			if( m_pEncoderInterface == NULL )
				return FALSE;

			// Initialize the COM libraries, establish the FilterManager interfaces.
			if( FALSE == m_pEncoderInterface->Create( this ) )
				return FALSE;

			m_pFM = m_pEncoderInterface->GetFMPointer();

			if( m_pFM != NULL )
			{
				//TODO: Check this code below, don't we need this to be FALSE?
				// Instruct the encoder that the decoder component will be used.
				m_pFM->PutPlaybackEnabled( FALSE );
				//i've checked with it disabled and it worked ok

				// Initialize all encoder sub-components.
				hr = m_pFM->Initialize();
				if(SUCCEEDED(hr))
				{
					m_bCreated = TRUE;
					m_pEncoderInterface->SetEncoderState( esInitialized );
				}
				else
				{
					FireEncoderError( hr, "Unable to initialize Encoder" );
				}
			}
		}
		return m_bCreated;
	}
	EXCEPTIONS_HANDLER
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnInitialize() is called when the user clicks on the "Cue" button.
// To prepare for an encode, this method calls the Filter Manager interface methods
// Reset(), Load(), PutMuxFileEnabled(), PutMuxFileName(), AddMultiCuts(), and Cue().
//
// It is assumed that the application has already called CreateFilterManagerInterface()
// to create and initialize an interface to the FilterManager component and its events.
//
// After calling Reset(), this function loads the encode settings from the registry.
// It then uses the window settings to set the mux output file name, the mark-in, the
// mark-out and the duration.  Note that the mark-in and mark-out are not used if
// the VTRControl registry field "SourceEnabled" is set to 0.  In that case, it is
// assumed that the tape or source is already running.
//
// After loading its settings, this function calls Cue().  In response, each of the
// encoder components performs whatever steps are needed to prepare for an encode.  For example,
// the video reader component loads the microcode, the VTR control component pre-rolls the tape
// deck (only if SourceEnabled is turned on), the audio reader component sets up the
// audio board, the storage component(s) open the output file, ...).
/////////////////////////////////////////////////////////////////////////////////////////
void CEncoderCtrl::Cue()
{
	if( !this ) return;
	COMMON_VARIABLES
	try
	{
		FireLog( 0, "Cueing..." );
		EXIT_IF_ENCODER_NOT_INITIALIZED
		m_pFM = m_pEncoderInterface->GetFMPointer();
		EXIT_IF_INVALID_POINTER_TO_ENCODER

		//defaults if invalid values
		if( m_szDuration.GetLength() < 1 )
		{
			FireLog( ERROR_INVALID_DATA, "Invalid parameter. Duration. Using default of 30 seconds" );
			m_szDuration = _T("02:00:00:00");
		}
		if( m_szMPEGFileName.GetLength() < 1 )
		{
			FireLog( ERROR_INVALID_DATA, "Invalid parameter. MPEGFile. Using default D:\\MPEGFiles\\Test.mpg" );
			m_szMPEGFileName = _T("D:\\MPEGFILES\\Test.mpg");
		}

		// Read string settings for file name and time codes from window.
		CComBSTR bstrMarkIn("");
		CComBSTR bstrMarkOut("");
		CComBSTR bstrDuration(m_szDuration);
		CComBSTR bstrTapeId( m_szTapeID );

		switch( m_pEncoderInterface->GetEncoderState() )
		{
		case esPaused:
			//unpause and... fall through to stop...
			FireEncoderError( 0, "Encoder is Paused. Resuming, Stopping and Attempting Re-Cue" );
			hr = m_pFM->Resume();
			if(FAILED(hr))
			{
				FireEncoderError( hr, "Resuming Encoder" );
				m_pEncoderInterface->SetEncoderState( esError );
			}
			else
			{
				m_pEncoderInterface->SetEncoderState( esStarted );
			}
		case esStarted:
			//stop
			FireEncoderError( 0, "Encoder is Started. Stopping and Attempting Re-Cue" );
			hr = m_pFM->End();
			if(FAILED(hr))
			{
				FireEncoderError( hr, "Stopping Encoder" );
				m_pEncoderInterface->SetEncoderState( esError );
			}
			else
			{
				m_pEncoderInterface->SetEncoderState( esInitialized );
			}
			break;
		case esNoState:
			//we shouldn't even be here :-×
		case esError:
			FireEncoderError( ERROR_GEN_FAILURE, "Encoder is IN ERROR. Attempting Reinitialization. This WILL take a while." );
			//bad, really bad, let's see if we can reinitialize...
			CleanUp();
			if( FALSE == CreateEncoderInterface() )
			{
				FireEncoderError( hr, "Unable to Re-Initialize Encoder. If this is the second consecutive time, please re-start computer." );
				m_pEncoderInterface->SetEncoderState( esError );
				return;
			}
			break;
		case esCued:
			//let's start and then stop, so the encoder is :¬)
			//that didn't work, so let's try another trick
			//wow :-O, it worked!
			FireEncoderError( 0, "Encoder is already Cued. Attempting Re-Cue" );
			hr = m_pFM->Cue();
			break;
		case esInitialized:
			//just the way i like it 
			break;
		}

		FireLog( 0, "Reseting Encoder" );
		// Reset all encoder components, check results.
		hr = m_pFM->Reset();
		if( hr != S_OK )
		{
			FireEncoderError( hr, "Unable to Reset Encoder" );
			return;
		}

		FireLog( 0, "Loading Registry Settings" );
		// Load registry settings. 
		// Use FALSE to NOT communicate with the VTRControl component. 
		hr = m_pFM->Load( FALSE );
		if( hr != S_OK )
		{
			FireEncoderError( hr, "Loading Encode Settings from Registry" );
			return;
		}

		// Set Mux file name and tell Filter Manager that we're storing a muxed file.
		m_pFM->PutMuxFileEnabled(TRUE);
		m_pFM->PutMuxFileName((LPCTSTR) m_szMPEGFileName );

		// Set mark-in, mark-out and duration of a single clip.
		
		// Note that bstrMarkIn and bstrMarkOut are ignored if the VTRControl
		// registry setting "SourceEnabled" is set to 0.
		
		// The first argument (1) indicates that there will be only one segment
		// encoded for this clip--no pause/resume.  The Tape Id is not currently used.
		//changed to allow multiple segment tapes
		m_pFM->AddMultiCuts( m_lngNumberOfTapes, &bstrTapeId, &bstrMarkIn, &bstrMarkOut, &bstrDuration );

		FireLog( 0, "Cueing components" );
		// Cue all components.  Check Results.
		hr = m_pFM->Cue();
		if( hr != S_OK )
		{
			FireEncoderError( hr, "Cueing components" );
			return;
		}

		//set timer to check when cue up finishes
//		SetTimer( CONTROL_TIMER_ID, CONTROL_TIMER_ELAPSE, 0 );

		Sleep(10000);
		// If the cue was successful, set the encoder state to cued and
		// activate the start button.
		m_pEncoderInterface->SetEncoderState( esCued );
		FireFinished( ENCODER_FINISH_EVENT_CUEING, "Cueing components" );

/*
//take a nap until it finishes cueing

		 HANDLE          hTimer;
		 BOOL            bSuccess;
		 __int64         qwDueTime;
		 LARGE_INTEGER   liDueTime;

		 if ( hTimer = CreateWaitableTimer(
			   NULL,                   // Default security attributes.
			   FALSE,                  // Create auto-reset timer.
			   "MPEGControlTimer" ) ) {// Name of waitable timer.

			 // Create a negative 64-bit integer that will be used to
			 // signal the timer 5 seconds from now.
			 qwDueTime = -5 * _SECOND;

			 // Copy the relative time into a LARGE_INTEGER.
			 liDueTime.LowPart  = (DWORD) ( qwDueTime & 0xFFFFFFFF );
			 liDueTime.HighPart = (LONG)  ( qwDueTime >> 32 );
		
			 m_pEncoderInterfaceTemp = m_pEncoderInterface;

			 bSuccess = SetWaitableTimer(
			   hTimer,                 // Handle to the timer object.
			   &liDueTime,             // When timer will become signaled.
			   4000,                   // Periodic timer interval of 4 seconds.
			   TimerAPCProc,           // Completion routine.
			   (void*)this,            // Argument to the completion routine.
			   TRUE );                 // Restore a suspended system.

			 if ( bSuccess ) {

				 SleepEx(
				   INFINITE,           // Wait forever.
				   TRUE );             // IMPORTANT!!! The thread must be in an
									   // alertable state to process the APC.

			 } else {
				 szError.Format( "Cueing components: SetWaitableTimer() failed with Error %d.", GetLastError() );
				FireEncoderError( hr, szError );
			 }

			 CloseHandle( hTimer );

		 } else {
				szError.Format( "Cueing components: CreateWaitableTimer() failed with Error %d.", GetLastError() );
				FireEncoderError( hr, szError );
		 }
//end nap
*/

	}
	EXCEPTIONS_HANDLER
}

/*
void CEncoderCtrl::OnTimer(UINT nIDEvent) 
{
	//this timer checks if the Cue up finished and
	// then it triggers an event

	switch( nIDEvent )
	{
	case CONTROL_TIMER_ID:
		switch( m_pEncoderInterface->GetEncoderState() )
		{
		case esStarted:
		case esCued:
			FireFinished( ENCODER_FINISH_EVENT_CUEING, "Cueing components" );
			KillTimer( CONTROL_TIMER_ID );
			break;
		case esPaused:
		case esInitialized:
			Beep( 1000, 700 );
			break;
		case esError:
		case esNoState:
		default:
			FireEncoderError( ERROR_GEN_FAILURE, "Cueing components" );
			KillTimer( CONTROL_TIMER_ID );
			break;
		}
		break;
	}
	COleControl::OnTimer(nIDEvent);
}

void CEncoderCtrl::FireFinishedCueing()
{
	FireFinished( ENCODER_FINISH_EVENT_CUEING, "Cueing components" );
}

VOID CALLBACK TimerAPCProc(
       LPVOID lpArg,               // Data value.
       DWORD dwTimerLowValue,      // Timer low value.
       DWORD dwTimerHighValue ) {  // Timer high value.

	CEncoderCtrl * pCtrl = (CEncoderCtrl*)lpArg;
	switch( pCtrl->m_pEncoderInterfaceTemp->GetEncoderState() )
	{
	case esStarted:
	case esCued:
		pCtrl->FireFinishedCueing();
		break;
	case esPaused:
	case esInitialized:
		Beep( 4000, 700 );
		break;
	case esError:
	case esNoState:
	default:
		Beep( 90, 900 );
//		pCtrl->FireEncoderError( ERROR_GEN_FAILURE, "Cueing components" );
		break;
	}
}
*/


////////////////////////////////////////////////////////////////////////////////////////////////
// OnStart()
//
// This method calls the FilterManager interface method Start() to start an encode.
// In reponse, if VTR control is enabled, the VTR component rolls the tape deck, starting 
// the other components when it determines that the mark-in is near.  The encoder then 
// encodes for the duration specified during the Cue() phase, unless the encode is
// manually terminated when the user clicks the Stop button.
////////////////////////////////////////////////////////////////////////////////////////////////
void CEncoderCtrl::Start() 
{
	if( !this ) return;
	COMMON_VARIABLES
	try
	{
		FireLog( 0, "Starting..." );
		EXIT_IF_ENCODER_NOT_INITIALIZED
		m_pFM = m_pEncoderInterface->GetFMPointer();
		EXIT_IF_INVALID_POINTER_TO_ENCODER

		switch( m_pEncoderInterface->GetEncoderState() )
		{
		case esPaused:
			//unpause and return
			FireEncoderError( 0, "Warning: You should have un-paused the encoder. Resuming encoding..." );
			hr = m_pFM->Resume();
			if(FAILED(hr))
			{
				FireEncoderError( hr, "Resuming Encoder" );
				m_pEncoderInterface->SetEncoderState( esError );
			}
			else
			{
				m_pEncoderInterface->SetEncoderState( esStarted );
			}
			return;
			break;
		case esStarted:
			FireEncoderError( 0, "Warning: Encoder is already started. Stop and Re-Cue." );
			return;
			break;
		case esNoState:
			//we shouldn't even be here
		case esError:
			//bad, really bad, let's see if we can reinitialize...
			FireEncoderError( ERROR_GEN_FAILURE, "Encoder is IN ERROR. Attempting Reinitialization. This WILL take a while." );
			//then fall through... to Cue
			CleanUp();
			if( FALSE == CreateEncoderInterface() )
			{
				FireEncoderError( hr, "Unable to Re-Initialize Encoder. If this is the second consecutive time, please re-start computer." );
				m_pEncoderInterface->SetEncoderState( esError );
				return;
			}
		case esInitialized:
			//let's be nice and Cue components, yes?
			FireEncoderError( 0, "Warning: You should Cued the encoder. Attempting to Cue..." );
			Cue();
			break;
		case esCued:
			//this is the desired state
			break;
		}

		hr = m_pFM->Start();
		if(FAILED(hr))
		{
			FireEncoderError( hr, "Starting Encoder" );
			m_pEncoderInterface->SetEncoderState( esError );
			return;
		}
		Sleep(4000);
		m_pEncoderInterface->SetEncoderState( esStarted );
	}
	EXCEPTIONS_HANDLER
}

////////////////////////////////////////////////////////////////////////////////////////////////
// OnPause()
//
// This method calls the FilterManager interface method Pause() to pause an encode.
////////////////////////////////////////////////////////////////////////////////////////////////
void CEncoderCtrl::Pause() 
{
	if( !this ) return;
	COMMON_VARIABLES
	try
	{
		FireLog( 0, "Pausing..." );
		EXIT_IF_ENCODER_NOT_INITIALIZED
		m_pFM = m_pEncoderInterface->GetFMPointer();
		EXIT_IF_INVALID_POINTER_TO_ENCODER

		switch( m_pEncoderInterface->GetEncoderState() )
		{
		case esPaused:
			FireEncoderError( 0, "Warning: Encoder is already paused." );
			return;
			break;
		case esInitialized:
		case esCued:
			FireEncoderError( 0, "Warning: Encoder has not started yet." );
			return;
			break;
		case esNoState:
			//we shouldn't even be here
		case esError:
			//bad, really bad, let's see if we can reinitialize...
			FireEncoderError( ERROR_GEN_FAILURE, "Encoder is IN ERROR. Attempting Reinitialization. This WILL take a while." );
			CleanUp();
			if( FALSE == CreateEncoderInterface() )
			{
				FireEncoderError( hr, "Unable to Re-Initialize Encoder. If this is the second consecutive time, please re-start computer." );
				m_pEncoderInterface->SetEncoderState( esError );
			}
			FireEncoderError( 0, "Warning: Encoder has not started yet." );
			return;
			break;
		case esStarted:
			//this is the desired state
			break;
		}

		hr = m_pFM->Pause();
		if(FAILED(hr))
		{
			FireEncoderError( hr, "Pausing Encoder" );
			m_pEncoderInterface->SetEncoderState( esError );
			return;
		}
		Sleep(2000);
		m_pEncoderInterface->SetEncoderState( esPaused );
	}
	EXCEPTIONS_HANDLER
}

////////////////////////////////////////////////////////////////////////////////////////////////
// OnResume()
//
// This method calls the FilterManager interface method Resume() to resume an encode.
////////////////////////////////////////////////////////////////////////////////////////////////
void CEncoderCtrl::Resume() 
{
	if( !this ) return;
	COMMON_VARIABLES
	try
	{
		FireLog( 0, "Resuming..." );
		EXIT_IF_ENCODER_NOT_INITIALIZED
		m_pFM = m_pEncoderInterface->GetFMPointer();
		EXIT_IF_INVALID_POINTER_TO_ENCODER

		switch( m_pEncoderInterface->GetEncoderState() )
		{
		case esStarted:
			FireEncoderError( 0, "Warning: Encoder is already started." );
			return;
			break;
		case esInitialized:
		case esCued:
			FireEncoderError( 0, "Warning: Encoder has not started yet." );
			return;
			break;
		case esNoState:
			//we shouldn't even be here
		case esError:
			//bad, really bad, let's see if we can reinitialize...
			FireEncoderError( ERROR_GEN_FAILURE, "Encoder is IN ERROR. Attempting Reinitialization. This WILL take a while." );
			CleanUp();
			if( FALSE == CreateEncoderInterface() )
			{
				FireEncoderError( hr, "Unable to Re-Initialize Encoder. If this is the second consecutive time, please re-start computer." );
				m_pEncoderInterface->SetEncoderState( esError );
				return;
			}
			FireEncoderError( 0, "Warning: Encoder has not started yet." );
			return;
			break;
		case esPaused:
			//this is the desired state
			break;
		}

		hr = m_pFM->Resume();
		if(FAILED(hr))
		{
			FireEncoderError( hr, "Resuming Encoder" );
			m_pEncoderInterface->SetEncoderState( esError );
			return;
		}
		Sleep(2000);
		m_pEncoderInterface->SetEncoderState( esStarted );
	}
	EXCEPTIONS_HANDLER
}

////////////////////////////////////////////////////////////////////////////////////////
// OnStop()
//
// This method is called when the user clicks the Stop button.
// The FilterManager will signal the encoder to stop.  When all components have
// stopped, the FilterManager will call FinishedEvent (the callback defined in
// FilterManagerEvents.cpp).
////////////////////////////////////////////////////////////////////////////////////////
void CEncoderCtrl::Stop() 
{
	if( !this ) return;
	COMMON_VARIABLES
	try
	{
		FireLog( 0, "Stoping..." );
		EXIT_IF_ENCODER_NOT_INITIALIZED
		m_pFM = m_pEncoderInterface->GetFMPointer();
		EXIT_IF_INVALID_POINTER_TO_ENCODER

		switch( m_pEncoderInterface->GetEncoderState() )
		{
		case esInitialized:
		case esCued:
			FireEncoderError( 0, "Warning: Encoder has not started yet." );
			return;
			break;
		case esNoState:
			//we shouldn't even be here
		case esError:
			//bad, really bad, let's see if we can reinitialize...
			FireEncoderError( ERROR_GEN_FAILURE, "Encoder is IN ERROR. Attempting Reinitialization. This WILL take a while." );
			CleanUp();
			if( FALSE == CreateEncoderInterface() )
			{
				FireEncoderError( hr, "Unable to Re-Initialize Encoder. If this is the second consecutive time, please re-start computer." );
				m_pEncoderInterface->SetEncoderState( esError );
				return;
			}
			FireEncoderError( 0, "Warning: Encoder has not started yet." );
			return;
			break;
		case esPaused:
			FireEncoderError( 0, "Warning: You should have un-paused the encoder. Resuming encoding before Stopping..." );
			hr = m_pFM->Resume();
			if(FAILED(hr))
			{
				FireEncoderError( hr, "Resuming Encoder" );
				m_pEncoderInterface->SetEncoderState( esError );
			}
			else
			{
				m_pEncoderInterface->SetEncoderState( esStarted );
			}
			break;
		case esStarted:
			//this is the desired state
			break;
		}

		hr = m_pFM->End();
		if(FAILED(hr))
		{
			FireEncoderError( hr, "Stopping Encoder" );
			m_pEncoderInterface->SetEncoderState( esError );
			return;
		}
		Sleep(3000);
		m_pEncoderInterface->SetEncoderState( esInitialized );
	}
	EXCEPTIONS_HANDLER
}

///////////////////////////////////////////////////////////////////////////////////////
// OnReset()
//
// This method calls the FilterManager interface method Reset() to clear previous
// settings in preparation for the next encode.
///////////////////////////////////////////////////////////////////////////////////////
void CEncoderCtrl::Reset() 
{
	if( !this ) return;
	COMMON_VARIABLES
	try
	{
		FireLog( 0, "Reseting..." );
		EXIT_IF_ENCODER_NOT_INITIALIZED
		m_pFM = m_pEncoderInterface->GetFMPointer();
		EXIT_IF_INVALID_POINTER_TO_ENCODER

		hr = m_pFM->Reset();
		if(FAILED(hr))
		{
			FireEncoderError( hr, "Resetting Encoder" );
			m_pEncoderInterface->SetEncoderState( esError );
			return;
		}
		Sleep(2000);
		m_pEncoderInterface->SetEncoderState( esInitialized );
	}
	EXCEPTIONS_HANDLER
}

long CEncoderCtrl::GetControlPointer() 
{
	return reinterpret_cast<long>(this);
}

void CEncoderCtrl::SetControlPointer(long nNewValue) 
{
	SetNotSupported();
}

CString GetComError(const _com_error& e)
{
	CString sMsg;
	sMsg.Format(
		_T("HRESULT: 0x%08lx; Error: %s\n"),
		e.Error(),
		e.ErrorMessage()
		);
	
	if(e.ErrorInfo())
	{
		sMsg += "Source: " + CString((LPCTSTR)e.Source()) +
			"; Description: " + CString((LPCTSTR)e.Description());
	}
	else
	{
		sMsg += e.Description();
	}

	return sMsg;
}



void CEncoderCtrl::OnszTapeIDChanged() 
{
	SetModifiedFlag();
}

void CEncoderCtrl::OnlngNumberOfTapesChanged() 
{
	SetModifiedFlag();
}
