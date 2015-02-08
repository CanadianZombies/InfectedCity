#ifndef _Server_Hpp
#define _Server_Hpp

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

#endif
