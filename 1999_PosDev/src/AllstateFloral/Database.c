#pragma pack(2)

#include "Starter.h"
#include "String.h" 
#include "Scan.h" 
#include "PDIFile.h" 

#define DISPLAY_IF_ERROR if( Error ) DisplayFileError( Error );

//###########################################################################################		
//
//###########################################################################################		

int AddItemToOrder( char * p_szRecord )
{
	Err Error = ERROR_FAILIURE;
	
	//add item to order file
	Error = AppendOrderItemRecord( p_szRecord );
	if( Error == SUCCESS )
	{
		//if item count in order is zero
		if( g_OrderInfo.lngOrderItemsCount - 1 == 0 ) //variable gets incremented above
		{
			g_lngOrderIdxRecordPosition = FindOrderIdxRecord( p_szRecord );
			if( g_lngOrderIdxRecordPosition >= 0 )
			{
				PopulateOrderInfoStruct( p_szRecord );
				Error = UpdateOrderIdxRecord();
			}
			else //add record to index file
			{
				Error = AppendOrderIdxRecord();
			}
		}
		else // otherwise, update order index file, ship date might have changed
		{
			g_lngOrderIdxRecordPosition = FindOrderIdxRecord( p_szRecord );
			if( g_lngOrderIdxRecordPosition >= 0 )
			{
				PopulateOrderInfoStruct( p_szRecord );
				Error = UpdateOrderIdxRecord();
			}
		}
	}
	return Error;
}

int DeleteItemFromOrder( void )
{
	Err Error = ERROR_FAILIURE;

	Error = DeleteOrderItemRecord();
	if( Error == SUCCESS )
	{
		// if we deleted the last item in the order
		// also delete the order from the order INDEX file
		if( g_OrderInfo.lngOrderItemsCount == 0 )
		{
			FileDelete( 0, g_OrderInfo.szOrderNumber );
			Error = DeleteOrderIdxRecord();
		}
	}
	return Error;
}

//################################################################################
//RECORD OPERATION, update, append and delete
//################################################################################

//order items file

int DeleteOrderItemRecord( void )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	
	hFile = FileOpen( 0, (char*)g_OrderInfo.szOrderNumber, 0, 0, fileModeUpdate, &Error );
	if( Error == SUCCESS )
	{
		Error = DeleteRecord( hFile, ORDERS_LENGTH_RECORD, g_OrderInfo.lngOrderItemPosition );
		if( Error == SUCCESS )
		{
			g_OrderInfo.lngOrderItemsCount--;
		}
	}
	FileClose( hFile );
	return Error;
}

int UpdateOrderItemRecord( char * p_szRecord )
{
	Err Error = 0;
	FileHand * hFile = NULL;
	hFile = FileOpen( 0, (char*)g_OrderInfo.szOrderNumber, 0, 0, fileModeUpdate, &Error );
	if( Error == SUCCESS )
	{
		Error = UpdateRecord( hFile, ORDERS_LENGTH_RECORD, p_szRecord, g_OrderInfo.lngOrderItemPosition );
	}
	FileClose( hFile );
	return Error;
}

int AppendOrderItemRecord( char * p_szRecord )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	
	hFile = FileOpen( 0, (char*)g_OrderInfo.szOrderNumber, 0, 0, fileModeUpdate, &Error );
	if( Error == SUCCESS )
	{
		Error = AppendRecord( hFile, ORDERS_LENGTH_RECORD, p_szRecord );
		if( Error == SUCCESS )
		{
			g_OrderInfo.lngOrderItemsCount++;
		}
	}
	FileClose( hFile );
	return Error;
}


//order.idx

int UpdateOrderIdxRecord( void )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	char szData[ ORDERSINDEX_LENGTH_RECORD + 10 ];
	
	hFile = FileOpen( 0, (char*)szFILE_ORDERS_IDX, 0, 0, fileModeUpdate, &Error );
	if( Error == SUCCESS )
	{
		ComposeOrderIdxRecord( szData );
		Error = UpdateRecord( hFile, ORDERSINDEX_LENGTH_RECORD, szData, g_lngOrderIdxRecordPosition );
	}
	FileClose( hFile );
	return Error;
}

