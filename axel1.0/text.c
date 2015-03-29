#include<stdio.h>
#include "axel.h"


/* For returning string values from functions				*/
static char string[MAX_STRING];

int main( int argc, char *argv[] )
{
	char fn[MAX_STRING] = "";
	int i, j, cur_head = 0;
	conf_t conf[1];
	axel_t *axel;
	char *s;
	s = malloc( MAX_STRING );
	
	memset(s, 0, sizeof(s));
	/*strcpy(s,"http://ftp.iij.ad.jp/pub/db/mysql/Downloads/MySQL-5.5/MySQL-5.5.16-1.linux2.6.x86_64.tar");
	*/
	/*strcpy(s,"http://bbs.chinaunix.net/forum.php?mod=attachment&aid=NTA3NTc0fDI3YmZjMmIzfDE0Mjc2MzE5MjV8MHwzNjAwOT");	*/
	
	strcpy(s,argv[1]);
	if( !conf_init( conf ) )
	{
		return( 1 );
	}
	
	
	axel = axel_new( conf, 0, s );
	
	if( *fn )
		{
			sprintf( string, "%s.st", fn );
		}
		else
	{
		/* Local file existence check					*/
		i = 0;
		s = axel->filename + strlen( axel->filename );
		while( 1 )
		{
			sprintf( string, "%s.st", axel->filename );
			if( access( axel->filename, F_OK ) == 0 )
			{
				if( axel->conn[0].supported )
				{
					if( access( string, F_OK ) == 0 )
						break;
				}
			}
			else
			{
				if( access( string, F_OK ) )
					break;
			}
			sprintf( s, ".%i", i );
			i ++;
		}
	}
	return 0;
}