// CaptureImage.cpp : Implementation of CCaptureImage

#include "stdafx.h"
#include "CaptureImagePixel.h"
#include "CaptureImage.h"
#include ".\captureimage.h"
#include "Gdiplus.h"
#include <math.h>
#include "pixel.h"

// CCaptureImage

const USHORT CCaptureImage::block_size = 8;

/**
    \brief  An example from the MS help files, takes a MIME type
            and returns the encoder CLSID.  Very useful.

    ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1033/gdicpp/gdi+/usinggdi+/usingimageencodersanddecoders/retrievingthe.htm
*/
int CCaptureImage::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

STDMETHODIMP CCaptureImage::loadFile (BSTR filename)
{
    Gdiplus::Bitmap bm(filename, false);

    this->put_height(bm.GetHeight());
    this->put_width(bm.GetWidth());

    for (int x = 0; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            Gdiplus::Color in_pixel;
            bm.GetPixel(x,y,&in_pixel);

            pixel_t * lpixel = getPixel(x,y);
            lpixel->color.a = in_pixel.GetA();
            lpixel->color.r = in_pixel.GetR();
            lpixel->color.g = in_pixel.GetG();
            lpixel->color.b = in_pixel.GetB();
        }

    return S_OK;
}

/**
    \brief  Load the image from a PNG
    \param  filename  Filename of the PNG
*/
STDMETHODIMP CCaptureImage::loadPng(BSTR filename)
{
    return loadFile(filename);
}

/**
    \brief  Load the image from a BMP
    \param  filename  Filename of the BMP
*/
STDMETHODIMP CCaptureImage::loadBmp(BSTR filename)
{
    return loadFile(filename);
}

/**
    \brief  Save the image to a PNG
    \param  filename  Filename of the PNG
*/
STDMETHODIMP CCaptureImage::savePng(BSTR filename)
{
    Gdiplus::Bitmap bm(_width, _height);

    for (int x = 0; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            pixel_t * lpixel = getPixel(x,y);
            Gdiplus::Color bmpixel(lpixel->color.a, lpixel->color.r, lpixel->color.g, lpixel->color.b);
            bm.SetPixel(x, y, bmpixel);
        }

    CLSID encoderClsid;
    int result;
    result = GetEncoderClsid(L"image/png", &encoderClsid);
    if(result < 0) {
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Unable to save PNG file", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    bm.Save(filename, &encoderClsid);

    return S_OK;
}

/**
    \brief  Save the image to a BMP
    \param  filename  Filename of the BMP
*/
STDMETHODIMP CCaptureImage::saveBmp(BSTR filename)
{
    Gdiplus::Bitmap bm(_width, _height);

    for (int x = 0; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            pixel_t * lpixel = getPixel(x,y);
            Gdiplus::Color bmpixel(lpixel->color.a, lpixel->color.r, lpixel->color.g, lpixel->color.b);
            bm.SetPixel(x, y, bmpixel);
        }

    CLSID encoderClsid;
    int result;
    result = GetEncoderClsid(L"image/bmp", &encoderClsid);
    if(result < 0)
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Unable to save BMP file", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));

    bm.Save(filename, &encoderClsid);

    return S_OK;
}

/**
    \brief  Find out the width of the image
    \param  pVal  Where the width gets placed
*/
STDMETHODIMP CCaptureImage::get_width(USHORT* pVal)
{
    *pVal = _width;
    return S_OK;
}

/**
    \brief  Set the width of the image
    \param  newVal  How big should it be?  (in pixels)

*/
STDMETHODIMP CCaptureImage::put_width(USHORT newVal)
{
    if (_width == newVal) return S_OK;  // Don't waste my time

    USHORT oldWidth = _width;
    _width = newVal;

    pixel_t * oldimage = _imageref;
    _imageref = (pixel_t *)malloc(sizeof(pixel_t) * _width * _height);

    /* Create a default pixel for filling up the rest of the lines */
    /* This is alot of extra code, but makes the default color all in one place */
    IDispatch * dispatch;
    CoCreateInstance(__uuidof(CCaptureImagePixel), NULL, CLSCTX_ALL, __uuidof(IDispatch), (void **)&dispatch);
    
    if (dispatch == NULL) {
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Can't create a new object (CaptureImagePixel)", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }
    
    CCaptureImagePixel * pixel;
    dispatch->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&pixel));

    if (pixel == NULL) {
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Can't get a handle for new object (CaptureImagePixel)", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    for (int y = 0; y < _height; y++) {
        int x;
        for (x = 0; x < (_width < oldWidth ? _width : oldWidth); x++) {
            getPixel(x,y)->fullpixel = getPixel(x,y,oldimage,oldWidth,_height)->fullpixel;
        }
        for (; x < _width; x++) {
            pixel->get_fullVal(&(getPixel(x,y)->fullpixel));
        }
    }

    pixel->Release();
    dispatch->Release();
    // printf("Free in width killing me?");
    free(oldimage);
    // printf("  nope\n");

    return S_OK;
}

/**
    \brief  Find out the height of the image
    \param  pVal  Where the height gets placed
*/
STDMETHODIMP CCaptureImage::get_height(USHORT* pVal)
{
    *pVal = _height;
    return S_OK;
}

/**
    \brief  Set the height of the image
    \param  newVal  How big should it be?  (in pixels)

*/
STDMETHODIMP CCaptureImage::put_height(USHORT newVal)
{
    if (_height == newVal) return S_OK; /* Don't waste my time */

    USHORT oldHeight = _height;
    _height = newVal;

    pixel_t *oldimage = _imageref;
    _imageref = (pixel_t *)malloc(sizeof(pixel_t) * _width * _height);

    int y;

    for(y = 0; y < (_height < oldHeight ? _height : oldHeight); y++)
        for (int x = 0; x < _width; x++)
            getPixel(x,y)->fullpixel = getPixel(x,y,oldimage,_width,oldHeight)->fullpixel;

    /* Create a default pixel for filling up the rest of the lines */
    /* This is alot of extra code, but makes the default color all in one place */
    IDispatch * dispatch;
    CoCreateInstance(__uuidof(CCaptureImagePixel), NULL, CLSCTX_ALL, __uuidof(IDispatch), (void **)&dispatch);
    CCaptureImagePixel * pixel;
    dispatch->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&pixel));

    for (; y < _height; y++)
        for (int x = 0; x < _width; x++)
            pixel->get_fullVal(&(getPixel(x,y)->fullpixel));

    pixel->Release();
    dispatch->Release();
    // printf("Free in height killing me?");
    free(oldimage);
    // printf("  nope\n");

    return S_OK;
}

/**
    \brief  Get a particular pixel in the image
    \param  x    X position of the pixel
    \param  y    Y position of the pixel
    \param  pVal The pixel to be returned
*/
STDMETHODIMP CCaptureImage::get_pixel(USHORT x, USHORT y, IDispatch ** pVal)
{
    int hResult;

    if (x >= _width || y >= _height)
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Pixel access out of range", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));

    hResult = CoCreateInstance(__uuidof(CCaptureImagePixel), NULL, CLSCTX_ALL, __uuidof(IDispatch), (void **)pVal);

    switch (hResult) {
        case REGDB_E_CLASSNOTREG:
            printf("REGDB_E_CLASSNOTREG\n");
            break;
        case CLASS_E_NOAGGREGATION:
            printf("CLASS_E_NOAGGREGATION\n");
            break;
        case E_NOINTERFACE:
            printf("E_NOINTERFACE\n");
            break;
        case S_OK:
            break;
        default:
            printf("Other error\n");
            break;
    }

    if (*pVal == NULL) {
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Can't create a pixel to return", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    CCaptureImagePixel * pixel;
    (*pVal)->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&pixel));

    pixel->setReference(&(getPixel(x,y)->fullpixel));
    pixel->Release();

    return S_OK;
}


