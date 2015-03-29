#include "axel.h"

char string[MAX_STRING];

/* Convert an URL to a conn_t structure					*/
int conn_set( conn_t *conn, char *set_url )
{
	char url[MAX_STRING];
	char *i, *j;
	
	/* protocol://							*/
	if( ( i = strstr( set_url, "://" ) ) == NULL )
	{
		conn->proto = PROTO_DEFAULT;
		strncpy( url, set_url, MAX_STRING );
	}
	else
	{
		if( set_url[0] == 'f' )
			conn->proto = PROTO_FTP;
		else if( set_url[0] == 'h' )
			conn->proto = PROTO_HTTP;
		else
		{
			return( 0 );
		}
		strncpy( url, i + 3, MAX_STRING );
	}
	
	printf("url : %s\n",url);
	
	/* Split							*/
	if( ( i = strchr( url, '/' ) ) == NULL )
	{
		strcpy( conn->dir, "/" );
	}
	else
	{
		*i = 0;
		snprintf( conn->dir, MAX_STRING, "/%s", i + 1 );
		if( conn->proto == PROTO_HTTP )
			http_encode( conn->dir );
	}
	strncpy( conn->host, url, MAX_STRING );
	j = strchr( conn->dir, '?' );
	if( j != NULL )
		*j = 0;
	i = strrchr( conn->dir, '/' );
	*i = 0;
	if( j != NULL )
		*j = '?';
	if( i == NULL )
	{
		strncpy( conn->file, conn->dir, MAX_STRING );
		strcpy( conn->dir, "/" );
	}
	else
	{
		strncpy( conn->file, i + 1, MAX_STRING );
		strcat( conn->dir, "/" );
	}
	
	printf("conn->file : %s\n",conn->file);
	/* Check for username in host field				*/
	if( strrchr( conn->host, '@' ) != NULL )
	{
		strncpy( conn->user, conn->host, MAX_STRING );
		i = strrchr( conn->user, '@' );
		*i = 0;
		strncpy( conn->host, i + 1, MAX_STRING );
		*conn->pass = 0;
	}
	/* If not: Fill in defaults					*/
	else
	{
		if( conn->proto == PROTO_FTP )
		{
			/* Dash the password: Save traffic by trying
			   to avoid multi-line responses		*/
			strcpy( conn->user, "anonymous" );
			strcpy( conn->pass, "mailto:axel-devel@lists.alioth.debian.org" );
		}
		else
		{
			*conn->user = *conn->pass = 0;
		}
	}
	
	/* Password?							*/
	if( ( i = strchr( conn->user, ':' ) ) != NULL )
	{
		*i = 0;
		strncpy( conn->pass, i + 1, MAX_STRING );
	}
	/* Port number?							*/
	if( ( i = strchr( conn->host, ':' ) ) != NULL )
	{
		*i = 0;
		sscanf( i + 1, "%i", &conn->port );
	}
	/* Take default port numbers from /etc/services			*/
	else
	{
#ifndef DARWIN
		struct servent *serv;
		
		if( conn->proto == PROTO_FTP )
			serv = getservbyname( "ftp", "tcp" );
		else
			serv = getservbyname( "www", "tcp" );
		
		if( serv )
			conn->port = ntohs( serv->s_port );
		else
#endif
		if( conn->proto == PROTO_HTTP )
			conn->port = 80;
		else
			conn->port = 21;
	}
	
	return( conn->port > 0 );
}


/* 打开到http服务器的连接或者打开到FTP服务器的连接并且切换到ftp的下载目录 */
int conn_init( conn_t *conn )
{
	char *proxy = conn->conf->http_proxy, *host = conn->conf->no_proxy;
	int i;
	
	if( *conn->conf->http_proxy == 0 )
	{
		proxy = NULL;
	}
	else if( *conn->conf->no_proxy != 0 )
	{
		for( i = 0; ; i ++ )
			if( conn->conf->no_proxy[i] == 0 )
			{
				if( strstr( conn->host, host ) != NULL )
					proxy = NULL;
				host = &conn->conf->no_proxy[i+1];
				if( conn->conf->no_proxy[i+1] == 0 )
					break;
			}
	}
	
	conn->proxy = proxy != NULL;
	
	if( conn->proto == PROTO_FTP && !conn->proxy )
	{
		conn->ftp->local_if = conn->local_if;
		conn->ftp->ftp_mode = FTP_PASSIVE;
		/*if( !ftp_connect( conn->ftp, conn->host, conn->port, conn->user, conn->pass ) )
		{
			conn->message = conn->ftp->message;
			conn_disconnect( conn );
			return( 0 );
		}
		conn->message = conn->ftp->message;
		if( !ftp_cwd( conn->ftp, conn->dir ) )
		{
			conn_disconnect( conn );
			return( 0 );
		}*/
	}
	else
	{
		conn->http->local_if = conn->local_if;
		if( !http_connect( conn->http, conn->proto, proxy, conn->host, conn->port, conn->user, conn->pass ) )
		{
			conn->message = conn->http->headers;
			conn_disconnect( conn );
			return( 0 );
		}
		conn->message = conn->http->headers;
		conn->fd = conn->http->fd;
	}
	return( 1 );
}

/* Simple...								*/
void conn_disconnect( conn_t *conn )
{
	if( conn->proto == PROTO_FTP && !conn->proxy )
		ftp_disconnect( conn->ftp );
	else
		http_disconnect( conn->http );
	conn->fd = -1;
}


/* Generate a nice URL string.						*/
char *conn_url( conn_t *conn )
{
	if( conn->proto == PROTO_FTP )
		strcpy( string, "ftp://" );
	else
		strcpy( string, "http://" );
	
	if( *conn->user != 0 && strcmp( conn->user, "anonymous" ) != 0 )
		sprintf( string + strlen( string ), "%s:%s@",
			conn->user, conn->pass );

	sprintf( string + strlen( string ), "%s:%i%s%s",
		conn->host, conn->port, conn->dir, conn->file );
	
	return( string );
}