  /********************************************************************\
  * Axel -- A lighter download accelerator for Linux and other Unices. *
  *                                                                    *
  * Copyright 2001 Wilmer van der Gaast                                *
  \********************************************************************/

/* Main control								*/

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License with
  the Debian GNU/Linux distribution in file /usr/doc/copyright/GPL;
  if not, write to the Free Software Foundation, Inc., 59 Temple Place,
  Suite 330, Boston, MA  02111-1307  USA
*/

#include "axel.h"

/* Axel */
static void save_state( axel_t *axel );
static void *setup_thread( void * );
static void axel_message( axel_t *axel, char *format, ... );
static void axel_divide( axel_t *axel );

static char *buffer = NULL;

/* Create a new axel_t structure					*/
axel_t *axel_new( conf_t *conf, int count, void *url )
{
	search_t *res;
	axel_t *axel;
	url_t *u;
	char *s;
	int i;
	
	/* 分配axel结构体内存 */
	axel = malloc( sizeof( axel_t ) );
	memset( axel, 0, sizeof( axel_t ) );
	
	/* 赋值axel结构体 begin */
	
	/* 赋值axel->conf */
	*axel->conf = *conf;
	/* 赋值axel->conn */
	axel->conn = malloc( sizeof( conn_t ) * axel->conf->num_connections );
	memset( axel->conn, 0, sizeof( conn_t ) * axel->conf->num_connections );
	
	if( axel->conf->max_speed > 0 )
	{
		if( (float) axel->conf->max_speed / axel->conf->buffer_size < 0.5 )
		{
			if( axel->conf->verbose >= 2 )
				axel_message( axel, _("Buffer resized for this speed.") );
			axel->conf->buffer_size = axel->conf->max_speed;
		}
		axel->delay_time = (int) ( (float) 1000000 / axel->conf->max_speed * axel->conf->buffer_size * axel->conf->num_connections );
	}
	
	if( buffer == NULL )
		buffer = malloc( max( MAX_STRING, axel->conf->buffer_size ) );
	
	/* 连接数为0 */
	if( count == 0 )
	{
		axel->url = malloc( sizeof( url_t ) );
		axel->url->next = axel->url;
		strncpy( axel->url->text, (char *) url, MAX_STRING );
	}
	else
	{
		res = (search_t *) url;
		u = axel->url = malloc( sizeof( url_t ) );
		for( i = 0; i < count; i ++ )
		{
			/* 赋值url为text */
			strncpy( u->text, res[i].url, MAX_STRING );
			if( i < count - 1 )
			{
				u->next = malloc( sizeof( url_t ) );
				u = u->next;
			}
			else
			{
				u->next = axel->url;
			}
		}
	}
	
	/* 重要点: 前面main中的conf已经赋予axel->conf,现在赋值axel->conn->conf */
	axel->conn[0].conf = axel->conf;
	
	printf("text:%s\n",axel->url->text);
	
	
	/* 用一个url来构造conn_t数据结构 */
	if( !conn_set( &axel->conn[0], axel->url->text ) )
	{
		axel_message( axel, _("Could not parse URL.\n") );
		axel->ready = -1;
		return( axel );
	}
   
  printf("file:%s\n",axel->conn[0].file);


	axel->conn[0].local_if = axel->conf->interfaces->text;
	axel->conf->interfaces = axel->conf->interfaces->next;
	
	printf("===========before conn init ...=====\n");
	printf("local_if:%s\n",axel->conn[0].local_if);
	printf("axel->conn[0].conf->no_proxy:%s\n",axel->conn[0].conf->no_proxy);
	printf("axel->conn[0].file:%s\n",axel->conn[0].file);
	printf("axel->conn[0].message:%s\n",axel->conn[0].message);
	
	
	/* 解析下载文件名 */
	strncpy( axel->filename, axel->conn[0].file, MAX_STRING );
	http_decode( axel->filename );
	printf("axel->filename:%s\n",axel->filename);
	if( *axel->filename == 0 )	/* Index page == no fn		*/
		strncpy( axel->filename, axel->conf->default_filename, MAX_STRING );
	if( ( s = strchr( axel->filename, '?' ) ) != NULL && axel->conf->strip_cgi_parameters )
		*s = 0;		/* Get rid of CGI parameters		*/
	
	/* 打开到http服务器的连接或者打开到FTP服务器的连接并且切换到ftp的下载目录 */
	if( !conn_init( &axel->conn[0] ) )
	{
		axel_message( axel, axel->conn[0].message );
		axel->ready = -1;
		return( axel );
	}
	printf("===========after conn init ...=====\n");
	printf("axel->conn[0].message:%s\n",axel->conn[0].message);
	printf("axel->conn[0].host:%s\n",axel->conn[0].host);
	printf("axel->conn[0].dir:%s\n",axel->conn[0].dir);
	printf("axel->conn[0].file:%s\n",axel->conn[0].file);
	printf("axel->conn[0].user:%s\n",axel->conn[0].user);
	printf("axel->conn[0].pass:%s\n",axel->conn[0].pass);
	printf("axel->conn[0].size:%s\n",axel->conn[0].size);
	printf("axel->conn[0].currentbyte:%s\n",axel->conn[0].currentbyte);
	printf("axel->conn[0].lastbyte:%s\n",axel->conn[0].lastbyte);
	
	
	/* Get file size and other information					*/
	/* This does more than just checking the file size, it all depends
	   on the protocol used.					*/
	/*if( !conn_info( &axel->conn[0] ) )
	{
		axel_message( axel, axel->conn[0].message );
		axel->ready = -1;
		return( axel );
	}
	*/
	
	
	/* Generate a nice URL string.						*/
	s = conn_url( axel->conn );
	strncpy( axel->url->text, s, MAX_STRING );
	printf("axel->url->text:%s\n",axel->url->text);
	if( ( axel->size = axel->conn[0].size ) != INT_MAX )
	{
		if( axel->conf->verbose > 0 )
			axel_message( axel, _("File size: %lld bytes"), axel->size );
	}
	
	/* Wildcards in URL --> Get complete filename			*/
	if( strchr( axel->filename, '*' ) || strchr( axel->filename, '?' ) )
		strncpy( axel->filename, axel->conn[0].file, MAX_STRING );

	
	return( axel );
}




/* Add a message to the axel->message structure				*/
/* axel_message( axel, _("Could not parse URL.\n") ); */
static void axel_message( axel_t *axel, char *format, ... )
{
	message_t *m = malloc( sizeof( message_t ) ), *n = axel->message;
	va_list params;
	
	memset( m, 0, sizeof( message_t ) );
	va_start( params, format );
	vsnprintf( m->text, MAX_STRING, format, params );
	va_end( params );
	
	/* 将format的信息置于axel->message的栈尾  */
	if( axel->message == NULL )
	{
		axel->message = m;
	}
	else
	{
		while( n->next != NULL )
			n = n->next;
		n->next = m;
	}
}