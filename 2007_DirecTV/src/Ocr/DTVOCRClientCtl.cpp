// DTVOCRClientCtl.cpp : Implementation of CDTVOCRClientCtl
#include "stdafx.h"
#include "DTVOCRClientCtl.h"
#include ".\dtvocrclientctl.h"
#include <comdef.h>
 
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

#include "ace/Service_Config.h"
#include "ace/Get_Opt.h"
#include "ace/ARGV.h"
using namespace Magick;

#include "common.h"

#define CLEAR_ERRORS m_ErrorCode = DTVOCRC_SUCCESS; m_bstrErrorMessage.Empty(); SetLastError(0);

std::string exception_error;
#define HANDLE_EXCEPTIONS( function ) \
    catch( _com_error& e ) \
    { \
		exception_error = e.Description(); \
		exception_error += ""; \
		exception_error += e.ErrorMessage(); \
	}catch(ParamsException e){ \
		exception_error = e.m_ErrorStatus; \
	}catch(ACEException e){ \
		exception_error = e.m_ErrorStatus; \
	}catch(WindowsException &e){ \
		exception_error = e.m_ErrorStatus; \
	}catch(VidCapException &e){ \
		exception_error = e.m_ErrorStatus; \
	}catch(Exception & e){ \
		exception_error = e.what(); \
	}catch(exception& e){ \
		exception_error = e.what(); \
	}catch(...){ \
        exception_error = "Unknown Exception: "; \
        exception_error += GetLastErrorMsg(GetLastError()); \
	} \
    m_ErrorCode = -1; \
    m_bstrErrorMessage = exception_error.c_str(); \
    ACE_ERROR(( LM_ERROR, ACE_TEXT( "(%P|%D) !!! Exception in %s: %s\n" ), #function, exception_error.c_str() ) );

bool Geometry_greaterthan(const Geometry& a, const Geometry& b) {
    int aw = a.width();
    int bw = b.width();
    int ah = a.height();
    int bh = b.height();
    int wdiff = abs(aw-bw);
    int hdiff = abs(ah-bh);
    //width is more important, but height should be considered if width difference is too small
//ACE_ERROR(( LM_ERROR, ACE_TEXT( "<op difs w:%d h:%d\n" ), wdiff, hdiff ) );
    if( wdiff > hdiff )
        return aw > bw;
    else
        return ah > bh;
}

bool FoundIcon_lessthan(const FoundIcon* a, const FoundIcon* b) {

    const Geometry * pga = a->Geometry();
    const Geometry * pgb = b->Geometry();

    UINT ay1, ay2,  by1, by2;

    //y
    ay2 = pga->yOff() + pga->height();
    by1 = pgb->yOff();

    //if b is below a
    if( by1 > ay2 ) //then, a is less than b (is above)
        return true;

    ay1 = pga->yOff();
    by2 = pgb->yOff() + pgb->height();

    //if a is below b
    if( ay1 > by2 ) //then, b is less than a (is above)
        return false; 

    //do y's overlap, if so, check x values...
    if( (ay1 >= by1 && ay1 <= by2) || //first y is between second y's
        (by1 >= ay1 && by1 <= ay2) )   //second y is between first y's
    {
        //here whichever icon x1 is first, is less than...
        return (pga->xOff() < pgb->xOff());
    }
    return false;
}


STDMETHODIMP CDTVOCRClientCtl::Test(void)
{
	try{

        LoadImageFile(L"C:\\DTVTest\\AutomationClient\\crops\\IconsInfoCrop.bmp");

        Crop(0,0,159,24); //the info area...

        WhichIcons(10);
        //so now we have matching icons in the array
        //for the icons that were not found, they could be icons with more than 1 artifacts, like the speaker icon
        //go through the array and see if we can merge the icons, we know that we can merge them if they are close
        //enough to each other, if we can merge them, then we will copy them into a new array and work from there

	    return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Test )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::HasIcon(USHORT iconID)
{
	try{
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n@@@CDTVOCRClientCtl::HasIcon( %d )\n" ),iconID) );

        UINT32 opTime = 0;
        m_Timer.Start();

        CLEAR_ERRORS
        FindBoundingSquaresInCrop(iconID,0);
        FindMatchingSinglePartIcons(iconID,0);
        FindMatchingMultiPartIcons(iconID,0);
        DeleteDuplicateCoordinatesFromResult();

        for( int i=0; i<m_ArtefactIconIDs.size(); i++ )
        {
            if( m_ArtefactIconIDs[i] == iconID )
            {
                Geometry * pGeometry = &m_IconsGeometry[i];
                m_x1 = pGeometry->xOff();
                m_y1 = pGeometry->yOff();
                m_x2 = pGeometry->xOff() + pGeometry->width();
                m_y2 = pGeometry->yOff() + pGeometry->height();
                if( ini_options->Debug() )
                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "(%D) Found Icon #%d ID:%d name:%s @ x1:%d y1:%d-x2:%d y2:%d \n" ), i, iconID, m_ImageMagick.GetIconName(iconID), m_x1, m_y1, m_x2, m_y2 ) );
            }
        }
        opTime = m_Timer.End();
        m_Time = opTime;
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n$$$CDTVOCRClientCtl::HasIcon() --- times:%d ms\n\n" ),m_Time) );
	    return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::HasIcon )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::WhichIcons(USHORT groupID)
{
	try{
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n@@@CDTVOCRClientCtl::WhichIcons( %d )\n" ),groupID) );

        UINT32 opTime = 0;
        m_Timer.Start();

        CLEAR_ERRORS
        m_WhichIconsQuantity = 0;
        FindBoundingSquaresInCrop(0,groupID);
        FindMatchingSinglePartIcons(0,groupID);
        FindMatchingMultiPartIcons(0,groupID);
        DeleteDuplicateCoordinatesFromResult();
        SortResults();
        m_WhichIconsQuantity = m_ArtefactIconIDs.size();

        opTime = m_Timer.End();
        m_Time = opTime;
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n$$$CDTVOCRClientCtl::WhichIcons() --- times:%d ms\n\n" ),m_Time) );
	    return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::WhichIcons )
    return S_FALSE;
}

void CDTVOCRClientCtl::SortResults(void) {

    int size = m_FoundIcons.size();
    for( int i=0; i<size; ++i ) 
    {
        FoundIcon * p = m_FoundIcons[i];
        delete p;
    }
    m_FoundIcons.clear();

    size = m_ArtefactIconIDs.size();
    for( int i=0; i<size; ++i ) 
    {
        FoundIcon * p = new FoundIcon( m_ArtefactIconIDs[i], m_IconsConfidenceLevels[i], m_IconsGeometry[i] );
        m_FoundIcons.push_back( p );
    }

    sort( m_FoundIcons.begin(), m_FoundIcons.end(), FoundIcon_lessthan );
}

STDMETHODIMP CDTVOCRClientCtl::WhichIconsGetCoordinate(USHORT index)
{
	try{
        CLEAR_ERRORS
        int size = m_FoundIcons.size();
        if( index >= size )
            return S_FALSE;

        m_x1 = 0;
        m_y1 = 0;
        m_x2 = 0;
        m_y2 = 0;

        const Geometry * pGeometry = 0;
        m_pFoundIcon = m_FoundIcons[index];
        if( m_pFoundIcon ) {
            pGeometry = m_pFoundIcon->Geometry();
            m_x1 = pGeometry->xOff();
            m_y1 = pGeometry->yOff();
            m_x2 = pGeometry->xOff() + pGeometry->width();
            m_y2 = pGeometry->yOff() + pGeometry->height();
        }

        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::WhichIconsGetCoordinate )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::WhichIconsGetIconID(USHORT index)
{
	try{
        CLEAR_ERRORS
        m_WhichIconsCurrentID = 0;

        int size = m_FoundIcons.size();
        if( index >= size )
            return S_FALSE;

        m_pFoundIcon = m_FoundIcons[index];
        if( m_pFoundIcon )
            m_WhichIconsCurrentID = m_pFoundIcon->ID();

        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::WhichIconsGetIconID )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::WhichIconsGetConfidenceLevel(USHORT index)
{
	try{
        CLEAR_ERRORS
        m_ConfidenceLevel = 0;

        int size = m_FoundIcons.size();
        if( index >= size )
            return S_FALSE;

        m_pFoundIcon = m_FoundIcons[index];
        if( m_pFoundIcon )
            m_ConfidenceLevel = m_pFoundIcon->ConfidenceLevel();

        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::WhichIconsGetConfidenceLevel )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::WhichIconsGetParams(USHORT index)
{
	try{
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n@@@CDTVOCRClientCtl::WhichIconsGetParams( %d )\n" ),index) );
        CLEAR_ERRORS
        m_WhichIconsCurrentID = 0;
        m_ConfidenceLevel = 0;
        m_x1 = 0;
        m_y1 = 0;
        m_x2 = 0;
        m_y2 = 0;

        int size = m_FoundIcons.size();
        if( index >= size )
            return S_FALSE;

        const Geometry * pGeometry = 0;
        m_pFoundIcon = m_FoundIcons[index];
        if( m_pFoundIcon ) {
            pGeometry = m_pFoundIcon->Geometry();
            m_x1 = pGeometry->xOff();
            m_y1 = pGeometry->yOff();
            m_x2 = pGeometry->xOff() + pGeometry->width();
            m_y2 = pGeometry->yOff() + pGeometry->height();
            m_ConfidenceLevel = m_pFoundIcon->ConfidenceLevel();
            m_WhichIconsCurrentID = m_pFoundIcon->ID();
        }

        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::WhichIconsGetParams )
    return S_FALSE;
}

void CDTVOCRClientCtl::DeleteDuplicateCoordinatesFromResult(void)
{
    int delete_count = 0;
    int size = m_IconsGeometryAll.size();
    for( int i=0; i<size; ++i ) 
    {
        Geometry * pGeometry = &(m_IconsGeometryAll[i]);
        if( !pGeometry || pGeometry->width() == 0 ) {
            continue;
        }

        //get current coordenates
        USHORT x1 = pGeometry->xOff();
        USHORT y1 = pGeometry->yOff();
        USHORT x2 = x1 + pGeometry->width();
        USHORT y2 = y1 + pGeometry->height();
        //walk the array searching for an very overlapping icon
        for( int j=i+1; j<size; ++j ) {
            Geometry * pNewGeometry = &(m_IconsGeometryAll[j]);
            if( !pNewGeometry || pNewGeometry->width() == 0 ) continue;
            USHORT nx1 = pNewGeometry->xOff();
            USHORT ny1 = pNewGeometry->yOff();
            USHORT nx2 = nx1 + pNewGeometry->width();
            USHORT ny2 = ny1 + pNewGeometry->height();

            bool compare; //use the one with highest confidence level, discard the others...

            if ( x1 == nx1 && x2 == nx2 && y1 == ny1 && y2 == ny2 ) 
            { //equal coordinates
                compare = true;
            }
            else if ( (x1  <= nx1 && x2 >= nx2 && y1 <= ny1 && y2 >= ny2) || 
                      (nx1 <= x1  && nx2 >= x2 && ny1 <= y1 && ny2 >= y2) ) 
            { // full area containment
                compare = true;
            } 
            else if ((x1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > nx2 || (nx1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > x2 || 
                (y1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > ny2 || (ny1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > y2) 
            { //no collision, by actual collision or proximity
                //ok, each one is separate
                compare = false;
            } 
            else 
            { //collision
                compare = true;
            }

            if( compare == true )
            {
                delete_count++;
                //NOTE: increasing by 100 will basically somehow disable it, it will not be copied...
                if( m_IconsConfidenceLevelsAll[i] <= 100 )
                {
                    if( m_IconsConfidenceLevelsAll[i] >= m_IconsConfidenceLevelsAll[j] )
                    {
                        m_IconsConfidenceLevelsAll[j] += 100;
                    }
                    else
                    {
                        m_IconsConfidenceLevelsAll[i] += 100;
                    }
                }
            }
        }
    }     
    if( delete_count && ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "---Deleted %d duplicate results\n" ),delete_count) );

    m_ArtefactIconIDs.clear();
    m_IconsConfidenceLevels.clear();
    m_IconsGeometry.clear();
    m_ArtefactIconIDs.reserve(size);
    m_IconsConfidenceLevels.reserve(size);
    m_IconsGeometry.reserve(size);
    for( int i=0; i<size; ++i ) 
    {
        if( m_IconsConfidenceLevelsAll[i] <= 100 )
        {
            m_IconsConfidenceLevels.push_back( m_IconsConfidenceLevelsAll[i] );
            m_ArtefactIconIDs.push_back( m_ArtefactIconIDsAll[i] );
            Geometry * pGeometry = &m_IconsGeometryAll[i];
            m_IconsGeometry.push_back( *pGeometry );
        }
    }
}

void CDTVOCRClientCtl::FindBoundingSquaresInCrop(USHORT id, USHORT group)
{
    if( ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n&&&CDTVOCRClientCtl::FindBoundingSquaresInCrop()\n" )) );
    m_ArtefactCoordinatesInCrop.clear();
    //get all aretefacts boundaries
    m_ImageMagick.LoadImageOutline(&m_MagickImageCrop, m_ArtefactCoordinatesInCrop, m_ArtefactGeometry );
    //NOTE: This is a bit tricky, it turns out that some icons have two or more parts...
    //one way to deal with this is to use the fact that the artifacts that SHOULD compose
    //the full icon SHOULD be close to each other, I've verified this with the icons from 
    //the UI spec and the parts of the icon are two pixels close to each other...
    //Now, the text artifacts, when they have a space, the space is usually at least 7 pixels wide
    //however, when the characters are close to each other they are 2 pixels apart.
    //obviously, this presents a challenge, the approach we will take will be to first attempt
    //to detect all icons individually. Then, for the ones that did not match and happened in adjacent 
    //pairs, we will merge the coordinates and attempt to find them that way.

    //there is also another problem, some icons have two parts that share a common vertical location,
    //that is one part is above the other, the issue here is that the algorithm used since it's a flood
    //fill like algorithm it detects two artifacts separately. The solution of course is to merge the 
    //coordinates for those artifacts, this must be done first
    NormalizeCoordinates();

    //try second set, edge detected coordinates
    //this set can (should) be used when there's too much luma variance and thresholding function would not
    //detect the artifacts, for example light blue icon in a white background, and the rest of the crop
    //image has black, would cause thresholding function to confuse light blue for background (white)
    //the edge detecting function does not confuse this, however is quite a bit slower too.
    if( ini_options->UseEdgeDetection() ) {
        m_ArtefactCoordinatesInCrop.clear();
        m_ImageMagick.LoadImageOutline(&m_MagickImageCrop, m_ArtefactCoordinatesInCrop, m_ArtefactGeometry, true );
        NormalizeCoordinates(false);
    }
    CalcValidDimensions(id, group);
    DeleteInvalidCoordinates();
}
void CDTVOCRClientCtl::DeleteInvalidCoordinates(void)
{
    //swap
    m_ArtefactGeometryMultiPartIcons.clear();
    for( int i=0; i<m_ArtefactGeometry.size(); i++ )
        m_ArtefactGeometryMultiPartIcons.push_back(m_ArtefactGeometry[i]);
    m_ArtefactGeometry.clear();

    int delete_count = 0;
    int size = m_ArtefactGeometryMultiPartIcons.size();
    for( int i=0; i<size; ++i ) 
    {
        Geometry * pGeometry = &(m_ArtefactGeometryMultiPartIcons[i]);
        if( !pGeometry || pGeometry->width() == 0 ) continue;
        //get current coordenates
        USHORT x1 = pGeometry->xOff();
        USHORT y1 = pGeometry->yOff();
        USHORT x2 = x1 + pGeometry->width();
        USHORT y2 = y1 + pGeometry->height();
        if( pGeometry->width() > m_MaxWidth ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact S#%d width too big, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( pGeometry->height() > m_MaxHeight ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact S#%d height too big, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }
        //walk the array searching for an identical artifact
        for( int j=i+1; j<size; ++j ) {
            Geometry * pNewGeometry = &(m_ArtefactGeometryMultiPartIcons[j]);
            if( !pNewGeometry || pNewGeometry->width() == 0 ) continue;
            USHORT nx1 = pNewGeometry->xOff();
            USHORT ny1 = pNewGeometry->yOff();
            USHORT nx2 = nx1 + pNewGeometry->width();
            USHORT ny2 = ny1 + pNewGeometry->height();
            if ( x1 == nx1 && x2 == nx2 && y1 == ny1 && y2 == ny2 ) 
            {
                pNewGeometry->xOff(0);
                pNewGeometry->yOff(0);
                pNewGeometry->width(0);
                pNewGeometry->height(0);
                delete_count++;
            }
        }
        m_ArtefactGeometry.push_back(*pGeometry);
    }     
    m_ArtefactGeometryMultiPartIcons.clear();
    if( delete_count && ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "---Deleted %d duplicate coordinates\n" ),delete_count) );
    sort( m_ArtefactGeometry.begin(), m_ArtefactGeometry.end(), Geometry_greaterthan );
}

void CDTVOCRClientCtl::NormalizeCoordinates(bool clear_coordinates)
{
    if( ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n&&&CDTVOCRClientCtl::NormalizeCoordinates(%d)\n"),clear_coordinates) );
    //NOTE: we want to leave a border ONE pixel wide AROUND our artifact, we might need it :/

    std::vector<USHORT>::iterator startIterator;
    std::vector<USHORT>::iterator tempIterator;

    //make sure we have an array with size multiple of 4
    int weird_left_overs = m_ArtefactCoordinatesInCrop.size() % 4;
    if( weird_left_overs ){
        tempIterator = m_ArtefactCoordinatesInCrop.end() - 1;
        m_ArtefactCoordinatesInCrop.erase( tempIterator, tempIterator + weird_left_overs );
    }

    int size = m_ArtefactCoordinatesInCrop.size();

    //if( size < 4 )
    //    throw ParamsException("Not enough coordinate values");

    if( clear_coordinates )
    {
        m_ArtefactGeometry.clear();
        m_ArtefactGeometry.reserve((size/4)+10);
    }

    for( int i=0; i<size; i+=4 ) {
        //get current coordenates
        USHORT x1 = m_ArtefactCoordinatesInCrop[i];
        USHORT y1 = m_ArtefactCoordinatesInCrop[i+1];
        USHORT x2 = m_ArtefactCoordinatesInCrop[i+2];
        USHORT y2 = m_ArtefactCoordinatesInCrop[i+3];
        if( y2 == 0 ) continue;
        USHORT width    = x2 - x1;
        USHORT height   = y2 - y1;

        //if it's the last one, add to the array and exit while loop
        if( i+4 >= size ) {
            width += 2;
            height += 2;
            x1--;
            if( x1 > ini_options->VideoCaptureDeviceWidth() )
                x1++;
            y1--;
            if( y1 > ini_options->VideoCaptureDeviceHeight() )
                y1++;
            m_ArtefactGeometry.push_back(Geometry(width, height, x1, y1));
            break;
        }
 
        //walk the array searching for an overlapping artifact, or one that's near enough 
        bool merged = false;
        for( int j=i+4; j<size; j+=4 ) {
            USHORT nx1 = m_ArtefactCoordinatesInCrop[j];
            USHORT ny1 = m_ArtefactCoordinatesInCrop[j+1];
            USHORT nx2 = m_ArtefactCoordinatesInCrop[j+2];
            USHORT ny2 = m_ArtefactCoordinatesInCrop[j+3];
            if( ny2 == 0 ) continue;
            //check for containment, we want to check all of them, for resiliency's sake..
            //we have another coordenate, check for collision
            if ( x1 == nx1 && x2 == nx2 && y1 == ny1 && y2 == ny2 ) 
            { // same coordinates
                //delete one...
                m_ArtefactCoordinatesInCrop[j] = 0;
                m_ArtefactCoordinatesInCrop[j+1] = 0;
                m_ArtefactCoordinatesInCrop[j+2] = 0;
                m_ArtefactCoordinatesInCrop[j+3] = 0;
            } 
            else if ( (x1 <= nx1 && x2 >= nx2 && y1 <= ny1 && y2 >= ny2) || 
                 (nx1 <= x1 && nx2 >= x2 && ny1 <= y1 && ny2 >= y2) ) 
            { // full area containment
                //do nothing we will process each one individually
            } 
            else if ((x1-(ini_options->ARTIFACT_PROXIMITY_MERGE_FACTOR_X())) > nx2 || (nx1-(ini_options->ARTIFACT_PROXIMITY_MERGE_FACTOR_X())) > x2 || 
                y1 > ny2 || ny1 > y2) 
            { //no collision, by actual collision or proximity
                //ok, each one is separate
            } 
            else 
            { //collision
                //merge the coordinates and ... ARTIFACT_PROXIMITY_MERGE_FACTOR_X ARTIFACT_PROXIMITY_MERGE_FACTOR_Y
                x1 = min(x1, nx1);
                y1 = min(y1, ny1);
                x2 = max(x2, nx2);
                y2 = max(y2, ny2);
                width    = x2 - x1;
                height   = y2 - y1;
                merged = true;
                //merge them now
                m_ArtefactCoordinatesInCrop[i] = x1;
                m_ArtefactCoordinatesInCrop[i+1] = y1;
                m_ArtefactCoordinatesInCrop[i+2] = x2;
                m_ArtefactCoordinatesInCrop[i+3] = y2;
                //... delete current
                m_ArtefactCoordinatesInCrop[j] = 0;
                m_ArtefactCoordinatesInCrop[j+1] = 0;
                m_ArtefactCoordinatesInCrop[j+2] = 0;
                m_ArtefactCoordinatesInCrop[j+3] = 0;
            }
        }
        if( merged == true ) { //back up one, so that it can check against others
            i-=4;
        } else {
            //add it...
            width += 2;
            height += 2;
            x1--;
            if( x1 > ini_options->VideoCaptureDeviceWidth() )
                x1++;
            y1--;
            if( y1 > ini_options->VideoCaptureDeviceHeight() )
                y1++;
            m_ArtefactGeometry.push_back(Geometry(width, height, x1, y1));
        }
    }
}

void CDTVOCRClientCtl::CalcValidDimensions(USHORT id, USHORT group)
{
    m_MaxWidth  = 0;
    m_MaxHeight = 0;
    m_MinWidth    = ini_options->VideoCaptureDeviceWidth(),
    m_MinHeight   = ini_options->VideoCaptureDeviceHeight();
    //first determine valid coordinates depending on icon id or icon group
    //for our purposes we either get an "id" or a "group", or none at all, NOT both...
    if( id ) {
        if( ini_options->Debug() ) {
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Using icon dimensions\n" )));
        }
        m_ImageMagick.GetIconDimensions(id, m_MinWidth, m_MinHeight);
        m_MaxWidth  = m_MinWidth;
        m_MaxHeight = m_MinHeight;
    } else if ( group ) {
        if( ini_options->Debug() ) {
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Using group dimensions\n" )));
        }
        m_ImageMagick.GetGroupMinMaxDimensions(group, m_MinWidth, m_MinHeight, m_MaxWidth, m_MaxHeight);
    } else {
        if( ini_options->Debug() ) {
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Using global dimensions\n" )));
        }
        m_ImageMagick.GetMinMaxDimensions(m_MinWidth, m_MinHeight, m_MaxWidth, m_MaxHeight);
    }
    m_MaxWidth  += ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE();
    m_MaxHeight += ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE();
    m_MinWidth  -= ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE();
    m_MinHeight -= ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE();
    if( ini_options->Debug() ) {
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Valid dimensions w/h: min (%d/%d) --- max (%d/%d)\n" ), m_MinWidth, m_MinHeight, m_MaxWidth, m_MaxHeight) );
    }
}

void CDTVOCRClientCtl::FindMatchingSinglePartIcons(USHORT id, USHORT group)
{
    if( ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n&&&CDTVOCRClientCtl::FindMatchingSinglePartIcons(%d, %d)\n"),id, group) );

    m_x1 = 0;
    m_y1 = 0;
    m_x2 = 0;
    m_y2 = 0;
    m_ConfidenceLevel = 0;

    
    m_ArtefactIconIDsAll.clear();
    m_IconsGeometryAll.clear();
    m_IconsConfidenceLevelsAll.clear();
    m_ArtefactGeometrySinglePartIcons.clear();
    m_ArtefactSinglePartIconIDs.clear();
    m_ArtefactGeometryMultiPartIcons.clear();
    m_ArtefactMultiPartIconIDs.clear();

    m_IconsConfidenceLevelsAll.reserve( m_ArtefactGeometry.size() );
    m_ArtefactIconIDsAll.reserve( m_ArtefactGeometry.size() );
    m_IconsGeometryAll.reserve( m_ArtefactGeometry.size() );
    m_ArtefactGeometrySinglePartIcons.reserve( m_ArtefactGeometry.size() );
    m_ArtefactSinglePartIconIDs.reserve( m_ArtefactGeometry.size() );
    m_ArtefactGeometryMultiPartIcons.reserve( m_ArtefactGeometry.size() );
    m_ArtefactMultiPartIconIDs.reserve( m_ArtefactGeometry.size() );

    Magick::Image * pImage = 0;

    int size = m_ArtefactGeometry.size();
    for( int i=0; i<size; i++ )
    {
        Geometry * pGeometry = &m_ArtefactGeometry[i];

        if( pGeometry->width() == 0 ) 
            continue;

        if( pGeometry->width() < m_MinWidth ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact S#%d width too small, skipping @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            m_ArtefactGeometryMultiPartIcons.push_back(*pGeometry);
            continue;
        }

        if( pGeometry->height() < m_MinHeight ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact S#%d height too small, skipping @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            m_ArtefactGeometryMultiPartIcons.push_back(*pGeometry);
            continue;
        }

        if( pGeometry->width() > m_MaxWidth ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact S#%d width too big, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( pGeometry->height() > m_MaxHeight ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact S#%d height too big, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( ini_options->Debug() ) {
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "####################################\nSearching artifact S#%d @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
        }

        UINT rows = pGeometry->height(), cols = pGeometry->width();
        const MagickLib::PixelPacket * pixels = m_MagickImageCrop.getConstPixels(pGeometry->xOff(),pGeometry->yOff(),cols,rows);

        pImage = 0;
        if( ini_options->ComparisonAlgorithms() & COMPARISON_ALGORITHM_FFT || ini_options->SaveBitmaps() ) {
            Blob blob;
            m_MagickImageCrop.magick( "BMP" );
            m_MagickImageCrop.write( &blob );
            m_MagickImageCropArtefact.read( blob);
            // Crop the image to specified size (width, height, xOffset, yOffset)
            m_MagickImageCropArtefact.crop( *pGeometry );
            pImage = &m_MagickImageCropArtefact;
        }

        std::string bmp_file = ini_options->WorkPath();
        char buf[250];
        buf[0] = 0;
        if( ini_options->SaveBitmaps() ) {
            sprintf(buf,"IconSinglePart%.3d.bmp", i );
            bmp_file += buf;
            m_MagickImageCropArtefact.write(bmp_file.c_str());
        }

        double fftIdx = 99999999.99;
        BYTE confidence = 0;
        USHORT iconID = m_ImageMagick.FindIconFile(pImage, pixels, rows, cols, fftIdx, confidence, id, group, i);

        USHORT icon = iconID;

        if( confidence < ini_options->MIN_CONFIDENCE_LEVEL_PCT() )
            iconID = INVALID_ICON;
        if( iconID < INVALID_ICON )
        {
            if( ini_options->SaveBitmaps() ) {
                std::string icon_file = m_ImageMagick.GetIconName(iconID);
                std::string found_icon_file = ini_options->WorkPath();
                sprintf(buf,"PASSED_icon_%.3d_s_conf_%.3d_", i, confidence );
                found_icon_file += buf;
                int pos = icon_file.rfind("\\");
                pos++;
                found_icon_file += icon_file.substr(pos, icon_file.length()-pos);
                DeleteFile(found_icon_file.c_str());
                if( false == MoveFile(bmp_file.c_str(), found_icon_file.c_str()) ) {
                    ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not rename file to: %s\n" ), found_icon_file.c_str() ) );
                } else {
                    if( ini_options->Debug() )
                        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Icon saved to file: %s\n" ), found_icon_file.c_str() ) );
                }
            }
            m_IconsConfidenceLevelsAll.push_back(confidence);
            m_ArtefactSinglePartIconIDs.push_back(iconID);
            m_ArtefactGeometrySinglePartIcons.push_back(*pGeometry);
            m_ArtefactIconIDsAll.push_back(iconID);
            m_IconsGeometryAll.push_back(*pGeometry);
            m_ConfidenceLevel = confidence;
            if( m_ConfidenceLevel >= ini_options->DiscardInnerComparisonOverConfidence() ) {
                USHORT x1 = pGeometry->xOff();
                USHORT y1 = pGeometry->yOff();
                USHORT x2 = x1 + pGeometry->width();
                USHORT y2 = y1 + pGeometry->height();
                for( int j=i+1; j<size; ++j ) {
                    Geometry * pNewGeometry = &(m_ArtefactGeometry[j]);
                    if( !pNewGeometry || pNewGeometry->width() == 0 ) continue;
                    USHORT nx1 = pNewGeometry->xOff();
                    USHORT ny1 = pNewGeometry->yOff();
                    USHORT nx2 = nx1 + pNewGeometry->width();
                    USHORT ny2 = ny1 + pNewGeometry->height();
                    if( ny2 == 0 ) continue;
                    //check for containment, we want to check all of them, for resiliency's sake..
                    //we have another coordenate, check for collision
                    bool delete_it = false;
                    if ( (x1  <= nx1 && x2 >= nx2 && y1 <= ny1 && y2 >= ny2) || 
                         (nx1 <= x1  && nx2 >= x2 && ny1 <= y1 && ny2 >= y2) ) 
                    { // full area containment
                        //delete it...
                        delete_it = true;
                        if( ini_options->Debug() && ini_options->Verbose() )
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\t\tcoordinate: @ xy(%d,%d) - w/h(%dx%d) --- full area containment, deleting...\n" ), nx1,ny1,pNewGeometry->width(),pNewGeometry->height() ) );
                    } 
                    else if ((x1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > nx2 || (nx1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > x2 || 
                        (y1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > ny2 || (ny1-(ini_options->DIMENSIONS_DIFFERENCE_TOLERANCE())) > y2) 
                    { //no collision, by actual collision or proximity
                        //ok, each one is separate
                    } 
                    else 
                    { //collision
                        //delete it...
                        delete_it = true;
                        if( ini_options->Debug() && ini_options->Verbose() )
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\t\tcoordinate: @ xy(%d,%d) - w/h(%dx%d) --- area collision, deleting...\n" ), nx1,ny1,pNewGeometry->width(),pNewGeometry->height() ) );
                    }
                    //delete_it = false;
                    if( delete_it == true ) {
                        pNewGeometry->xOff(0);
                        pNewGeometry->yOff(0);
                        pNewGeometry->width(0);
                        pNewGeometry->height(0);
                    }
                }
            }else{
                m_ArtefactGeometryMultiPartIcons.push_back(*pGeometry);
            }
            if( ini_options->Debug() ){
                if( ini_options->ComparisonAlgorithms() & COMPARISON_ALGORITHM_FFT )
                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Found Icon #%d ID:%d name:%s FFT: %f CONF:%d\n++++++++++++++++++++++++++++++++++++\n" ), i, iconID, m_ImageMagick.GetIconName(iconID), fftIdx, confidence ) );
                else
                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Found Icon #%d ID:%d name:%s CONF:%d\n++++++++++++++++++++++++++++++++++++\n" ), i, iconID, m_ImageMagick.GetIconName(iconID), confidence ) );
            }
        }
        else //save geometry to try to find as multi part icons
        {
            if( ini_options->SaveBitmaps() ) {
                if( ini_options->DeleteNonIconFiles() ) {
                    DeleteFile(bmp_file.c_str());
                } else {
                    if ( icon < INVALID_ICON ) {
                        std::string icon_file = m_ImageMagick.GetIconName(icon);
                        std::string found_icon_file = ini_options->WorkPath();
                        if( confidence == 0 )
                            sprintf(buf,"DUBIOUS_icon_%.3d_s_ZEROCONF_%.3d_", i, confidence );
                        else
                            sprintf(buf,"DUBIOUS_icon_%.3d_s_LOWCONF_%.3d_", i, confidence );
                        found_icon_file += buf;
                        int pos = icon_file.rfind("\\");
                        pos++;
                        found_icon_file += icon_file.substr(pos, icon_file.length()-pos);
                        DeleteFile(found_icon_file.c_str());
                        if( ini_options->Debug() && ini_options->Verbose() )
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Renaming file to: %s\n" ), found_icon_file.c_str() ) );
                        if( false == MoveFile(bmp_file.c_str(), found_icon_file.c_str()) ) {
                            ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not rename file to: %s\n" ), found_icon_file.c_str() ) );
                        } else {
                            if( ini_options->Debug() )
                                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Icon saved to file: %s\n" ), found_icon_file.c_str() ) );
                        }
                    } else if( icon == INVALID_DIMENSIONS_ICON ) {
                        if( ini_options->SaveOtherBitmaps() ) {
                            std::string icon_file = ini_options->WorkPath();
                            sprintf(buf,"INVALID_IconSinglePart%.3d.bmp", i );
                            icon_file += buf;
                            DeleteFile(icon_file.c_str());
                            if( ini_options->Debug() && ini_options->Verbose() )
                                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Renaming invalid file: %s to: %s\n" ), bmp_file.c_str(), icon_file.c_str() ) );
                            if( false == MoveFile(bmp_file.c_str(), icon_file.c_str()) ) {
                                ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not rename file to: %s\n" ), icon_file.c_str() ) );
                            } else {
                                if( ini_options->Debug() )
                                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Icon saved to file: %s\n" ), icon_file.c_str() ) );
                            }
                        } else {
                            if( false == DeleteFile(bmp_file.c_str()) )
                                ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not delete file: %s\n" ), bmp_file.c_str() ) );
                        }
                    } else {
                        if( !ini_options->SaveOtherBitmaps() )
                            if( false == DeleteFile(bmp_file.c_str()) )
                                ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not delete file: %s\n" ), bmp_file.c_str() ) );
                    }
                }
            }
            if( ini_options->Debug() ) {
                if( icon < INVALID_ICON ) {
                    if( confidence < ini_options->MIN_CONFIDENCE_LEVEL_PCT() )
                        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "TOO LOW CONF Found Icon #%d ID:%d name:%s CONF:%d \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" ), i, icon, m_ImageMagick.GetIconName(icon), confidence ) );
                }else{
                    if( ini_options->Debug() ){
                        if( ini_options->SaveBitmaps() ) 
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\nIcon NOT Found, file: %s\n------------------------------------\n" ), bmp_file.c_str() ));
                        else
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\nIcon NOT Found\n------------------------------------\n" )));
                    }
                }
            }
            m_ArtefactGeometryMultiPartIcons.push_back(*pGeometry);
        }
    }
}

void CDTVOCRClientCtl::FindMatchingMultiPartIcons(USHORT id, USHORT group)
{
    if( ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n&&&CDTVOCRClientCtl::FindMatchingMultiPartIcons(%d, %d)\n"),id, group) );

    //iterate through the geometry objects and for the ones that were not found as icons
    //see if they are close enough to perhaps be multipart icons

    //put the objects in the *right* place
    m_ArtefactGeometry.clear();
    for( int i=0; i<m_ArtefactGeometryMultiPartIcons.size(); i++ )
        m_ArtefactGeometry.push_back(m_ArtefactGeometryMultiPartIcons[i]);
    m_ArtefactGeometryMultiPartIcons.clear();

    //similar to normalize, just find the close artifacts
    int size = m_ArtefactGeometry.size();
    for( int i=0; i<size; ++i ) {
        Geometry * pGeometry = &(m_ArtefactGeometry[i]);
        if( !pGeometry || pGeometry->width() == 0 ) continue;
        //get current coordenates
        USHORT x1 = pGeometry->xOff();
        USHORT y1 = pGeometry->yOff();
        USHORT x2 = x1 + pGeometry->width();
        USHORT y2 = y1 + pGeometry->height();
        USHORT width    = x2 - x1;
        USHORT height   = y2 - y1;

        //walk the array searching for an overlapping artifact, or one that's near enough 
        bool merged = false;
        for( int j=i+1; j<size; ++j ) {
            Geometry * pNewGeometry = &(m_ArtefactGeometry[j]);
            if( !pNewGeometry || pNewGeometry->width() == 0 ) continue;
            USHORT nx1 = pNewGeometry->xOff();
            USHORT ny1 = pNewGeometry->yOff();
            USHORT nx2 = nx1 + pNewGeometry->width();
            USHORT ny2 = ny1 + pNewGeometry->height();
            if ( 
                ( (x2+(ini_options->MULTIPART_ARTIFACT_PROXIMITY_MERGE_FACTOR_X())) >= nx1 ) 
                && //the x coordinate, below the y coordenate intersections
                (
                    ( y2 >= ny1 && y2 <= ny2 ) || // Y intersection higher 2 lower
                    ( y1 >= ny1 && y1 <= ny2 ) || // Y intersection lower 2 higher 
                    ( ny1 >= y1 && ny1 <= y2 ) || // Y containment big before small
                    ( y1 >= ny1 && y1 <= ny2 )    // Y containment small before big
                )
            )
            { //closeness
                //merge the coordinates and ...
                x1 = min(x1, nx1);
                y1 = min(y1, ny1);
                x2 = max(x2, nx2);
                y2 = max(y2, ny2);
                width    = x2 - x1;
                height   = y2 - y1;
                merged = true;
                pGeometry->xOff(x1);
                pGeometry->yOff(y1);
                pGeometry->width(width);
                pGeometry->height(height);
                pNewGeometry->xOff(0);
                pNewGeometry->yOff(0);
                pNewGeometry->width(0);
                pNewGeometry->height(0);
            }
        }
        if( merged == true ) { //back up one, so that it can check agains others
            pGeometry->xNegative(true); //flag coordenate
            --i;
        } else { //nothing was merged, discard coordenate
            if( pGeometry->xNegative() == true ) {
                pGeometry->xNegative(false); //restore flag
                m_ArtefactGeometryMultiPartIcons.push_back(Geometry(width, height, x1, y1));
            }
        }
    }     

    Magick::Image * pImage = 0;

    for( int i=0; i<m_ArtefactGeometryMultiPartIcons.size(); i++ )
    {
        Geometry * pGeometry = &m_ArtefactGeometryMultiPartIcons[i];

        if( pGeometry->width() == 0 ) 
            continue;

        if( pGeometry->width() < m_MinWidth ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact M#%d width too small, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( pGeometry->height() < m_MinHeight ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact M#%d height too small, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( pGeometry->width() > m_MaxWidth ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact M#%d width too big, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( pGeometry->height() > m_MaxHeight ) 
        {
            if( ini_options->Debug() ) {
                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "### Artifact M#%d height too big, discarding @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
            }
            continue;
        }

        if( ini_options->Debug() ) {
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "####################################\nSearching artifact M#%d @ xy(%d,%d) --- w/h(%dx%d) \n" ), i, pGeometry->xOff(), pGeometry->yOff(), pGeometry->width(), pGeometry->height()) );
        }

        UINT rows = pGeometry->height(), cols = pGeometry->width();
        const MagickLib::PixelPacket * pixels = m_MagickImageCrop.getConstPixels(pGeometry->xOff(),pGeometry->yOff(),cols,rows);;

        std::string bmp_file = ini_options->WorkPath();
        char buf[250];
        buf[0] = 0;

        pImage = 0;
        if( ini_options->ComparisonAlgorithms() & COMPARISON_ALGORITHM_FFT || ini_options->SaveBitmaps() ) {
            Blob blob;
            m_MagickImageCrop.magick( "BMP" );
            m_MagickImageCrop.write( &blob );
            m_MagickImageCropArtefact.read( blob);
            // Crop the image to specified size (width, height, xOffset, yOffset)
            m_MagickImageCropArtefact.crop( *pGeometry );
            pImage = &m_MagickImageCropArtefact;
        }

        if( ini_options->SaveBitmaps() ) {
            sprintf(buf,"IconMultiPart%.3d.bmp", i );
            bmp_file += buf;
            m_MagickImageCropArtefact.write(bmp_file.c_str());
        }

        double fftIdx = 99999999.99;
        BYTE confidence = 0;
        USHORT iconID = m_ImageMagick.FindIconFile(pImage, pixels, rows, cols, fftIdx, confidence, id, group, i+1000);

        USHORT icon = iconID;

        if( confidence < ini_options->MIN_CONFIDENCE_LEVEL_PCT() )
            iconID = INVALID_ICON;
        else
            m_ArtefactMultiPartIconIDs.push_back(iconID);
        if( iconID < INVALID_ICON )
        {
            if( ini_options->SaveBitmaps() ) {
                std::string icon_file = m_ImageMagick.GetIconName(iconID);
                std::string found_icon_file = ini_options->WorkPath();
                sprintf(buf,"PASSED_icon_%.3d_m_conf_%.3d_", i, confidence );
                found_icon_file += buf;
                int pos = icon_file.rfind("\\");
                pos++;
                found_icon_file += icon_file.substr(pos, icon_file.length()-pos);
                DeleteFile(found_icon_file.c_str());
                if( false == MoveFile(bmp_file.c_str(), found_icon_file.c_str()) ) {
                    ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not rename file to: %s\n" ), found_icon_file.c_str() ) );
                } else {
                    if( ini_options->Debug() )
                        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Icon saved to file: %s\n" ), found_icon_file.c_str() ) );
                }
            }
            if( ini_options->Debug() ){
                if( ini_options->ComparisonAlgorithms() & COMPARISON_ALGORITHM_FFT )
                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Found Icon mp #%d ID:%d name:%s FFT: %f CONF:%d\n++++++++++++++++++++++++++++++++++++\n" ), i, iconID, m_ImageMagick.GetIconName(iconID), fftIdx, confidence ) );
                else
                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Found Icon mp #%d ID:%d name:%s CONF:%d\n++++++++++++++++++++++++++++++++++++\n" ), i, iconID, m_ImageMagick.GetIconName(iconID), confidence ) );
            }

            m_IconsConfidenceLevelsAll.push_back(confidence);
            m_ArtefactIconIDsAll.push_back(iconID);
            m_IconsGeometryAll.push_back(*pGeometry);
            m_ConfidenceLevel = confidence;
        } else {
            if( ini_options->SaveBitmaps() ) {
                if( ini_options->DeleteNonIconFiles() ) {
                    DeleteFile(bmp_file.c_str());
                } else {
                    if ( icon < INVALID_ICON ) {
                        std::string icon_file = m_ImageMagick.GetIconName(icon);
                        std::string found_icon_file = ini_options->WorkPath();
                        if( confidence == 0 )
                            sprintf(buf,"DUBIOUS_icon_%.3d_m_ZEROCONF_%.3d_", i, confidence );
                        else
                            sprintf(buf,"DUBIOUS_icon_%.3d_m_LOWCONF_%.3d_", i, confidence );
                        found_icon_file += buf;
                        int pos = icon_file.rfind("\\");
                        pos++;
                        found_icon_file += icon_file.substr(pos, icon_file.length()-pos);
                        DeleteFile(found_icon_file.c_str());
                        if( ini_options->Debug() && ini_options->Verbose() )
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Renaming file to: %s\n" ), found_icon_file.c_str() ) );
                        if( false == MoveFile(bmp_file.c_str(), found_icon_file.c_str()) ) {
                            ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not rename file to: %s\n" ), found_icon_file.c_str() ) );
                        } else {
                            if( ini_options->Debug() )
                                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Icon saved to file: %s\n" ), found_icon_file.c_str() ) );
                        }
                    } else if( icon == INVALID_DIMENSIONS_ICON ) {
                        if( ini_options->SaveOtherBitmaps() ) {
                            std::string icon_file = ini_options->WorkPath();
                            sprintf(buf,"INVALID_IconMultiPart%.3d.bmp", i );
                            icon_file += buf;
                            DeleteFile(icon_file.c_str());
                            if( ini_options->Debug() && ini_options->Verbose() )
                                ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Renaming invalid file: %s to: %s\n" ), bmp_file.c_str(), icon_file.c_str() ) );
                            if( false == MoveFile(bmp_file.c_str(), icon_file.c_str()) ) {
                                ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not rename file to: %s\n" ), icon_file.c_str() ) );
                            } else {
                                if( ini_options->Debug() )
                                    ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "Icon saved to file: %s\n" ), icon_file.c_str() ) );
                            }
                        } else {
                            if( false == DeleteFile(bmp_file.c_str()) )
                                ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not delete file: %s\n" ), bmp_file.c_str() ) );
                        }
                    } else {
                        if( !ini_options->SaveOtherBitmaps() )
                            if( false == DeleteFile(bmp_file.c_str()) )
                                ACE_ERROR(( LM_ERROR, ACE_TEXT( "Could not delete file: %s\n" ), bmp_file.c_str() ) );
                    }
                }
            }
            if( ini_options->Debug() ) {
                if( iconID < INVALID_ICON ) {
                    if( confidence < ini_options->MIN_CONFIDENCE_LEVEL_PCT() )
                        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "TOO LOW CONF Found Icon #%d ID:%d CONF:%d \n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" ), i, icon, m_ImageMagick.GetIconName(icon), confidence ) );
                }else{
                    if( ini_options->Debug() ){
                        if( ini_options->SaveBitmaps() ) 
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\nIcon NOT Found, file: %s\n------------------------------------\n" ), bmp_file.c_str() ));
                        else
                            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\nIcon NOT Found\n------------------------------------\n" )));
                    }
                }
            }
        }
    }
}

#define MIN_BITMAP_HEIGHT 15
#define MIN_BITMAP_WIDTH 10
STDMETHODIMP CDTVOCRClientCtl::Crop(USHORT x1, USHORT y1, USHORT x2, USHORT y2)
{
    try
    {
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n@@@CDTVOCRClientCtl::Crop(%d,%d,%d,%d)\n"),x1, y1, x2, y2) );
        CLEAR_ERRORS
        UINT cols = m_MagickImage.columns();
        UINT rows = m_MagickImage.rows();

        if( x1 > cols )
            throw ParamsException("x1 value too big!");
        if( y1 > rows )
            throw ParamsException("y1 value too big!");

        USHORT width = x2-x1;
        USHORT height = y2-y1;

        if( width < MIN_BITMAP_WIDTH )
            throw ParamsException("width value too small!");
        if( height < MIN_BITMAP_HEIGHT )
            throw ParamsException("height value too small!");

        m_MagickImageCrop = m_MagickImage;
        Blob blob;
        m_MagickImage.magick( "BMP" );
        m_MagickImage.write( &blob );
        m_MagickImageCrop.read( blob);

        // Crop the image to specified size (width, height, xOffset, yOffset)
        m_MagickImageCrop.crop( Geometry(width, height, x1, y1) );     
        return S_OK;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Crop )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::RecognizeCrop(BYTE CharacterProperties, BYTE RecognitionDefaults, BYTE RecognitionType, BYTE ImageProcessingType, BYTE Language)
{
    if( ini_options->Debug() )
        ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n@@@CDTVOCRClientCtl::RecognizeCrop(%d,%d,%d,%d,%d)\n"),CharacterProperties, RecognitionDefaults, RecognitionType, ImageProcessingType, Language) );
    int max_retries = ini_options->ReconnectionRetries();
    UINT32 opTime;
    m_ErrorRetries = 0;
    m_bstrErrorRetriesMessages = "";
Error_Retries:
    m_ErrorRetries++;
    opTime = 0;
    try
    {
        if( m_ErrorRetries <= max_retries ) {

            CLEAR_ERRORS
            m_ConfidenceLevel = 0;
            m_bstrText = "";
            m_bstrProcessedText = "";
            m_Time = 0;

            USHORT width = m_MagickImageCrop.columns();
            USHORT height = m_MagickImageCrop.rows();

            if( width < MIN_BITMAP_WIDTH )
                throw ParamsException("width value too small!");
            if( height < MIN_BITMAP_HEIGHT )
                throw ParamsException("height value too small!");

	        HBITMAP hbmp = 0;

            switch( ImageProcessingType )
            {
            case Monochrome:
                m_ImageMagick.LoadImage(&m_MagickImageCrop, m_BitmapCrop, hbmp, true );
                break;
            case Color:
                m_ImageMagick.LoadImage(&m_MagickImageCrop, m_BitmapCrop, hbmp, false );
                break;
            default:
                throw ParamsException("Bad image processing type");
                break;
            }
            if( !hbmp )
                throw ParamsException("Bad bitmap");

            m_Timer.Start();
            //BITMAPINFO * pInfo = const_cast<BITMAPINFO*>(m_BitmapCrop.GetBitmapInfo());
            //pInfo->bmiHeader.biBitCount = 323;
            //throw ParamsException("Just a test");
            int res = m_OCRServer.Recognize(m_BitmapCrop, CharacterProperties, RecognitionDefaults, RecognitionType == 0 ? RequestPlainText : RequestText, Language );
    
            opTime = m_Timer.End();
            m_Time = opTime;
            switch( res ) {
            case OCRServer::DTVOCRS_SUCCESS:
                CLEAR_ERRORS
                break;
            case OCRServer::DTVOCRS_ERROR_NETWORK:
                m_ErrorCode = DTVOCRC_ERROR_NETWORK;
                m_bstrErrorMessage = "Network error while recognizing bitmap: ";
                m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
                m_bstrErrorRetriesMessages += "\n";
                m_bstrErrorRetriesMessages += m_bstrErrorMessage;
                ACE_ERROR(( LM_ERROR, ACE_TEXT( "%W\n" ), m_bstrErrorMessage.m_str ) );
                if( m_OCRServer.Reconnect() == false ) {
                    m_bstrErrorRetriesMessages += m_OCRServer.GetLastErrorMessage();
                }
                goto Error_Retries;
                break;
            case OCRServer::DTVOCRS_ERROR_PROTOCOL:
                m_ErrorCode = DTVOCRC_ERROR_PROTOCOL;
                m_bstrErrorMessage = "Protocol error while recognizing bitmap: ";
                m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
                ACE_ERROR(( LM_ERROR, ACE_TEXT( "%W\n" ), m_bstrErrorMessage.m_str ) );
                m_bstrErrorRetriesMessages += "\n";
                m_bstrErrorRetriesMessages += m_bstrErrorMessage;
                if( m_OCRServer.Reconnect() == false ) {
                    m_bstrErrorRetriesMessages += m_OCRServer.GetLastErrorMessage();
                }
                goto Error_Retries;
                break;
            case OCRServer::DTVOCRS_ERROR_BITMAP:
                m_ErrorCode = DTVOCRC_ERROR_BITMAP;
                m_bstrErrorMessage = "Bitmap server error while loading bitmap: ";
                m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
                ACE_ERROR(( LM_ERROR, ACE_TEXT( "%W\n" ), m_bstrErrorMessage.m_str ) );
                m_bstrErrorRetriesMessages += "\n";
                m_bstrErrorRetriesMessages += m_bstrErrorMessage;
                if( m_OCRServer.Reconnect() == false ) {
                    m_bstrErrorRetriesMessages += m_OCRServer.GetLastErrorMessage();
                }
                goto Error_Retries;
                break;
            case OCRServer::DTVOCRS_ERROR_EXCEPTION:
                m_ErrorCode = DTVOCRC_ERROR_EXCEPTION;
                m_bstrErrorMessage = "Server exception error while recognizing bitmap: ";
                m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
                ACE_ERROR(( LM_ERROR, ACE_TEXT( "%W\n" ), m_bstrErrorMessage.m_str ) );
                m_bstrErrorRetriesMessages += "\n";
                m_bstrErrorRetriesMessages += m_bstrErrorMessage;
                if( m_OCRServer.Reconnect() == false ) {
                    m_bstrErrorRetriesMessages += m_OCRServer.GetLastErrorMessage();
                }
                goto Error_Retries;
                break;
            case OCRServer::DTVOCRS_ERROR_UNKNOWN:
            default:
                m_ErrorCode = DTVOCRC_ERROR_UNKNOWN;
                m_bstrErrorMessage = "Unknown error while recognizing bitmap: ";
                m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
                ACE_ERROR(( LM_ERROR, ACE_TEXT( "%W\n" ), m_bstrErrorMessage.m_str ) );
                m_bstrErrorRetriesMessages += "\n";
                m_bstrErrorRetriesMessages += m_bstrErrorMessage;
                if( m_OCRServer.Reconnect() == false ) {
                    m_bstrErrorRetriesMessages += m_OCRServer.GetLastErrorMessage();
                }
                goto Error_Retries;
                break;
            }
        }  
        else
        {
            m_bstrErrorMessage = m_bstrErrorRetriesMessages;
            ACE_ERROR(( LM_ERROR, ACE_TEXT( "!!! ERROR RETRIES EXCEEDED !!! Giving up: %W\n" ), m_bstrErrorMessage.m_str ) );
            return S_FALSE;
        }

        ProcessRecognizedCharacters();

        m_Time = opTime;
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n$$$CDTVOCRClientCtl::RecognizeCrop() --- times:%d ms\n\n" ),m_Time) );

        return S_OK;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::RecognizeCrop )
    if( m_OCRServer.Reconnect() == false ) {
        m_bstrErrorRetriesMessages += m_OCRServer.GetLastErrorMessage();
    }
    goto Error_Retries;
    return S_FALSE;
}

void CDTVOCRClientCtl::ProcessRecognizedCharacters()
{

    UINT16      len     = m_OCRServer.GetArrayLength();
    WCHAR *     pText   = m_OCRServer.GetText();
    BYTE *      pConf   = m_OCRServer.GetConfidenceArray();
    UINT16 *    pCoord  = m_OCRServer.GetCharacterCoordinatesArray();

    if( !pText )
        return;

    WCHAR wc = pText[len];
    pText[len] = 0;
    m_bstrText = pText;
    m_bstrProcessedText = pText;
    pText[len] = wc;

    if( pConf )
    {
        double confLevel = 0;
        int count = 0;
        for ( UINT16 i=0; i<len; i++ )
        {
            BYTE conf = *pConf++;
            if( conf <= 100 ) {
                confLevel += conf;
                count++;
            }
        }
        double length = count;
        m_ConfidenceLevel = static_cast<BYTE>(confLevel / length);
    }
}

STDMETHODIMP CDTVOCRClientCtl::TakeScreenShot()
{
    try
    {
        CLEAR_ERRORS
        if( m_GrabVideoCap.Grab() != 0 )
            throw VidCapException("Unable to take screenshot!");
        m_Bitmap.Attach(*m_GrabVideoCap.GetBitmapInfo(), m_GrabVideoCap.GetBitmapBuffer(), m_GrabVideoCap.GetBitmapBufferLength());
        m_MagickImage.read(m_Bitmap.GetBitmapInfo()->bmiHeader.biWidth, m_Bitmap.GetBitmapInfo()->bmiHeader.biHeight, "BGR", CharPixel, m_Bitmap.GetBitmapBuffer() );
        m_MagickImage.flip();
        return S_OK;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::TakeScreenShot )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::LoadImageFile(BSTR ImageFilePath)
{
    try
    {
        CLEAR_ERRORS
	    HBITMAP hbmp = 0;
	    USES_CONVERSION;
	    LPSTR pszFileName = OLE2A (ImageFilePath);
        if( !pszFileName || !*pszFileName )
            throw ParamsException("Bad file path");
        m_MagickImage.read( pszFileName );
        m_ImageMagick.LoadImage(&m_MagickImage, m_Bitmap, hbmp );
        if( !hbmp )
            throw ParamsException("Bad bitmap");
        if( hbmp ) DeleteObject(hbmp);
        return S_OK;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::LoadImageFile )
    return S_FALSE;
}


STDMETHODIMP CDTVOCRClientCtl::SaveImageFile(BSTR ImageFilePath)
{
    try
    {
        CLEAR_ERRORS
	    USES_CONVERSION;
	    LPSTR pszFileName = OLE2A (ImageFilePath);
        if( !pszFileName || !*pszFileName )
            throw ParamsException("Bad file path");
        m_MagickImage.write(pszFileName);
        return S_OK;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::SaveImageFile )
    return S_FALSE;
}


STDMETHODIMP CDTVOCRClientCtl::SaveImageFileCrop(BSTR ImageFilePath)
{
    try
    {
        CLEAR_ERRORS
	    USES_CONVERSION;
	    LPSTR pszFileName = OLE2A (ImageFilePath);
        if( !pszFileName || !*pszFileName )
            throw ParamsException("Bad file path");
        m_MagickImageCrop.write(pszFileName);
        return S_OK;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::SaveImageFileCrop )
    return S_FALSE;
}

void CDTVOCRClientCtl::Initialize(void)
{
    try
    {
        CLEAR_ERRORS

        UINT32 opTime;
        m_Timer.Start();

        ini_options->MergeConfigFiles();

        #define MAX_TEXT 512
        char buf[MAX_TEXT];

        ACE_ARGV args;
 
        ACE_OS::snprintf( buf, MAX_TEXT, "\"dynamic Logger Service_Object * ACE:_make_ACE_Logging_Strategy() '-s %s -f %s -p %s -m %d -N %d'\"",
            ini_options->LogFilePath(), ini_options->Debug() ? "STDERR|OSTREAM" : "OSTREAM", ini_options->LoggingPriorities(), ini_options->MaxLogFileSize(), ini_options->MaxLogFiles() );

        ACE_CString svc = buf;
        args.add( ACE_TEXT( "-d" ) );
        args.add( ACE_TEXT( "-S" ) );
        args.add( ACE_TEXT( svc.c_str() ) );

        if ( ACE_Service_Config::open( args.argc(), args.argv(), ACE_DEFAULT_LOGGER_KEY, 1, 1, 0 ) < 0 )
            ACE_ERROR(( LM_ERROR, ACE_TEXT( "%p\n" ), ACE_TEXT( "Service Config open" ) ) );

        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n@@@CDTVOCRClientCtl::Initialize()\n")));

        if ( ini_options->Debug() )
            ini_options->Dump();

        ini_options->PrintVersion();
 
        m_Debug                    = ini_options->Debug();
        m_Verbose                  = ini_options->Verbose();
        m_Port                     = ini_options->Port();
        m_Host                     = ini_options->Host();
        m_SaveBitmaps              = ini_options->SaveBitmaps();
        m_DeleteNonIconBitmaps     = ini_options->DeleteNonIconFiles();
        m_MinConfidenceLevel       = ini_options->MIN_CONFIDENCE_LEVEL_PCT();
        m_MinWidthOCRInsertSpace   = ini_options->OCR_INSERT_SPACE_MIN_PIXEL_WIDTH();

        ACE_DEBUG(( LM_INFO, ACE_TEXT( "(%P|%D) Starting up OCR client\n" ) ) );

	    m_ImageMagick.Initialize();
        m_GrabVideoCap.Initialize();
	    m_GrabVideoCap.Grab();

        if( false == m_OCRServer.Initialize() ) {
            m_ErrorCode = DTVOCRC_ERROR_NETWORK;
            m_bstrErrorMessage = "Could initialize client/server connector: ";
            m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
            return;
        }

        opTime = m_Timer.End();
        m_Time = opTime;
        if( ini_options->Debug() )
            ACE_DEBUG(( LM_DEBUG, ACE_TEXT( "\n\n$$$CDTVOCRClientCtl::Initialize() --- times:%d ms\n\n" ),m_Time) );

        return;
    }
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Initialize )
}

void CDTVOCRClientCtl::Shutdown(void)
{
    int size = m_FoundIcons.size();
    for( int i=0; i<size; ++i ) 
    {
        FoundIcon * p = m_FoundIcons[i];
        delete p;
    }
    m_FoundIcons.clear();
}

STDMETHODIMP CDTVOCRClientCtl::Connect(void)
{
	try{
        CLEAR_ERRORS
        m_Port = ini_options->Port();
        m_Host = ini_options->Host();
        ACE_DEBUG(( LM_INFO, ACE_TEXT( "(%P|%D) Connecting to: host: %s port: %d \n" ), ini_options->Host(), ini_options->Port() ) );
        if( false == m_OCRServer.Connect( ini_options->Host(), ini_options->Port() ) ) {
            m_ErrorCode = DTVOCRC_ERROR_NETWORK;
            m_bstrErrorMessage = "Could not connect to server: ";
            m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
            return S_FALSE;
        }
        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Connect )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::Disconnect(void)
{
	try{
        CLEAR_ERRORS
        ACE_DEBUG(( LM_INFO, ACE_TEXT( "(%P|%D) Disconnecting from: host: %s port: %d \n" ), ini_options->Host(), ini_options->Port() ) );
        if( false == m_OCRServer.Disconnect() ) {
            m_ErrorCode = DTVOCRC_ERROR_NETWORK;
            m_bstrErrorMessage = "Could not disconnect from server: ";
            m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
            return S_FALSE;
        }
        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Disconnect )
    return S_FALSE;
}

STDMETHODIMP CDTVOCRClientCtl::Ping(void)
{
	try{
        CLEAR_ERRORS
        ACE_DEBUG(( LM_INFO, ACE_TEXT( "(%P|%D) Pinging server... \n" )));
        if( false == m_OCRServer.Ping() ) {
            m_ErrorCode = DTVOCRC_ERROR_NETWORK;
            m_bstrErrorMessage = "Could not ping to server: ";
            m_bstrErrorMessage += m_OCRServer.GetLastErrorMessage();
            return S_FALSE;
        }
        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Ping )
    return S_FALSE;
}
 
STDMETHODIMP CDTVOCRClientCtl::CreateIconsDatabase(void)
{
	try{
        CLEAR_ERRORS
        ACE_DEBUG(( LM_INFO, ACE_TEXT( "(%P|%D) Creating Icons Database... \n" )));
        m_ImageMagick.CreateIconsDatabase();
        return S_OK;
	}
    HANDLE_EXCEPTIONS( CDTVOCRClientCtl::Ping )
    return S_FALSE;
}

//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################
//##########################################################################################

STDMETHODIMP CDTVOCRClientCtl::get_Text(BSTR* pVal)
{
	*pVal = m_bstrText.m_str;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_ProcessedText(BSTR* pVal)
{
	*pVal = m_bstrProcessedText.m_str;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_Time(ULONG* pVal)
{
    *pVal = m_Time;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_ConfidenceLevel(BYTE* pVal)
{
    *pVal = m_ConfidenceLevel;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_ErrorMessage(BSTR* pVal)
{
    *pVal = m_bstrErrorMessage;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_cx1(ULONG* pVal)
{
    *pVal = m_x1;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_cy1(ULONG* pVal)
{
    *pVal = m_y1;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_cx2(ULONG* pVal)
{
    *pVal = m_x2;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_cy2(ULONG* pVal)
{
    *pVal = m_y2;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_WhichIconsQuantity(USHORT* pVal)
{
    *pVal = m_WhichIconsQuantity;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_WhichIconsCurrentID(USHORT* pVal)
{
    *pVal = m_WhichIconsCurrentID;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_Error(LONG* pVal)
{
    *pVal = m_ErrorCode;
    return S_OK;
}


STDMETHODIMP CDTVOCRClientCtl::get_Debug(VARIANT_BOOL* pVal)
{
    *pVal = m_Debug;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_Debug(VARIANT_BOOL newVal)
{
    m_Debug = newVal;
    ini_options->Debug(m_Debug);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- Debug: %d \n" ), m_Debug ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_Verbose(VARIANT_BOOL* pVal)
{
    *pVal = m_Verbose;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_Verbose(VARIANT_BOOL newVal)
{
    m_Verbose = newVal;
    //if( m_Verbose )
    //    ini_options->Verbose(1);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- Verbose: %d \n" ), m_Verbose ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_SaveBitmaps(VARIANT_BOOL* pVal)
{
    *pVal = m_SaveBitmaps;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_SaveBitmaps(VARIANT_BOOL newVal)
{
    m_SaveBitmaps = newVal;
    ini_options->SaveBitmaps(m_SaveBitmaps);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- SaveBitmaps: %d \n" ), m_SaveBitmaps ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_DeleteNonIconBitmaps(VARIANT_BOOL* pVal)
{
    *pVal = m_DeleteNonIconBitmaps;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_DeleteNonIconBitmaps(VARIANT_BOOL newVal)
{
    m_DeleteNonIconBitmaps = newVal;
    ini_options->DeleteNonIconFiles(m_DeleteNonIconBitmaps);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- DeleteNonIconBitmaps: %d \n" ), m_DeleteNonIconBitmaps ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_MinConfidenceLevel(VARIANT_BOOL* pVal)
{
    *pVal = m_MinConfidenceLevel;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_MinConfidenceLevel(VARIANT_BOOL newVal)
{
    m_MinConfidenceLevel = newVal;
    ini_options->SetMIN_CONFIDENCE_LEVEL_PCT(m_MinConfidenceLevel);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- MinConfidenceLevel: %d \n" ), m_MinConfidenceLevel ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_MinWidthOCRInsertSpace(VARIANT_BOOL* pVal)
{
    *pVal = m_MinWidthOCRInsertSpace;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_MinWidthOCRInsertSpace(VARIANT_BOOL newVal)
{
    m_MinWidthOCRInsertSpace = newVal;
    ini_options->SetOCR_INSERT_SPACE_MIN_PIXEL_WIDTH(m_MinWidthOCRInsertSpace);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- MinWidthOCRInsertSpace: %d \n" ), m_MinWidthOCRInsertSpace ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_Port(LONG* pVal)
{
    *pVal = m_Port;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_Port(LONG newVal)
{
    m_Port = newVal;
    ini_options->SetPort(m_Port);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- Port: %d \n" ), m_Port ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_Host(BSTR* pVal)
{
    *pVal = m_Host;
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::put_Host(BSTR newVal)
{
    m_Host = newVal;
	USES_CONVERSION;
	LPSTR pszHost = OLE2A (m_Host.m_str);
    ini_options->Host(pszHost);
    ACE_ERROR(( LM_INFO, ACE_TEXT( "Property changed --- Host: %s \n" ), pszHost ) );
    return S_OK;
}

STDMETHODIMP CDTVOCRClientCtl::get_VersionString(BSTR* pVal)
{
    m_VersionString = ini_options->VersionString();
    *pVal = m_VersionString;
    return S_OK;
}


STDMETHODIMP CDTVOCRClientCtl::get_IsConnected(VARIANT_BOOL* pVal)
{
    *pVal = m_OCRServer.IsConnected();
    return S_OK;
}

