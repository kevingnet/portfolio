#include "Config.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>

//---------------------------------------------------------------------

char * ConfigMap::m_config_files[] = { 
	"/etc/"CONFIG, 
	"~/bin/"CONFIG,
	"~/."CONFIG,
	"./"CONFIG,
};

ConfigMap::ConfigMap()
{
	cout << "Info: searching for configuration files" << endl;
	
	char buf[512];
	char fnbuf[512];

	char * home = getenv("HOME");
	if ( home && *home )
		strcpy(fnbuf,home);
	char * pfnbuf = fnbuf + strlen(fnbuf);
	
	char * fn = 0;
	char * file = 0;
	int count = sizeof(m_config_files) / sizeof(char *);
	for (int i=0; i<count; ++i){

		fn = m_config_files[i];
		if( *fn == '~' ){
			strcpy(pfnbuf,fn+1);
			fn = fnbuf;
		}

		file = realpath(fn, buf);

		if( file ){
			cout << "Info: using config file: " << file << endl;
			ReadConfig(file);
		}else{
			cout << "Info: config file " << fn << " not found: " << strerror(errno) << endl;
		}
	}
	cout << endl;
}

void ConfigMap::Dump() 
{
	cout << "---Settings read from configuration files: " << endl;
	for( configmap::iterator iter = m_Settings.begin(); iter != m_Settings.end(); ++iter ) {
		 cout << "'" << (*iter).first << "' = '" << (*iter).second << "'" << endl;
	}
	cout << endl;
}

ConfigMap::~ConfigMap()
{
}

void ConfigMap::ReadConfig(const char * file)
{
	char buf[LINE_LEN];
	ifstream infile;
	infile.open (file, ifstream::in);
	char * key = 0;
	char * val = 0;
	char * tmp = 0;
	int i;
	while (infile.good()){
		buf[LINE_LEN-1] = 0;
		infile.getline(buf,LINE_LEN);

		//discard comments
		tmp = strchr(buf,'#');
		if( tmp ) *tmp = 0;

		//must have an equal sign
		val = strchr(buf,'=');
		if ( !val || !*val ) continue;
		*val++ = 0;
		
		key = buf;

		//skip whitespace
		while(*key && !isalnum(*key)) key++;
		while(*val && !isalnum(*val)) val++;

		//truncate whitespace and convert key to lowercase
		tmp = key;
		while(*tmp && isalnum(*tmp)) *tmp++ = tolower(*tmp);
		*tmp = 0;
		tmp = val;
		while(*tmp && isalnum(*tmp)) tmp++;
		*tmp = 0;

		if( key && val && *key && *val ){
			m_Settings[key] = val;
		}
	}
	infile.close();
}

int ConfigMap::Get(const char * key, bool& val ) 
{
	configmap::iterator iter = m_Settings.find(key);
	if( iter != m_Settings.end() ) {
		const char * pStr = iter->second.c_str();
		if( 0 == strcasecmp(pStr, "true") ){
			val = true;
			return SettingValid;
		}else if( 0 == strcasecmp(pStr, "yes") ) {
			val = true;
			return SettingValid;
		}else if( 0 == strcmp(pStr, "1") ) {
			val = true;
			return SettingValid;
		}else if( 0 == strcasecmp(pStr, "false") ) {
			val = false;
			return SettingValid;
		}else if( 0 == strcasecmp(pStr, "no") ) {
			val = false;
			return SettingValid;
		}else if( 0 == strcmp(pStr, "0") ) {
			val = false;
			return SettingValid;
		}
		return SettingInvalid;
	}
	return SettingNotFound;
}

int ConfigMap::Get(const char * key, int& val ) 
{
	configmap::iterator iter = m_Settings.find(key);
	if( iter != m_Settings.end() ) {
		val = atoi(iter->second.c_str());
		return SettingValid;
	}
	return SettingNotFound;
}

int ConfigMap::Get(const char * key, long& val ) 
{
	configmap::iterator iter = m_Settings.find(key);
	if( iter != m_Settings.end() ) {
		val = atol(iter->second.c_str());
		return SettingValid;
	}
	return SettingNotFound;
}

int ConfigMap::Get(const char * key, string& val )
{
	configmap::iterator iter = m_Settings.find(key);
	if( iter != m_Settings.end() ) {
		val = iter->second;
		return SettingValid;
	}
	return SettingNotFound;
}

//---------------------------------------------------------------------

char * Config::m_sig_dirs[] = { 
	".",
	"~",
	"~/bin",
	"/etc", 
	"/tmp", 
};