int AppendOrderIdxRecord( void )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	char szData[ ORDERSINDEX_LENGTH_RECORD + 10 ];
	
	hFile = FileOpen( 0, (char*)szFILE_ORDERS_IDX, 0, 0, fileModeUpdate, &Error );
	if( Error == SUCCESS )
	{
		ComposeOrderIdxRecord( szData );
		Error = AppendRecord( hFile, ORDERSINDEX_LENGTH_RECORD, szData );
		if( Error == SUCCESS )
		{
			if( g_lngOrderIdxRecordCount == 0 )
				g_lngOrderIdxRecordPosition = 0;
			g_lngOrderIdxRecordCount++;
		}
	}
	FileClose( hFile );
	return Error;
}

int DeleteOrderIdxRecord( void )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	
	hFile = FileOpen( 0, (char*)szFILE_ORDERS_IDX, 0, 0, fileModeUpdate, &Error );
	if( Error == SUCCESS )
	{
		if( g_lngOrderIdxRecordPosition < 0 )
		{
			g_lngOrderIdxRecordPosition = SearchRecord( hFile, ORDERSINDEX_LENGTH_RECORD, 
											g_OrderInfo.szOrderNumber, 0, ORDERS_LENGTH_CUSTOMERNUMBER, 0L, 
											g_lngOrderIdxRecordCount );
		}
		Error = DeleteRecord( hFile, ORDERSINDEX_LENGTH_RECORD, g_lngOrderIdxRecordPosition );
		if( Error == SUCCESS )
		{
			g_lngOrderIdxRecordCount--;
		}
	}
	FileClose( hFile );
	return Error;
}

//###########################################################################################		
//GET RECORDS FROM FILES item.txt order.idx and order files
//###########################################################################################		

long GetDBItemRecord( char * p_szRecord )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	long lngRecordNumber = ERROR_FAILIURE;
	
	p_szRecord[0] = 0;
	hFile = FileOpen( 0, (char*)szFILE_ITEMS_TXT, 0, 0, fileModeReadOnly, &Error );
	if( Error == SUCCESS )
	{
		// binary search for data
		lngRecordNumber = BinarySearchRecordOffset( hFile, ITEMRECORDLENGTH, ITEMS_START_MFGCODE,
											ITEMS_LENGTH_ITEMNUMBER, g_Item.szItemNumber );

			if( lngRecordNumber >= 0 )
			{
				if( ReadRecord( hFile, ITEMRECORDLENGTH, p_szRecord, lngRecordNumber ) != SUCCESS )
				{
					MsgBoxError( g_szUnableToReadItemFromDatabase );
					ClearFieldText( UI_FIELD_ITEMNUMBER );
					lngRecordNumber = ERROR_FAILIURE;
				}
			}
			else
			{
				lngRecordNumber = ERROR_NOTFOUND;
			}
	}
	FileClose( hFile );
	return lngRecordNumber;
}

long FindOrderIdxRecord( char * p_szRecord )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	long lngRecordNumber = ERROR_NOTFOUND;

	p_szRecord[0] = 0;
	
	hFile = FileOpen( 0, (char*)szFILE_ORDERS_IDX, 0, 0, fileModeReadOnly, &Error );
	if( Error == SUCCESS )
	{
		g_lngOrderIdxRecordCount = GetRecordCount( hFile, ORDERSINDEX_LENGTH_RECORD );
		if( g_lngOrderIdxRecordCount > 0 )
		{
			lngRecordNumber = SearchRecord( hFile, ORDERSINDEX_LENGTH_RECORD, 
											g_OrderInfo.szOrderNumber, 0, ORDERS_LENGTH_CUSTOMERNUMBER, 0L, 
											g_lngOrderIdxRecordCount );
		
			if( lngRecordNumber >= 0 )
			{
				if( ReadRecord( hFile, ORDERSINDEX_LENGTH_RECORD, p_szRecord, lngRecordNumber ) != SUCCESS )
				{
					lngRecordNumber = ERROR_NOTFOUND;
				}
			}
			else
			{
				lngRecordNumber = ERROR_NOTFOUND;
			}
		}
	}
	FileClose( hFile );
	return lngRecordNumber;
}

int GetOrderIdxRecord( char * p_szRecord )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;

	p_szRecord[0] = 0;
	
	hFile = FileOpen( 0, (char*)szFILE_ORDERS_IDX, 0, 0, fileModeReadOnly, &Error );
	if( Error == SUCCESS )
	{
		Error = ReadRecord( hFile, ORDERSINDEX_LENGTH_RECORD, p_szRecord, g_lngOrderIdxRecordPosition );
	}
	FileClose( hFile );
	return Error;
}

