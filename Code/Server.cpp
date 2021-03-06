#include "Engine.hpp"

Server::Server() {

};

Server::~Server() {

}

bool Server::InitiatePort(int portNum) {
	static struct sockaddr_in sa_zero;    
	struct sockaddr_in sa;   
	int x = 1;   
	int fd;     
	
	if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )    
	{	
		// -- TODO: insert logging mechanism
		return false;
	} 
	
	if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,    (char *) &x, sizeof(x) ) < 0 )    
	{
		// -- TODO: insert logging mechanism
		close(fd);	
		exit( 1 );    
	}
	
	#if defined(SO_DONTLINGER) && !defined(SYSV)    
	{	
		struct	linger	ld; 	
		ld.l_onoff  = 1;	
		ld.l_linger = 1000; 	
		
		if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,	(char *) &ld, sizeof(ld) ) < 0 ) {
			// -- TODO: insert logging mechanism
			close(fd);	    
			return false;
		}  
	}
	#endif     
	
	sa		    = sa_zero;    
	sa.sin_family	    = AF_INET;    
	sa.sin_port	    = htons( port );     
	
	if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )    
	{	
		// -- TODO: insert logging mechanism
		close(fd);
		return false;
	}      
	if ( listen( fd, 3 ) < 0 )    
	{
		// -- TODO: insert logging mechanism
		close(fd);	
		return false;  
	}
	mControl = fd;
	return true;
}

bool Server::ResetPollDescriptors() {
	FD_ZERO( &mRead  );	
	FD_ZERO( &mWrite );
	FD_ZERO( &mError );
	FD_SET( mControl, &mRead );	
	
	maxedConnection	= mControl;
	return true;
}

bool Server::ResetFDControllers() {
  std::list<Socket *>::iterator iter, itern;
  for(iter = System.mSocketList.begin(); iter != System.mSocketList.end(); iter = itern) {
    Socket *s = (*iter);
    itern = ++iter;
    
	   maxedConnection = Maximum<int>( maxedConnection, s->getFileDescriptor() );	    
	   FD_SET( s->getFileDescriptor(), &mRead  );	    
	   FD_SET( s->getFileDescriptor(), &mWrite );	   
	   FD_SET( s->getFileDescriptor(), &mError );    
  }
  return;
}

bool Server::SelectAndPoll() {
  static struct timeval base_time;
	if ( select( maxedConnection+1, &mRead, &mWrite, &mError, &base_time ) < 0 )	
	{	    
	    return false;
	} 	
	
	if ( FD_ISSET( mControl, &mRead ) )	    
	  startNewSocket( mControl );
	return true;
}

void Server::startNewSocket(int ctrl) {
    	Socket *s;   
    	struct sockaddr_in sock;   
    	
    	int desc;    
    	
    	socklen_t size;     
    	size = sizeof(sock);    
    	
    	getsockname( ctrl, (struct sockaddr *) &sock, &size );    
    	
    	if ( ( desc = accept( ctrl, (struct sockaddr *) &sock, &size) ) < 0 )    
    	{	
		// -- TODO: insert log mechanism here
    		return;    
    	} 
    	
    	#if !defined(FNDELAY)
    		#define FNDELAY O_NDELAY
    	#endif    
    	
    	if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )    
    	{
    		// -- TODO: insert log mechanism here
    		return;
    	}     
    	
    	s = new Socket();

	// -- attempt to glean the host address/ip
    	size = sizeof(sock);
    	if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )    
    	{	
    		// -- TODO: insert logging mechanism
    		s->setHost("UNKNOWN");
    		s->setIpAddress("UNKNOWN");
    	} else {
    		struct hostent *from = gethostbyaddr( (char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET );
    		
    		s->setHost(from ? from->h_name : "UNKNOWN");
    		h->setIpAddress(inet_ntoa(sock.sin_addr));
    	} 
    	
    	System.mSocketList.push_back(s);
    	s->sendGreeting();
    	return;
}

bool Server::ReadFromSockets() {
  std::list<Socket *>::iterator iter, itern;
  
  for(iter = System.mSocketList.begin(); iter != System.mSocketList.end(); iter = itern) {
  	Socket *s = (*iter);
  	itern = ++iter;

	if(FD_ISSET(s->getFileDescriptor(), &mRead)) {
		if(!s->Read()) {
			CloseSocket(s, true);
		}
	}
  	
  }
}

bool Server::ProcessInput() {

  	std::list<Socket *>::iterator iter, itern;
  
  	for(iter = System.mSocketList.begin(); iter != System.mSocketList.end(); iter = itern) {
	  	Socket *s = (*iter);
	  	itern = ++iter;

		if(!s->getInputBuffer().empty()) {
			// -- handle processing of the input buffer here.
			// -- here we will attempt to process commands/functions
		}

		s->clearInputBuffer();

  	}	

	return true;
}

bool Server::FlushSockets() {
  std::list<Socket *>::iterator iter, itern;
  
  for(iter = System.mSocketList.begin(); iter != System.mSocketList.end(); iter = itern) {
  	Socket *s = (*iter);
  	itern = ++iter;

	if(!s->Flush()) {
		CloseSocket(s, true);
	}
  }
  
  return true;
}

void Server::CloseSocket(Socket *s, bool forced) {

	// -- attempt to dump any remaining data to the socket
	if(!forced) {
		// -- we don't attempt to trap errors here, if it returns false we just don't
		// -- care as they were disconnecting anyways.
		s->Flush();
	}

	// -- strip the socket from the socket-list
	System.mSocketList.remove(s);
	
	// -- reset the control values for said socket to CLR status.
	FD_CLR(s->getFileDescriptor(), &mRead);
	FD_CLR(s->getFileDescriptor(), &mWrite);
	FD_CLR(s->getFileDescriptor(), &mError);

	// -- handle accounts/players here
	
	// -- close out the descriptor
	close(s->getFileDescriptor());
	
	// -- delete the actual socket-data (kill any lingering data)
	delete s;
}
