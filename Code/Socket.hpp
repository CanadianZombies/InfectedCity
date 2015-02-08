#ifndef _Socket_Hpp
#define _Socket_Hpp

class Socket {
  public:
    Socket();
    ~Socket();
  private:
    std::string mInputBuffer;
    std::string mOutputBuffer;
    
    int mFileDescriptor;
    int mState;
  public:
    bool Read();
    void Write(const std::string &str);
    void Write(const char *str);
    void Flush();
    void ClearInputBuffer();
    
    int GetFileDescriptor();
};

#endif
