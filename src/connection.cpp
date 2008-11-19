/*-
 *   Copyright (C) 2008 by gelraen
 *   gelraen.ua@gmail.com
 *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *    modification, are permitted provided that the following conditions   *
 *    are met:                                                             *
 *     * Redistributions of source code must retain the above copyright    *
 *       notice, this list of conditions and the following disclaimer.     *
 *     * Redistributions in binary form must reproduce the above copyright *
 *       notice, this list of conditions and the following disclaimer in   *
 *       the documentation and/or other materials provided with the        *
 *       distribution.                                                     *
 *     * Neither the name of the gelraen nor the names of its contributors *
 *       may be used to endorse or promote products derived from this      *
 *       software without specific prior written permission.               *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR *
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      *
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, *
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY *
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   *
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE *
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  *
 *
 *  $Id$
 */
#include "connection.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

Connection::Connection()
{
	m_bConnected=false;
}


Connection::~Connection()
{
}


/*!
    \fn Connection::FdSet(fd_set& fdset)
 */
bool Connection::FdSet( fd_set& fdset )
{
	if ( !m_bConnected ) return false;

	FD_SET( m_socket,&fdset ); // adding our socket descriptor to fdset

	return true;
}


/*!
    \fn Connection::io()
 */
void Connection::io()
{
	_write();
	_read();
}


/*!
    \fn Connection::Connect(const struct sockaddr *name, socklen_t namelen)
 */
int Connection::Connect(const struct sockaddr *name, socklen_t namelen)
{
	if (m_bConnected)
	{
		Close();
	}
	m_socket=socket(name->sa_family,SOCK_STREAM,0);
	if (m_socket!=-1)
	{
		perror("Connection::Connect(): socket(2) failed:");
		exit(1);
	}
	if (connect(m_socket,name,namelen)==-1)
	{
		perror("Connection::Connect(): connect(2) failed:");	
	}
	else
	{
		m_bConnected=true;
	}
}


/*!
    \fn Connection::Close()
 */
int Connection::Close()
{
	if (m_bConnected)
	{
		m_recvbuf.erase();
		m_sendbuf.erase();
		return close(m_socket);
	}
	return 0;
}


/*!
    \fn Connection::_write()
 */
void Connection::_write()
{
	if (!m_bConnected) return;
	int n=0;
	while(m_sendbuf.length()>0)
	{
		n=write(m_socket,m_sendbuf.c_str(),m_sendbuf.length());
		if (n==-1) break;
		m_sendbuf.erase(0,n);
	}
	
	if (n==-1)
	{
		switch(errno)
		{
			case ECONNRESET:
				close(m_socket);
				m_bConnected=false;
				break;
			default:
				perror("Connection::_write(): write(2) failed:");
				break;
		};
	}}


/*!
    \fn Connection::_read()
 */
void Connection::_read()
{
	if (!m_bConnected) return;
	char buf[4096]={0};
	int n=0;
	while(n=read(m_socket,(void*)buf,4096)>0)
	{
		m_recvbuf+=string(buf,n);
	}
	
	if (n==-1)
	{
		switch(errno)
		{
			case ECONNRESET:
				close(m_socket);
				m_bConnected=false;
				break;
			default:
				perror("Connection::_read(): read(2) failed:");
				break;
		};
	}}