#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <netinet/in.h>
#include "rr3.h"
#include "signaldb.h"
#include "util.h"


#if defined (__GNUC_)
#ident "$Id: dtvrrd.c,v 0.1 2007/02/18 09:00:52 KGuerra Exp $"
#endif

#define PROGRAM_NAME 		"dtvrrd"
#define RUNNING_DIR		"/tmp"
#define MSG_FILE		RUNNING_DIR"/"PROGRAM_NAME".msg"
#define LOG_FILE		RUNNING_DIR"/"PROGRAM_NAME".log"
#define LOCK_FILE		RUNNING_DIR"/"PROGRAM_NAME".lock"
#define LISTEN_PORT_ENV 	"DTVRR_PORT"
#define LISTEN_PORT 		7077
#define CONN_TIMEOUT_ENV 	"DTVRR_TIMEOUT"
#define CONN_TIMEOUT		0 //disabled by default
#define MAX_TEXT 		256
#define MAX_REDRATS 		20
#define MAX_CONNECTIONS		20
#define TRUE   			1
#define FALSE  			0
#define ALLREDRATS		0xFFFF
#define MAX_LOG_SIZE		10485760 //10 meg

#define CLNTERR_OK			0
#define CLNTERR_SIGNAL_NOTFOUND		1
#define CLNTERR_DEVICE_ERROR		2
#define CLNTERR_INVALID_DEVICE		3
#define CLNTERR_DEVICE_NOTPRESENT	4
#define CLNTERR_REDRAT_NOTFOUND		5
#define CLNTERR_REDRAT_UNAVAILABLE	6
#define CLNTERR_REDRAT_INUSE		7
#define CLNTERR_REDRAT_ALREADY_INUSE	8
#define CLNTERR_REDRAT_NOTINUSE		9
#define CLNTERR_REDRAT_NOTOWNED		10
#define CLNTERR_REDRAT_NONE_AVAILABLE	11

int				port		= LISTEN_PORT;
double				conn_timeout    = CONN_TIMEOUT;
char  				xmlfile		[MAX_TEXT];
int				debug		= 0;

int 				server_socket	= 0;
int				isDaemon	= 0;
int				numClients	= 0;
int 				numRedRats 	= 0;
int 				max_connections_reached = 0;
char  				text		[MAX_TEXT];
char				* pMsg		= 0;
int				max_socket_number = 0;
struct	client	{
	int			redrats[MAX_REDRATS];
	int			count;
	int			socket;
	time_t			time;
};
struct  redrat_map {
	int			device_present;
	int			usedby;
	int			index;
	unsigned long		sn;
	char			serial	[128];
};
struct 	client			clients		[MAX_CONNECTIONS];
struct	redrat_map		usb_dev_map	[MAX_REDRATS];
struct 	usb_device 		* redrats	[MAX_REDRATS];
struct 	usb_dev_handle 		* rrDevices	[MAX_REDRATS];
struct 	SignalDb 		* dbSignal 	= 0;

const char * getClientStatusMessage(int code);
void 	parseOptions(int argc, char ** argv);
void 	usage(void);
void 	output_message(const char* fmt,...);
void 	output_msg(const char* fmt,...);
void 	signal_handler(int sig);
void 	daemonize();
void 	shutdown_app(void);
void 	initialize_app(void);
void	close_client(int idx);
void	poll_redrats();
void 	update_redrat(const char * sn, int idx);
void	calc_max_socket_number();
int	free_redrats( struct client * pClient );
int	add_next_redrat( struct client * pClient );
int	alloc_all_available_redrats( struct client * pClient );
int	add_redrat( struct client * pClient, const char * pSN );
int	del_redrat( struct client * pClient, const char * pSN );
int	set_redrat( struct client * pClient, const char * pSN );
int	send_redrat_signal( struct client * pClient, const char * pSignalName );
int	blink_redrat( struct client * pClient );
int	get_redrat_index(const char * pSerialNumber);
void 	ouput_redrat_info();

