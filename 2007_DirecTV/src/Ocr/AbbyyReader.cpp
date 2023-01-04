#include "StdAfx.h"
#include ".\abbyyreader.h"
#include "Options.h"
#include "common.h"
#include <ctime> 

static HMODULE s_libraryHandle = 0;

AbbyyReader::AbbyyReader(const char * PathName)
	: m_ImageResolution(DEFAULT_IMAGE_RESOLUTION),
	m_IsInitialized(false),
    m_LicenseExpired(false),
    m_bstrPathName(PathName),
    m_bstrIniFileName(PathName)
{
}

AbbyyReader::~AbbyyReader(void)
{
	Shutdown();
}

void AbbyyReader::Shutdown()
{
	if( !UnloadFREngine() ) {
        ACE_ERROR(( LM_CRITICAL, ACE_TEXT( "(%D) Error unloading ABBYY FineReader Engine. \n" ) ));
	}
	m_IsInitialized = false;
}

    /*
	0x2028 - Line break symbol 
	0x2029 - Paragraph break symbol 
	0xFFFC - Object replacement character 
	0x0009 - Tabulation 
	0x005E - Circumflex accent 
	*/ 

long AbbyyReader::ReadBlock( const BITMAPINFO * pInfo, BYTE * pBitmapBuffer, BYTE * pArraysBuffer, UINT16 & arraysSize, BYTE character_properties, BYTE recognition, BYTE language, bool asText )
{
#pragma warning( disable: 4189)
#pragma warning( disable: 4244)
    UINT32 ImageTime;
    UINT32 OCRTime;
    UINT32 PostProcessingTime;

    FREngine::IImageDocumentPtr pImageDoc = 0;
    HBITMAP hBitmap = 0;

    WCHAR * pOCR = 0;
	WCHAR * pBuf = (WCHAR*)pArraysBuffer;
	UCHAR * pArrays = (UCHAR*)pArraysBuffer;
	*pBuf++ = 0;
	*pBuf++ = 0;
	pBuf = (WCHAR*)pArraysBuffer;
	UCHAR * pConf = 0;
	UCHAR * pProps = 0;
	USHORT * pCoordinates = 0;
	USHORT * pFontSizes = 0;
	long strlen = 0;
    UINT16 arrsize = 0;

    if( m_Dummy )
    {
        WCHAR * dummy = L"dummy";
	    strlen = (int)wcslen( dummy );
        arrsize += (UINT16)(strlen * sizeof(WCHAR));
        if( !(recognition & DiscardConfidenceArray) )
            arrsize += (UINT16)strlen;
        if( recognition & ReturnCharacterCoordinatesArray )
            arrsize += (UINT16)(strlen * sizeof(UINT16) * 4);
        if( recognition & ReturnCharacterSizesArray )
            arrsize += (UINT16)(strlen * sizeof(UINT16));
        if( recognition & ReturnCharacterPropertiesArray )
            arrsize += (UINT16)strlen;
        arraysSize = arrsize;
        memset(pBuf,0,arrsize);
        wcscpy(pBuf,dummy);
        return strlen;
    }

    DetectItalics( character_properties & IsItalic );
    ProcessConfidence(!(recognition & DiscardConfidenceArray));
    ProcessSpanish(language & Spanish);

    m_Timer.Start();

    if( pInfo->bmiHeader.biBitCount == 1 ) //Black and White Image
    {
        hBitmap = CreateBitmap((int)pInfo->bmiHeader.biWidth, pInfo->bmiHeader.biHeight, 1, pInfo->bmiHeader.biBitCount, pBitmapBuffer);
        if( !hBitmap )
        {
            return READBLOCK_ERROR;
        }
        #pragma warning( disable: 4311)
        pImageDoc = m_FREngine->OpenBitmapImage( reinterpret_cast<long>(hBitmap), (LONG)m_ImageResolution );
    }
    else    //Color Image
    {
        hBitmap = CreateBitmap((int)pInfo->bmiHeader.biWidth, pInfo->bmiHeader.biHeight, 1, pInfo->bmiHeader.biBitCount, pBitmapBuffer);
        if( !hBitmap )
        {
            return READBLOCK_ERROR;
        }
        #pragma warning( disable: 4311)
        pImageDoc = m_FREngine->PrepareAndOpenBitmap( reinterpret_cast<long>(hBitmap), (LONG)m_ImageResolution, (LONG)m_ImageResolution, m_PrepareImageMode );
    }

    if( pImageDoc == 0 )
        return READBLOCK_ERROR;

    if( options->Debug() && options->DebugImageFilePath() && *options->DebugImageFilePath() )
    	pImageDoc->BlackWhiteImage->WriteToFile( options->DebugImageFilePath(), FREngine::IFF_BmpBwUncompressed, 0, 0 );

    ImageTime = m_Timer.End();
    m_Timer.Start();

    if( asText )
    {
	    FREngine::IPlainTextPtr PlainText = m_FREngine->RecognizeImageDocumentAsPlainText( pImageDoc, m_PageProcessingParams, 0 );
        OCRTime = m_Timer.End();
        m_Timer.Start();

        if( hBitmap )
            DeleteObject(hBitmap);

	    pOCR = PlainText->Text;
	    strlen = (int)wcslen( pOCR );
        if( !strlen ) 
        {
            return NOTEXT_ERROR;
        }
        arrsize += (UINT16)(strlen * sizeof(WCHAR));
	    pArrays += strlen * sizeof(WCHAR);

        if( !(recognition & DiscardConfidenceArray) )
        {
	        pConf = pArrays;
	        pArrays += strlen;
            arrsize += (UINT16)strlen;
        }
        if( recognition & ReturnCharacterCoordinatesArray )
        {
	        pCoordinates = (ACE_UINT16*)pArrays;
            arrsize += (UINT16)(strlen * sizeof(UINT16) * 4);
        }
        arraysSize = arrsize;
	    pBuf = (WCHAR*)pArraysBuffer;

	    for ( int i=0; i<strlen; i++ ){
		    *pBuf = *pOCR++;
		    if ( *pBuf == 0x2028 )  //convert to new line char
			    *pBuf = '\n';
		    else if ( *pBuf == 0x2029 )  //convert to vertical tab char
			    *pBuf = '\n';
            pBuf++;
            if( !(recognition & DiscardConfidenceArray) )
            {
		        *pConf++ = static_cast<UCHAR>(PlainText->CharConfidence[i]);
            }
            if( recognition & ReturnCharacterCoordinatesArray )
            {
                *pCoordinates++ = static_cast<ACE_UINT16>(PlainText->Top[i]);
                *pCoordinates++ = static_cast<ACE_UINT16>(PlainText->Left[i]);
                *pCoordinates++ = static_cast<ACE_UINT16>(PlainText->Bottom[i]);
                *pCoordinates++ = static_cast<ACE_UINT16>(PlainText->Right[i]);
            }
	    }
     }
    else
    {
	    FREngine::ILayoutPtr pLayout = m_FREngine->CreateLayout();

        m_FREngine->AnalyzeAndRecognizePage( pImageDoc, m_PageProcessingParams, pLayout, 0 );
        //m_FREngine->RecognizePage( pImageDoc, m_PageProcessingParams->PageSynthesisParams, pLayout, 0 );

        if( hBitmap )
            DeleteObject(hBitmap);

        OCRTime = m_Timer.End();
        m_Timer.Start();

        FREngine::IBlocksCollectionPtr pBlocksCollection = pLayout->Blocks;
        FREngine::ICharParamsPtr pCharParams = m_FREngine->CreateCharParams();
        FREngine::IBlockPtr pBlock = 0;
	    FREngine::IParagraphsPtr pParagraphs = 0;

        _bstr_t nl = "\n";
        for( long i = 0; i < pBlocksCollection->Count; i++ ) 
        {
            pBlock = pBlocksCollection->Item( i );
	        pParagraphs = pBlock->TextBlockProperties->Text->Paragraphs;

            FREngine::IParagraphPtr pParagraph = pParagraphs->Item(pParagraphs->Count-1);
            pParagraph->GetCharParams( pParagraph->Length, pCharParams );
            pParagraph->Insert( pParagraph->Length, nl, pCharParams );

            for( long j = 0; j < pParagraphs->Count; j++ ) 
            {
                pParagraph = pParagraphs->Item(j);
                strlen += pParagraph->Length;
            }
        }

        if( !strlen ) 
        {
            return NOTEXT_ERROR;
        }
        arrsize += (UINT16)(strlen * sizeof(WCHAR));
	    pArrays += strlen * sizeof(WCHAR);

        if( !(recognition & DiscardConfidenceArray) )
        {
	        pConf = pArrays;
	        pArrays += strlen;
            arrsize += (UINT16)strlen;
        }
        if( recognition & ReturnCharacterCoordinatesArray )
        {
	        pCoordinates = (ACE_UINT16*)pArrays;
	        pArrays += strlen * sizeof(UINT16) * 4;
            arrsize += (UINT16)(strlen * sizeof(UINT16) * 4);
        }
        if( recognition & ReturnCharacterSizesArray )
        {
	        pFontSizes = (ACE_UINT16*)pArrays;
	        pArrays += strlen * sizeof(UINT16);
            arrsize += (UINT16)(strlen * sizeof(UINT16));
        }
        if( recognition & ReturnCharacterPropertiesArray )
        {
	        pProps = pArrays;
            arrsize += (UINT16)strlen;
        }
        arraysSize = arrsize;
	    pBuf = (WCHAR*)pArraysBuffer;

        for( long i = 0; i < pBlocksCollection->Count; i++ ) 
        {
            pBlock = pBlocksCollection->Item( i );
	        pParagraphs = pBlock->TextBlockProperties->Text->Paragraphs;

            for( long j = 0; j < pParagraphs->Count; j++ ) 
            {
                FREngine::IParagraphPtr pParagraph = pParagraphs->Item(j);
                _bstr_t ItemText = pParagraphs->Item(j)->Text;
                WCHAR * pItemText = ItemText;
                for( long k = 0; k < pParagraph->Length; k++ ) 
                {
		            *pBuf = pItemText[k];
		            if ( *pBuf == 0x2028 )  //convert to new line char
			            *pBuf = '\n';
		            else if ( *pBuf == 0x2029 )  //convert to vertical tab char
			            *pBuf = '\n';
                    pBuf++;
                    pParagraph->GetCharParams( k, pCharParams );
                    if( recognition & ReturnCharacterPropertiesArray )
                    {
                        UCHAR props = 0;
                        if( pCharParams->IsBold )
                            props |= IsBold; 
                        if( pCharParams->IsItalic )
                            props |= IsItalic; 
                        if( pCharParams->IsSmallCaps )
                            props |= IsSmallCaps; 
                        if( pCharParams->IsStrikeout )
                            props |= IsStrikeout; 
                        if( pCharParams->IsSubscript )
                            props |= IsSubscript; 
                        if( pCharParams->IsSuperscript )
                            props |= IsSuperscript; 
                        if( pCharParams->IsUnderlined )
                            props |= IsUnderlined; 
                        if( pCharParams->IsSuspicious )
                            props |= IsSuspicious; 
                        *pProps++ = props;
                    }
                    if( recognition & ReturnCharacterCoordinatesArray )
                    {
                        *pCoordinates++ = static_cast<ACE_UINT16>(pCharParams->Top);
                        *pCoordinates++ = static_cast<ACE_UINT16>(pCharParams->Left);
                        *pCoordinates++ = static_cast<ACE_UINT16>(pCharParams->Bottom);
                        *pCoordinates++ = static_cast<ACE_UINT16>(pCharParams->Right);
                    }
                    if( recognition & ReturnCharacterSizesArray )
                    {
                        *pFontSizes++ = static_cast<ACE_UINT16>(pCharParams->FontSize);
                    }
                    if( !(recognition & DiscardConfidenceArray) )
                    {
                        FREngine::IExtendedRecAttributesPtr pERecAttributes = pCharParams->ExtendedRecAttributes;
                        if( pERecAttributes != 0 ) 
		                    *pConf++ = static_cast<UCHAR>(pERecAttributes->CharConfidence);
                    }
                }
            }
        }
    }

    PostProcessingTime = m_Timer.End();

    if( options->Debug() && options->Verbose() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Engine Times --- image: %d - ocr: %d  - text: %d \n" ), ImageTime, OCRTime, PostProcessingTime ));
	return strlen;
}


void AbbyyReader::Initialize(const char * PathName, HWND hwnd, bool debug, bool verbose, bool dummy)
{
	if( m_IsInitialized == true )
		return;

    m_bstrPathName = PathName;
	m_bstrIniFileName = PathName;
	m_Debug = debug;
	m_Verbose = verbose;
	m_IsInitialized = false;
	m_Window = hwnd;
	m_bstrIniFileName.Append(".ini");

	if( !LoadFREngine() ) {
		throw OCRException("Error while loading ABBYY FineReader Engine");
	}

	if(m_Debug)
		m_FREngine->StartLogging( m_bstrPathName + L"OCREngine.log", m_Verbose ? VARIANT_TRUE : VARIANT_FALSE );

    #pragma warning( disable: 4311)
	m_FREngine->ParentWindow = reinterpret_cast<long>(m_Window);
	m_FREngine->ApplicationTitle = "AbbyyReader";

	if( FILE_NOT_FOUND == GetFileAttributesW(m_bstrPathName + L".ini") )
		throw OCRException("ABBYY FineReader Engine INI file not found!");

	m_FREngine->LoadProfile( m_bstrPathName + L"Engine.ini" );

    m_Dummy = dummy;

    long RemainingUnits = m_FREngine->CurrentLicense->RemainingUnits;
    long RelativeDays = m_FREngine->CurrentLicense->RelativeDays;
    long y, m, d;
    m_FREngine->CurrentLicense->ExpirationDate( &y, &m, &d );

    SYSTEMTIME st;
    GetSystemTime(&st);

    struct std::tm current       = {0,st.wMinute,st.wHour,st.wDay,st.wMonth,st.wYear - 1900};
    struct std::tm expiration    = {0,0,0,d,m,y - 1900};
    double difference = -1;
    std::time_t cur = std::mktime(&current);
    std::time_t exp = std::mktime(&expiration);
    if ( cur != (std::time_t)(-1) && exp != (std::time_t)(-1) )
        difference = std::difftime(exp, cur) / (60 * 60 * 24);

    //m_LicenseExpired = difference > 0 ? false : true;
    m_LicenseExpired = false;

	m_FREngine->LoadModule( FREngine::FREM_ImageSupport );
	m_FREngine->LoadModule( FREngine::FREM_Recognizer );
	m_FREngine->LoadModule( FREngine::FREM_DocumentAnalyzer );

    m_PrepareImageMode = m_FREngine->CreatePrepareImageMode();
    m_ImageProcessingParams = m_FREngine->CreateImageProcessingParams();
	m_PageProcessingParamsDefault = m_FREngine->CreatePageProcessingParams();
	m_PageProcessingParams = m_FREngine->CreatePageProcessingParams();

    //MakeTextLanguages();

    if( options->TrainAbbyy() )
    {
	    m_PageProcessingParamsDefault->RecognizerParams->TrainUserPatterns = VARIANT_TRUE;
	    if( FILE_NOT_FOUND == GetFileAttributesW(m_bstrPathName + L".pat") )
		    m_FREngine->CreateEmptyUserPattern( m_bstrPathName + L".pat" );
	    m_PageProcessingParamsDefault->RecognizerParams->UserPatternsFile = m_bstrPathName + L".pat";
	    m_PageProcessingParams->RecognizerParams->TrainUserPatterns = VARIANT_TRUE;
	    m_PageProcessingParams->RecognizerParams->UserPatternsFile = m_bstrPathName + L".pat";
    }
    else
    {
    	m_PageProcessingParamsDefault->RecognizerParams->TrainUserPatterns = VARIANT_FALSE;
    	m_PageProcessingParams->RecognizerParams->TrainUserPatterns = VARIANT_FALSE;
    }

    //################################################################################################
    //This object contains different attributes specifying how an image 
    //will be prepared during conversion to the internal format
    // -> PrepareImage PrepareBitmap PrepareAndOpenImage
    m_PrepareImageMode->SmoothColorImage = VARIANT_FALSE; //Default TRUE
    m_PrepareImageMode->RemoveGarbage = VARIANT_TRUE; //Default FALSE
    m_PrepareImageMode->CorrectSkew = VARIANT_FALSE; //Default TRUE
        m_PrepareImageMode->CalcSkewByBlackSquares = VARIANT_FALSE;
    m_PrepareImageMode->DiscardColorImage = VARIANT_FALSE;
    m_PrepareImageMode->CreatePreview = VARIANT_FALSE;
    m_PrepareImageMode->InvertImage = VARIANT_FALSE;
    m_PrepareImageMode->MirrorImage = VARIANT_FALSE;

    //################################################################################################
    //This object specifies how an image will be preprocessed before analysis and recognition.
    // -> AnalyzePage AnalyzeAndRecognizeBlocks AnalyzeAndRecognizePage AnalyzeRegion 
    m_ImageProcessingParams->RemoveTexture = VARIANT_FALSE; //Default TRUE
    m_ImageProcessingParams->AutodetectInversion = VARIANT_FALSE;
    m_ImageProcessingParams->InvertImage = VARIANT_FALSE;
    m_ImageProcessingParams->MirrorImage = VARIANT_FALSE;
    m_ImageProcessingParams->RemoveGarbage = VARIANT_FALSE;
    m_ImageProcessingParams->ProhibitCorrectLocalSkew = VARIANT_TRUE;

    //          m_PageProcessingParamsDefault
    // ->  AnalyzeAndRecognizePage, AnalyzePage, AnalyzeRegion, RecognizeImageDocumentAsPlainText
    //################################################################################################
    //This object is used for setting up the parameters of the recognized text synthesis.
    // -> AnalyzeAndRecognizeBlocks RecognizeBlocks RecognizePage
    //m_PageProcessingParamsDefault->PageSynthesisParams->DetectItalic = VARIANT_TRUE; 
    m_PageProcessingParamsDefault->PageSynthesisParams->FormatWithSpaces = VARIANT_FALSE; 
    //m_PageProcessingParamsDefault->PageSynthesisParams->FormatWithSpaces = VARIANT_TRUE; 
    m_PageProcessingParamsDefault->PageSynthesisParams->ParagraphExtractionMode = FREngine::PEM_RoughExtraction;
//PEM_NormalExtraction PEM_RoughExtraction PEM_SingleLineParagraphsWithSpaceFormatting PEM_SingleLineParagraphsWithWordSeparationOnly

    m_PageProcessingParamsDefault->PageSynthesisParams->DetectItalic = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->PageSynthesisParams->CorrectDynamicRange = VARIANT_FALSE; 
        //If this property is TRUE, image colors will be corrected so that the background is white 
        //and the text is black, or vice versa, which improves image quality. Recognition, however, 
        //will slow down. We recommend using this property only if the DetectBackgroundColor and 
        //DetectTextColor properties are TRUE. This property is set to FALSE by default.
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectBackgroundColor = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectBold = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectDropCaps = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectFontSize = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectSerifs = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectSmallCaps = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectSubscriptsSuperscripts = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectUnderlineStrikeout = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->DetectTextColor = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageSynthesisParams->ExtractBlackSeparators  = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->PageSynthesisParams->HighlightHyperlinks = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->PageSynthesisParams->InsertEmptyParagraphsForBigInterlines = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->PageSynthesisParams->KeepBullets = VARIANT_FALSE; 

    //################################################################################################
    //This object provides access to parameters used for tuning the page layout analysis process. 
    m_PageProcessingParamsDefault->PageAnalysisParams->DetectPictures = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageAnalysisParams->DetectTables = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitColorImage = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitClockwiseRotation = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitCounterclockwiseRotation = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitUpsidedownRotation = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->PageAnalysisParams->RemoveTexture = VARIANT_FALSE; //Default TRUE
    m_PageProcessingParamsDefault->PageAnalysisParams->SingleColumnMode = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->PageAnalysisParams->DetectBarcodes = VARIANT_FALSE;
    m_PageProcessingParamsDefault->PageAnalysisParams->DetectInvertedImage = VARIANT_FALSE;
    m_PageProcessingParamsDefault->PageAnalysisParams->DetectOrientation = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitModelAnalysis = VARIANT_TRUE;
        //If this property is FALSE, typical variants of page layout will be gone through during page analysis 
        //and the best variant will be selected, which can improve recognition quality. If the best variant of 
        //page layout cannot be selected, standard page layout analysis will be performed.
    m_PageProcessingParamsDefault->PageAnalysisParams->FastObjectsExtraction = VARIANT_TRUE; //Default FALSE
        //If this property is TRUE, layout analysis will speed up, but its quality may deteriorate. The value 
        //of this property is only relevant if the ProhibitModelAnalysis property is TRUE
    m_PageProcessingParamsDefault->PageAnalysisParams->FlexiFormsDA = VARIANT_FALSE;
        //This property set to TRUE tells ABBYY FineReader Engine to locate all text on the page, 
        //including small text areas of low quality and text in diagrams and pictures.
    //Check this one when OCR FAILS SMALL BITMAPS
    m_PageProcessingParamsDefault->PageAnalysisParams->FullTextIndexDA = VARIANT_FALSE;
        //This property set to TRUE tells ABBYY FineReader Engine to detect all text on an image, 
        //including text embedded into the image.

    //################################################################################################
    m_PageProcessingParamsDefault->RecognizerParams->FastMode = VARIANT_FALSE; 
    //m_PageProcessingParamsDefault->RecognizerParams->FastMode = VARIANT_TRUE; //not very good
        //This property set to TRUE provides 2-2.5 times faster recognition speed at the cost of a 
        //moderately increased error rate (1.5-2 times more errors). 
    m_PageProcessingParamsDefault->RecognizerParams->BalancedMode  = VARIANT_FALSE;  
        //If this property is TRUE, the recognition will run in balanced mode. The balanced mode is an 
        //intermediate mode between full and fast modes.
    //This one should be tested with on and off
    m_PageProcessingParamsDefault->RecognizerParams->DisableSecondStageRecognizer = VARIANT_FALSE; 
    //m_PageProcessingParamsDefault->RecognizerParams->DisableSecondStageRecognizer = VARIANT_TRUE; 
        //If this property is set to TRUE, no second-stage recognition will be performed. The second stage 
        //of recognition is optional during recognition of small areas on the image, like in the FieldLevelRecognition 
        //sample. This function is needed to speed up recognition process. If this stage is skipped during recognition 
        //of full-page images, however, recognition quality may get worse. 
    //This one should change depending on what we're recognizing
    //m_PageProcessingParamsDefault->RecognizerParams->ProhibitItalic = VARIANT_FALSE; //Default FALSE
    m_PageProcessingParamsDefault->RecognizerParams->ProhibitItalic = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->RecognizerParams->ProhibitHyphenation = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->RecognizerParams->ProhibitInterblockHyphenation = VARIANT_TRUE; //Default FALSE
    //careful with this one FALSE == Use patterns from pattern file ONLY
    m_PageProcessingParamsDefault->RecognizerParams->UseBuiltInPatterns = VARIANT_TRUE;  
    //m_PageProcessingParamsDefault->RecognizerParams->TrainUserPatterns = VARIANT_FALSE;
    m_PageProcessingParamsDefault->RecognizerParams->OneLinePerBlock = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->RecognizerParams->OneWordPerLine = VARIANT_FALSE; 
    m_PageProcessingParamsDefault->RecognizerParams->ProhibitSubscript = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->RecognizerParams->ProhibitSuperscript = VARIANT_TRUE; //Default FALSE
    m_PageProcessingParamsDefault->RecognizerParams->ExactConfidenceCalculation = VARIANT_TRUE; //Default FALSE 
    m_PageProcessingParamsDefault->RecognizerParams->SaveCharacterRecognitionVariants  = VARIANT_FALSE;
    m_PageProcessingParamsDefault->RecognizerParams->SaveWordRecognitionVariants = VARIANT_FALSE;
  
    //m_PageProcessingParamsDefault->RecognizerParams->CJKTextDirection = FREngine::CJKTD_Horizontal;
	    //CJKTD_Horizontal CJKTD_Vertical CJKTD_Autodetect

	//m_PageProcessingParamsDefault->RecognizerParams->TextLanguage = m_EnglishTextLanguage;

    ResetAllDefaults();

    m_IsInitialized = true;
}

void AbbyyReader::ResetAllDefaults()
{
    m_PageProcessingParams->PageSynthesisParams->DetectItalic = m_PageProcessingParamsDefault->PageSynthesisParams->DetectItalic;
    m_PageProcessingParams->PageSynthesisParams->CorrectDynamicRange = m_PageProcessingParamsDefault->PageSynthesisParams->CorrectDynamicRange;
    m_PageProcessingParams->PageSynthesisParams->DetectBackgroundColor = m_PageProcessingParamsDefault->PageSynthesisParams->DetectBackgroundColor;
    m_PageProcessingParams->PageSynthesisParams->DetectBold = m_PageProcessingParamsDefault->PageSynthesisParams->DetectBold;
    m_PageProcessingParams->PageSynthesisParams->DetectDropCaps = m_PageProcessingParamsDefault->PageSynthesisParams->DetectDropCaps;
    m_PageProcessingParams->PageSynthesisParams->DetectFontSize = m_PageProcessingParamsDefault->PageSynthesisParams->DetectFontSize;
    m_PageProcessingParams->PageSynthesisParams->DetectSerifs = m_PageProcessingParamsDefault->PageSynthesisParams->DetectSerifs;
    m_PageProcessingParams->PageSynthesisParams->DetectSmallCaps = m_PageProcessingParamsDefault->PageSynthesisParams->DetectSmallCaps;
    m_PageProcessingParams->PageSynthesisParams->DetectSubscriptsSuperscripts = m_PageProcessingParamsDefault->PageSynthesisParams->DetectSubscriptsSuperscripts;
    m_PageProcessingParams->PageSynthesisParams->DetectUnderlineStrikeout = m_PageProcessingParamsDefault->PageSynthesisParams->DetectUnderlineStrikeout;
    m_PageProcessingParams->PageSynthesisParams->DetectTextColor = m_PageProcessingParamsDefault->PageSynthesisParams->DetectTextColor;
    m_PageProcessingParams->PageSynthesisParams->ExtractBlackSeparators  = m_PageProcessingParamsDefault->PageSynthesisParams->ExtractBlackSeparators;
    m_PageProcessingParams->PageSynthesisParams->FormatWithSpaces = m_PageProcessingParamsDefault->PageSynthesisParams->FormatWithSpaces;
    m_PageProcessingParams->PageSynthesisParams->HighlightHyperlinks = m_PageProcessingParamsDefault->PageSynthesisParams->HighlightHyperlinks;
    m_PageProcessingParams->PageSynthesisParams->InsertEmptyParagraphsForBigInterlines = m_PageProcessingParamsDefault->PageSynthesisParams->InsertEmptyParagraphsForBigInterlines;
    m_PageProcessingParams->PageSynthesisParams->KeepBullets = m_PageProcessingParamsDefault->PageSynthesisParams->KeepBullets;
    m_PageProcessingParams->PageSynthesisParams->FormatWithSpaces = m_PageProcessingParamsDefault->PageSynthesisParams->FormatWithSpaces; 
    m_PageProcessingParams->PageSynthesisParams->ParagraphExtractionMode = m_PageProcessingParamsDefault->PageSynthesisParams->ParagraphExtractionMode;

    m_PageProcessingParams->PageAnalysisParams->DetectPictures = m_PageProcessingParamsDefault->PageAnalysisParams->DetectPictures;
    m_PageProcessingParams->PageAnalysisParams->DetectTables = m_PageProcessingParamsDefault->PageAnalysisParams->DetectTables;
    m_PageProcessingParams->PageAnalysisParams->ProhibitColorImage = m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitColorImage;
    m_PageProcessingParams->PageAnalysisParams->ProhibitClockwiseRotation = m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitClockwiseRotation;
    m_PageProcessingParams->PageAnalysisParams->ProhibitCounterclockwiseRotation = m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitCounterclockwiseRotation;
    m_PageProcessingParams->PageAnalysisParams->ProhibitUpsidedownRotation = m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitUpsidedownRotation;
    m_PageProcessingParams->PageAnalysisParams->RemoveTexture = m_PageProcessingParamsDefault->PageAnalysisParams->RemoveTexture;
    m_PageProcessingParams->PageAnalysisParams->SingleColumnMode = m_PageProcessingParamsDefault->PageAnalysisParams->SingleColumnMode;
    m_PageProcessingParams->PageAnalysisParams->DetectBarcodes = m_PageProcessingParamsDefault->PageAnalysisParams->DetectBarcodes;
    m_PageProcessingParams->PageAnalysisParams->DetectInvertedImage = m_PageProcessingParamsDefault->PageAnalysisParams->DetectInvertedImage;
    m_PageProcessingParams->PageAnalysisParams->DetectOrientation = m_PageProcessingParamsDefault->PageAnalysisParams->DetectOrientation;
    m_PageProcessingParams->PageAnalysisParams->ProhibitModelAnalysis = m_PageProcessingParamsDefault->PageAnalysisParams->ProhibitModelAnalysis;
    m_PageProcessingParams->PageAnalysisParams->FastObjectsExtraction = m_PageProcessingParamsDefault->PageAnalysisParams->FastObjectsExtraction;
    m_PageProcessingParams->PageAnalysisParams->FlexiFormsDA = m_PageProcessingParamsDefault->PageAnalysisParams->FlexiFormsDA;
    m_PageProcessingParams->PageAnalysisParams->FullTextIndexDA = m_PageProcessingParamsDefault->PageAnalysisParams->FullTextIndexDA;

    m_PageProcessingParams->RecognizerParams->FastMode = m_PageProcessingParamsDefault->RecognizerParams->FastMode;
    m_PageProcessingParams->RecognizerParams->BalancedMode  = m_PageProcessingParamsDefault->RecognizerParams->BalancedMode;
    m_PageProcessingParams->RecognizerParams->DisableSecondStageRecognizer = m_PageProcessingParamsDefault->RecognizerParams->DisableSecondStageRecognizer;
    m_PageProcessingParams->RecognizerParams->ProhibitItalic = m_PageProcessingParamsDefault->RecognizerParams->ProhibitItalic;
    m_PageProcessingParams->RecognizerParams->ProhibitHyphenation = m_PageProcessingParamsDefault->RecognizerParams->ProhibitHyphenation;
    m_PageProcessingParams->RecognizerParams->ProhibitInterblockHyphenation = m_PageProcessingParamsDefault->RecognizerParams->ProhibitInterblockHyphenation;
    m_PageProcessingParams->RecognizerParams->UseBuiltInPatterns = m_PageProcessingParamsDefault->RecognizerParams->UseBuiltInPatterns;
    //m_PageProcessingParams->RecognizerParams->TrainUserPatterns = m_PageProcessingParamsDefault->RecognizerParams->TrainUserPatterns;
    m_PageProcessingParams->RecognizerParams->OneLinePerBlock = m_PageProcessingParamsDefault->RecognizerParams->OneLinePerBlock;
    m_PageProcessingParams->RecognizerParams->OneWordPerLine = m_PageProcessingParamsDefault->RecognizerParams->OneWordPerLine;
    m_PageProcessingParams->RecognizerParams->ProhibitSubscript = m_PageProcessingParamsDefault->RecognizerParams->ProhibitSubscript;
    m_PageProcessingParams->RecognizerParams->ProhibitSuperscript = m_PageProcessingParamsDefault->RecognizerParams->ProhibitSuperscript ;
    m_PageProcessingParams->RecognizerParams->ExactConfidenceCalculation = m_PageProcessingParamsDefault->RecognizerParams->ExactConfidenceCalculation;
    m_PageProcessingParams->RecognizerParams->SaveCharacterRecognitionVariants  = m_PageProcessingParamsDefault->RecognizerParams->SaveCharacterRecognitionVariants;
    m_PageProcessingParams->RecognizerParams->SaveWordRecognitionVariants = m_PageProcessingParamsDefault->RecognizerParams->SaveWordRecognitionVariants;
	m_PageProcessingParams->RecognizerParams->TextLanguage = m_PageProcessingParamsDefault->RecognizerParams->TextLanguage;
	//m_PageProcessingParams->RecognizerParams->CJKTextDirection = m_PageProcessingParamsDefault->RecognizerParams->CJKTextDirection;
}

void AbbyyReader::ResetDefaults()
{
    m_PageProcessingParams->PageSynthesisParams->DetectItalic = m_PageProcessingParamsDefault->PageSynthesisParams->DetectItalic;
    m_PageProcessingParams->RecognizerParams->ProhibitItalic = m_PageProcessingParamsDefault->RecognizerParams->ProhibitItalic;
    m_PageProcessingParams->RecognizerParams->ExactConfidenceCalculation = m_PageProcessingParamsDefault->RecognizerParams->ExactConfidenceCalculation;
	m_PageProcessingParams->RecognizerParams->TextLanguage = m_PageProcessingParamsDefault->RecognizerParams->TextLanguage;
	//m_PageProcessingParams->RecognizerParams->CJKTextDirection = m_PageProcessingParamsDefault->RecognizerParams->CJKTextDirection;
}

void AbbyyReader::ProcessSpanish(bool op)
{
    //m_PageProcessingParams->RecognizerParams->TextLanguage = op ? m_SpanishTextLanguage : m_EnglishTextLanguage;
    m_PageProcessingParams->RecognizerParams->SetPredefinedTextLanguage( op ? L"Spanish" : L"English" );
}

void AbbyyReader::DetectItalics(bool op)
{
    m_PageProcessingParams->PageSynthesisParams->DetectItalic = op ? VARIANT_TRUE : VARIANT_FALSE;
    m_PageProcessingParams->RecognizerParams->ProhibitItalic = !op ? VARIANT_TRUE : VARIANT_FALSE;
}

void AbbyyReader::ProcessConfidence(bool op)
{
    m_PageProcessingParams->RecognizerParams->ExactConfidenceCalculation = op ? VARIANT_TRUE : VARIANT_FALSE;
}

bool AbbyyReader::LoadFREngine()
{
	if( m_FREngine != 0 ) {
		// Already loaded
		return true;
	}

	USES_CONVERSION;
	LPSTR pszIniFileName = OLE2A (m_bstrIniFileName.m_str);

	const int BufferLen = MAX_PATH;
	static wchar_t BufferW[BufferLen + 1];
	static char Buffer[BufferLen + 1];
	Buffer[0] = 0;
	BufferW[0] = 0;

	int Len = GetPrivateProfileString( "Server", "EngineDLLPath", "", Buffer, BufferLen, pszIniFileName);
	if( Len == 0 ) {
        throw OCRException("'EngineDLLPath' not found in [Server] in INI file");
	}
	int convertedLen = MultiByteToWideChar( CP_ACP, 0, Buffer, -1, BufferW, BufferLen );
	if( convertedLen == 0 ) {
		BufferW[0] = 0;
        throw OCRException("Error translating 'EngineDLLPath'");
	}

	// First step: load FREngine.dll
	if( s_libraryHandle == 0 ) {
		USES_CONVERSION;
		LPSTR pszAbbyyEngineDLL = OLE2A (BufferW);
		s_libraryHandle = LoadLibraryEx( pszAbbyyEngineDLL, 0, LOAD_WITH_ALTERED_SEARCH_PATH );
		if( s_libraryHandle == 0 ) {
            throw OCRException("Error loading Engine DLL");
		}
	}

	Buffer[0] = 0;
	BufferW[0] = 0;

	Len = GetPrivateProfileString( "Server", "SerialNumber", "", Buffer, BufferLen, pszIniFileName);
	if( Len == 0 ) {
        throw OCRException("'SerialNumber' not found in [Server] in INI file");
	}
	convertedLen = MultiByteToWideChar( CP_ACP, 0, Buffer, -1, BufferW, BufferLen );
	if( convertedLen == 0 ) {
		BufferW[0] = 0; // a problem during translation
        throw OCRException("Error translating 'SerialNumber'");
	}

	// Second step: obtain FineReader Engine object
	typedef HRESULT (STDAPICALLTYPE* GetEngineObjectFunc)( BSTR, BSTR, BSTR, FREngine::IEngine** );
	GetEngineObjectFunc pGetEngineObject = (GetEngineObjectFunc)GetProcAddress( s_libraryHandle, "GetEngineObject" );

	if( pGetEngineObject == 0 || pGetEngineObject( BufferW, 0, 0, &m_FREngine ) != S_OK ) {
		UnloadFREngine();
        throw OCRException("Error loading Abbyy Engine Object");
	}
	return true;
}

bool AbbyyReader::UnloadFREngine()
{
	if( s_libraryHandle == 0 ) {
		return true;
	}
	
	// Release Engine object
	if( m_FREngine != 0 ) {
		m_FREngine.Release();
	}

	// Check if FREngine.dll can be unloaded
	typedef HRESULT (STDAPICALLTYPE* DeinitializeEngineFunc)();
	DeinitializeEngineFunc pDeinitializeEngine = (DeinitializeEngineFunc)GetProcAddress( s_libraryHandle, "DeinitializeEngine" );

	if( pDeinitializeEngine == 0 || pDeinitializeEngine() != S_OK ) {
        throw OCRException("Error Deinitializing Abbyy Engine");
	}

	// Now we can free library safely
	FreeLibrary( s_libraryHandle );
	s_libraryHandle = 0;

	return true;
}
