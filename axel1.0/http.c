#include "axel.h"


int http_connect( http_t *conn, int proto, char *proxy, char *host, int port, char *user, char *pass )
{
	char base64_encode[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz0123456789+/";
	char auth[MAX_STRING];
	conn_t tconn[1];
	int i;
	
	strncpy( conn->host, host, MAX_STRING );
	conn->proto = proto;
	
	if( proxy != NULL ) { if( *proxy != 0 )
	{
		sprintf( conn->host, "%s:%i", host, port );
		if( !conn_set( tconn, proxy ) )
		{
			/* We'll put the message in conn->headers, not in request */
			sprintf( conn->headers, _("Invalid proxy string: %s\n"), proxy );
			return( 0 );
		}
		host = tconn->host;
		port = tconn->port;
		conn->proxy = 1;
	}
	else
	{
		conn->proxy = 0;
	} }
	
	if( ( conn->fd = tcp_connect( host, port, conn->local_if ) ) == -1 )
	{
		/* We'll put the message in conn->headers, not in request */
		sprintf( conn->headers, _("Unable to connect to server %s:%i\n"), host, port );
		return( 0 );
	}
	
	if( *user == 0 )
	{
		*conn->auth = 0;
	}
	else
	{
		memset( auth, 0, MAX_STRING );
		snprintf( auth, MAX_STRING, "%s:%s", user, pass );
		for( i = 0; auth[i*3]; i ++ )
		{
			conn->auth[i*4] = base64_encode[(auth[i*3]>>2)];
			conn->auth[i*4+1] = base64_encode[((auth[i*3]&3)<<4)|(auth[i*3+1]>>4)];
			conn->auth[i*4+2] = base64_encode[((auth[i*3+1]&15)<<2)|(auth[i*3+2]>>6)];
			conn->auth[i*4+3] = base64_encode[auth[i*3+2]&63];
			if( auth[i*3+2] == 0 ) conn->auth[i*4+3] = '=';
			if( auth[i*3+1] == 0 ) conn->auth[i*4+2] = '=';
		}
	}
	
	return( 1 );
}

void http_encode( char *s )
{
	char t[MAX_STRING];
	int i, j;
	
	for( i = j = 0; s[i]; i ++, j ++ )
	{
		/* Fix buffer overflow */
		if (j >= MAX_STRING - 1) {
			break;
		}
		
		t[j] = s[i];
		if( s[i] == ' ' )
		{
			/* Fix buffer overflow */
			if (j >= MAX_STRING - 3) {
				break;
			}
			
			strcpy( t + j, "%20" );
			j += 2;
		}
	}
	t[j] = 0;
	
	strcpy( s, t );
}

/* Decode%20a%20file%20name						*/
void http_decode( char *s )
{
	char t[MAX_STRING];
	int i, j, k;
	
	for( i = j = 0; s[i]; i ++, j ++ )
	{
		t[j] = s[i];
		if( s[i] == '%' )
			if( sscanf( s + i + 1, "%2x", &k ) )
			{
				t[j] = k;
				i += 2;
			}
	}
	t[j] = 0;
	
	strcpy( s, t );
}


void http_disconnect( http_t *conn )
{
	if( conn->fd > 0 )
		close( conn->fd );
	conn->fd = -1;
}