/**
   \brief  Set a particular pixel in the image
   \param  x      X position of the pixel
   \param  y      Y position of the pixel
   \param  newVal What the pixel should be
*/
STDMETHODIMP CCaptureImage::put_pixel(USHORT x, USHORT y, IDispatch * newVal)
{
    if (x >= _width || y >= _height)
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Pixel access out of range", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));

    CCaptureImagePixel * pixel;
    newVal->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&pixel));

    pixel->get_fullVal(&(getPixel(x, y)->fullpixel));
    pixel->Release();

    return S_OK;
}

/**
    \brief  Fill the image with a particular pixel color
    \param  color  Color to set the whole image to
*/
STDMETHODIMP CCaptureImage::fill(IDispatch * color)
{
    CCaptureImagePixel * pixel;
    color->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&pixel));

    for (int x = 0; x < _width; x++) {
        for (int y = 0; y < _height; y++) {
            pixel->get_fullVal(&(getPixel(x,y)->fullpixel));
        }
    }

    pixel->Release();

    return S_OK;
}

/**
    \brief  A quick function to standardize pixel access
    \param  x       X position in the image
    \param  y       Y position in the image
    \param  image   The array of pixels to use
    \param  width   Width of the image
    \param  height  Height of the image
    \return  A pointer to the requested pixel in the image
*/
pixel_t *
CCaptureImage::getPixel (USHORT x, USHORT y, pixel_t * image, USHORT width, USHORT height)
{
    return &image[x + y * width];
}

