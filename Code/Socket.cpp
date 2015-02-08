#include "Engine.hpp"

Socket::Socket() { }
Socket::~Socket() { }

bool Socket::Read() {
    char temp[READSIZ + 2];  
    int size;   
    
    while (true) 
    {
      if ((size = read(mFileDescriptor, temp, READSIZ)) > 0) 
      {
          temp[size] = '\0';      
          mInputBuffer += (std::string) temp;       
          
          if (temp[size - 1] == '\n' || temp[size-1] == '\r')        
          break;    
      }    
      else if (size == 0)
        return false;
      else if (errno == EAGAIN || size == READSIZ)
        break;
      else
        return false;  
    } 
    return true;
}

void Socket::Write(const std::string &str) {
  mOutputBuffer += str;
}

void Socket::Write(const char *str) {
  mOutputBuffer += str;
}

void Socket::Flush() {
  int b, w;   
  
  // empty the entire buffer  
  while (outBuffer.length() > 0) 
  {
    b = (mOutputBuffer.length() < READSIZ) ? mOutputBuffer.length() : READSIZ;
    
    // any write failures ?    
    if ((w = write(mFileDescriptor, mOutputBuffer.c_str(), b)) == -1)     
      return false;     
    
    // move the buffer down    
    mOutputBuffer.erase(0, w);  
  }   
  return true;
}

void Socket::ClearInputBuffer() 
{  
  mInputBuffer.erase(0, mInputBuffer.length());
}