Config::Config(): 
	m_Port(PORT), 
	m_MaxClients(MAXCLIENTS),
	m_TimeOut(TIMEOUT), 
	m_NetworkTestMode(NETWORKTESTMODE), 
	m_LocalClientsOnly(LOCALCLIENTSONLY), 
	m_WorkingDir(WORKINGDIR),
	m_IsRoot(false),
	m_Debug(false)
{
	string error;
	struct stat fs;

	m_UID = getuid();
	if(m_UID == 0)
		m_IsRoot = true;

	char buf[MAX_TEXT];
	char homedir[MAX_TEXT];

	snprintf(homedir, MAX_TEXT,"/proc/%d/exe",getpid());
	if( -1 == readlink(homedir,buf,MAX_TEXT) ){ 
		throw ConfigException("Error: could not get exe path", errno);
	}

	m_ExeFilePath = buf;

	homedir[0] = 0;
	char * home = getenv("HOME");
	if ( home && *home )
		strcpy(homedir,home);
	int homedirlen = strlen(homedir);
	if( homedir[homedirlen-1] != '/' ){
		homedir[homedirlen++] = '/';
		homedir[homedirlen] = 0;
	}

	int count = sizeof(m_sig_dirs) / sizeof(char *);
	string sSigFileName = "/";
	sSigFileName += SIG_FILE_NAME;
	sSigFileName += SIG_FILE_EXT;
	
	char * fn = 0;
	char * file = 0;
	for (int i=0; i<count; ++i){

		fn = m_sig_dirs[i];
		if( *fn == '~' ){
			m_SigFilePath = homedir;
			m_SigFilePath += fn+1;
		}else{
			m_SigFilePath = fn;
		}
		m_SigFilePath += sSigFileName;

		file = realpath(m_SigFilePath.c_str(), buf);

		if( file ){
			if( 0 == stat(file, &fs) ){
				cout << "Info: using sig file: " << file << endl;
				break;
			}
		}
	}

	if( -1 == stat(m_WorkingDir.c_str(), &fs) ){
		error = "Error: Invalid working directory: ";
		error += m_WorkingDir;
		throw ConfigException(error.c_str());
	}

	char * fname = basename(m_ExeFilePath.c_str());
	if( !fname || !*fname )
		fname = SIG_FILE_NAME;

	if( m_IsRoot ) {
		m_RunFilePath = RUNDIR;
		m_LogFilePath = LOGDIR;
		if( -1 == stat(m_RunFilePath.c_str(), &fs) ){
			error = "Error: Invalid run directory: ";
			error += m_RunFilePath;
			throw ConfigException(error.c_str());
		}

	}else{
		m_RunFilePath = WORKINGDIR;
		m_LogFilePath = WORKINGDIR;
	}
	if( -1 == stat(m_LogFilePath.c_str(), &fs) ){
		error = "Error: Invalid log directory: ";
		error += m_LogFilePath;
		throw ConfigException(error.c_str());
	}

	m_RunFilePath += "/";
	m_LogFilePath += "/";
	m_RunFilePath += fname;
	m_LogFilePath += fname;
	m_RunFilePath += RUN_FILE_EXT;
	m_LogFilePath += LOG_FILE_EXT;
}

#define ERROR_PORT "Error: Invalid : port number: %d. Port must be greater than %d  and less than %d.", m_Port, MIN_PORT, MAX_PORT
#define ERROR_MAXCLIENTS "Error: Invalid number of maximum clients: %d. This setting must be greater than 0 and less than %d, or -1 for system managed operation.", m_MaxClients, MAX_CLIENTS 
#define ERROR_NETWORKTESTMODE "Error: Invalid network test mode: %d. This setting must be greater than 0 and less than %d", m_NetworkTestMode, MAX_CLIENTS 
#define ERROR_TIMEOUT "Error: Invalid timeout: %d. This setting must be zero or greater than %d  and less than %d.", m_TimeOut, MIN_TIMEOUT, MAX_TIMEOUT 
#define ERROR_LOCALCLIENTSONLY "Error: Invalid parameter"

void Config::Validate()
{
	char buf[1024];
	buf[0] = 0;
	if( m_Port < MIN_PORT || m_Port > MAX_PORT ){
		sprintf(buf, ERROR_PORT );
		throw ConfigException(buf);
	}

	if( -1 != m_MaxClients ){
		if( m_MaxClients < 1 || m_MaxClients > MAX_CLIENTS ){
			sprintf(buf, ERROR_MAXCLIENTS );
			throw ConfigException(buf);
		}
	}

	if( 0 != m_NetworkTestMode ){
		if( m_NetworkTestMode < 1 || m_NetworkTestMode > MAX_CLIENTS ){
			sprintf(buf, ERROR_NETWORKTESTMODE );
			throw ConfigException(buf);
		}
	}

	if( 0 != m_TimeOut ){
		if( m_TimeOut < MIN_TIMEOUT || m_TimeOut > MAX_TIMEOUT ){
			sprintf(buf, ERROR_TIMEOUT );
			throw ConfigException(buf);
		}
	}
}