/**
    \brief  A function to do a quick binary comparison between two
            images.  They must have the same height/width and exactly
            the same pixels.
    \param  otherImageD  IDispatch to the comparison image
    \param  found        Filled with true or false depending on if they
                         are the same or not.
*/
STDMETHODIMP CCaptureImage::compare(IDispatch* otherImageD, BOOL * found)
{
    CCaptureImage * otherImage;
    otherImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&otherImage));

    if (otherImage == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (otherImage->_width != _width ||
        otherImage->_height != _height) {
        // An error for this function
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    *found = true;

    int x, y;

    for (x = 0; x < _width; x++)
        for (y = 0; y < _height; y++) {
            pixel_t * pixel      = getPixel(x,y);
            pixel_t * otherPixel = otherImage->getPixel(x,y);
            if (otherPixel->color.a != 0 && pixel2Y(pixel) != pixel2Y(otherPixel)) {
                *found = false;
                goto end;
            }
        }

    _foundX = x;
    _foundY = y;

end: otherImage->Release();
     return S_OK;
}

/**
    \brief  Take the data from an ABGR array and import it
    \param  width       Width of the image to import
    \param  height      Height of the image to import
    \param  abgr_array  A large data space of pixels for importation
*/
bool CCaptureImage::importPixels(USHORT width, USHORT height, ULONG * abgr_array)
{
    this->put_width(width);
    this->put_height(height);

    struct abgr_t {
        UCHAR b;
        UCHAR g;
        UCHAR r;
        UCHAR a;
    };
    abgr_t * inpixels = (abgr_t *)abgr_array;
    for (int x = 0; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            pixel_t * lpixel = getPixel(x,y);
            abgr_t *  ipixel = &inpixels[x + width * (height - y)];

            lpixel->color.a = 0xff;
            lpixel->color.r = ipixel->r;
            lpixel->color.g = ipixel->g;
            lpixel->color.b = ipixel->b;
        }

    return true;
}

/**
    \brief  Compare images over a given region
*/
STDMETHODIMP CCaptureImage::compareRegion(IDispatch * otherImageD, USHORT minx, USHORT miny, USHORT maxx, USHORT maxy, BOOL * found)
{
    if (maxx >= _width)  maxx = _width - 1;
    if (maxy >= _height) maxy = _height - 1;
    if (minx > maxx) minx = maxx;
    if (miny > maxy) miny = maxy;

    CCaptureImage * otherImage;
    otherImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&otherImage));

    if (otherImage == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Checking to ensure that there is enough space for this image */
    if (maxx - minx < otherImage->_width ||
            maxy - miny < otherImage->_height) {
        otherImage->Release();
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    int blah = 5; /* ignore this */
    for (int xout = minx; xout < maxx - otherImage->_width; xout++) {
        for (int yout = miny; yout < maxy - otherImage->_height; yout++) {

            for (int xin = xout; xin < otherImage->_width; xin++) {
                for (int yin = yout; yin < otherImage->_height; yin++) {
                    pixel_t * pixel =      getPixel(xin,yin);
                    pixel_t * otherPixel = otherImage->getPixel(xin - xout, yin - yout);
                    if (otherPixel->color.a != 0 && pixel2Y(pixel) != pixel2Y(otherPixel)) {
                        goto giveupinner; /* break out of two loops */
                    }
                } /* yin */
            } /* xin */

            /* If we get through the inner loops without finding a problem,
               then we've matched our image. */
            *found = true;
            _foundX = xout;
            _foundY = yout;
            otherImage->Release();
            return S_OK;

giveupinner: blah++; /* not used, but makes the compiler happy.  Should optimize out anyway. */
        } /* yout */
    } /* xout */

    *found = false;
    otherImage->Release();
    return S_OK;
}

/**
    \brief  Calculate the sum of the magnitudes of pixels in an 8x8 region
    \param  start_x  X position to start with (upper left)
    \param  start_y  Y position to start with (upper right)

    This function just increments to create an 8x8 square with the upper
    left corner being (start_x, start_y).  If it goes over the edge of
    the image, the last edge on the image is used in a repeated fasion.
*/

ULONG CCaptureImage::mag8x8(USHORT start_x, USHORT start_y){
    return mag8x8(start_x, start_y, _width, _height);
}

ULONG CCaptureImage::mag8x8(USHORT start_x, USHORT start_y, USHORT maxx, USHORT maxy)
{
    ULONG accumulator = 0;
    for (int x = 0; x < block_size; x++)
        for (int y = 0; y < block_size; y++) {
            int xpos = x + start_x;
            int ypos = y + start_y;
            if (xpos >= maxx) xpos = maxx - 1;
            if (ypos >= maxy) ypos = maxy - 1;
            accumulator += pixel2Y(this->getPixel(xpos, ypos));
        }
    return accumulator;
}

/**
    \brief  This function does a simple comparison with error
    \param  otherImageD  The image to compare against
    \param  fuzziness    The percentage of error in the value allowed
    \param  found        Whether or not the images matched

    This function compares two images by looking at the sums of the
    pixel's magnitude over an 8x8 region.  In that way, one pixel can't
    screw you and the comparison itself works over a larger region.  Both
    images get their magnitutes calculated via \c mag8x8, but then that
    value is compared with a \c fudgefactor that is determined using the
    passed in \c fuzziness.
*/
STDMETHODIMP CCaptureImage::compareFuzzy(IDispatch * otherImageD, USHORT fuzziness, BOOL * found)
{
    CCaptureImage * otherImage;
    otherImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&otherImage));

    if (otherImage == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (otherImage->_width != _width ||
        otherImage->_height != _height) {
        // An error for this function
        otherImage->Release();
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    // printf("Fuzziness: %d\n", fuzziness);
    ULONG fudgefactor = ((ULONG)(block_size * block_size * 256 / 2) * (ULONG)fuzziness) / 100;

    int x, y;

    for (x = 0; x < _width; x += block_size)
        for (y = 0; y < _height; y += block_size) {
            ULONG mymacromag = this->mag8x8(x, y);
            ULONG othermacromag = otherImage->mag8x8(x, y);
            ULONG minval = mymacromag - fudgefactor;
            if (fudgefactor > mymacromag) minval = 0;
            ULONG maxval = mymacromag + fudgefactor;

            if (othermacromag < minval ||
                    othermacromag > maxval) {
                // printf("X: %d  Y: %d\n", x, y);
                // printf("My: %d  Other: %d  Fudge: %d  Fuzziness: %f\n", mymacromag, othermacromag, fudgefactor, fuzziness);
                otherImage->Release();
                *found = false;
                return S_OK;
            }
        }

    otherImage->Release();
    *found = true;
    _foundX = x;
    _foundY = y;
    return S_OK;
}

STDMETHODIMP CCaptureImage::compareFuzzyRegion(IDispatch * otherImageD, USHORT fuzziness, USHORT minx, USHORT miny, USHORT maxx, USHORT maxy, BOOL * found)
{
    if (maxx >= _width)  maxx = _width - 1;
    if (maxy >= _height) maxy = _height - 1;
    if (minx > maxx) minx = maxx;
    if (miny > maxy) miny = maxy;

    CCaptureImage * otherImage;
    otherImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&otherImage));

    if (otherImage == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Checking to ensure that there is enough space for this image */
    if (maxx - minx < otherImage->_width ||
            maxy - miny < otherImage->_height) {
        otherImage->Release();
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    // printf("Fuzziness: %d\n", fuzziness);
    ULONG fudgefactor = ((ULONG)(block_size * block_size * 255) * (ULONG)fuzziness) / 100;

    /** \todo take care of case where input image is less than one block */

    USHORT firstblock_x_start = minx;
    USHORT firstblock_x_stop  = maxx - otherImage->_width;
    USHORT firstblock_y_start = miny;
    USHORT firstblock_y_stop  = maxy - otherImage->_height;

    ULONG firstblockY = otherImage->mag8x8(0,0);

    USHORT mostideal_x = 0;
    USHORT mostideal_y = 0;
    ULONG mostideal_distance = fudgefactor;

    USHORT x = 0, y = 0;

    for (x = firstblock_x_start; x < firstblock_x_stop; x++)
        for (y = firstblock_y_start; y < firstblock_y_stop; y++) {
            ULONG thisblockY = mag8x8(x,y);
            ULONG distance = thisblockY < firstblockY ? firstblockY - thisblockY : thisblockY - firstblockY;
            
            if (distance < mostideal_distance) {
                mostideal_x = x;
                mostideal_y = y;
            }
        }

    for (x = 0; x < otherImage->_width; x += block_size)
        for (y = 0; y < otherImage->_height; y += block_size) {
            ULONG mymacromag = this->mag8x8(x + mostideal_x, y + mostideal_y, mostideal_x + otherImage->_width, mostideal_y + otherImage->_height);
            ULONG othermacromag = otherImage->mag8x8(x, y);
            ULONG minval = mymacromag - fudgefactor;
            if (fudgefactor > mymacromag) minval = 0;
            ULONG maxval = mymacromag + fudgefactor;

            if (othermacromag < minval ||
                    othermacromag > maxval) {
                printf("X: %d  Y: %d\n", x, y);
                printf("My: %d  Other: %d  Fudge: %d  Fuzziness: %f\n", mymacromag, othermacromag, fudgefactor, fuzziness);
                otherImage->Release();
                *found = false;
                return S_OK;
            }
        }

    otherImage->Release();
    *found = true;
    _foundX = x;
    _foundY = y;
    return S_OK;
}

STDMETHODIMP CCaptureImage::getBuffer(ULONG** ppBuffer)
{
    *ppBuffer = &(_imageref[0].fullpixel);
    return S_OK;
}

/**
    \brief  Creates a new image that is the difference between
            this image and a passed in image.
    \param  compareTo  The image this should be differenced against
    \param  newImage   Where the newly generated image is placed
*/
STDMETHODIMP CCaptureImage::difference(IDispatch * compareTo, IDispatch ** newImage)
{
    CCaptureImage * compImage;
    compareTo->QueryInterface(__uuidof(ICaptureImage), (void **)(&compImage));

    if (_width != compImage->_width ||
            _height != compImage->_height) {
        compImage->Release();
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not of identical size", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Create a new image of the same size as the others */    
    CoCreateInstance(__uuidof(CCaptureImage), NULL, CLSCTX_ALL, __uuidof(IDispatch), (void **)newImage);
    CCaptureImage * diffImage;
    (*newImage)->QueryInterface(__uuidof(ICaptureImage), (void **)(&diffImage));
    diffImage->put_width(_width);
    diffImage->put_height(_height);

    for(int x = 0; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            pixel_t * thisPixel = getPixel(x, y);
            pixel_t * compPixel = compImage->getPixel(x, y);
            pixel_t * diffPixel = diffImage->getPixel(x, y);

            if (thisPixel->color.a != 0xFF || compPixel->color.a != 0xFF) {
                diffPixel->color.a = 0x00;
                diffPixel->color.b = 0x00;
                diffPixel->color.g = 0x00;
                diffPixel->color.r = 0x00;
                continue;
            }

            class bf { public:
            static BYTE bytefunc (BYTE s1, BYTE s2) {
#if 0
            /* Basically, getting the absolute value of the difference, by
               color.  Not a useful image, but useful for post-mortem analysis. */
                return s1 < s2 ? s2 - s1 : s1 - s2;
#else
            /* Taking everything to a center value, and the looking at the
               difference on each side of that center.  So if the input is 0x00
               and 0xFF the output will either be 0x00 or 0xFF depending on the
               ordering. */
                return 0x80 + (s1 >> 1) - (s2 >> 1);
#endif
            }
            };

            diffPixel->color.a = 0xFF;
            diffPixel->color.b = bf::bytefunc(thisPixel->color.b, compPixel->color.b);
            diffPixel->color.g = bf::bytefunc(thisPixel->color.g, compPixel->color.g);
            diffPixel->color.r = bf::bytefunc(thisPixel->color.r, compPixel->color.r);
        }

    diffImage->Release();
    compImage->Release();

    return S_OK;
}

/**
    \brief  Crop the current image to a smaller size, at a specific
            location.
    \param  x       X position of the crop
    \param  y       Y position of the crop
    \param  width   Width of the resulting image
    \param  height  Height of the image
*/
STDMETHODIMP CCaptureImage::crop(USHORT x, USHORT y, USHORT width, USHORT height)
{
    pixel_t * newImage = (pixel_t *)malloc(sizeof(pixel_t) * width * height);

    for (int xi = 0; xi < width; xi++)
        for (int yi = 0; yi < height; yi++) {
            pixel_t * oldpixel = getPixel(xi + x, yi + y);
            pixel_t * newpixel = getPixel(xi, yi, newImage, width, height);

            newpixel->fullpixel = oldpixel->fullpixel;
        }

    _width = width;
    _height = height;
    free(_imageref);
    _imageref = newImage;

    return S_OK;
}

/**
    \brief  Copy all the data out of an image into the current one
    \param  original  Image to copy from
*/
STDMETHODIMP CCaptureImage::copy(IDispatch * original)
{
    CCaptureImage * oimage;
    original->QueryInterface(__uuidof(ICaptureImage), (void **)(&oimage));

    put_width(oimage->_width);
    put_height(oimage->_height);

    memcpy((void *)_imageref, (void *)oimage->_imageref, sizeof(pixel_t) * _width * _height);

    oimage->Release();

    return S_OK;
}

STDMETHODIMP CCaptureImage::differenceLuma(IDispatch * compareTo, IDispatch ** newImage)
{
    CCaptureImage * compImage;
    compareTo->QueryInterface(__uuidof(ICaptureImage), (void **)(&compImage));

    if (_width != compImage->_width ||
            _height != compImage->_height) {
        compImage->Release();
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not of identical size", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Create a new image of the same size as the others */    
    CoCreateInstance(__uuidof(CCaptureImage), NULL, CLSCTX_ALL, __uuidof(IDispatch), (void **)newImage);
    CCaptureImage * diffImage;
    (*newImage)->QueryInterface(__uuidof(ICaptureImage), (void **)(&diffImage));
    diffImage->put_width(_width);
    diffImage->put_height(_height);

    for(int x = 0; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            pixel_t * thisPixel = getPixel(x, y);
            pixel_t * compPixel = compImage->getPixel(x, y);
            pixel_t * diffPixel = diffImage->getPixel(x, y);

            if (thisPixel->color.a != 0xFF || compPixel->color.a != 0xFF) {
                diffPixel->color.a = 0x00;
                diffPixel->color.b = 0x00;
                diffPixel->color.g = 0x00;
                diffPixel->color.r = 0x00;
                continue;
            }

            diffPixel->color.a = 0xFF;

            BYTE thisLuma = pixel2Y(thisPixel);
            BYTE compLuma = pixel2Y(compPixel);
            BYTE diffLuma = 0x80 + (thisLuma >> 1) - (compLuma >> 1);

            Y2pixel(diffLuma, diffPixel);
        }

    diffImage->Release();
    compImage->Release();

    return S_OK;
}

/** \brief  This is a helper class for \c Bitmap.  It just makes it so
            that bitmap can be accessed with two brakets for x, y parameters
            in the bitmap.
*/

/** \brief  A template class to make a 2-d array of T's */
template <class T>
class bitmap {
protected:
    /** Array of T's */
    T * _map;
    /** Height of the map */
    int _height;
    /** Width of the map */
    int _width;
public:
    /** Set the width and height, allocate memory for the bitmap */
    bitmap(int width, int height) : _width(width), _height(height) {
        _map = (T *)malloc(sizeof(T) * area());
        return;
    }
    /** Free the bitmap */
    ~bitmap(void) {
        free(_map);
        return;
    }
    /** Accesses the bitmap with the help of \c BitmapRow.  This function
        mostly allocated a BitmapRow with all the variables it will need
        to do an array lookup as it is called with the second bracket next. */
    T * p (int x, int y) {
        if (x >= _width || y >= _height) printf("FAIL\n");
        return &_map[y * _width + x];
    }

    /** Returns \c _width */
    int width (void)  { return _width;  };
    /** Returns \c _height */
    int height (void) { return _height; };

    /** Finds the smallest value in the bitmap and returns it */
    T minimum (void) {
        T returnval = _map[0];
        for (int x = 0; x < area(); x++)
            if (_map[x] < returnval) returnval = _map[x];
        return returnval;
    }
    /** Returns the area of the bitmap (width times height) */
    int area (void) {return _width * _height;}
    /** Returns the number of values which are true. */
    int numTrue (void) {
        int returnval = 0;
        for (int x = 0; x < area(); x++)
            if (_map[x]) returnval++;
        return returnval;
    }
    /**  Finds a high watermark in the array.  It does this by creating
         an array of all the values in the bitmap, and then counts how
         many times those values occur.  It then counts down until 5%
         of the values are higher than the returned value.  O(N+k) */
    T highval_percent (void) {
        int values[256] = {0};
        for (int i = 0; i < area(); i++) {
            values[_map[i]]++;
        }
        
        int count = (area() * 5) / 100;
        for (int i = 255; i >= 0; i--) {
            if (values[i] > count)
                return i;
            count -= values[i];
        }
        return 0;
    }
    /**  Finds a low watermark in the array.  It does this by creating
         an array of all the values in the bitmap, and then counts how
         many times those values occur.  It then counts up until 5%
         of the values are lower than the returned value.  O(N+k) */
    T lowval_percent (void) {
        int values[256] = {0};
        for (int i = 0; i < area(); i++) {
            values[_map[i]]++;
        }
        
        int count = (area() * 5) / 100;
        for (int i = 0; i < 256; i++) {
            if (values[i] > count)
                return i;
            count -= values[i];
        }
        return 0;
    }

    T locally_run (int size, bool low) {
        slide_bitmap<T> localmap (size, size);
        int step_size = size / 2;
        T returnval;

        if (low)
            returnval = 255;
        else
            returnval = 0;

        // 
        for (int y = 0; y < _height + size; y += step_size) {

            for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++) {
                int bitmx = i;
                int bitmy = j + y;

                if (!(bitmx < _width))
                    bitmx = _width - 1;
                if (y + size > _height)
                    bitmy = (_height - size) + j;

                *(localmap.p(i, j)) = *(this->p(bitmx, bitmy));
            } // i,j for loops

            for (int x = size; x < _width + size; x += step_size) {
                if (low) {
                    T lowest = localmap.lowval_percent();
                    if (lowest < returnval)
                        returnval = lowest;
                } else {
                    T highest = localmap.highval_percent();
                    if (highest > returnval)
                        returnval = highest;
                }
                
                for (int eachstep = 0; eachstep < step_size; eachstep++) {
                    localmap.slide();
                    for (int ly = 0; ly < size; ly += 1) {
                        int bitmx = x + eachstep;
                        int bitmy = ly + y;

                        if (x + size > _width) {
                            if (eachstep + x >= _width) {
                                bitmx = (_width - size) + eachstep;
                            }
                        }
                        if (y + size > _height)
                            bitmy = (_height - size) + ly;

                        *(localmap.p(localmap._width - 1,ly)) = *(this->p(bitmx, bitmy));
                    } // ly loop
                } // each step
            } // x loop
        } // y for loop

        return returnval;
    }

    T lowval (void) {
        return locally_run(16, true);
    }

    T highval (void) {
        return locally_run(16, false);
    }

    /** \brief A function to scale the bitmap from a given range to the
               full range of the variable.
        \param low  The low end of the defined range
        \param high The high end of the defined range

        First the range is found, and compared to a maximum expansion.  The
        range will only be expanded by so much, this limits the normalization
        on relatively solid colors.

        Each entry is then checked to make sure it is not out of the bounds
        of the range, and then expanded to the larger range of the type.
    */
    void scale (T low, T high) {
        const static BYTE minrange = 50;
        // printf("Scale with low: %d and  high: %d\n", low, high);
        T range = high - low;
        if (range < minrange) {
            if (low > 255 - minrange) {
                low = 255 - minrange;
                high = 255;
            } else {
                high = low + minrange;
            }
            range = minrange;
        }
        for (int i = 0; i < area(); i++) {
            if (_map[i] < low) _map[i] = low;
            if (_map[i] > high) _map[i] = high;
            _map[i] = (T)(((int)(_map[i] - low) * (int)255) / (int)range);
        }
    }
    /** \brief  Save the bitmap as a PNG
        \param  Filename to use

        This function turns the bitmap into a new CaptureImage.  It then
        uses that class' function to save the resulting image as a PNG.
    */
    void savePng (BSTR filename) {
        IDispatch * newImageD;
        CoCreateInstance(__uuidof(CCaptureImage), NULL, CLSCTX_ALL, __uuidof(IDispatch), (void **)&newImageD);
        CCaptureImage * newImage;
        newImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&newImage));
        
        newImage->put_width(_width);
        newImage->put_height(_height);

        for (int x = 0; x < _width; x++)
            for (int y = 0; y < _height; y++) {
                Y2pixel(*(this->p(x, y)), newImage->getPixel(x,y));
            }

        newImage->savePng(filename);
        newImage->Release();
        newImageD->Release();
        return;
    }

    /** \brief  Find the average value of all the entires in the map
        \warning  This function uses a long for the total sum, large types
                  with large values may overflow this.  */
    T average (void) {
        long sum = 0;
        for (int i = 0; i < area(); i++) {
            sum += _map[i];
        }
        sum /= area();
        return (T)sum;
    }

    T standardDeviation (void) {
        T av = average();
        float sum = 0;
        for (int i = 0; i < area(); i++) {
            sum += (_map[i] - av) * (_map[i] - av);
        }
        sum /= area();
        return (T)((float)sqrt(sum));
    }

    /** \brief  Subtract a value from every entry in the map */
    short operator-= (short in_val) {
        for (int i = 0; i < area(); i++) {
            _map[i] -= in_val;
        }
        return in_val;
    }

    /** \brief  Remove all the negatives from a map */
    void abs (void) {
        for (int i = 0; i < area(); i++) {
            if (_map[i] < 0)
                _map[i] = -_map[i];
        }

    }
};

