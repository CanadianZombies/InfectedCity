#include "Engine.hpp"

int main ( int argc, char **argv )
{
  int mainLoopRetValue = PROCESS_SUCCESS; // -- default value
  
  // -- Process arguments.

  // -- dump our pid data
	{
		FILE *fp = fopen ( "InfectedCity.pid", "w" );

		if ( fp ) {
			fprintf ( fp, "%d\n", getpid() );

			fflush ( fp );
			fclose ( fp );
		} else {
			raise(SIGSEGV);
		}
	}
  
  // -- bootup databases

  // -- execute the MainLoop
  mainLoopRetValue = MainLoop();
  
  if(mainLoopRetValue != PROCESS_SUCCESS) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
