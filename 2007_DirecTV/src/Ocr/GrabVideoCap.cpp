#include "stdafx.h"
#include "GrabVideoCap.h"

//TODO: lots of stuff, error and exception handling, inspection of all COM DShow functions,etc...
typedef struct _callbackinfo 
{
    double dblSampleTime;
    long lBufferSize;
    BYTE *pBuffer;
    BITMAPINFOHEADER bih;
	DWORD biSize;

} CALLBACKINFO;
CALLBACKINFO cbInfo={0};

class CSampleGrabberCB : public ISampleGrabberCB 
{
public:

    long Width;
    long Height;

    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        CheckPointer(ppv,E_POINTER);
        if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
        {
            *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
            return NOERROR;
        }    
        return E_NOINTERFACE;
    }

    STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
    {
        return 0;
    }

	STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
    {
        if (!pBuffer)
            return E_POINTER;

        if( cbInfo.lBufferSize < lBufferSize )
        {
            delete [] cbInfo.pBuffer;
            cbInfo.pBuffer = NULL;
            cbInfo.lBufferSize = 0;
        }

        cbInfo.dblSampleTime = dblSampleTime;

        if (!cbInfo.pBuffer)
        {
            cbInfo.pBuffer = new BYTE[lBufferSize];
            cbInfo.lBufferSize = lBufferSize;
        }

        if( !cbInfo.pBuffer )
        {
            cbInfo.lBufferSize = 0;
            return E_OUTOFMEMORY;
        }

        memcpy(cbInfo.pBuffer, pBuffer, lBufferSize);
        return 0;
    }
};
CSampleGrabberCB CB;

GrabVideoCap::GrabVideoCap()
{
}

GrabVideoCap::~GrabVideoCap()
{
}

void GrabVideoCap::Initialize()
{
    HRESULT hr;

    CComPtr< ISampleGrabber >	m_pGrabber;
	CComPtr< IBaseFilter >		m_pGrabberBase;
    CComPtr< IBaseFilter >		m_pSource;
    CComPtr< IGraphBuilder >	m_pGraph;
    CComPtr< IPin >				m_pSourcePin;
	ICaptureGraphBuilder2* m_pCaptureGraphBuilder ;

    m_pGrabber.CoCreateInstance( CLSID_SampleGrabber );
    if( !m_pGrabber )
    {
        return;
    }
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabberBase( m_pGrabber );
	m_pGrabberBase = pGrabberBase.p;
	pGrabberBase.p->AddRef();

	GetDefaultCapDevice(&m_pSource);
	if( !m_pSource )
    {
        return;
    }

    m_pGraph.CoCreateInstance( CLSID_FilterGraph );
    if( !m_pGraph )
    {
        return;
    }

	// Create the capture graph builder
	hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
							IID_ICaptureGraphBuilder2, (void **) &m_pCaptureGraphBuilder);
	if (FAILED(hr))
		return;

    // Attach the filter graph to the capture graph
    hr = m_pCaptureGraphBuilder->SetFiltergraph(m_pGraph);
    if (FAILED(hr))
    {
        //DisplayMesg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
        return;
    }

    hr = m_pGraph->AddFilter( m_pSource, L"Source" );
    hr = m_pGraph->AddFilter( m_pGrabberBase, L"Grabber" );

    CMediaType GrabType;
    GrabType.SetType( &MEDIATYPE_Video );
    GrabType.SetSubtype( &MEDIASUBTYPE_RGB24 );
    //GrabType.SetSubtype( &MEDIASUBTYPE_RGB555 );
    hr = m_pGrabber->SetMediaType( &GrabType );

    m_pSourcePin = GetOutPin( m_pSource, 0 );

	IAMCrossbar *pX;

    hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Interleaved, m_pSource,
        IID_IAMCrossbar, (void **)&pX);
    if(hr != NOERROR)
        hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, m_pSource,
            IID_IAMCrossbar, (void **)&pX);

	LONG lInpin, lOutpin;
	hr = pX->get_PinCounts(&lOutpin , &lInpin); 
	
	BOOL IPin=TRUE; LONG pIndex=0 , pRIndex=0 , pType=0;
	
	while( pIndex < lInpin)
	{
		hr = pX->get_CrossbarPinInfo( IPin , pIndex , &pRIndex , &pType); 
	
		//if( pType == PhysConn_Video_Tuner)
		//if( pType == PhysConn_Video_Composite)
		//if( pType == PhysConn_Video_SVideo)
		if( pType == ini_options->VideoCaptureDeviceSource() )
				break;
				pIndex++;
	}
		
	BOOL OPin=FALSE; LONG pOIndex=0 , pORIndex=0 , pOType=0;
	while( pOIndex < lOutpin)
	{
		hr = pX->get_CrossbarPinInfo( OPin , pOIndex , &pORIndex , &pOType); 
		if( pOType == PhysConn_Video_VideoDecoder)
			break;
	}

	pX->Route(pOIndex,pIndex); 

    pX->Release();

 
    m_pGrabPin   = GetInPin( m_pGrabberBase, 0 );

    m_pGrabOutPin = GetOutPin( m_pGrabberBase, 0 );

    //InitVideoWindow(ini_options->VideoCaptureDeviceWidth(),ini_options->VideoCaptureDeviceHeight());
    //InitVideoWindow(720,480);

    CComPtr<IAMStreamConfig> pConfig;
    IEnumMediaTypes *pMedia;
    AM_MEDIA_TYPE *pmt = NULL, *pfnt = NULL;

    hr = m_pSourcePin->EnumMediaTypes( &pMedia );
    if(SUCCEEDED(hr))
    { 

        while(pMedia->Next(1, &pmt, 0) == S_OK)
        {
            if( pmt->formattype == FORMAT_VideoInfo )
            {
                VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)pmt->pbFormat;
                // printf("Size %i  %i\n", vih->bmiHeader.biWidth, vih->bmiHeader.biHeight );
                if( vih->bmiHeader.biWidth == ini_options->VideoCaptureDeviceWidth() && vih->bmiHeader.biHeight == ini_options->VideoCaptureDeviceHeight() )
                {
                    pfnt = pmt;
                    break;
                }
                DeleteMediaType( pmt );
            }                        
        }
        if( pfnt == NULL ) {

            while(pMedia->Next(1, &pmt, 0) == S_OK)
            {
                if( pmt->formattype == FORMAT_VideoInfo )
                {
                    VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)pmt->pbFormat;
                    // printf("Size %i  %i\n", vih->bmiHeader.biWidth, vih->bmiHeader.biHeight );
                    if( vih->bmiHeader.biWidth == 720 && vih->bmiHeader.biHeight == 480 )
                    {
                        pfnt = pmt;
                        break;
                    }
                    DeleteMediaType( pmt );
                }                        
            }
        }

        pMedia->Release();
    }

    hr = m_pSourcePin->QueryInterface( IID_IAMStreamConfig, (void **) &pConfig );
    if(SUCCEEDED(hr))
    {
        if( pfnt != NULL )
        {
            hr=pConfig->SetFormat( pfnt );

            DeleteMediaType( pfnt );
        }
        hr = pConfig->GetFormat( &pfnt );
        if(SUCCEEDED(hr))
        {
			
            m_nWidth = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biWidth;
            m_nHeight = ((VIDEOINFOHEADER *)pfnt->pbFormat)->bmiHeader.biHeight;
			
            DeleteMediaType( pfnt );
        }
    }

    CB.Width  = m_nWidth;
    CB.Height = m_nHeight;

    memset( &(cbInfo.bih), 0, sizeof( cbInfo.bih ) );
    cbInfo.bih.biSize = sizeof( cbInfo.bih );
    cbInfo.bih.biWidth = CB.Width;
    //cbInfo.bih.biHeight = ~CB.Height +1;
    cbInfo.bih.biHeight = CB.Height;
    cbInfo.bih.biPlanes = 1;
    cbInfo.bih.biBitCount = 24;
    //cbInfo.bih.biBitCount = 16;

    hr = m_pGraph->Connect( m_pSourcePin, m_pGrabPin );
    if( FAILED( hr ) )
    {
        return;
    }

    hr = m_pGraph->Render( m_pGrabOutPin );
    if( FAILED( hr ) )
    {
        return;
    }

    //hr = m_pGrabber->SetBufferSamples( FALSE );
    hr = m_pGrabber->SetBufferSamples( TRUE );
    hr = m_pGrabber->SetOneShot( TRUE );
    hr = m_pGrabber->SetCallback( &CB, 1 );

	CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = m_pGraph;
    if (pWindow)
    {
        hr = pWindow->put_AutoShow(OAFALSE);
    }

    CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( m_pGraph );
	m_pControl = pControl.p;
	pControl.p->AddRef();

    CComQIPtr< IMediaEvent, &IID_IMediaEvent > pEvent( m_pGraph );
	m_pEvent = pEvent.p;
	pEvent.p->AddRef();
}