int main (int argc, char* const argv[])
{
	int i;

	snprintf(text, MAX_TEXT,"/proc/%d/exe",getpid());
	//output_message("Info: Exe path: %s\n", text);
	if( -1 == readlink(text,xmlfile,MAX_TEXT) ) {
		output_message("Warning: could not get exe path: %s\n", strerror(errno));
		strcpy(xmlfile, argv[0]);
	}
	strcat(xmlfile, ".xml");
	
	char * env_port = getenv(LISTEN_PORT_ENV);
	if ( env_port && *env_port )
		port = atoi( env_port );
	char * conn_timeout_str = getenv(CONN_TIMEOUT_ENV);
	if ( conn_timeout_str && *conn_timeout_str )
		conn_timeout = atof( conn_timeout_str );

	parseOptions(argc, argv);
	output_message("Info: DTVRR_PORT: %d DTVRR_TIMEOUT: %f xml file: %s\n", port, conn_timeout, xmlfile );

	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		clients[i].socket 	= 0;
		clients[i].time 	= 0;
		clients[i].count 	= 0;
		clients[i].redrats[0] 	= 0;
	}

	for (i=0; i<MAX_REDRATS; i++)
	{
		usb_dev_map[i].device_present 	= 0;
		usb_dev_map[i].usedby		= -1;
		usb_dev_map[i].index 		= 0;
		usb_dev_map[i].serial[0] 	= 0;
		usb_dev_map[i].sn 		= 0;
	}

	initialize_app();
	output_message("Info: DTVRR_PORT: %d DTVRR_TIMEOUT: %f xml file: %s\n", port, conn_timeout, xmlfile );

	if ((server_socket = socket(AF_INET,SOCK_STREAM,0))==0) 
	{
		output_message("Error: create server socket failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	max_socket_number = server_socket;
	int opt=TRUE;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))<0) 
	{
		output_message("Error: set socket failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(server_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{
		output_message("Error: bind socket failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (listen(server_socket, 3)<0) 
	{
		output_message("Error: listen socket failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if( -1 == fcntl( server_socket, F_SETFL, fcntl(server_socket, F_GETFL) | O_NONBLOCK ) )
	{
		output_message("Error: fcntl failed to set server to non-blocking mode: %s\n", strerror(errno));
	}

	struct timeval timeout;
	while (1) 
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(server_socket, &readfds);
		for (i=0; i<numClients; i++) 
		{
			if (clients[i].socket > 0) 
			{
				FD_SET(clients[i].socket, &readfds);
			}
		}

		timeout.tv_usec = 500000; //half a second, so this thing in theory should process almost two calls per second
		timeout.tv_sec  = 0;
		int activity = select( max_socket_number+3, &readfds, NULL, NULL, &timeout );
		if ( activity == -1 ) 
		{
			output_message("Error: socket select failed: %s\n", strerror(errno));
		}
		else
		{
			if ( !max_connections_reached )
			{
				if (FD_ISSET(server_socket, &readfds)) 
				{
					if ( numClients < MAX_CONNECTIONS ) 
					{	
						int addrlen=sizeof(address);
						int new_socket;
						if ((new_socket = accept(server_socket, (struct sockaddr *)&address, &addrlen))<0)
						{
							output_message("Error: accept socket failed: %s\n", strerror(errno));
							exit(EXIT_FAILURE);
						}
						if( -1 == getpeername(new_socket,&address,&addrlen) )
						{
							output_message("Error: unable to get client's address: %s\n", strerror(errno));
						}else{
							output_message("Info: new connection s#%d %s\n", new_socket, inet_ntoa(address.sin_addr));
						}
						if( -1 == fcntl( new_socket, F_SETFL, fcntl(new_socket, F_GETFL) | O_NONBLOCK ) )
						{
							output_message("Error: fcntl failed to set server to non-blocking mode: %s\n", strerror(errno));
						}
						clients[numClients++].socket = new_socket;
						calc_max_socket_number();
					}
					else
					{
						output_message("Error: Reached maximum number of connections %d\n", MAX_CONNECTIONS);
						max_connections_reached = 1;
						if ( shutdown( server_socket, 0 ) != 0 )
						{
							output_message("Error: shutdown socket failed: %s\n", strerror(errno));
							exit(EXIT_FAILURE);
						}
					}
				}
			}

			for (i=0; i<numClients; i++) 
			{
				if ( FD_ISSET ( clients[i].socket, &readfds ) ) 
				{
					unsigned char length = 0;
					size_t len_count, text_count;
					len_count = read(clients[i].socket, &length, sizeof(length));
					if( -1 == len_count )
					{
						output_message("Warning: read failed for s#%d : %s\n", clients[i].socket, strerror(errno));
						continue;
					}
					else if ( len_count == 0 )
					{
						output_message("Info: s#%d disconnected\n", clients[i].socket);
						close_client(i);
						continue;
					} 
					if ( length < 1 || length > MAX_TEXT )
					{
						output_message("Error!: Message string is NULL or too large for s#%d count %d len %d flusing buffer...\n", clients[i].socket, len_count, length);
						//read ( clients[i].socket, text, MAX_TEXT ); 
						close_client(i);
						continue;
					}

					text_count = read ( clients[i].socket, text, length ); 
					if( -1 == text_count )
					{
						output_message("Warning: read failed for s#%d : %s\n", clients[i].socket, strerror(errno));
					}
					else if ( text_count == 0 )
					{
						output_message("Info: s#%d disconnected\n", clients[i].socket);
						close_client(i);
					} 
					else 
					{
						time_t now_t;
						text[text_count] = 0;
						int res = 0;
	
						if ( text[0] == '~' )
						{
							res = free_redrats( &clients[i] );
						}
						else if ( text[0] == '^' )
						{
							res = add_next_redrat( &clients[i] );
						}
						else if ( text[0] == '*' )
						{
							res = alloc_all_available_redrats( &clients[i] );
						}
						else if ( text[0] == '#' )
						{
							pMsg = text;
							pMsg++;
							res = set_redrat( &clients[i], pMsg );
						}
						else if ( text[0] == '+' )
						{
							pMsg = text;
							pMsg++;
							res = add_redrat( &clients[i], pMsg );
						}
						else if ( text[0] == '-' )
						{
							pMsg = text;
							pMsg++;
							res = del_redrat( &clients[i], pMsg );
						}
						else if ( text[0] == '$' )
						{
							time(&now_t);
							clients[i].time = now_t;
							res = 0;
						}
						else if ( text[0] == '!' )
						{
							res = blink_redrat( &clients[i] );
						}
						else
						{
							time(&now_t);
							clients[i].time = now_t;
							struct tm now;
							now = *localtime(&now_t);
							res = send_redrat_signal( &clients[i], text );
							output_msg("%02d.%02d.%02d %02d:%02d:%02d s#%d '%s' = %s\n", now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, clients[i].socket, text, getClientStatusMessage(res) );
						}
						sprintf(text, "%d \0", res );
						size_t wres = write( clients[i].socket, text, 2 ); 
						if( -1 == wres )
						{
							output_message("Warning: write failed for s#%d : %s\n", clients[i].socket, strerror(errno));
						}
						else if ( wres < 2 )
						{
							output_message("Error: write failed for s#%d : length \n", clients[i].socket);
						}
						//send( clients[i].socket, text, 2, MSG_NOSIGNAL | MSG_DONTWAIT ); 
					}
				}
			}
		}
		if ( conn_timeout ){
			for (i=0; i<numClients; i++) 
			{
				if ( clients[i].time )
				{
					time_t now_t2;
					time(&now_t2);
					double tdiff = difftime( now_t2, clients[i].time );
					if ( tdiff >= conn_timeout )
					{
						output_message("Warning: client timed out, closing s#%d\n", clients[i].socket);
						close_client(i);
					}
				}
			}
		}
		poll_redrats(); //this enables hot-plugin or whatever capability
	}
	return 0;
}

int send_redrat_signal( struct client * pClient, const char * pSignalName )
{
	struct 	RR3ModulatedSignal * rrsignal = 0;
	if ((rrsignal = signalDbGetSignal(DB_DEFAULT_AVDEVICENAME, pSignalName, dbSignal)) == NULL)
	{
		output_message("Error: Could not find signal '%s' in db\n", pSignalName);
		return CLNTERR_SIGNAL_NOTFOUND;
	} 
	else
	{
		int i;
		int res = CLNTERR_DEVICE_NOTPRESENT;
		for (i=0; i<pClient->count; i++) 
		{
			int rridx = pClient->redrats[i];
			if ( usb_dev_map[ rridx ].device_present == 1 )
			{
				int index = usb_dev_map[ rridx ].index;
				if ( index != -1 && index < MAX_REDRATS )
				{
					if (rr3OutputModulatedSignal( rrDevices[index], rrsignal) < 0)
					{
						output_message("Error: Output failed rr#%i: %s\n", i, r3Strerror());
						return CLNTERR_DEVICE_ERROR;
					}
				}
				else
				{
					output_message("Error: Invalid device %d for rr#%i: signal: %s. Device may be disconnected.\n", index, rridx, pSignalName );
					return CLNTERR_INVALID_DEVICE;
				}
				res = CLNTERR_OK;
			}
			else
			{
				output_message("Warning: Device rr#%i: is unavailable when sending signal %s\n", rridx, pSignalName );
				return CLNTERR_DEVICE_NOTPRESENT;
			}
		}
		return res;
	}
}

int blink_redrat( struct client * pClient )
{
	int i;
	for (i=0; i<pClient->count; i++) 
	{
		int rridx = pClient->redrats[i];
		if ( usb_dev_map[ rridx ].device_present == 1 )
		{
			int index = usb_dev_map[ rridx ].index;
			if ( index != -1 && index < MAX_REDRATS )
			{
				if (rr3Blink( rrDevices[index]) < 0)
				{
					output_message("Error: Blink failed rr#%i\n", i);
					//output_message("Error: Blink rr#%i: %s\n", i, r3Strerror());
					return CLNTERR_DEVICE_ERROR;
				}
			}
		}
	}
	return CLNTERR_OK;
}

int free_redrats( struct client * pClient )
{
	output_message("Info: Clearing redrat list for s#%d \n", pClient->socket);
	int i;
	for (i=0; i<pClient->count; i++) 
	{
		int idx = pClient->redrats[i];
		if ( idx >= 0 && idx < MAX_REDRATS )
		{
			if ( pClient->socket == usb_dev_map[idx].usedby )
			{
				usb_dev_map[ pClient->redrats[i] ].usedby = -1;
			}
			else
			{
				output_message("Error!!!: rr#%i did not belong to s#%d, so it will NOT be freed! \n", idx, pClient->socket);
			}
		}
	}
	pClient->redrats[0] = 0;
	pClient->count = 0;
	return CLNTERR_OK;
}

int alloc_all_available_redrats( struct client * pClient )
{
	free_redrats( pClient );
	output_message("Info: Setting ALL available redrats for s#%d \n", pClient->socket);
	int allocated = 0;
	int i;
	for (i=0; i<MAX_REDRATS; i++)
	{
		if ( usb_dev_map[i].device_present == 1 && usb_dev_map[i].usedby == -1 )
		{
			pClient->redrats[pClient->count++] = i;
			usb_dev_map[i].usedby = pClient->socket;
			allocated = 1;
		}
	}
	if ( !allocated )
	{
		output_message("Error: Could not find an available redrat for s#%d \n", pClient->socket);
		return CLNTERR_REDRAT_NONE_AVAILABLE;
	}
	return CLNTERR_OK;
}

int set_redrat( struct client * pClient, const char * pSN )
{
	free_redrats( pClient );
	return add_redrat( pClient, pSN );
}

int add_redrat( struct client * pClient, const char * pSN )
{
	unsigned long sn = atol(pSN);
	int rrsn = get_redrat_index(pSN);
	output_message("Info: Adding redrat sn: '%s' for s#%d idx: %d\n", pSN, pClient->socket, rrsn);
	if ( rrsn == -1 )
	{
		output_message("Error: Redrat not found! \n");
		return CLNTERR_REDRAT_NOTFOUND;
	}
	else
	{
		int i;
		for (i=0; i<pClient->count; i++) 
		{
			int rridx = pClient->redrats[i];
			int index = usb_dev_map[ rridx ].index;
			if ( index != -1 && index < MAX_REDRATS )
			{
				if ( index == rrsn )
				{
					output_message("Warning: redrat already in list \n");
					if ( pClient->socket != usb_dev_map[rridx].usedby )
					{
						output_message("Warning: rr#%i was not owned by s#%i \n", rridx , pClient->socket );
					}
					return CLNTERR_REDRAT_ALREADY_INUSE;
				}
			}
		}
		for (i=0; i<MAX_REDRATS; i++)
		{
			if ( usb_dev_map[i].sn == sn ) 
			{
				if ( usb_dev_map[i].device_present == 1 )
				{
					if ( usb_dev_map[i].usedby == -1 )
					{
						usb_dev_map[i].usedby = pClient->socket;
						pClient->redrats[pClient->count++] = i;
						return CLNTERR_OK;
					}
					else
					{
						output_message("Error: Redrat already in use!\n");
						return CLNTERR_REDRAT_INUSE;
					}
				}
				else
				{
					output_message("Error: Redrat unavailable!\n");
					return CLNTERR_REDRAT_UNAVAILABLE;
				}
			}
		}
	}
	return CLNTERR_REDRAT_NOTFOUND;
}

int add_next_redrat( struct client * pClient )
{
	int i;
	for (i=0; i<MAX_REDRATS; i++)
	{
		if ( usb_dev_map[i].device_present == 1 && usb_dev_map[i].usedby == -1 )
		{
			usb_dev_map[i].usedby = pClient->socket;
			pClient->redrats[pClient->count++] = i;
			return CLNTERR_OK;
		}
	}
	return CLNTERR_REDRAT_UNAVAILABLE;
}

int del_redrat( struct client * pClient, const char * pSN )
{
	unsigned long sn = atol(pSN);
	output_message("Info: Deleting redrat sn: '%s' for s#%d\n", pSN, pClient->socket);
	int rrsn = get_redrat_index(pSN);
	if ( rrsn == -1 )
	{
		output_message("Warning: Redrat not found! \n");
		return CLNTERR_REDRAT_NOTFOUND;
	}
	int ret = CLNTERR_REDRAT_NOTFOUND;
	int i;
	for (i=0; i<MAX_REDRATS; i++)
	{
		if ( usb_dev_map[i].sn == sn )
		{
			if ( usb_dev_map[i].device_present == 0 )
			{
				output_message("Warning: Redrat was unavailable!\n");
			}
			
			if ( usb_dev_map[i].usedby == -1 )
			{
				output_message("Warning: Redrat was not being used\n");
				ret = CLNTERR_REDRAT_NOTINUSE;
			}
			else
			{
				if ( usb_dev_map[i].usedby == pClient->socket )
				{
					usb_dev_map[i].usedby = -1;
					ret = CLNTERR_OK;
				}
				else
				{
					output_message("Error: rr#%i did not belong to s#%d\n", i, pClient->socket);
					ret = CLNTERR_REDRAT_NOTOWNED;
				}
			}

			for (i=0; i<pClient->count; i++) 
			{
				if ( usb_dev_map[pClient->redrats[i]].index == rrsn )
				{
					int j = i + 1;
					for ( ; j<pClient->count; j++ )
					{
						pClient->redrats[j-1] = pClient->redrats[j];
					}
					pClient->redrats[pClient->count--] = 0;
					return ret;
				}
			}
			return ret;
		}
	}
	return ret;
}

int get_redrat_index(const char * pSerialNumber)
{
	char * pSN = pSerialNumber;
	while ( *pSN == '0' ) pSN++;
	output_message("Info: Searching redrat with serial number '%s'\n", pSN);
	char buf[128];
	int i;
	for (i=0; i<numRedRats; i++) 
	{
		unsigned long sn = rr3GetSerialNumber(rrDevices[i], buf, 127 );
		if (sn == atol(pSN)){
			return i;
		}
	}
	return -1;
}

void close_client(int idx)
{
	free_redrats( &clients[idx] );
	struct sockaddr_in address;
	long addrlen = sizeof(address);
	getpeername(clients[idx].socket,&address,&addrlen);
	output_message("Info: client disconnected s#%d %s\n", clients[idx].socket, inet_ntoa(address.sin_addr));
	close(clients[idx].socket);
	clients[idx].socket = 0;
	clients[idx].time = 0;
	int j = idx + 1;
	for ( ; j<numClients; j++ )
	{
		memcpy( clients[j-1].redrats, clients[j].redrats, sizeof(int) * MAX_REDRATS );
		clients[j-1].count = clients[j].count;
		clients[j-1].socket = clients[j].socket;
		clients[j-1].time = clients[j].time;
	}
	numClients--;
	calc_max_socket_number();
	if ( max_connections_reached )
	{
		if (listen(server_socket, 3)<0) 
		{
			output_message("Error: listen socket failed: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		max_connections_reached = 0;
	}
}

void calc_max_socket_number()
{
	max_socket_number = server_socket;
	int i;
	for (i=0; i<numClients; i++) 
		if( clients[i].socket > max_socket_number )
			max_socket_number = clients[i].socket;

}

void initialize_app(void)
{
	remove(MSG_FILE);
	remove(LOG_FILE);
	remove(LOCK_FILE);

	if(!debug)	
		daemonize();
	output_message("Initializing....\n");
	output_message("Info: Reading xml file: %s\n", xmlfile);
	chmod( MSG_FILE, S_IROTH | S_IRUSR | S_IWUSR | S_IRGRP );
	chmod( LOG_FILE, S_IROTH | S_IRUSR | S_IWUSR | S_IRGRP );
	
	memset(redrats, 0, sizeof(struct usb_device *) * MAX_REDRATS);

	if (rr3DoUsbInit() < 0)
	{
		output_message("Error: USB init failed: %s\n", rr3Strerror());
		exit(EXIT_FAILURE);
	}
	if ((dbSignal = loadSignalDb(xmlfile)) == NULL)
	{
		output_message("Error: Could not read signalDb: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	poll_redrats();
}

void poll_redrats()
{
	if (rr3DoRedRatDevicesInit() < 0)
	{
		output_message("Error: USB init failed: %s\n", rr3Strerror());
		exit(EXIT_FAILURE);
	}
	int nrrs = rr3CountDevices();
	if ( nrrs != numRedRats )
	{
		int i; 
		for (i=0; i<numRedRats; i++)
		{
			rr3CloseDevice(rrDevices[i]);
		}
		for (i=0; i<MAX_REDRATS; i++)
		{
			usb_dev_map[i].device_present = 0;
			usb_dev_map[i].index = -1;
		}
		if ((nrrs = rr3FindDevices(redrats, MAX_REDRATS)) <= 0)
		{
			output_message("Error: Could not find a redrat\n");
		}
		numRedRats = nrrs;
		if (numRedRats > MAX_REDRATS)
			numRedRats = MAX_REDRATS;
		for (i=0; i<numRedRats; i++)
		{
			if ((rrDevices[i] = rr3OpenDevice(redrats[i])) == NULL)
			{
				output_message("Error: Could not open redrat: %s\n", usb_strerror());
				exit(EXIT_FAILURE);
			}
			//unsigned long sn = rr3GetSerialNumber(rrDevices[i], text, MAX_TEXT );
			rr3GetSerialNumber(rrDevices[i], text, MAX_TEXT );
			char * pRRSN = text;
			update_redrat( pRRSN, i );
		}
		output_message("\nInfo: RedRat devices connection info changed\n");
		ouput_redrat_info();
	}
}

void update_redrat( const char * pSN, int idx )
{
	unsigned long sn = atol(pSN);
	int i;
	for (i=0; i<MAX_REDRATS; i++) //reuse previous
	{
		if ( usb_dev_map[i].sn == sn )
		{
			usb_dev_map[i].device_present = 1;
			usb_dev_map[i].index = idx;
			return;
		}
	}
	for (i=0; i<MAX_REDRATS; i++) //newly connected
	{
		if ( usb_dev_map[i].serial[0] == 0 )
		{
			strncpy( usb_dev_map[i].serial, pSN, 127 );
			usb_dev_map[i].device_present = 1;
			usb_dev_map[i].index = idx;
			usb_dev_map[i].sn = sn;
			return;
		}
	}
	for (i=0; i<MAX_REDRATS; i++) //slots are full, reuse any available, overwrite serial number
	{
		if ( usb_dev_map[i].device_present == 0 )
		{
			strncpy( usb_dev_map[i].serial, pSN, 127 );
			usb_dev_map[i].device_present = 1;
			usb_dev_map[i].index = idx;
			usb_dev_map[i].sn = sn;
			return;
		}
	}
}

void ouput_redrat_info()
{
	output_message("--------------------------------------------\n");
	int i;
	for (i=0; i<MAX_REDRATS; i++)
	{
		if ( usb_dev_map[i].serial[0] )
		{
			output_message("redrat #%d idx: %d sn: '%s' used by s#%d\n", i, usb_dev_map[i].index, usb_dev_map[i].serial, usb_dev_map[i].usedby );
		}
	}
	output_message("--------------------------------------------\n\n");
}

void shutdown_app(void)
{
	output_message("Shutting down...\n");
	int i;
	for (i=0; i<numClients; i++)
	{
		if (0 != close (clients[i].socket))
		{
			output_message("Error: listen socket failed: %s\n", strerror(errno));
		}
	}
	for (i=0; i<numRedRats; i++)
	{
		rr3CloseDevice(rrDevices[i]);
	}
	destroySignalDb(dbSignal);
	remove(LOCK_FILE);
}

void signal_handler(sig)
int sig;
{
	switch(sig) {
	case SIGHUP:
		output_message("hangup signal caught\n");
		break;
	case SIGTERM:
		output_message("terminate signal caught\n");
		shutdown_app();
		exit(EXIT_SUCCESS);
		break;
	}
}

void daemonize()
{
	int i,lfp;
	char str[10];

	if(getppid()==1) return; /* already a daemon */
	i=fork();
	if (i<0) exit(1); /* fork error */
	if (i>0) exit(0); /* parent exits */
	/* child (daemon) continues */
	setsid(); /* obtain a new process group */
	for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
	i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */
	umask(027); /* set newly created file permissions */
	chdir(RUNNING_DIR); /* change running directory */
	lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
	if (lfp<0) exit(1); /* can not open */
	if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
	/* first instance continues */
	sprintf(str,"%d\n",getpid());
	write(lfp,str,strlen(str)); /* record pid to lockfile */
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
	signal(SIGTERM,signal_handler); /* catch kill signal */
	
	isDaemon = 1;
}

void output_message(const char *fmt, ...)
{
	time_t now_t;
	time(&now_t);
	struct tm now;
	now = *localtime(&now_t);
	FILE * file = stdout;
	if (isDaemon)
	{
		struct stat fs;
		stat(LOG_FILE, &fs);
		file = fopen(LOG_FILE, fs.st_size > MAX_LOG_SIZE ? "w" : "a");
		if(!file) 
			return;
	}
	fprintf(file, "%02d.%02d.%02d %02d:%02d:%02d ", now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec );
	va_list ap;
	va_start(ap, fmt);
	vfprintf(file,fmt,ap);
	va_end(ap);
	if (isDaemon)
		fclose(file);
}

void output_msg(const char *fmt, ...)
{
	FILE * file = stdout;
	if (isDaemon)
	{
		struct stat fs;
		stat(MSG_FILE, &fs);
		file = fopen(MSG_FILE, fs.st_size > MAX_LOG_SIZE ? "w" : "a");
		if(!file) 
			return;
	}
	va_list ap;
	va_start(ap, fmt);
	vfprintf(file,fmt,ap);
	va_end(ap);
	if (isDaemon)
		fclose(file);
}

void usage(void)
{
	fprintf(stderr, "Usage: " PROGRAM_NAME " [-h help] [-D debug] "
	                "[-p port(int)] \n[-t client timeout in minutes(int)]"
	                " [-d xmldb file(string) FULL PATH NAME]\n");
}

char  				xmlfile		[MAX_TEXT];
void parseOptions(int argc, char ** argv)
{
	char * endptr = NULL;
	int c = 0;

	while ((c = getopt(argc, argv, "hp:p:t:Dd:d:")) != -1)
	{
		switch (c)
		{
		case 'h':
			usage();
			exit(1);
			break;

		case 'p':
			port = strtol(optarg, &endptr, 10);
			
			if (*endptr != '\0')
			{
				fprintf(stderr, "Invalid argument - %s\n", optarg);
				exit(1);
			}

			if ((port <= 0))
			{
				fprintf(stderr, "Port must be greater than zero\n");
				exit(1);
			}
			break;

	 	case 't':
			conn_timeout = strtol(optarg, &endptr, 10);
			conn_timeout *= 60;
			
			if (*endptr != '\0')
			{
				fprintf(stderr, "Invalid argument - %s\n", optarg);
				exit(1);
			}
			break;

		case 'D':
			debug = 1;
			rr3SetDebug(debug);
			break;
		case 'd':
			strcpy(xmlfile, optarg);
			break;
		}
	}

}

const char * getClientStatusMessage(int code)
{
	switch(code)
	{
	case CLNTERR_OK:
		return "ok";
	case CLNTERR_SIGNAL_NOTFOUND:
		return "signal not found";
	case CLNTERR_DEVICE_ERROR:
		return "device error!!!";
	case CLNTERR_INVALID_DEVICE:
		return "invalid device";
	case CLNTERR_DEVICE_NOTPRESENT:
		return "device not present";
	case CLNTERR_REDRAT_NOTFOUND:
		return "redrat not found";
	case CLNTERR_REDRAT_UNAVAILABLE:
		return "redrat unavailable";
	case CLNTERR_REDRAT_INUSE:
		return "redrat in use";
	case CLNTERR_REDRAT_ALREADY_INUSE:
		return "redrat already in use";
	case CLNTERR_REDRAT_NOTINUSE:
		return "redrat not in use";
	case CLNTERR_REDRAT_NOTOWNED:
		return "client does not own redrat";
	}
}
