#include <new>
#include <typeinfo>
#include <string>
#include <list>
#include <map>

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <ctime>


#include "Server.hpp"
#include "Socket.hpp"
#include "Prototypes.hpp"

struct SystemData {
  std::list<Socket *>mSocketList;
  
  Server *mServer;
  EventHandler *mEventManager;
  
  time_t mCurrentSystemTime;
};