long FindOrderItemRecord( char * p_szRecord )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	long lngRecordNumber = ERROR_NOTFOUND;

	p_szRecord[0] = 0;

	hFile = FileOpen( 0, g_OrderInfo.szOrderNumber, 0, 0, fileModeReadOnly, &Error );

	if( Error == SUCCESS )
	{
		g_OrderInfo.lngOrderItemsCount = GetRecordCount( hFile, ORDERS_LENGTH_RECORD );
		if( g_OrderInfo.lngOrderItemsCount > 0 )
		{
			lngRecordNumber = SearchRecord( hFile, ORDERS_LENGTH_RECORD, g_Item.szItemDescription, 
									ORDERS_START_ITEMNUMBER, ORDERS_LENGTH_ITEMNUMBER, 0L, 
									g_OrderInfo.lngOrderItemsCount );
//{char c[200];StrPrintF(c,"%li &i %s",lngRecordNumber,(int)StrLen(g_Item.szItemDescription),g_Item.szItemDescription);MsgBoxInfo(c)}	
			if( lngRecordNumber >= 0 )
			{
				Error = ReadRecord( hFile, ORDERS_LENGTH_RECORD, p_szRecord, lngRecordNumber );
				if( Error != SUCCESS )
				{
					lngRecordNumber = ERROR_NOTFOUND;
					MsgBoxError( g_szUnableToReadItemFromFile );
				}
			}
			else
			{
				lngRecordNumber = ERROR_NOTFOUND;
			}
		}
	}
	FileClose( hFile );
	return lngRecordNumber;
}

//###########################################################################################		
//COMPOSE RECORDS TO BE SAVED TO FILES order.idx and order item file
//###########################################################################################		

void ComposeOrderIdxRecord( char * p_szData )
{
	PadRight( g_OrderInfo.szDiscountNoDec, '0', ORDERSINDEX_LENGTH_DISCOUNT );
	StrPrintF( p_szData, "%s%s%c\r\n", g_OrderInfo.szOrderNumber, g_OrderInfo.szDiscountNoDec, g_OrderInfo.iShipDate+ASCII_ZERO );
}

void ComposeOrderItemRecord( char * p_szData )
{
	char szTemp[ 40 ];

	StrCopy( p_szData, g_OrderInfo.szOrderNumber );

	StrCopy( szTemp, g_Item.szItemDescription );
	PadRight( szTemp, ASCII_SPACE, ITEMS_LENGTH_DESCRIPTION );
	StrCat( p_szData, szTemp );

	StrCopy(szTemp, g_Item.szQuantity);
	PadLeft(szTemp, ASCII_ZERO, ORDERS_LENGTH_ITEMQUANTITY );
	StrCat( p_szData, szTemp );

	StrCopy( szTemp, g_Item.szNetPriceNoDec );
	PadLeft( szTemp, ASCII_ZERO, ORDERS_LENGTH_ITEMNETPRICE );
	StrCat( p_szData, szTemp );

	StrCat( p_szData, g_szCRLF );
}

//###########################################################################################		
//FILL STRUCTURES WITH RECORD DATA FROM item.txt order.idx and order item files
//###########################################################################################		

