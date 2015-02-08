#include "Engine.hpp"

int ProcessMudFunctions() {

  // -- Handle our server protocols
  if(!Server->ResetPollDescriptors()) { return PROCESS_FAILED; }
  if(!Server->ResetFDControllers()) { return PROCESS_FAILED; }
  if(!Server->SelectAndPoll()) { return PROCESS_FAILED; }
  
  EventManager->processEvents(EV_BEFORE_INPUT);
  // -- TODO: add input handling Server call here
  
  EventManager->processEvents(EV_BEFORE_OUTPUT);
  // -- TODO: add output handling Server call here
  
  return PROCESS_SUCCESS;
}


int MainLoop() {
  struct timeval last_time;
  int processRetValue;

  signal( SIGPIPE, SIG_IGN );    
    
  gettimeofday( &last_time, NULL );    
  System.CurrentSystemTime = (time_t) last_time.tv_sec; 
  
  while((processRetValue = ProcessMudFunctions()) != PROCESS_FAILED) {

    // --------------------------------------------------------------------------------------------
    // -- process End of Process functions
    EventManager->processEvents(EV_POST_OUTPUT);
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
    System.CurrentSystemTime = (time_t) last_time.tv_sec;
    // --------------------------------------------------------------------------------------------
  } // -- end while loop
  
  // -- generate a core file
  if(processRetValue == PROCESS_FAILED) {
    kill(getpid(), SIGSEGV);
  }
  
  return processRetValue;
}

// -- EOF