/** \brief  A bitmap which allows itself to be 'slid' along, saving the
            values on the right while exposing the last values as a new
            row.  Makes a sliding window much easier to code.
*/
template <class T>
class slide_bitmap : public bitmap<T> {
private:
    /**  Current amount that we've slid */
    int _slide;
public:
    /**  Same initializer as \c bitmap, but set the slide to zero */
    slide_bitmap (int width, int height) : bitmap<T>(width, height), _slide(0) {};
    /**  Actually increment the slide variable.  It always makes sure
         that it is smaller than the width. */
    void slide (int amount = 1) {
        _slide += amount;
        while (_slide >= _width)
            _slide -= _width;
        return;
    };
    T * p (int x, int y) {
        if (x >= _width)
            printf("Width too big!  %d is more than %d!\n", x, _width);
        x = x + _slide;
        if (x >= _width)
            x -= _width;

        return bitmap<T>::p(x,y);
    }
    /**  Takes the slide back to zero. */
    void clear_slide (void) { _slide = 0; }

};

/** \brief  In a nutshell, the edge class is an enum with lots of
            operators.  It is used to define a discovered edge.
*/
class edge {
public:
    /**  An edge is actually a char */
    typedef char edge_t;
    /**  The value when there is no edge found */
    static const edge_t NONE = 0x0;
    /**  When the edge is horizontal */
    static const edge_t HORZ = 0x1;
    /**  When the edge is vertical */
    static const edge_t VERT = 0x2;
    /**  When the edge is both horizontal and vertical */
    static const edge_t BOTH = 0x3;
private:
    /**  This is the actual value of the class.  With the inlines, this
         is really all that should exist anywhere. */
    edge_t value;
public:
    /**  Initializes the value to NONE on creation */
    edge (void) : value(NONE) { };
    /**  It can be created with an \c edge_t and take on that value also */
    edge (edge_t in_val) : value(in_val) { };
    /**  If it is on the left size of an equals it can take the edge_t value on the right */
    edge_t operator= (edge_t in_val) { value = in_val; return value;};
    /**  Allows bits to be set up with \c edge_t values */
    edge_t operator|= (edge_t in_val) { value |= in_val; return value;};
    /**  Allows edges to be anded against the value */
    edge_t operator&= (edge_t in_val) { value &= in_val; return value;};
    /**  Sets bits down in the internal value */
    edge_t operator-= (edge_t in_val) { value &= ~in_val; return value;};
    /**  Checks to see if there is overlap in the edges found by the two values */
    bool operator== (edge_t in_val) { return ((value & in_val) != 0) || ((value == NONE) && (in_val == NONE)); };
    //operator BYTE (void) { return (BYTE)value; };
    /**  Allow this class to be converted to an \c edge_t so that all
         the above functions work like you'd expect. */
    operator edge_t (void) { return value; };
    /**  Allow this class to be turned into a boolean which is based only
         on whether it is NONE or not. */
    operator bool (void) { return value != NONE; };
};