int GrabVideoCap::Grab()
{
    HRESULT hr;
    long EvCode = 0;
    if( !m_pControl )
        return -1;
    hr = m_pControl->Run();
    hr = m_pEvent->WaitForCompletion( INFINITE, &EvCode );
    Sleep(ini_options->ScreenshotWaitMilliseconds());

	m_BitmapInfoHeader		= cbInfo.bih;
	m_BitmapInfo.bmiHeader  = cbInfo.bih;
    m_BitmapBuffer			= cbInfo.pBuffer;
	cbInfo.biSize			= CalcBitmapInfoSize(cbInfo.bih);
	m_BitmapBufferLength	= cbInfo.lBufferSize;
	return 0;
}
 
ULONG GrabVideoCap::CalcBitmapInfoSize(const BITMAPINFOHEADER &bmiHeader)
{
	UINT bmiSize = (bmiHeader.biSize != 0) ? bmiHeader.biSize : sizeof(BITMAPINFOHEADER);
	return bmiSize + bmiHeader.biClrUsed * sizeof(RGBQUAD);
}

HRESULT GrabVideoCap::GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    ULONG ulFound;
    IPin *pPin;
    hr = E_FAIL;

    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        pPin->QueryDirection(&pindir);
        if(pindir == dirrequired)
        {
            if(iNum == 0)
            {
                *ppPin = pPin;
                hr = S_OK;
                break;
            }
            iNum--;
        } 

        pPin->Release();
    } 

    return hr;
}


IPin * GrabVideoCap::GetInPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
    return pComPin;
}


IPin * GrabVideoCap::GetOutPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
    return pComPin;
}

void GrabVideoCap::GetDefaultCapDevice(IBaseFilter **ppCap)
{
	HRESULT hr;

	ASSERT(ppCap);
	if (!ppCap)
		return;
	*ppCap = NULL;
	
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	pCreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
	
	ASSERT(pCreateDevEnum);
	if(!pCreateDevEnum)
		return;

	CComPtr<IEnumMoniker> pEm;
	pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

	ASSERT(pEm);
	if(!pEm)
		return;
	pEm->Reset();

	while (true)
	{
		ULONG ulFetched = 0;
		CComPtr<IMoniker> pM;
		hr = pEm->Next(1, &pM, &ulFetched);
		if(hr != S_OK)
			break;

		hr = pM->BindToObject(0,0,IID_IBaseFilter, (void **)ppCap);
		if(*ppCap)
			break;
	}
	return;
}
