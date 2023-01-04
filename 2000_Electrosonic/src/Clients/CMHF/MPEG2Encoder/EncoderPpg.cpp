// EncoderPpg.cpp : Implementation of the EncoderPropPage property page class.

#include "stdafx.h"
#include "MPEG2Encoder.h"
#include "EncoderPpg.h"
#include "EncoderCtl.h"
#include "EncoderInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(EncoderPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(EncoderPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(EncoderPropPage)
	ON_BN_CLICKED(IDC_RADIONTSC, OnRadioNTSC)
	ON_BN_CLICKED(IDC_RADIOPAL, OnRadioPAL)
	ON_BN_CLICKED(IDC_RADIO_MPEG1, OnRadioMPEG1)
	ON_BN_CLICKED(IDC_RADIO_MPEG2, OnRadioMPEG2)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnButtonAdvanced)
	ON_BN_CLICKED(IDC_BUTTON_HELP, OnButtonHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(EncoderPropPage, "MPEG2ENCODER.MPEG2EncoderPropPage.1",
	0x485c71a7, 0xbfe, 0x11d5, 0x8b, 0xe8, 0, 0, 0, 0, 0, 0)


/////////////////////////////////////////////////////////////////////////////
// EncoderPropPage::EncoderPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for EncoderPropPage

BOOL EncoderPropPage::EncoderPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_MPEG2ENCODER_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// EncoderPropPage::EncoderPropPage - Constructor

EncoderPropPage::EncoderPropPage() :
	COlePropertyPage(IDD, IDS_MPEG2ENCODER_PPG_CAPTION)
{
	m_pEncoderControl = NULL;
	//{{AFX_DATA_INIT(EncoderPropPage)
	m_szMPEGFileName = _T("");
	m_szDuration = _T("");
	m_lngVideoBitRate = 0;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// EncoderPropPage::DoDataExchange - Moves data between page and properties

void EncoderPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(EncoderPropPage)
	DDX_Control(pDX, IDC_STATIC_SETTINGS, m_ctrlSettings);
	DDX_Control(pDX, IDC_EDIT_BITRATE, m_ctrlEditBitRate);
	DDP_Text(pDX, IDC_EDIT_MPEGFILENAME, m_szMPEGFileName, _T("szMPEGFileName") );
	DDX_Text(pDX, IDC_EDIT_MPEGFILENAME, m_szMPEGFileName);
	DDP_Text(pDX, IDC_EDIT_DURATION, m_szDuration, _T("szDuration") );
	DDX_Text(pDX, IDC_EDIT_DURATION, m_szDuration);
	DDV_MaxChars(pDX, m_szDuration, 12);
	DDP_Text(pDX, IDC_EDIT_BITRATE, m_lngVideoBitRate, _T("lngVideoBitRate") );
	DDX_Text(pDX, IDC_EDIT_BITRATE, m_lngVideoBitRate);
	DDX_Control(pDX, IDC_RADIONTSC, m_ctlRadioNTSC);
	DDX_Control(pDX, IDC_RADIOPAL, m_ctlRadioPAL);
	DDX_Control(pDX, IDC_RADIO_MPEG1, m_ctlRadioMPEG1);
	DDX_Control(pDX, IDC_RADIO_MPEG2, m_ctlRadioMPEG2);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
//	DDX_Control(pDX, IDC_SPIN_BITRATE, m_ctrlSpinBitRate);
}


/////////////////////////////////////////////////////////////////////////////
// EncoderPropPage message handlers

BOOL EncoderPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();
	
//	m_ctrlSpinBitRate.SetBuddy( &m_ctrlEditBitRate );
//	UDACCEL pAccel[] = { 0,1,3,10,6,100,9,1000,12,10000};
//	m_ctrlSpinBitRate.SetAccel( 5, pAccel );

	ULONG uNumControls;
	// Get the array of IDispatchs stored in the property page.
	LPDISPATCH *lpDispatchControls = GetObjectArray(&uNumControls);
	for (ULONG i = 0; i < uNumControls; i++)
	{
		m_pEncoderControl = GetControl( lpDispatchControls, i );
	}
    
	if( m_pEncoderControl )
	{
//		m_ctrlSpinBitRate.SetPos( m_pEncoderControl->m_lngVideoBitRate );
		switch( m_pEncoderControl->m_lngMPEGType )
		{
		case MPEG_TYPE_MPEG1:
			m_ctlRadioMPEG1.SetCheck( 1 );
			m_ctlRadioMPEG2.SetCheck( 0 );
			switch( m_pEncoderControl->m_lngVideoFormat )
			{
			case VF_NTSC:
				m_ctlRadioNTSC.SetCheck( 1 );
				m_ctlRadioPAL.SetCheck( 0 );
				break;
			case VF_PAL:
				m_ctlRadioNTSC.SetCheck( 0 );
				m_ctlRadioPAL.SetCheck( 1 );
				break;
			}
			break;
		case MPEG_TYPE_MPEG2:
			m_ctlRadioMPEG1.SetCheck( 0 );
			m_ctlRadioMPEG2.SetCheck( 1 );
			switch( m_pEncoderControl->m_lngVideoFormat )
			{
			case VF_NTSC:
				m_ctlRadioNTSC.SetCheck( 1 );
				m_ctlRadioPAL.SetCheck( 0 );
				break;
			case VF_PAL:
				m_ctlRadioNTSC.SetCheck( 0 );
				m_ctlRadioPAL.SetCheck( 1 );
				break;
			}
			break;
		}
	}

	UpdateControls();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void EncoderPropPage::OnRadioNTSC() 
{
	if( m_pEncoderControl )
	{
		switch( m_pEncoderControl->m_lngMPEGType )
		{
		case MPEG_TYPE_MPEG1:
			m_pEncoderControl->m_lngVideoFormat = m_pEncoderControl->m_Mode_MPEG_1_NTSC.lngVideoFormat;
			break;
		case MPEG_TYPE_MPEG2:
			m_pEncoderControl->m_lngVideoFormat = m_pEncoderControl->m_Mode_MPEG_2_NTSC.lngVideoFormat;
			break;
		}
		m_pEncoderControl->OnVideoFormatChanged();
		UpdateControls();
	}
}

void EncoderPropPage::OnRadioPAL() 
{
	if( m_pEncoderControl )
	{
		switch( m_pEncoderControl->m_lngMPEGType )
		{
		case MPEG_TYPE_MPEG1:
			m_pEncoderControl->m_lngVideoFormat = m_pEncoderControl->m_Mode_MPEG_1_PAL.lngVideoFormat;
			break;
		case MPEG_TYPE_MPEG2:
			m_pEncoderControl->m_lngVideoFormat = m_pEncoderControl->m_Mode_MPEG_2_PAL.lngVideoFormat;
			break;
		}
		m_pEncoderControl->OnVideoFormatChanged();
		UpdateControls();
	}
}

void EncoderPropPage::OnRadioMPEG1() 
{
	if( m_pEncoderControl )
	{
		m_pEncoderControl->m_lngMPEGType = MPEG_TYPE_MPEG1;
		m_pEncoderControl->OnMPEGTypeChanged();
		UpdateControls();
	}
}

void EncoderPropPage::OnRadioMPEG2() 
{
	if( m_pEncoderControl )
	{
		m_pEncoderControl->m_lngMPEGType = MPEG_TYPE_MPEG2;
		m_pEncoderControl->OnMPEGTypeChanged();
		UpdateControls();
	}
}

LPCTSTR pstrSettings = \
"Chroma Format:\t\t%s\n\
Horz Res:\t\t%li\n\
Vert Res:\t\t%li\n";

CHROMA_FORMAT_MAP ChromaFormats[] =
{
	CF_4_2_0,	"4:2:0",
	CF_4_2_2,	"4:2:2",
	CF_4_4_4,	"4:4:4"
};

void EncoderPropPage::UpdateControls()
{
	CString szSettings;
	switch( m_pEncoderControl->m_lngMPEGType )
	{
	case MPEG_TYPE_MPEG1:
//		m_ctrlSpinBitRate.SetRange32( VIDEO_420_MIN_BITRATE, VIDEO_MPEG1_MAX_BITRATE );
		switch( m_pEncoderControl->m_lngVideoFormat )
		{
		case VF_NTSC:
			szSettings.Format(  pstrSettings, 
								ChromaFormats[m_pEncoderControl->m_Mode_MPEG_1_NTSC.lngChroma].szValue,
								m_pEncoderControl->m_Mode_MPEG_1_NTSC.lngHorizontalRes,
								m_pEncoderControl->m_Mode_MPEG_1_NTSC.lngVerticalRes
							);
			break;
		case VF_PAL:
			szSettings.Format(  pstrSettings, 
								ChromaFormats[m_pEncoderControl->m_Mode_MPEG_1_PAL.lngChroma].szValue,
								m_pEncoderControl->m_Mode_MPEG_1_PAL.lngHorizontalRes,
								m_pEncoderControl->m_Mode_MPEG_1_PAL.lngVerticalRes
							);
			break;
		}
		break;
	case MPEG_TYPE_MPEG2:
//		m_ctrlSpinBitRate.SetRange32( VIDEO_420_MIN_BITRATE, VIDEO_422_MAX_BITRATE );
		switch( m_pEncoderControl->m_lngVideoFormat )
		{
		case VF_NTSC:
			szSettings.Format(  pstrSettings, 
								ChromaFormats[m_pEncoderControl->m_Mode_MPEG_2_NTSC.lngChroma].szValue,
								m_pEncoderControl->m_Mode_MPEG_2_NTSC.lngHorizontalRes,
								m_pEncoderControl->m_Mode_MPEG_2_NTSC.lngVerticalRes
							);
			break;
		case VF_PAL:
			szSettings.Format(  pstrSettings, 
								ChromaFormats[m_pEncoderControl->m_Mode_MPEG_2_PAL.lngChroma].szValue,
								m_pEncoderControl->m_Mode_MPEG_2_PAL.lngHorizontalRes,
								m_pEncoderControl->m_Mode_MPEG_2_PAL.lngVerticalRes
							);
			break;
		}
		break;
	}
	m_ctrlSettings.SetWindowText( szSettings );
	UpdateData();
}

void EncoderPropPage::OnButtonAdvanced() 
{
	if( m_pEncoderControl )
		m_pEncoderControl->DoAdvancedDialog();
}

LPCTSTR pstrHelpText = "This control uses Registry Key:\n\
\t%s\n\
Vela components use Registry Key:\n\
\t%s\n\n\
The application invoked by the Advanced Button operates on Velas Registry\n\
Use extreme care when changing values from there as it may render this control\n\
unusable until settings are fixed.\n\n\
The Video Format settings in this property page use settings that have been\n\
proven to work under one set of circumstances. Those settings are saved in the\n\
Electrosonic Registry and later they are transfered to the Vela Registry.\n\
If those settings do not work, use the Advanced Button to change the settings\n\
and then open the registry and transfer the new valid settings from Vela to\n\
Electrosonic Registry. We use two sets in NTSC and PAL so you must transfer the\n\
one you will use or both if desired. Note that the Settings in Electrosonic Registry\n\
use a format prefix for each, e.g. VideoMode is used as NTSCVideoMode.\n\
Then those settings will be in effect from this property page.";

void EncoderPropPage::OnButtonHelp() 
{
	CRegistry Settings;
	CRegistry Video ;
	CString szHelp;
	CString szHelpText;

    if( Settings.Open(HKEY_CURRENT_USER, ESI_KEY ) == TRUE )
	{
		if( Video.Open(Settings.hKey(), ESI_SUBKEY ) == TRUE )
		{
			Video.GetValue( _T("Help"), &szHelpText, (LPCTSTR)pstrHelpText );
			Video.Close();
		}
		Settings.Close();
	}

	if( m_pEncoderControl )
		szHelp.Format( szHelpText, ESI_KEY, m_pEncoderControl->m_szVelaRegistryKey );
	else
		szHelp.Format( szHelpText, ESI_KEY, ARGUS_KEY );
	
	MessageBox( szHelp );

}

CEncoderCtrl * GetControl(LPDISPATCH *lpDispatchControls, ULONG iControlIndex)
{
    CEncoderCtrl * pMyCtrl = NULL; 

    // Get the CCmdTarget object associated.
    pMyCtrl = (CEncoderCtrl*) CCmdTarget::FromIDispatch(lpDispatchControls[iControlIndex]);

    if (!pMyCtrl) // Above failed. Container must have aggregated the control.
    {
       long ControlPointer;

       _DMPEG2Encoder control(lpDispatchControls[iControlIndex]);           

       // GetObjectArray() docs state must not release pointer.
       control.m_bAutoRelease = FALSE;
       ControlPointer = control.GetControlPointer();

       pMyCtrl = reinterpret_cast<CEncoderCtrl*>(ControlPointer);
    }
    return pMyCtrl;
} 