/** \brief  Compare two images looking for text
    \param  otherImageD  A reference to the smaller comparison image
    \param  factor       A term to control how selective the search is
    \param  found        The return value of whether the text was found.
*/
STDMETHODIMP CCaptureImage::compareText(IDispatch * otherImageD, USHORT factor, BOOL * found)
{
    // printf("Start compareText\n");
    return compareTextRegion(otherImageD, factor, 0, 0, 0xFFFF, 0xFFFF, found);
}

/**
    \def    DEBUG_TEXT
    \brief  A macro to define whether the text testing code should be
            included in the build or not.
*/
#if 0
#define DEBUG_TEXT(x)  x
#else
#define DEBUG_TEXT(x)  if (_debug) x
#endif

/** \brief  Compare two images looking for text within a specified region
    \param  otherImageD  A reference to the smaller comparison image
    \param  factor       A term to control how selective the search is
    \param  found        The return value of whether the text was found.
    \param  minx         Minimum X defining the region
    \param  miny         Minimum Y defining the region
    \param  maxx         Maximum X defining the region
    \param  maxy         Maximum Y defining the region

    Wow, this is a complex task that basically looks at to edge detected
    versions of the images and compares them, but that's just the overview.

    First thing that is done is a Luma map is made for each of the images.
    The luma values for the pixels are reused several times, and the memory
    becomes worthwhile if we need them so many times.  Both images have
    their luma values calculated using \c pixel2Y.

    Next, both of the images are normalized to try and equalize the values
    in the images.  This is done by finding the 95% and 5% values in the
    image to try and reduce noise.  The images are rescaled to the full
    range of a byte.

    The images then both go through the same edge detection algorithm to
    generate a new bitmap of the edges.  The algorithm is simple, just
    looking for luma changes on the trailing X and Y axises.  Each
    sitution generates a different value from the \c edge class, and is
    tracked in a bitmap of that class.

    For the next section of code a sliding window is created that is the
    height of the region being searched, and a square.  This makes the
    assumption that all searched text will be written horizontal.  The
    window moves along looking for matching edges and counting how much
    congrueance between the two images.  If, over the entire image, the
    errors are within tollerance, then the images are said to match.
*/
STDMETHODIMP CCaptureImage::compareTextRegion(IDispatch * textD, USHORT factor, USHORT minx, USHORT miny, USHORT maxx, USHORT maxy, BOOL * found)
{
    _bestX = 0;
    // printf("Start compareTextRegion\n");
    CCaptureImage * text;
    textD->QueryInterface(__uuidof(ICaptureImage), (void **)(&text));

    if (text == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (maxx > _width) maxx = _width;
    if (maxy > _height) maxy = _height;
        /* These two will pretty much cause an error, but
           that'll get grabbed in the next check */
    if (minx >= maxx) minx = maxx - 1;
    if (miny >= maxy) miny = maxy - 1;

    if (text->_width > maxx - minx ||
        text->_height > maxy - miny) {
        // An error for this function
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Build Luma maps of our two images, this is done before hand
       here because they'll be itterated over several times */
    bitmap<BYTE> origLumaMap(maxx - minx, maxy - miny);
    bitmap<BYTE> textLumaMap(text->_width, text->_height);

    for (int x = 0; x < maxx - minx; x++)
        for (int y = 0; y < maxy - miny; y++)
            *origLumaMap.p(x,y) = pixel2Y(getPixel(x + minx,y + miny));

    for (int x = 0; x < text->_width; x++)
        for (int y = 0; y < text->_height; y++)
            *textLumaMap.p(x,y) = pixel2Y(text->getPixel(x,y));
    text->Release();

    /* Scale the images so that we're all talking 0-255 here.  The high and
       low val functions find the 95% and 5% value to remove noise. */
    // printf("Orig  Low: %d  High: %d\n", origLumaMap.lowval(), origLumaMap.highval());
    // printf("Text  Low: %d  High: %d\n", textLumaMap.lowval(), textLumaMap.highval());
    DEBUG_TEXT(origLumaMap.savePng(L"c:\\test\\orig-luma.png"));
    DEBUG_TEXT(textLumaMap.savePng(L"c:\\test\\text-luma.png"));
    origLumaMap.scale(origLumaMap.lowval(), origLumaMap.highval());
    textLumaMap.scale(textLumaMap.lowval(), textLumaMap.highval());

    /* Create maps of where the edges are in the two diagrams.  Note that
       the factor to look at the difference is 128, which means that there
       needs to be a significant transition.  Since this is Luma, it would
       be closest to what the user actually sees -- so therefor a large
       transition would equate to high contrast, which is what the text
       should be if the "guys with Macs" did things right :)  */
    bitmap<edge> origEdgeMap(origLumaMap.width(), origLumaMap.height());
    bitmap<edge> textEdgeMap(textLumaMap.width(), textLumaMap.height());

    class InlineFunctions { public:
        inline static void luma2edge (bitmap<BYTE> &lumaMap, bitmap<edge> &edgeMap) {
            const BYTE EDGE_THRESHOLD = 55;
            for (int x = 0; x < lumaMap.width(); x++)
                for (int y = 0; y < lumaMap.height(); y++) {
                    int xminusone = (x == 0 ? 0 : x - 1);
                    int yminusone = (y == 0 ? 0 : y - 1);
                    int xedge = (int)*lumaMap.p(x,y) - (int)*lumaMap.p(xminusone,y);
                    int yedge = (int)*lumaMap.p(x,y) - (int)*lumaMap.p(x,yminusone);
                    *edgeMap.p(x,y) = edge::NONE;
                    if (xedge > EDGE_THRESHOLD  || xedge < -EDGE_THRESHOLD)
                        *edgeMap.p(x,y) |= edge::HORZ;
                    if (yedge > EDGE_THRESHOLD  || yedge < -EDGE_THRESHOLD)
                        *edgeMap.p(x,y) |= edge::VERT;
                }

            return;
        }
        inline static void edgeSpeckle (bitmap<edge> &edgeMap) {
            for (int x = 1; x < edgeMap.width() - 1; x++)
                for (int y = 1; y < edgeMap.height() - 1; y++) {
                    int neighbors = 0;
                    for (int cnt = 0; cnt < 9; cnt++) {
                        int xval = x - 1 + (cnt % 3);
                        int yval = y - 1 + (cnt / 3);

                        if (xval < 0 || xval == edgeMap.width()) continue;
                        if (yval < 0 || yval == edgeMap.height()) continue;

                        if ((edge::edge_t)*edgeMap.p(xval, yval) != edge::NONE) {
                            neighbors++;
                        }    
                    }

                    if (neighbors == 1) {
                        *edgeMap.p(x,y) = edge::NONE;
                    }
                }

            return;
        }
    };

    InlineFunctions::luma2edge(origLumaMap, origEdgeMap);
    InlineFunctions::luma2edge(textLumaMap, textEdgeMap);

    // Remove lone edges in both images
    InlineFunctions::edgeSpeckle(origEdgeMap);
    InlineFunctions::edgeSpeckle(textEdgeMap);

    /* Test to find out how things are */
    DEBUG_TEXT(printf("Truth in Orig: %d\n", origEdgeMap.numTrue()));
    DEBUG_TEXT(printf("Truth in Text: %d\n", textEdgeMap.numTrue()));

    DEBUG_TEXT(origLumaMap.savePng(L"c:\\test\\orig-norm.png"));
    DEBUG_TEXT(textLumaMap.savePng(L"c:\\test\\text-norm.png"));
    DEBUG_TEXT(origEdgeMap.savePng(L"c:\\test\\orig-edge.png"));
    DEBUG_TEXT(textEdgeMap.savePng(L"c:\\test\\text-edge.png"));

    int window_size = textEdgeMap.height();
    if (textEdgeMap.height() > textEdgeMap.width() >> 2) {
        window_size = textEdgeMap.width() >> 2;
    }

    slide_bitmap<bool> window(window_size, window_size);
    int requiredTruth = ((int)window.area() * (int)factor) / 100;

    for (int base_x = 0; base_x < origEdgeMap.width()  - textEdgeMap.width()  + 1; base_x++) {
    for (int base_y = 0; base_y < origEdgeMap.height() - textEdgeMap.height() + 1; base_y++) {
        bool completed = true;

        for (int ty = 0; ty < textEdgeMap.height() - window_size + 1; ty += window_size / 2) {
            class compedge { public:
                inline static bool comp_edge (int x, int y, edge orig, edge text) {
                    if (x == 0)
                        orig -= edge::HORZ;
                    if (y == 0)
                        orig -= edge::VERT;

                    return (orig == text);
                };
            };

            window.clear_slide();
            for (int x = 0; x < window.width(); x++)
                for (int y = 0; y < window.height(); y++)
                    *window.p(x,y) = compedge::comp_edge(x, ty + y, *origEdgeMap.p(base_x + x,base_y + y + ty), *textEdgeMap.p(x,y + ty));

            const static int step_size = 1;
            for (int x = window.width(); x < textEdgeMap.width(); x += step_size) {
                if (x > _bestX) { _bestX = (ULONG)x; }
                if (window.numTrue() < requiredTruth) {
                    completed = false;
                    break;
                }

                window.slide(step_size);
                for (int y = 0; y < window.height(); y++) {
                    *window.p(window.width() - 1, y) = compedge::comp_edge(x,y + ty, *origEdgeMap.p(base_x + x, base_y + y + ty), *textEdgeMap.p(x, y + ty));
                } // y
            } // x

            if (!completed) {
                break;
            }
        }

        if (completed) {
            *found = true;
            DEBUG_TEXT(printf("Found at: X: %d  Y: %d\n", base_x, base_y));
            _foundX = base_x;
            _foundY = base_y;
            return S_OK;
        }
    } /* base_y */
    } /* base_x */


    *found = false;
    return S_OK;
}

/**
    \brief  Compare two images using their luminance and ignoring alpha regions
    \param  factor  A factor measuring how close they need to be.  0-100.
    \param  otherImageD  The image to compare against
    \param  found  A return value to say whether the image was found.

    The function calls \c comparePercentRegion with a region of zero
    to infinity to force Region to crop those down to _height and _width.
*/
STDMETHODIMP CCaptureImage::comparePercent(IDispatch * otherImageD, USHORT factor, BOOL * found)
{
    // printf("Start comparePercent\n");
    return comparePercentRegion(otherImageD, factor, 0, 0, 0xFFFF, 0xFFFF, found);
}

/**
    \brief  Compare lumas with error over a specified region
    \param  otherImageD  Image to compare to
    \param  factor  Adjustment for the error checking code. 0-100.
    \param  found   Return value on whether an image was matched
    \param  minx    Minimum X defining the region
    \param  miny    Minimum Y defining the region
    \param  maxx    Maximum X defining the region
    \param  maxy    Maximum Y defining the region

    This function first checks to make sure that the bounding box is within
    the orignal image, and that it is big enough to hold the compared image.
    It then determines the amount of error that is tollerable, both in the range
    of Luma values allowed, and the number of errored pixels allowed.

    Then we loop.

    The outer loops go over the possible places that the passed in image can
    occur within the bounding box.  If the images are the same size this can
    only occur at 0,0.  Then, for each of those locations, the entire range
    of pixels is examined to see if the luma values are within the tollerance
    allowed.  If they aren't, the number of errors are incremented.
*/
STDMETHODIMP CCaptureImage::comparePercentRegion(IDispatch * otherImageD, USHORT factor, USHORT minx, USHORT miny, USHORT maxx, USHORT maxy, BOOL * found)
{
    // printf("Start comparePercentRegion\n");
    CCaptureImage * otherImage;
    otherImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&otherImage));

    if (otherImage == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (maxx > _width) maxx = _width;
    if (maxy > _height) maxy = _height;
        /* These two will pretty much cause an error, but
           that'll get grabbed in the next check */
    if (minx >= maxx) minx = maxx - 1;
    if (miny >= maxy) miny = maxy - 1;

    if (otherImage->_width > maxx - minx ||
        otherImage->_height > maxy - miny) {
        // An error for this function
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Build Luma maps of our two images, this is done before hand
       here because they'll be itterated over several times */
    bitmap<BYTE> origLumaMap(maxx - minx, maxy - miny);
    bitmap<BYTE> othrLumaMap(otherImage->_width, otherImage->_height);

    for (int x = 0; x < maxx - minx; x++)
        for (int y = 0; y < maxy - miny; y++)
            *origLumaMap.p(x,y) = pixel2Y(getPixel(x + minx,y + miny));

    for (int x = 0; x < otherImage->_width; x++)
        for (int y = 0; y < otherImage->_height; y++)
            *othrLumaMap.p(x,y) = pixel2Y(otherImage->getPixel(x,y));
    otherImage->Release();


    if (factor > 100) factor = 100;
    factor = 100 - factor;
    BYTE offset = (BYTE)((USHORT)255 * factor / (USHORT)100);
    int pixels = (othrLumaMap.width() * othrLumaMap.height() * (int)factor) / (int)1000;

    for (int base_x = 0; base_x <= origLumaMap.width() - othrLumaMap.width();   base_x += 1) {
    for (int base_y = 0; base_y <= origLumaMap.height() - othrLumaMap.height(); base_y += 1) {
        int errors = 0;

        for (int x = 0; x < othrLumaMap.width(); x++)
            for (int y = 0; y < othrLumaMap.height(); y++) {
                UCHAR luma = *origLumaMap.p(base_x + x, base_y + y);
                UCHAR otherLuma = *othrLumaMap.p(x,y);

                UCHAR low = luma < offset ? (UCHAR)0 : luma - offset;
                UCHAR high = luma > (0xFF - offset) ? 0xFF : luma + offset;

                if (!(otherLuma <= high && otherLuma >= low)) {
                    errors++;
                    if (errors > pixels) goto end_inner;
                }
            }

end_inner:
        if (errors > pixels) {
              // printf("No luck sir.\n");
            *found = false;
        } else {
              // printf("No, I really found it.\n");
            *found = true;
            _foundX = base_x;
            _foundY = base_y;
            return S_OK;
        }
    } /* base_y */
    } /* base_x */

    *found = false;
    // printf("Function Exited: %d\n", *found);
    return S_OK;
}

/**
    \brief  This function removes all the saturation in an image

    It does this by converting a pixel into a luma value, and then
    converting it back to a pixel.  This is done for every pixel in
    the image.
*/
STDMETHODIMP CCaptureImage::satZero(void)
{
    for (int i = 0; i < _width * _height; i++) {
        pixel_t * thispixel = &_imageref[i];
        BYTE thisluma = pixel2Y(thispixel);
        Y2pixel(thisluma, thispixel);
    }

    return S_OK;
}

/**
    \brief  This function compares two black and white images
    \param  otherImageD  A reference to the smaller comparison image
    \param  factor       A term to control how selective the search is
    \param  found        The return value of whether the text was found.
    \param  minx         Minimum X defining the region
    \param  miny         Minimum Y defining the region
    \param  maxx         Maximum X defining the region
    \param  maxy         Maximum Y defining the region


*/
STDMETHODIMP CCaptureImage::compareImageBWRegion(IDispatch * otherImageD, USHORT factor, USHORT minx, USHORT miny, USHORT maxx, USHORT maxy, BOOL * found)
{
    if (_width == 0 || _height == 0) {
        *found = FALSE;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: This image is too small", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    // printf("Start compareImageBWRegion\n");
    CCaptureImage * otherImage = NULL;
    otherImageD->QueryInterface(__uuidof(ICaptureImage), (void **)(&otherImage));

    if (otherImage == NULL) {
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (otherImage->_width == 0 || otherImage->_height == 0) {
        *found = FALSE;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image has a size of zero", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (maxx > _width) maxx = _width;
    if (maxy > _height) maxy = _height;
        /* These two will pretty much cause an error, but
           that'll get grabbed in the next check */
    if (minx >= maxx) minx = maxx - 1;
    if (miny >= maxy) miny = maxy - 1;

    if (otherImage->_width > maxx - minx ||
        otherImage->_height > maxy - miny) {
        // An error for this function
        *found = false;
	return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image larger than search region", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    /* Build Luma maps of our two images, this is done before hand
       here because they'll be itterated over several times */
    bitmap<BYTE> origLumaMap(maxx - minx, maxy - miny);
    bitmap<BYTE> othrLumaMap(otherImage->_width, otherImage->_height);

    for (int x = 0; x < maxx - minx; x++)
        for (int y = 0; y < maxy - miny; y++)
            *origLumaMap.p(x,y) = pixel2Y(getPixel(x + minx,y + miny));

    for (int x = 0; x < otherImage->_width; x++)
        for (int y = 0; y < otherImage->_height; y++)
            *othrLumaMap.p(x,y) = pixel2Y(otherImage->getPixel(x,y));
    otherImage->Release();


    bitmap<short> differenceMap(othrLumaMap.width(), othrLumaMap.height());
    bitmap<bool>  errorMap     (othrLumaMap.width(), othrLumaMap.height());
    const static int block_size = 10;

    int errorTolerance = ((100 - factor) * block_size * block_size) / 100;

    *found = FALSE;
    for (int base_y = 0; base_y < origLumaMap.height() - othrLumaMap.height() + 1 && *found == FALSE;  base_y++) {
    for (int base_x = 0; base_x < origLumaMap.width()  - othrLumaMap.width()  + 1 && *found == FALSE;  base_x++) {

        for (int y = 0; y < othrLumaMap.height(); y++) {
        for (int x = 0; x < othrLumaMap.width(); x++) {
            *differenceMap.p(x,y) = *origLumaMap.p(base_x + x, base_y + y) - *othrLumaMap.p(x, y);
        } /* x */
        } /* y */

        // printf("Average at:  X: %d  Y: %d  Average: %d  Deviation: %d\n", base_x, base_y, differenceMap.average(), differenceMap.standardDeviation());
        differenceMap -= differenceMap.average();
        differenceMap.abs();
        // differenceMap.scale(0, differenceMap.highval());
        //printf("Average at:  X: %d  Y: %d  Average: %d  Deviation: %d\n", base_x, base_y, differenceMap.average(), differenceMap.standardDeviation());

        short differenceAmount = 20;

        for (int y = 0; y < differenceMap.height(); y++) {
        for (int x = 0; x < differenceMap.width(); x++) {
            if (*differenceMap.p(x, y) > differenceAmount) {
                *errorMap.p(x,y) = true;
            } else {
                *errorMap.p(x,y) = false;
            }
        } /* x */
        } /* y */

        // printf("Error Map has %d errors out of %d entries.\n", errorMap.numTrue(), errorMap.area());

        bool stillhaveit = true;
        for (int block_y = 0; block_y < errorMap.height() + block_size && stillhaveit; block_y++) {
        for (int block_x = 0; block_x < errorMap.width() + block_size  && stillhaveit; block_x++) {
            int numErrors = 0;
            int pixelsReviewed = 0;
            for (int y = 0; y < block_size && y + block_y < errorMap.height(); y++) {
            for (int x = 0; x < block_size && x + block_x < errorMap.width();  x++) {
                if (*errorMap.p(block_x + x, block_y + y))
                    numErrors++;
                pixelsReviewed++;
            } /* x */
            } /* y */

            int errorToUse;
            if (pixelsReviewed == block_size * block_size) {
                errorToUse = errorTolerance;
            } else {
                errorToUse = ((100 - factor) * pixelsReviewed) / 100;
            }
            
            if (numErrors > errorToUse) {
                // printf("Lost block at: %dx%d with %d errors and %d allowed.\n", block_x + base_x, block_y + base_y, numErrors, errorTolerance);
                stillhaveit = false;
            }
        } /* block_x */
        } /* block_y */

        if (stillhaveit) {
            *found = TRUE;
            _foundX = base_x;
            _foundY = base_y;
        }

    } /* base_x */
    } /* base_y */

    return S_OK;
}

STDMETHODIMP CCaptureImage::replaceColor (IDispatch * oldColor, IDispatch * newColor)
{
    CCaptureImagePixel * oldColorP;
    oldColor->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&oldColorP));
    pixel_t oldC;
    oldColorP->get_fullVal(&(oldC.fullpixel));
    oldColorP->Release();

    CCaptureImagePixel * newColorP;
    newColor->QueryInterface(__uuidof(ICaptureImagePixel), (void **)(&newColorP));
    pixel_t newC;
    newColorP->get_fullVal(&(newC.fullpixel));
    newColorP->Release();

    for (int i = 0; i < _width * _height; i++) {
        if (_imageref[i] == oldC)
            _imageref[i] = newC;
    }

    return S_OK;
}

STDMETHODIMP CCaptureImage::get_debug(CHAR* pVal)
{
    *pVal = _debug;
    return S_OK;
}

STDMETHODIMP CCaptureImage::put_debug(CHAR newVal)
{
    if (newVal == 0)
        _debug = false;
    else
        _debug = true;
    return S_OK;
}

STDMETHODIMP CCaptureImage::get_foundX(ULONG* pVal)
{
    *pVal = _foundX;
    return S_OK;
}

STDMETHODIMP CCaptureImage::get_foundY(ULONG* pVal)
{
    *pVal = _foundY;
    return S_OK;
}

STDMETHODIMP CCaptureImage::hConv(ULONG factor)
{
    for (int x = 1; x < _width; x++)
        for (int y = 0; y < _height; y++) {
            pixel_t * pixel      = getPixel(x,y);
            pixel_t * otherPixel = getPixel(x - 1,y);
	    pixelAverage(factor, *pixel, *otherPixel, *pixel);
        }

    return S_OK;
}

STDMETHODIMP CCaptureImage::bitblt(SHORT x, SHORT y, IDispatch * inimage)
{
    if (_width == 0 || _height == 0) {
		return AtlReportError(GetObjectCLSID(), "CaptureImage: This image is too small", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    // printf("Start compareImageBWRegion\n");
    CCaptureImage * in_image = NULL;
    inimage->QueryInterface(__uuidof(ICaptureImage), (void **)(&in_image));

    if (in_image == NULL) {
		return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image not defined", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

    if (in_image->_width == 0 || in_image->_height == 0) {
		return AtlReportError(GetObjectCLSID(), "CaptureImage: Comparison image has a size of zero", __uuidof(ICaptureImage), MAKE_HRESULT(1,4,1000));
    }

	for (int loc_x = x; loc_x < _width && loc_x - x < in_image->_width; loc_x++){
	for (int loc_y = y; loc_y < _height && loc_y - y < in_image->_height; loc_y++) {
		if (loc_x < 0) continue;
		if (loc_y < 0) continue;

		pixel_t * lpixel = getPixel(loc_x, loc_y);
		pixel_t * rpixel = in_image->getPixel(loc_x - x, loc_y - y);
		lpixel->fullpixel = rpixel->fullpixel;
	} //loc_y
	} //loc_x

	in_image->Release();

	return S_OK;
}

STDMETHODIMP CCaptureImage::get_bestX(ULONG* pVal)
{
    *pVal = _bestX;
    return S_OK;
}

