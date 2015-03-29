#include "axel.h"

int conf_init( conf_t *conf )
{
	char s[MAX_STRING], *s2;
	int i;
	
	/* Set defaults							*/
	memset( conf, 0, sizeof( conf_t ) );
	strcpy( conf->default_filename, "default" );
	*conf->http_proxy		= 0;
	*conf->no_proxy			= 0;
	conf->strip_cgi_parameters	= 1;
	conf->save_state_interval	= 10;
	conf->connection_timeout	= 45;
	conf->reconnect_delay		= 20;
	conf->num_connections		= 4;
	conf->buffer_size		= 5120;
	conf->max_speed			= 0;
	conf->verbose			= 1;
	conf->alternate_output		= 0;
	
	conf->search_timeout		= 10;
	conf->search_threads		= 3;
	conf->search_amount		= 15;
	conf->search_top		= 3;
	conf->add_header_count		= 0;
	strncpy( conf->user_agent, DEFAULT_USER_AGENT, MAX_STRING );
	
	/* if_t *interfaces; */
	conf->interfaces = malloc( sizeof( if_t ) );
	memset( conf->interfaces, 0, sizeof( if_t ) );
	conf->interfaces->next = conf->interfaces;
	
	if( ( s2 = getenv( "http_proxy" ) ) != NULL )
		strncpy( conf->http_proxy, s2, MAX_STRING );
	else if( ( s2 = getenv( "HTTP_PROXY" ) ) != NULL )
		strncpy( conf->http_proxy, s2, MAX_STRING );
	/*
	if( !conf_loadfile( conf, ETCDIR "/axelrc" ) )
		return( 0 );
	*/
	if( ( s2 = getenv( "HOME" ) ) != NULL )
	{
		sprintf( s, "%s/%s", s2, ".axelrc" );
		/*if( !conf_loadfile( conf, s ) )
			return( 0 );
			*/
	}
	
	/* Convert no_proxy to a 0-separated-and-00-terminated list..	*/
	for( i = 0; conf->no_proxy[i]; i ++ )
		if( conf->no_proxy[i] == ',' )
			conf->no_proxy[i] = 0;
	conf->no_proxy[i+1] = 0;
	
	return( 1 );
}