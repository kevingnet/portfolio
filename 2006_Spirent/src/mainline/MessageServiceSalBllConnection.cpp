// TODO: Remove this later after Andy's refactor.
// This is used to prevent Windows from including Winsock
#ifdef WIN32
#define _WINSOCKAPI_
#endif


#include "MessageServiceSalBllConnection.h"
#include "SalLogger.h"
#include "HandleReserver.h"
#include "SalMessage.h"
#include "frameworkConst.h"
#include "SalBllConnection.h"

const char * sz_BLLConnLoggerName = "fmwk.bll.sal.bllconnection";
DEFINE_STATIC_LOGGER(sz_BLLConnLoggerName, scriptLogger);

using namespace stc::sal;

void MessageServiceSalBllConnection::CreateRequestIfNewCommand()
{
    if( m_pRequestMessage == 0 ) 
    {
        m_pRequestMessage = new SalRequestMessage();
        m_Result.clear();
        m_countCommands = 0;
        m_countAttributes = 0;
        m_requireResult = false;
        m_cumulativeTimeoutSeconds = 0;
    }
}

MessageServiceSalBllConnection::MessageServiceSalBllConnection() {
    m_pRequestMessage = 0;
    m_cumulativeTimeoutSeconds = 0;
}

MessageServiceSalBllConnection::~MessageServiceSalBllConnection()
{
    if( m_pRequestMessage != NULL ) 
    {
        delete m_pRequestMessage;
    }
    m_Result.clear();
}

void MessageServiceSalBllConnection::Connect() {
    // TODO: Call to BLL
    LOGINFO("MessageServiceSalBllConnection::Connect");
}

void MessageServiceSalBllConnection::Disconnect() {
    // TODO: Call to BLL
    LOGINFO("MessageServiceSalBllConnection::Disconnect");
}

void MessageServiceSalBllConnection::Create(SalHandle handleNewObject,
                                            const string& className, 
                                            SalHandle parentHandle) {

    CreateRequestIfNewCommand();
	CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
	pMsgRequest->GetWriter() << className;
	pMsgRequest->GetWriter() << parentHandle;
	//pMsgRequest->WriteCommand(CMD_CREATE, -1, handleNewObject);
	pMsgRequest->WriteCommand(CMD_CREATE, CMD_STRING, handleNewObject);
    ++m_countCommands;
    m_cumulativeTimeoutSeconds += 60;
}

PropertyVector MessageServiceSalBllConnection::Get(SalHandle handle, 
                                                   const vector<string>& names) {
    CreateRequestIfNewCommand();
	CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
    m_countAttributes += names.size();
	//pMsgRequest->GetWriter() << names; ?? don't know if it does the *right* thing
    for( intcounter i=0; i<names.size(); ++i )
    {  
        pMsgRequest->GetWriter() << names[i];
    }
	pMsgRequest->SetAttributeCount(static_cast<uint16_t>(m_countAttributes));
    //TODO: what about DAN in subcommand type?
	//pMsgRequest->WriteCommand(CMD_GET, -1, handle);
	pMsgRequest->WriteCommand(CMD_GET, CMD_STRING, handle);
    ++m_countCommands; 
    m_cumulativeTimeoutSeconds += 20 + ( static_cast<int>(names.size()) * 5 );
    m_requireResult = true;
    m_Result.clear();
    Commit();
    PropertyVector vec;
    for( intcounter i=0; i<m_Result.size(); ++i )
    { 
        string& s1 = m_Result[i];
        string& s2 = m_Result[++i];
        vec.push_back( make_pair( s1, s2 ) );
    }
    return vec;
}

void MessageServiceSalBllConnection::Set(SalHandle handle, 
                                         const PropertyVector& properties, 
                                         bool bAllowDescendantCreation) {
    CreateRequestIfNewCommand();
	CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
    m_countAttributes += properties.size();
    for( intcounter i=0; i<properties.size(); ++i )
    {  
        const pair<string, string> & p = properties[i];
        pMsgRequest->GetWriter() << p.first;
        pMsgRequest->GetWriter() << p.second;
    }
	pMsgRequest->SetAttributeCount(static_cast<uint16_t>(m_countAttributes));
	pMsgRequest->WriteCommand(
        CMD_SET, 
        ( bAllowDescendantCreation ? CMD_STRING_CREATE : CMD_STRING ), 
        handle);
    ++m_countCommands;
    m_cumulativeTimeoutSeconds += 10 + ( static_cast<int>(properties.size()) * 3 );
}

void MessageServiceSalBllConnection::Perform(SalHandle handleCommand) {
    CreateRequestIfNewCommand();
	CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
    //TODO: subtype
    //0x00007002
	pMsgRequest->WriteCommand(CMD_ACTION, 
        FRAMEWORK_Command_Operation_Execute, handleCommand);
    ++m_countCommands;
    m_cumulativeTimeoutSeconds += 560;
    CommitPerform();

    bool result = false;
    int counter = 10;
    while( result != true && counter > 0 )
    {
        result = GetPerformCommandState( handleCommand );
        switch( result )
        {
        case true:
            return;
            break;
        case false:
            --counter;
            SleepMS( 1000 );
            break;
        }
    }
}