//fills item structure only with data from the item.txt file only
void PopulateItemStruct( const char * p_szRecord )
{
	FloatType			ftHundred;
	FloatType			ftDiscount;

	int					iLen = 0;
	int					iPos = 0;

	ftHundred 	= FplLongToFloat( 0L );
	ftDiscount 	= FplLongToFloat( 0L );
	
	//item number
	StrNCopy( g_Item.szItemDescription, p_szRecord + ITEMS_START_DESCRIPTION, ITEMS_LENGTH_DESCRIPTION );
	AllTrim( g_Item.szItemDescription, ASCII_SPACE );
	
	//unit of measure
	StrNCopy( g_Item.szUnitOfMeasure, p_szRecord + ITEMS_START_UNITOFMEASURE, ITEMS_LENGTH_UNITOFMEASURE );

	//list price
	StrNCopy( g_Item.szListPriceNoDec, p_szRecord + ITEMS_START_LISTPRICE, ITEMS_LENGTH_LISTPRICE );
	AllTrim( g_Item.szListPriceNoDec, ASCII_SPACE );
	TrimLeft( g_Item.szListPriceNoDec, ASCII_ZERO );

	//list price with decimal point
	FormatDecimal( g_Item.szListPrice, g_Item.szListPriceNoDec, APP_DECIMAL_PLACES );

	//list price as float
	g_Item.flListPrice = FplAToF( g_Item.szListPrice );

	ftDiscount = FplMul( g_Item.flListPrice, g_OrderInfo.flDiscount );
	ftHundred = FplLongToFloat( 100L );
	ftDiscount = FplDiv( ftDiscount, ftHundred );
	g_Item.flNetPrice = FplSub( g_Item.flListPrice, ftDiscount );
	
	FloatToString( g_Item.szNetPrice, g_Item.flNetPrice, APP_DECIMAL_PLACES );

	StrCopy( g_Item.szNetPriceNoDec, g_Item.szNetPrice );
	RemoveCharacter( g_Item.szNetPriceNoDec, ASCII_PERIOD );

	// minimum order quantity
	StrNCopy( g_Item.szMinQty, p_szRecord + ITEMS_START_MINIMALQUANTITY, ITEMS_LENGTH_MINIMALQUANTITY );
	AllTrim( g_Item.szMinQty, ASCII_SPACE );
	TrimLeft( g_Item.szMinQty, ASCII_ZERO );
	g_Item.lngMinQty = StrAToI( g_Item.szMinQty );
	
	// case quantity
	StrNCopy( g_Item.szCaseQty, p_szRecord + ITEMS_START_CASEQUANTITY, ITEMS_LENGTH_CASEQUANTITY );
	AllTrim( g_Item.szCaseQty, ASCII_SPACE );
	TrimLeft( g_Item.szCaseQty, ASCII_ZERO );
	g_Item.lngCaseQty = StrAToI( g_Item.szCaseQty );
	
	// available quantity
	StrNCopy( g_Item.szAvailableQty, p_szRecord + ITEMS_START_AVAILABLEQUANTITY, ITEMS_LENGTH_AVAILABLEQUANTITY );
	AllTrim( g_Item.szAvailableQty, ASCII_SPACE );
	TrimLeft( g_Item.szAvailableQty, ASCII_ZERO );
	g_Item.lngAvailableQty = StrAToI( g_Item.szAvailableQty );

}

//fills item structure only with data from the order items file
void PopulateItemStructQuantityAndNetPrice( const char * p_szRecord )
{
	StrNCopy( g_Item.szQuantity, p_szRecord + ORDERS_START_ITEMQUANTITY, ORDERS_LENGTH_ITEMQUANTITY );
	AllTrim( g_Item.szQuantity, ASCII_SPACE );
	TrimLeft( g_Item.szQuantity, ASCII_ZERO );
	g_Item.flQuantity = FplAToF( g_Item.szQuantity );

	StrNCopy( g_Item.szNetPriceNoDec, p_szRecord + ORDERS_START_ITEMNETPRICE, ORDERS_LENGTH_ITEMNETPRICE );
	AllTrim( g_Item.szNetPriceNoDec, ASCII_SPACE );
	TrimLeft( g_Item.szNetPriceNoDec, ASCII_ZERO );

	FormatDecimal( g_Item.szNetPrice, g_Item.szNetPriceNoDec, APP_DECIMAL_PLACES );
	g_Item.flNetPrice = FplAToF( g_Item.szNetPrice );
}

//fills OrderInfo structure only with data from order.idx
void PopulateOrderInfoStruct( char * p_szRecord )
{
	StrNCopy( g_OrderInfo.szOrderNumber, p_szRecord, ORDERSINDEX_LENGTH_CUSTOMERNUMBER );
	StrNCopy( g_OrderInfo.szDiscountNoDec, p_szRecord + ORDERSINDEX_START_DISCOUNT, ORDERSINDEX_LENGTH_DISCOUNT );
	
	g_OrderInfo.iShipDate = (int)( p_szRecord[ ORDERSINDEX_START_SHIPDATE ] - ASCII_ZERO );
	FormatDecimal( g_OrderInfo.szDiscount, g_OrderInfo.szDiscountNoDec, APP_DECIMAL_PLACES );
	g_OrderInfo.flDiscount = FplAToF( g_OrderInfo.szDiscount );
}

void PopulateOrderInfoStructDiscount( char * p_szData )
{
	if( p_szData[ 2 ] == ASCII_PERIOD )
	{
		g_OrderInfo.flDiscount = FplAToF( (char*)p_szData );
		PadRight( g_OrderInfo.szDiscountNoDec, '0', ORDERSINDEX_LENGTH_DISCOUNT + 1 );
		StrCopy( g_OrderInfo.szDiscount, p_szData );
		StrCopy( g_OrderInfo.szDiscountNoDec, p_szData );
		RemoveCharacter( g_OrderInfo.szDiscountNoDec, ASCII_PERIOD );
	}
	else
	{
		StrCopy( g_OrderInfo.szDiscountNoDec, p_szData );
		RemoveCharacter( g_OrderInfo.szDiscountNoDec, ASCII_PERIOD );
		PadRight( g_OrderInfo.szDiscountNoDec, '0', ORDERSINDEX_LENGTH_DISCOUNT );
		FormatDecimal( g_OrderInfo.szDiscount, g_OrderInfo.szDiscountNoDec, APP_DECIMAL_PLACES );
		g_OrderInfo.flDiscount = FplAToF( g_OrderInfo.szDiscount );
	}
}