void Config::MergeConfigFiles( ConfigMap& cfg )
{
	char buf[1024];
	bool changed = false;
	int res;
	res = cfg.Get(sPort.c_str(), m_Port);
	if( ConfigMap::SettingInvalid == res ) {
		snprintf(buf, 1000, ERROR_PORT );
		throw ConfigException(buf);
	}else if( ConfigMap::SettingValid == res ){
		changed = true;
	}
	res = cfg.Get(sMaxClients.c_str(), m_MaxClients);
	if( ConfigMap::SettingInvalid == res ) {
		snprintf(buf, 1000, ERROR_MAXCLIENTS );
		throw ConfigException(buf);
	}else if( ConfigMap::SettingValid == res ){
		changed = true;
	}
	res = cfg.Get(sNetworkTestMode.c_str(), m_NetworkTestMode);
	if( ConfigMap::SettingInvalid == res ) {
		snprintf(buf, 1000, ERROR_NETWORKTESTMODE );
		throw ConfigException(buf);
	}else if( ConfigMap::SettingValid == res ){
		changed = true;
	}
	res = cfg.Get(sTimeOut.c_str(), m_TimeOut);
	if( ConfigMap::SettingInvalid == res ) {
		snprintf(buf, 1000, ERROR_TIMEOUT );
		throw ConfigException(buf);
	}else if( ConfigMap::SettingValid == res ){
		changed = true;
	}
	res = cfg.Get(sLocalClientsOnly.c_str(), m_LocalClientsOnly);
	if( ConfigMap::SettingInvalid == res ) {
		snprintf(buf, 1000, ERROR_LOCALCLIENTSONLY );
		throw ConfigException(buf);
	}else if( ConfigMap::SettingValid == res ){
		changed = true;
	}
	if( changed )
		Validate();
}

void Config::MergeCommandLineOptions( int argc, char ** argv )
{
	char * endptr = NULL;
	int c = 0;
	bool changed = false;

	while ((c = getopt(argc, argv, "p:t:m:n:lhD")) != -1)
	{
		switch (c)
		{
		case 'p':
			m_Port = strtol(optarg, &endptr, 10);
			changed = true;
			break;

	 	case 't':
			m_TimeOut = strtol(optarg, &endptr, 10);
			m_TimeOut *= 60;
			changed = true;
			break;

		case 'm':
			m_MaxClients = strtol(optarg, &endptr, 10);
			changed = true;
			break;

		case 'n':
			m_NetworkTestMode = strtol(optarg, &endptr, 10);
			changed = true;
			break;

		case 'l':
			m_LocalClientsOnly = true;
			break;

		case 'h':
			PrintUsage();
			exit(1);
			break;

		case 'D':
			m_Debug = true;
			break;

		}
	}
	if( changed )
		Validate();
}

void Config::PrintUsage() const
{
	char * fname = basename(m_ExeFilePath.c_str());
	cout << "Usage: " << fname 
	<< " [-h help] "
	<< " [-D debug mode] "
	<< " [-p tcp port number] "
	<< " [-m max clients] " << endl
	<< " [-t client inactivity timeout in minutes] "
	<< " [-l local clients only] "
	<< " [-n network test mode, number of ghost redrat devices to use] " << endl;
}

void Config::Dump() const
{
	cout << "---Application Settings:" << endl
	<< "Port: " << m_Port << endl 
	<< "MaxClients: " << m_MaxClients << endl 
	<< "TimeOut: " << m_TimeOut << endl 
	<< "LocalClientsOnly: " << m_LocalClientsOnly << endl 
	<< "NetworkTestMode: " << m_NetworkTestMode << endl 
	<< "Debug: " << m_Debug << endl 
	<< "IsRoot: " << m_IsRoot << endl 
	<< "UID: " << m_UID << endl 
	<< "WorkingDir: " << m_WorkingDir << endl 
	<< "SigFilePath: " << m_SigFilePath << endl 
	<< "RunFilePath: " << m_RunFilePath << endl 
	<< "LogFilePath: " << m_LogFilePath << endl 
	<< "ExeFilePath: " << m_ExeFilePath << endl
	<< endl;
}

//---------------------------------------------------------------------

