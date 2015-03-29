#include "axel.h"


void ftp_disconnect( ftp_t *conn )
{
	if( conn->fd > 0 )
		close( conn->fd );
	if( conn->data_fd > 0 )
		close( conn->data_fd );
	if( conn->message )
	{
		free( conn->message );
		conn->message = NULL;
	}

	*conn->cwd = 0;
	conn->fd = conn->data_fd = -1;
}