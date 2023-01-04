#include "StdAfx.h"
#include ".\monobitmapgrid.h"

ULONG C2ULONG(USHORT x, USHORT y){ ULONG ul = x; ul <<= 16; ul += y; return ul; }

int MonoBitmapGrid::sm_RecursionLevel = 0;

int g_Stack[STACK_SIZE];
int g_StackPointer;

USHORT MonoBitmapGrid::sm_xFactor = 0;
USHORT MonoBitmapGrid::sm_yFactor = 0;

MonoBitmapGrid::MonoBitmapGrid(BYTE * pData, USHORT w, USHORT h, std::vector<USHORT> & coordinates, bool recursing, bool increase ): m_Width(w), m_Height(h), m_pData(pData), m_Coordinates(coordinates), m_IncreaseCoordinates(increase) 
{
    if ( recursing == true ) {
    //    sm_RecursionLevel++;
    } else {
        sm_xFactor = 0;
        sm_yFactor = 0;
    }
    g_StackPointer = 0;
    m_Size = m_Width * m_Height;
    m_pDataEnd = m_pData + m_Size - 1;
    m_BackgroundColor = *pData;
    m_ForegroundColor = m_BackgroundColor == GRID_COLOR_BLACK ? GRID_COLOR_WHITE : GRID_COLOR_BLACK;;
    m_pCurrentBusyGridLocation = m_pData; //start at first row
    m_x = 0;
    m_y = 0;
    m_MAX_ARTIFACT_SIZE = ini_options->VideoCaptureDeviceWidth() * ini_options->VideoCaptureDeviceHeight();
    m_MIN_ARTIFACT_SIZE = ini_options->MIN_ARTIFACT_AXIS() * ini_options->MIN_ARTIFACT_AXIS();
    m_CoordinatesCohesiveGroup.reserve(m_MAX_ARTIFACT_SIZE + 20);
    FindBoundingRectangles();


/*
This is the segmentation function. We use the Region Growing technique modified not to use recursion. It is very,
very fast, comparing with the other available methods. After we find the main segments, we recurse two more times
to find more segments within the segment. We need one variable to let us know where we're at.
*/
void MonoBitmapGrid::FindBoundingRectangles()
{
    while( FindBusyRow() )
    {
        CalculateBoundingRectangle();
        //CalculateBoundingRectangle2();
    }
    m_SecondPassVectorStart = m_Coordinates.size();
}

bool MonoBitmapGrid::FindBusyRow(void)
{
    BYTE * pCurData = m_pCurrentBusyGridLocation;
    while( *pCurData++ == m_BackgroundColor && pCurData < m_pDataEnd ){}
    if( pCurData >= m_pDataEnd )
        return false;
    pCurData--;
    m_pCurrentBusyGridLocation = pCurData;
    return true;
}

void MonoBitmapGrid::CalculateBoundingRectangle()
{
    m_cur_x = (USHORT)((m_pCurrentBusyGridLocation - m_pData) % m_Width);
    m_cur_y = (USHORT)((m_pCurrentBusyGridLocation - m_pData) / m_Width);

    m_x = m_cur_x;
    m_y = m_cur_y;

    /*
    ok this is all very tricky, but basically it's highly optimized for speed, here's what's happening.
    1) We found the starting pixel already
    2) We start looking for adjacent pixels to -90o angle (up), clockwise. When we look for more pixels the
    next place to look depends on the direction, which is set depending on what we found last,
    basically we search clockwise, but we take into consideration the direction, it's not nice
    to go into infinite loops...
    3) Drawback is, images CANNOT overlap in the se, e or s directions, if they do, the second image will be cut,
    the second algorithm which uses flood fill prevents this, however it uses recursion and it's slower, 
    furthermore, if the image is very large, it WILL overflow the stack!
    4) Turns out the overlapping of images is not a problem at all. All we're doing is finding segments.
    */

    m_CoordinatesCohesiveGroup.clear();
    SaveCurrentCoordenate();
    PerimeterTracer pixelWalker(PixelUpperRight);
    int counter = m_MAX_ARTIFACT_SIZE;
    while(counter--) //just in case, we don't really want to loop forever here
    {
        for( int i=0; i<PixelLocations; ++i ){
            if( GotoPixelIfOn(pixelWalker()) )
                break;
            ++pixelWalker;
        }
        //it should be ok to get here regardless of the previous for loop,
        //if it couldn't find another pixel is going to back track
        //until it gets back to its origin
        SaveCurrentCoordenate();
        pixelWalker.backup();
        if( m_x == m_cur_x && m_y == m_cur_y ) //we're back to where we started
            break;
    }
    if ( counter )
        AddCoordinatesToGrid();
}

//we already know the y1 coordenate, don't we?
//void AddCoordinatesToGrid()
void MonoBitmapGrid::AddCoordinatesToGrid()
{
    ULONG x1 = m_Width, y1 = m_cur_y, x2 = 0, y2 = 0;
    std::vector<ULONG>::iterator iter = m_CoordinatesCohesiveGroup.begin();
    while( iter != m_CoordinatesCohesiveGroup.end() )
    {
        ULONG x = *iter >> 16;
        ULONG y = *iter & 0x0FFFF;
        if( x < x1 ) 
            x1 = x;
        else if( x > x2 ) 
            x2 = x;
        if( y > y2 ) y2 = y;
        ++iter;
    }
    x2++;
    y2++;

    USHORT w = x2 - x1;
    USHORT h = y2 - y1;

    m_x = x1;
    m_y = y1;

    if( w > ini_options->VideoCaptureDeviceWidth() || h > ini_options->VideoCaptureDeviceHeight() ||
        w < ini_options->MIN_ARTIFACT_AXIS() || h < ini_options->MIN_ARTIFACT_AXIS() ) {
        floodFill8Stack(m_cur_x, m_cur_y, m_BackgroundColor, m_ForegroundColor);
        return;
    }

    if( m_IncreaseCoordinates == true ) {
        m_Coordinates.push_back((USHORT)x1 + sm_xFactor);
        m_Coordinates.push_back((USHORT)y1 + sm_yFactor);
        m_Coordinates.push_back((USHORT)x2 + sm_xFactor);
        m_Coordinates.push_back((USHORT)y2 + sm_yFactor);
    } else {
        m_Coordinates.push_back((USHORT)x1 + sm_xFactor + 1);
        m_Coordinates.push_back((USHORT)y1 + sm_yFactor + 1);
        m_Coordinates.push_back((USHORT)x2 + sm_xFactor - 1);
        m_Coordinates.push_back((USHORT)y2 + sm_yFactor - 1);
    }

    if( w < ini_options->MIN_ICON_WIDTH() || h < ini_options->MIN_ICON_HEIGHT() ) {
        floodFill8Stack(m_cur_x, m_cur_y, m_BackgroundColor, m_ForegroundColor);
        return;
    }

    //extract current rectangle to buffer
    BYTE * pBoundingRectangle = (m_pData + (m_Width * m_y + m_x ));
    BYTE * pTempBitmapBuffer = m_TempBitmapBuffer;
    int skip = m_Width - w;
    for (int y=0; y < h; y++)
    {
        for (int x=0; x < w; x++)
            *pTempBitmapBuffer++ = *pBoundingRectangle++;
        pBoundingRectangle += skip;
    }
    floodFill8Stack(m_cur_x, m_cur_y, m_BackgroundColor, m_ForegroundColor);

    m_TempBitmapWidth = w;
    m_TempBitmapHeight = h;
    //fill the surrounding pixels with foreground, so it will become the new background
    //go around the perimeter and execute a flood fill
    //top
    pTempBitmapBuffer = m_TempBitmapBuffer;
    for (int x=0; x < m_TempBitmapWidth; x++) {
        if( *pTempBitmapBuffer == m_BackgroundColor )
            floodFill8StackTempBuffer(x, 0, m_ForegroundColor, m_BackgroundColor);
        pTempBitmapBuffer++;
    }
    //bottom
    pTempBitmapBuffer = m_TempBitmapBuffer;
    pTempBitmapBuffer += m_TempBitmapWidth * (m_TempBitmapHeight-1);
    for (int x=0; x < m_TempBitmapWidth; x++) {
        if( *pTempBitmapBuffer == m_BackgroundColor )
            floodFill8StackTempBuffer(x, m_TempBitmapHeight-1, m_ForegroundColor, m_BackgroundColor);
        pTempBitmapBuffer++;
    }
    //left
    pTempBitmapBuffer = m_TempBitmapBuffer;
    for (int y=0; y < m_TempBitmapHeight; y++) {
        if( *pTempBitmapBuffer == m_BackgroundColor )
            floodFill8StackTempBuffer(0, y, m_ForegroundColor, m_BackgroundColor);
        pTempBitmapBuffer += m_TempBitmapWidth;
    }
    //right
    pTempBitmapBuffer = m_TempBitmapBuffer;
    pTempBitmapBuffer += (m_TempBitmapWidth - 1);
    for (int y=0; y < m_TempBitmapHeight; y++) {
        if( *pTempBitmapBuffer == m_BackgroundColor )
            floodFill8StackTempBuffer(m_TempBitmapWidth - 1, y, m_ForegroundColor, m_BackgroundColor);
        pTempBitmapBuffer += m_TempBitmapWidth;
    }

    sm_xFactor += m_x;
    sm_yFactor += m_y;

    //segment new square
    MonoBitmapGrid tmpSquare( m_TempBitmapBuffer, m_TempBitmapWidth, m_TempBitmapHeight, m_Coordinates, true, m_IncreaseCoordinates );

    sm_xFactor -= m_x;
    sm_yFactor -= m_y;
}

bool MonoBitmapGrid::GotoPixelIfOn( PixelLocation loc ) 
{ 
    USHORT x = m_x;
    USHORT y = m_y;
    switch( loc )
    {
    case PixelUpperLeft: //fall through
        y--; 
    case PixelLeft:
        x--;
        break;

    case PixelUpperRight: //fall through
        x++;
    case PixelUpper:
        y--;
        break;

    case PixelLowerRight: //fall through
        y++;
    case PixelRight:
        x++;
        break;

    case PixelLowerLeft: //fall through
        x--;
    case PixelLower:
        y++;
        break;
    default:
        return false;
    }
    if( y > m_Height || x > m_Width ) 
        return false;
    if( IsLocationOn( x, y ) )
    {
        m_x = x;
        m_y = y;
        return true;
    }
    return false;
}

bool MonoBitmapGrid::IsLocationOn(USHORT x, USHORT y) 
{
    BYTE color = *(m_pData + (m_Width * y + x ));
    return (m_BackgroundColor == GRID_COLOR_BLACK) ? color == GRID_COLOR_WHITE : color == GRID_COLOR_BLACK;
}

void MonoBitmapGrid::SaveCurrentCoordenate()
{
    SaveCoordinate( m_x, m_y );
}
void MonoBitmapGrid::SaveCoordinate(USHORT x, USHORT y)
{
    m_CoordinatesCohesiveGroup.push_back(C2ULONG( x, y ));
}

BYTE& MonoBitmapGrid::grid(int x, int y)
{
    return * (m_pData + (m_Width * y + x ));
}
BYTE& MonoBitmapGrid::gridtb(int x, int y)
{
    return * (m_TempBitmapBuffer + (m_TempBitmapWidth * y + x ));
}

bool MonoBitmapGrid::pop(int &x, int &y)
{
    if(g_StackPointer > 0)
    {
        int p = g_Stack[g_StackPointer];
        x = p / m_Height;
        y = p % m_Height;
        g_StackPointer--;
        return 1;
    }    
    else
    {
        return 0;
    }   
}   
    
bool MonoBitmapGrid::push(int x, int y)
{
    if(g_StackPointer < STACK_SIZE - 1)
    {
        g_StackPointer++;
        g_Stack[g_StackPointer] = m_Height * x + y;
        return 1;
    }    
    else
    {
        return 0;
    }   
}    

void MonoBitmapGrid::emptyStack()
{
    int x=0, y=0;
    while(pop(x, y));
}

bool MonoBitmapGrid::poptb(int &x, int &y)
{
    if(g_StackPointer > 0)
    {
        int p = g_Stack[g_StackPointer];
        x = p / m_TempBitmapHeight;
        y = p % m_TempBitmapHeight;
        g_StackPointer--;
        return 1;
    }    
    else
    {
        return 0;
    }   
}   
    
bool MonoBitmapGrid::pushtb(int x, int y)
{
    if(g_StackPointer < STACK_SIZE - 1)
    {
        g_StackPointer++;
        g_Stack[g_StackPointer] = m_TempBitmapHeight * x + y;
        return 1;
    }    
    else
    {
        return 0;
    }   
}    

void MonoBitmapGrid::emptyStacktb()
{
    int x=0, y=0;
    while(poptb(x, y));
}

//8-way floodfill using our own g_Stack routines
void MonoBitmapGrid::floodFill8Stack(int x, int y, UINT32 newColor, UINT32 oldColor)
{
    if(newColor == oldColor) return; //if you don't do this: infinite loop!
    emptyStack();

    if(!push(x, y)) return; 
    while(pop(x, y))
    {
        grid( x, y ) = newColor; 

        if(x + 1 < m_Width && grid( x + 1, y ) == oldColor)
        {
            if(!push(x + 1, y)) return;            
        }    
        if(x - 1 >= 0 && grid( x - 1, y ) == oldColor)
        {
            if(!push(x - 1, y)) return;            
        }    
        if(y + 1 < m_Height && grid( x, y + 1 ) == oldColor)
        {
            if(!push(x, y + 1)) return;            
        }    
        if(y - 1 >= 0 && grid( x, y - 1 ) == oldColor)
        {
            if(!push(x, y - 1)) return;            
        }
        if(x + 1 < m_Width && y + 1 < m_Height && grid( x + 1, y + 1 ) == oldColor)
        {
            if(!push(x + 1, y + 1)) return;            
        }    
        if(x + 1 < m_Width && y - 1 >= 0 && grid( x + 1, y - 1 ) == oldColor)
        {
            if(!push(x + 1, y - 1)) return;            
        }    
        if(x - 1 > 0 && y + 1 < m_Height && grid( x - 1, y + 1 ) == oldColor)
        {
            if(!push(x - 1, y + 1)) return;            
        }    
        if(x - 1 >= 0 && y - 1 >= 0 && grid( x - 1, y - 1 ) == oldColor)
        {
            if(!push(x - 1, y - 1)) return;            
        }            
    }     
} 

//8-way floodfill using our own g_Stack routines
void MonoBitmapGrid::floodFill8StackTempBuffer(int x, int y, UINT32 newColor, UINT32 oldColor)
{
    if(newColor == oldColor) return; //if you don't do this: infinite loop!
    emptyStacktb();

    if(!pushtb(x, y)) return; 
    while(poptb(x, y))
    {
        gridtb( x, y ) = newColor; 

        if(x + 1 < m_TempBitmapWidth && gridtb( x + 1, y ) == oldColor)
        {
            if(!pushtb(x + 1, y)) return;            
        }    
        if(x - 1 >= 0 && gridtb( x - 1, y ) == oldColor)
        {
            if(!pushtb(x - 1, y)) return;            
        }    
        if(y + 1 < m_TempBitmapHeight && gridtb( x, y + 1 ) == oldColor)
        {
            if(!pushtb(x, y + 1)) return;            
        }    
        if(y - 1 >= 0 && gridtb( x, y - 1 ) == oldColor)
        {
            if(!pushtb(x, y - 1)) return;            
        }
        if(x + 1 < m_TempBitmapWidth && y + 1 < m_TempBitmapHeight && gridtb( x + 1, y + 1 ) == oldColor)
        {
            if(!pushtb(x + 1, y + 1)) return;            
        }    
        if(x + 1 < m_TempBitmapWidth && y - 1 >= 0 && gridtb( x + 1, y - 1 ) == oldColor)
        {
            if(!pushtb(x + 1, y - 1)) return;            
        }    
        if(x - 1 > 0 && y + 1 < m_TempBitmapHeight && gridtb( x - 1, y + 1 ) == oldColor)
        {
            if(!pushtb(x - 1, y + 1)) return;            
        }    
        if(x - 1 >= 0 && y - 1 >= 0 && gridtb( x - 1, y - 1 ) == oldColor)
        {
            if(!pushtb(x - 1, y - 1)) return;            
        }            
    }     
} 