void PopulateOrderInfoStructTotals( void )
{
	StrPrintF( g_OrderInfo.szTotalItems, "%li", g_OrderInfo.lngOrderItemsCount );
	FloatToString( g_OrderInfo.szTotalAmount, g_OrderInfo.flTotalAmount, APP_DECIMAL_PLACES );
}

//###########################################################################################		
//
//###########################################################################################		

// read all items for this order and get total amount, etc. returns total amount read
void CalculateOrderTotalAmount( void )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	char 		szRecord				[60];
	char			szTemp				[20];
	char 		szNetPrice			[12];
	char			szQuantity			[10];

	int 			i					= 0;

	FloatType		flNetPrice;
	
	g_OrderInfo.lngOrderItemsCount = 0;
	g_OrderInfo.flTotalAmount = FplLongToFloat( 0L );
	g_OrderInfo.szTotalItems[0] = '0';
	g_OrderInfo.szTotalAmount[0] = '0';
	g_OrderInfo.szTotalItems[1] = 0;
	g_OrderInfo.szTotalAmount[1] = 0;

	if( FindOrderIdxRecord( szRecord ) < 0 )
	{
		//the file ain't supposed to be... hasta la bye bye
		FileDelete( 0, g_OrderInfo.szOrderNumber );
		return;
	}
	
	hFile = FileOpen( 0, g_OrderInfo.szOrderNumber, 0, 0, fileModeReadOnly, &Error );
	if( Error == SUCCESS )
	{
		g_OrderInfo.lngOrderItemsCount = GetRecordCount( hFile, ORDERS_LENGTH_RECORD );
		if( g_OrderInfo.lngOrderItemsCount > 0 )
		{
			for( i=0 ; i < g_OrderInfo.lngOrderItemsCount ; i++ )
			{
				ZEROOUT( szRecord );
				ZEROOUT( szQuantity );
				ZEROOUT( szNetPrice );
				ZEROOUT( szTemp );
				Error = ReadRecord( hFile, ORDERS_LENGTH_RECORD, szRecord, i );
				if( Error == SUCCESS )
				{
					StrNCopy( szQuantity, szRecord + ORDERS_START_ITEMQUANTITY, ORDERS_LENGTH_ITEMQUANTITY );
					AllTrim( szQuantity, ASCII_SPACE );
					TrimLeft( szQuantity, ASCII_ZERO );
					StrNCopy( szTemp, szRecord + ORDERS_START_ITEMNETPRICE, ORDERS_LENGTH_ITEMNETPRICE );
					AllTrim( szTemp, ASCII_SPACE );
					TrimLeft( szTemp, ASCII_ZERO );
					FormatDecimal( szNetPrice, szTemp, APP_DECIMAL_PLACES );
					flNetPrice = FplMul( FplAToF(szQuantity), FplAToF(szNetPrice) );
					g_OrderInfo.flTotalAmount = FplAdd( g_OrderInfo.flTotalAmount, flNetPrice );
				}
			}
			PopulateOrderInfoStructTotals();
		}
	}
	FileClose( hFile );
}

