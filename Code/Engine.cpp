#include "Engine.hpp"

int ProcessMudFunctions() {

  // -- Handle our server protocols
  if(!System.mServer->ResetPollDescriptors()) { return PROCESS_FAILED; }
  if(!System.mServer->ResetFDControllers()) { return PROCESS_FAILED; }
  if(!System.mServer->SelectAndPoll()) { return PROCESS_FAILED; }
  if(!System.mServer->ReadFromSockets()) { return PROCESS_FAILED; }
  
  // -- now we focus on handling/process Events before input, and before output
  // -- so we can have seperate event stylings.
  System.mEventManager->processEvents(EV_BEFORE_INPUT);
  if(!System.mServer->ProcessInput()) { return PROCESS_FAILED; }
  
  System.mEventManager->processEvents(EV_BEFORE_OUTPUT);
  if(!System.mServer->FlushSockets()) { return PROCESS_FAILED; }
  
  return PROCESS_SUCCESS;
}


int MainLoop() {
  struct timeval last_time;
  int processRetValue;

  signal( SIGPIPE, SIG_IGN );    
    
  gettimeofday( &last_time, NULL );    
  System.mCurrentSystemTime = (time_t) last_time.tv_sec; 
  
  while((processRetValue = ProcessMudFunctions()) != PROCESS_FAILED) {

    // --------------------------------------------------------------------------------------------
    // -- process End of Process functions
    System.mEventManager->processEvents(EV_POST_OUTPUT);
    if(!System.mServer->FlushSockets()) { processRetValue = PROCESS_FAILED; }

    // --------------------------------------------------------------------------------------------
    // -- Process time management and system speed adjustments
    // -- Need to figure out a better way to do this
    struct timeval now_time;	    
    long secDelta;	    
    long usecDelta;
    gettimeofday( &now_time, NULL );	    
    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)			+ 1000000 / 4;	    
    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );	   
    
    while ( usecDelta < 0 )	    
    {		
      usecDelta += 1000000;		
      secDelta  -= 1;	    
    } 	    
    while ( usecDelta >= 1000000 )	    
    {		
      usecDelta -= 1000000;		
      secDelta  += 1;	    
    } 	    
    
    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )	    
    {		
      struct timeval stall_time; 		
      
      stall_time.tv_usec = usecDelta;		
      stall_time.tv_sec  = secDelta;		
      if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )		
      {
        // -- TODO: insert logging mechanism
        kill(getpid(), SIGSEGV);
      }
    }
    gettimeofday( &last_time, NULL );
    System.mCurrentSystemTime = (time_t) last_time.tv_sec;
    // --------------------------------------------------------------------------------------------
  } // -- end while loop
  
  // -- generate a core file
  if(processRetValue == PROCESS_FAILED) {
    kill(getpid(), SIGSEGV);
  }
  
  return processRetValue;
}

// -- EOF