bool MessageServiceSalBllConnection::GetPerformCommandState( SalHandle handleCommand )
{
    CreateRequestIfNewCommand();
	CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
    m_countAttributes += 1;
    pMsgRequest->GetWriter() << "State";
 	pMsgRequest->SetAttributeCount(static_cast<uint16_t>(m_countAttributes));
	pMsgRequest->WriteCommand( CMD_GET, CMD_STRING, handleCommand );
    ++m_countCommands; 
    m_cumulativeTimeoutSeconds += 50;
    m_requireResult = true;
    m_Result.clear();
    return CommitGetPerformState();
}

void MessageServiceSalBllConnection::Delete(SalHandle handle) {

    CreateRequestIfNewCommand();
    CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
    pMsgRequest->WriteCommand(CMD_DELETE, 0, handle);
    ++m_countCommands;
    m_cumulativeTimeoutSeconds += 60;
}

void MessageServiceSalBllConnection::ReserveHandles(int handleCount, vector<SalHandle>& reservedHandles) {
    HandleReserver reserver;
    reserver.reserveHandles( reservedHandles, handleCount );
}

void MessageServiceSalBllConnection::GetPropertyTypeInfo(string className,
                                                         PropertyNameVector& propertyNames,
                                                         PropertyTypeVector& propertyTypes,
                                                         HandleMap* pHandleMap ) {
    //TODO: get command subtype
    //HACK: another nasty hack! you see, the command object wasn't being created properly
    
    CreateRequestIfNewCommand();

    string strType = "getpropertytypemetainfocommand";
    
    // Get handle from map -- a two-step process
    string strHandle = pHandleMap->assignHandle(strType);
    SalHandle handle = pHandleMap->getSalHandle(strHandle);

    Create( handle, strType, STC_SYSTEM_HANDLE );

    PropertyVector props;
    props.push_back(make_pair("ExecuteSynchronous", "true"));
    props.push_back(make_pair("ClassName", className));
    Set(handle, props, false);

    Commit();
    Perform( handle );

    vector<string> names;
    PropertyVector pvResult;
    pvResult = Get( handle, names );

    //NOTE: we are assuming that the names and types do not contain spaces
    //so we need to tokenize them, before adding to the arrays
    propertyNames.clear();
    propertyTypes.clear();
    for( intcounter i=0; i<pvResult.size(); ++i )
    {  
        pair<string, string> & p = pvResult[i];
        if( p.first == "-PropertyNames" )
        {
            StringToVector( p.second, propertyNames );
        } else if( p.first == "-PropertyTypes" )
        {
            PropertyNameVector vTypes;
            StringToVector( p.second, vTypes );
            for( intcounter j=0; j<vTypes.size(); ++j )
            {  
                propertyTypes.push_back( GetPropertyType( vTypes[j] ) ); 
            }
        }
    }

    //make sure names and types arrays have the same number of items

    Delete(handle);
    // Send queued-up Delete
    Commit();
}

void MessageServiceSalBllConnection::Commit() {

    if( m_pRequestMessage == 0 )
    { 
        // throw no command queued
        return;
    }

    CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
	pMsgRequest->WriteMessageHeader();
    m_pRequestMessage->SendRequest( m_cumulativeTimeoutSeconds );

    // Check responses for errors
    m_pRequestMessage->ProcessResponse( &m_Result, m_requireResult, 
        static_cast<int>(m_countCommands), static_cast<int>(m_countAttributes) );
 
    delete m_pRequestMessage;
    m_pRequestMessage = 0;
}

void MessageServiceSalBllConnection::CommitPerform() {

    if( m_pRequestMessage == 0 )
    { 
        // throw no command queued
        return;
    }

    CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
	pMsgRequest->WriteMessageHeader();
    m_pRequestMessage->SendRequest( m_cumulativeTimeoutSeconds );

    // Check responses for errors
    m_pRequestMessage->ProcessResponsePerform( &m_Result );

    delete m_pRequestMessage;
    m_pRequestMessage = 0;
}

bool MessageServiceSalBllConnection::CommitGetPerformState()
{
    if( m_pRequestMessage == 0 )
    { 
        // throw no command queued
        return false;
    }

    CMessage* pMsgRequest  = m_pRequestMessage->GetRequest();
	pMsgRequest->WriteMessageHeader();

    // Check responses for errors
    int state = FRAMEWORK_Command_EnumState_FAILED;
    m_pRequestMessage->SendRequest( m_cumulativeTimeoutSeconds );
    state = m_pRequestMessage->ProcessResponseGetPerformState();
    bool result = false;
    switch( state )
    {
    case FRAMEWORK_Command_EnumState_INIT:
    case FRAMEWORK_Command_EnumState_START:
    case FRAMEWORK_Command_EnumState_FAILED:
    case FRAMEWORK_Command_EnumState_RUNNING:
        result = false;
        break;
    case FRAMEWORK_Command_EnumState_COMPLETED:
        result = true;
        break;
    }

    delete m_pRequestMessage;
    m_pRequestMessage = 0;

    return result;
}

#ifdef WIN32
    #include <windows.h>
#else
    #include <time.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif

void MessageServiceSalBllConnection::SleepMS(int ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	struct timespec t;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (ms % 1000) * 1000 * 1000; 
	nanosleep(&t, NULL);
#endif
}