// builds a pointer list (ChoicesPtrsHandle) of all available order numbers
void RefreshOrderList( int p_iOrderNumberList, int  p_iOrderNumberPopTrigger, Boolean p_bShowDates )
{
	Err Error = ERROR_FAILIURE;
	FileHand * hFile = NULL;
	
	Boolean					bFits				= false;

	Int						i					= 0;
	Int						iListItemIndex			= -1;
	Int						iListWidth			= 0;
	Int						iChoicesOffset 		= 0;
	Int						iRecordCount 			= 0;
	int						iTextLength			= 0;
	SWord					swTextLength			= 0;
	VoidHand 				hChoices				= NULL;
	VoidHand					hChoicesPtrs 			= NULL;

	CharPtr					pChoices 			= NULL;
	Char						szRecord				[30];

	FormPtr					pForm 				= FrmGetActiveForm();
	RectangleType			lstRect;


	ZEROOUT( szRecord );			// initialize string

	hFile = FileOpen( 0, (char*)szFILE_ORDERS_IDX, 0, 0, fileModeReadOnly, &Error );
	if( Error == SUCCESS )
	{
		// refresh order index count
		g_lngOrderIdxRecordCount = GetRecordCount( hFile, ORDERSINDEX_LENGTH_RECORD );
	
		// get a pointer to the list object
		iListItemIndex = (int)FrmGetObjectIndex( pForm, (Word)p_iOrderNumberList );
	
		// build list of all order numbers (if any exist)
		if( g_lngOrderIdxRecordCount > 0 )
		{
			for( iListItemIndex=0; iListItemIndex < g_lngOrderIdxRecordCount; iListItemIndex++ )
			{
				ZEROOUT( szRecord );
	
				ReadRecord( hFile, ORDERSINDEX_LENGTH_RECORD, szRecord, iListItemIndex );
	
				if( !szRecord[0] )			// ignore empty record (should NOT happen!)
					continue;
	
				// first time through loop, initialize handle to choices
				if( iRecordCount == 0 )
				{
					if( hChoicesPtrs )
					{
						MemHandleUnlock( hChoicesPtrs );
						hChoicesPtrs = NULL;
					}
	
					// get the usable width of the list rectangle
					FrmGetObjectBounds(pForm, (Word)iListItemIndex, &lstRect );
					iListWidth = lstRect.extent.x - 2;
	
					// allocate initial block for the list choices
					hChoices = MemHandleNew( sizeof(char) );
	
					// lock down the block and set it's initial value to an empty string
					pChoices = MemHandleLock( hChoices );
					*pChoices = 0;
				}
	
				iRecordCount++;
	
				szRecord[6] = '\0';					// only need the order #
	
				// if show dates, append ship date to list
				if( p_bShowDates )
				{
					// append ship date
					StrNCat( szRecord, " (", 9 );
					StrNCat( szRecord, g_aShipDates[ (int)szRecord[ 10 ] - ASCII_ZERO ], 18 );
					StrNCat( szRecord, ")", 19 );
				}
	
				// determine the length of text that will fit within the list bounds
				iTextLength = (int)StrLen( szRecord );
	
				// need to copy it to a Short Word before passing in address otherwise
				// it might tweak the value inside the function.
				swTextLength = iTextLength;
				FntCharsInWidth( szRecord, &iListWidth, &swTextLength, &bFits );
	
				// grow the choices buffer to hold the new string-- must unlock the chunk
				// so the memory manager can move the chunk if neccessary to grow it
				if( hChoices )
					MemHandleUnlock( hChoices );
					
				Error = MemHandleResize( hChoices, iTextLength + iChoicesOffset + sizeof('\0') );
				pChoices = MemHandleLock(hChoices );
	
				// check for fatal Error.
				ErrFatalDisplayIf( Error, g_szUnableToExpandList );
	
				// copy the text from the record to the choices buffer
				for( i = 0; i < iTextLength; i++ )
				{
					pChoices[ iChoicesOffset + i ] = szRecord[ i ];
				}
	
				// update the end of choices offset and terminate with null
				iChoicesOffset += iTextLength;
				pChoices[ iChoicesOffset++ ] = '\0';
			}
		}
	
		// only fill the popup list if there are valid entries
		if( iRecordCount > 0 )
		{
			// create an array of pointers from the choices string
			hChoicesPtrs = SysFormPointerArrayToStrings( pChoices, iRecordCount );
	
	// sorting this list might be a good idea.. although we could call p_fixlen_sort(..)
	// whenever a new order is added and it'd probably be easier =)
	//		SysQSort(pChoices, actualRecords, sizeof(textLen), (CmpFuncPtr)StrNCompareFunctionCall, 0);
	
			// set the list choices from the array of pointers
			LstSetListChoices( GetObjectPtr( (unsigned short)p_iOrderNumberList ), 
							MemHandleLock( hChoicesPtrs ), (unsigned short)iRecordCount );
	
			CtlSetEnabled( GetObjectPtr( (unsigned short)p_iOrderNumberPopTrigger ), true );
		}
		else
		{
			CtlSetEnabled( GetObjectPtr( (unsigned short)p_iOrderNumberPopTrigger ), false );
		}
	}
	else
	{
		CtlSetEnabled( GetObjectPtr( (unsigned short)p_iOrderNumberPopTrigger ), false );
	}
	FileClose( hFile );
}

