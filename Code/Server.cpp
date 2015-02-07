#include "Engine.hpp"

class Server {
  public:
    Server();
    ~Server();
  protected:
    int mPort;
    int mControl;
    
    int maxedConnection;
  private:
    time_t mLastSelect;

    fd_set mRead;
    fd_set mWrite;
    fd_set mError;
    
    bool mServerOn;
  public:
    bool InitiatePort(int portNum);
    bool ResetPollDescriptors();
    bool ResetFDControllers();
    bool SelectAndPoll();
    
    void startNewSocket(int ctrl);
}

Server::Server() {

};

Server::~Server() {

}

bool Server::InitiatePort(int portNum) {

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
  
}




