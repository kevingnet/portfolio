// This is used to prevent Windows from including Winsock
#ifdef WIN32
#define _WINSOCKAPI_
#endif


#include "SalLogger.h"
#include "SalInterface.h"
#include "SalInterfaceHelpers.h"
#include "SalException.h"
#include "StcException.h"
#include "SalMessage.h"
#include "SalStringVectorFacade.h"
#include "MessageServiceSalStringBllConnection.h"
#include "BllStringSalFacade.h"
#include "DdnToDanFacade.h"
#include "MessageServer.h"

/*
SalInterface
1) logs function calls and params upon entering and calls upon leaving
2) strips hypens and reports error if asymetrical (incomplete) vectors are sent
3) handles all types of exceptions and reports only "standard" exceptions to the callers
*/

namespace stc {
using namespace stc::sal;

const char * szErr_UnknownException = "Unknown Exception!!!";

const char * sz_LoggerName = "fmwk.bll.sal.INTERFACE";

#define COMMAND_NAME( name ) const char * sz_##name = #name;
COMMAND_NAME( salInit )
COMMAND_NAME( salShutdown )
COMMAND_NAME( salConnect )
COMMAND_NAME( salDisconnect )
COMMAND_NAME( salCreate )
COMMAND_NAME( salDelete )
COMMAND_NAME( salSet )
COMMAND_NAME( salGet )
COMMAND_NAME( salPerform )
COMMAND_NAME( salReserve )
COMMAND_NAME( salRelease )
COMMAND_NAME( salSubscribe )
COMMAND_NAME( salUnsubscribe )

//TODO: log may log parameter on function start
//TODO: display command help on failure, may be on some...

DEFINE_STATIC_LOGGER(sz_LoggerName, scriptLogger);
 
#define HANDLE_EXCEPTIONS \
    catch( stc::sal::SalException& e ) \
    { \
        sResult += ": "; \
        sResult += e.what(); \
    } \
    catch( stc::framework::CStcException& e ) \
    { \
        sResult += ": "; \
        sResult += e.what(); \
    } \
    catch( exception& e ) \
    { \
        sResult += ": "; \
        sResult += e.what(); \
    } \
    catch(...) \
    { \
        sResult += ": "; \
        sResult += szErr_UnknownException; \
    } \
    LOGERROR( sResult ); \
    throw runtime_error(sResult);

string strtolower(string& strToLower)
{
    string str = strToLower;
    transform(str.begin(), str.end(), str.begin(), ptr_fun(::tolower));
    return str;
}

//CASE: swig and other scripting languages some times concatenate the params
//into a string!!! what da... this is a problem! so what we have to do is parse the
//individual strings in the vector and make them be the right thing.... ugh!

//to complicate matters and make things more interesting, the attr:value can contain spaces
//however, the attr:name is assumed to not contain spaces, it's almost guaranteed, from the xml
// oh! joy!! this is going to be painful
void NormalizeStringVectorParameters( StringVector& sv )
{
}

void StripHyphensAndCheckSymmetry( StringVector& sv )
{
    size_t count = sv.size();
    if( count < 2 ) {
        //TODO: 
        //throw 
    }
    if( !(count % 2) ) { //is it even or odd? we need even numbers, pairs!
        //odd
        //TODO: 
        //throw 
    }
    for ( size_t i=0; i<count; ++i )
    {
        string & s = sv[i];
        if( s[0] == '-' ) {
            s.erase( 0, 1 );
        }
        ++i; //skip value string
    }
}

void StripHyphensFromNonpairs( StringVector& sv )
{
    size_t count = sv.size();
    for ( size_t i=0; i<count; ++i )
    { 
        string & s = sv[i];
        if( s[0] == '-' ) {
            s.erase( 0, 1 );
        }
    }
}

void VerifyParentHandle( string& parentHandle, StringVector& sv )
{
    if (sv.size() >= 2 && strtolower(sv[0]) == "-under")
    {
        parentHandle = sv[1];
        sv.erase(sv.begin(), sv.begin() + 2);
    }
    else
    {
        throw std::runtime_error("no parent found in create");
    }
}

void VerifyParentHandleForCreateProjectObject( string& type,  string& parentHandle, StringVector& sv )
{
    if( strtolower(type) != "project" )
    {
        VerifyParentHandle( parentHandle, sv );
        return;
    }
    if (sv.size() >= 2 && strtolower(sv[0]) == "-under")
    {
        parentHandle = sv[1];
        sv.erase(sv.begin(), sv.begin() + 2);
    }
    else 
    {
        parentHandle = "system1";
        //TODO: should we make sure this has to be "system1" ???
        //TODO: should we make sure this has to be "system1" ??? I guess! (read! did not work)
        parentHandle = "system1"; //now, provide the default
    }
}

SAL_EXPORT void salLog( int logLevel, string msg )
{
    switch( logLevel )
    {
    case SAL_LOG_LEVEL_DEBUG:
	    LOGDEBUG( msg );
        break;
    case SAL_LOG_LEVEL_INFO:
	    LOGINFO( msg );
        break;
    case SAL_LOG_LEVEL_WARN:
	    LOGWARN( msg );
        break;
    case SAL_LOG_LEVEL_ERROR:
	    LOGERROR( msg );
        break;
    }
}

//GLOBAL OBJECTS :-(
//unfortunately necessary because this is an API interface and access must be
//granted to all functions here

MessageServiceSalStringBllConnection * g_pMessageServiceSalStringBllConnection = 0;
BllStringSalFacade * g_pBllStringSalFacade = 0;
DdnToDanFacade * g_pDdnToDanFacade = 0;
SalStringVectorFacade * g_pSalStringVectorFacade = 0;

static int g_ReferenceCounter = 0;

#define DELETE_POINTER( p ) if( p ) { delete p; p=0; }

extern "C" int STC_PUBLIC StcInit() ;

SAL_EXPORT void salInit(void)
{
    string sResult(sz_salInit);
    try
    {
        LOGINFO(sResult);
        if( g_ReferenceCounter == 0 )
        {
            LOGDEBUG("Initializing library...");
	        StcInit() ;
            ScriptingServer::Instance().ConnectToServer( "local" );
            g_pMessageServiceSalStringBllConnection = new MessageServiceSalStringBllConnection();
            g_pBllStringSalFacade = new BllStringSalFacade( g_pMessageServiceSalStringBllConnection );
            g_pDdnToDanFacade = new DdnToDanFacade( g_pBllStringSalFacade );
            g_pSalStringVectorFacade = new SalStringVectorFacade( g_pDdnToDanFacade );
            g_pSalStringVectorFacade->init();

            static bool atexit_set = false;
            if (!atexit_set)
            {
                atexit(salShutdown);
                atexit_set = true;
            }
        }
        else
        {
            LOGWARN("Library already initialized! Increasing reference counter");
        }
        g_ReferenceCounter++;
        return;
    }
    HANDLE_EXCEPTIONS;
}

extern "C" int StcShutdown();
SAL_EXPORT void salShutdown(void)
{
    string sResult(sz_salShutdown);
    try
    {
        LOGINFO(sResult);
        g_ReferenceCounter--;
        if( g_ReferenceCounter <= 0 )
        {
            LOGDEBUG("Shutting down library...");
            ScriptingServer::Instance().DisconnectFromServer() ;
            DELETE_POINTER( g_pMessageServiceSalStringBllConnection );
            DELETE_POINTER( g_pBllStringSalFacade );
            DELETE_POINTER( g_pDdnToDanFacade );
            DELETE_POINTER( g_pSalStringVectorFacade );
            //g_pSalStringVectorFacade->shutdown?
            g_ReferenceCounter = 0;
            StcShutdown();
        }
        else
        {
            LOGWARN("Library NOT shutdown! Decreasing reference counter");
        }
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salConnect( StringVector hostNames )
{
    string sResult(sz_salConnect);
    try
    {
	    LOGINFO(sResult); 
        NormalizeStringVectorParameters( hostNames );
        g_pSalStringVectorFacade->connect( hostNames );
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salDisconnect( StringVector hostNames )
{
    string sResult(sz_salDisconnect);
    try
    {
	    LOGINFO(sResult);
        NormalizeStringVectorParameters( hostNames );
        StripHyphensAndCheckSymmetry(hostNames);
        g_pSalStringVectorFacade->disconnect( hostNames );
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT string salCreate( string type, StringVector propertyPairs )
{
    string sResult(sz_salCreate);
    try 
    {
        throw SalException( 0, E_ARGS );
	    LOGINFO(sResult);
        string parentHandle;
        NormalizeStringVectorParameters( propertyPairs );
        StripHyphensAndCheckSymmetry( propertyPairs );
        VerifyParentHandleForCreateProjectObject( type, parentHandle, propertyPairs );
        return g_pSalStringVectorFacade->create( type, parentHandle, propertyPairs );
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salDelete( string handle )
{
    string sResult(sz_salDelete);
    try
    {
	    LOGINFO(sResult);
        g_pSalStringVectorFacade->deleteObject( handle );
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salSet( string handle, StringVector propertyPairs )
{
    string sResult(sz_salSet);
    try
    {
	    LOGINFO(sResult);
        //if( propertyPairs[0] == "-myhandlearray" )
        //    string s = "yes!";
        NormalizeStringVectorParameters( propertyPairs );
        StripHyphensAndCheckSymmetry( propertyPairs );
        g_pSalStringVectorFacade->set( handle, propertyPairs );
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT StringVector salGet( string handle, StringVector propertyNames )
{
    string sResult(sz_salGet);
    try
    {
	    LOGINFO(sResult);
        NormalizeStringVectorParameters( propertyNames );
        StripHyphensFromNonpairs(propertyNames);
        return g_pSalStringVectorFacade->get( handle, propertyNames );
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT StringVector salPerform( string commandName, StringVector propertyPairs )
{
    string sResult(sz_salPerform);
    try
    {
	    LOGINFO(sResult);
        NormalizeStringVectorParameters( propertyPairs );
        StripHyphensAndCheckSymmetry( propertyPairs );
        return g_pSalStringVectorFacade->perform( commandName, propertyPairs );
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT StringVector salReserve( StringVector CSPs )
{
    string sResult(sz_salReserve);
    try
    {
	    LOGINFO(sResult);
        NormalizeStringVectorParameters( CSPs );
        return g_pSalStringVectorFacade->reserve( CSPs );
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salRelease( StringVector CSPs )
{
    string sResult(sz_salRelease);
    try
    {
	    LOGINFO(sResult);
        NormalizeStringVectorParameters( CSPs );
        g_pSalStringVectorFacade->release( CSPs );
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salSubscribe( string resultType, string style, string fileName )
{
    string sResult(sz_salSubscribe);
    try
    {
	    LOGINFO(sResult);
        g_pSalStringVectorFacade->subscribe( resultType, style, fileName );
        return;
    }
    HANDLE_EXCEPTIONS;
}

SAL_EXPORT void salUnsubscribe( string resultType )
{
    string sResult(sz_salUnsubscribe);
    try
    {
	    LOGINFO(sResult);
        g_pSalStringVectorFacade->unsubscribe( resultType );
        return;
    }
    HANDLE_EXCEPTIONS;
}

}
