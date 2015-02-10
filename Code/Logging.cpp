#include "Engine.h"

const char *grab_date_log ( time_t the_time )
{
	struct tm *tm_ptr;

	tm_ptr = localtime ( &the_time );

	return Format ( "%02d-%02d-%02d", tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday );
}
// *Same as grab_time, just without the \n at the end.* //
const char *grab_time_log ( time_t the_ttime )
{
	struct tm *tmt_ptr;

	tmt_ptr = localtime ( &the_ttime );

	return Format ( "%02d:%02d:%02d", tmt_ptr->tm_hour, tmt_ptr->tm_min, tmt_ptr->tm_sec );
}

void _Error ( const std::string &errorStr, const std::string &fileStr, const std::string &funcStr, int lineOfFile )
{
	// -- log with p-error, ensure we have our time and all pertinent associated data.
	perror ( Format ( "(%s)%s:%s:%d : Errno: (%d):(%s), %s", grab_time_log ( time ( 0 ) ), C_STR ( fileStr ), C_STR ( funcStr ), lineOfFile, errno, strerror ( errno ), C_STR ( errorStr ) ) );
	errno = 0; // -- reset our errno value
}

void catchException ( bool willAbort, const std::string &file, const std::string &function, int lineATcatch )
{
#ifndef (_RAISE_ON_THROW_)
	try {
		throw;
	} catch ( std::exception &e ) {
		{ std::cerr << "Thrown exception from " << file << ":" << function << " -> " << e.what() << std::endl; }
	} catch ( int a ) {
		{ std::cerr << "Thrown exception from " << file << ":" << function << " -> " << a << std::endl; }
	} catch ( std::string s ) {
		{ std::cerr << "Thrown exception from " << file << ":" << function << " -> " << s << std::endl; }
	} catch ( const char *ti ) {
		{ std::cerr << "Thrown exception from " << file << ":" << function << " -> " << ti << std::endl; }
	}  catch ( char *t ) {
		{ std::cerr << "Thrown exception from " << file << ":" << function << " -> " << t << std::endl; }
	} catch ( ... ) {
		{ std::cerr << "Thrown exception from " << file << ":" << function << " -> Unknown exception!" << std::endl; }
	}

	// are we crashing out of the mud ?  Unrecoverable error encountered.
	if ( willAbort ) {
		std::cerr << "Aborting The Infected City." << std::endl;
		kill(getpid(), SIGSEGV);
	}
	errno = 0;
	return;
#else
	// -- drop a corefile if we reach this.
	raise(SIGSEGV);
#endif
}

bool file_exists ( const char *name )
{
	struct stat fst;

	if ( IS_NULLSTR ( name ) )
	{ return false; }

	if ( stat ( name, &fst ) != -1 )
	{ return true; }
	else
	{ return false; }

	return false;
}

// ------------------------------------------------------------------------------
// -- Function:     _log_hd
// -- Arguments:    flag identifier, logged string
// -- Example:      log_hd(LOG_BASIC, "Generic Log Entry Here");
// -- Example 2:    log_hd(LOG_BASIC|LOG_ERROR, "Multipurpose flagged log creates the log entry in two logfiles.");
// -- Example 3:    log_hd(LOG_ALL, "This puts a log entry in EVERY log file.");
// -- Date Written: 15 January 2013
// -- Revised Date: 29 July 2014
// ------------------------------------------------------------------------------
// -- Purpose of Function:
// -- The purpose of this function is the keep accurate and well written logs.
// -- Upon calling this function, we attempt to create a new directory within the
// -- log directory for the current date, if that date exists, we attempt to create
// -- a log for the current pid.
// --       Example: /home/mudlogin/InfectEngine/Logs/2014-07-29/54451/logname.log
// -- By sorting like this, logs will not become too large, and can be reviewed by date
// -- and by PID if necessary.  For those who hotreboot often or crash, keeping detailed
// -- track of your log files by PID allows for faster detection/correction of associated
// -- issues.
// ------------------------------------------------------------------------------
// -- Upon appropriately logging detection we attempt to sitrep out the new log
// -- to those whom can view those particular logs via the sitrep system.
// -- If multiple flags are associated with the log entry, then multiple sitreps will
// -- be made.  This can get spammy if staff are listening to too many sitrep channels at
// -- once.
// ------------------------------------------------------------------------------
void _log_hd ( long logFlag, const char *mFile, const char *mFunction, int mLine, const std::string &logStr )
{
	if ( System.mNoLogging ) { return; }

	try {
		static bool called_tzset = false;
		static int debugCounter = 0;
		static int current_yday = 0;
		static int last_yday = 0;
		// called before time(0); to make sure we get the most accurate time
		if ( !called_tzset ) {
			tzset();
			called_tzset = true;
			debugCounter = 0;
			current_yday = 0;
			last_yday = 0;
			unlink ( Format ( "%s%s/%d/%s", LOG_DIR, grab_date_log ( time ( 0 ) ), getpid(), DEBUG_FILE ) ); // -- wipe out old debugging information
		}

		// -- cdt current date time
		time_t cdt = time ( 0 );

		struct tm *tyme = localtime ( &cdt );
		if ( last_yday == 0 ) {
			last_yday = tyme->tm_yday;
		}
		current_yday = tyme->tm_yday;

		// -- associate the time and date appropriately
		const char *the_time = grab_time_log ( cdt );
		const char *the_date = grab_date_log ( cdt );

		// -- if this happens there is a big problem!
		if ( !the_time )
		{ the_time = "{ No Time }"; }

		// -- if this happens, we have BIGGER issues.
		if ( !the_date )
		{ the_date = "Today"; }

		struct Log_types {
			const char *extension;
			const char *broadcast;
			const char *colour;
			long logtype;
			int crFlagSitrep;
		};

		const struct Log_types log_table[] = {
/*    To be added as we develop further!
      {"log",         "Log: Basic",    "\a[F542]",    LOG_BASIC,          CF_SEE_LOGS},
			{"error",       "Log: Error",    "\a[F500]",    LOG_ERROR,          CF_SEE_BUGS},
			{"security",    "Log: Security", "\a[F213]",    LOG_SECURITY,       CF_SEE_SECURITY},
			{"commands",    "Log: Command",  "\a[F455]",    LOG_COMMAND,        CF_SEE_COMMANDS},
			{"debug",       "Log: Debug",    "\a[F343]",    LOG_DEBUG,          CF_SEE_DEBUG},
			{"script",	"Log: Script",	 "\a[F535]",    LOG_SCRIPT,         CF_SEE_SCRIPT},
			{"suicide",	"Log: Suicide",  "\a[F500]",    LOG_SUICIDE,          CF_SEE_ABORT},
*/
			{NULL, NULL, NULL, -1, -1 },
		};

		// -- No need to go any further if we are just outputting to the stdout
		// -- otherwise we want our proper logs handled and we continue.
		if ( IS_SET ( logFlag, LOG_DEBUG ) )
		{ std::cout << "\tLog << " << the_time << " << (" << debugCounter << ") " << logStr << std::endl; }
		else
		{ std::cout << "Log << " << the_time << " << " << logStr << std::endl; }

		// -- establish the current-date for the logs, write the log to a pid within the directory.
		struct stat st;
		if ( stat ( Format ( "%s%s", LOG_DIR, the_date ), &st ) == 0 )
		{ }
		else {
			// -- unable to make the directory for the logs?
			if ( mkdir ( Format ( "%s%s", LOG_DIR, the_date ), 0777 ) ) {
				kill(getpid(), SIGSEGV);
			}
		}

		// -- create the sub-directory with the pid for the log
		if ( stat ( Format ( "%s%s/%d", LOG_DIR, the_date, getpid() ), &st ) == 0 )
		{ }
		else {
			// -- unable to make the directory for the logs by pid?
			if ( mkdir ( Format ( "%s%s/%d", LOG_DIR, the_date, getpid() ), 0777 ) ) {
				kill(getpid(), SIGSEGV);
			}
		}

		// -- handle our logging mechanism's
		for ( int logX = 0; log_table[logX].extension != NULL; logX++ ) {
			// -- not the same day anymore, the first log entry will have to be the update from the previous day
			// -- simply because we like having bloated logs and keeping ourselves informed of such things.
			if ( last_yday != current_yday ) {
				FILE *fp = NULL;

				if ( log_table[logX].logtype == LOG_DEBUG ) {
					debugCounter = 0;
					if ( ( fp = fopen ( Format ( "%s%s/%d/%s", LOG_DIR, the_date, getpid(), DEBUG_FILE ), "a" ) ) != NULL ) {
						fprintf ( fp, "\t%s : %s\n", the_time, Format ( "Continuing new logfile from previous yday: %d", last_yday ) );
						fprintf ( fp, "\tEngine Version: %s\n", getVersion() );
					}
				} else {
					// -- not a debug message? Log it to its appropriate log.
					if ( ( fp = fopen ( Format ( "%s%s/%d/%s.log", LOG_DIR, the_date, getpid(), log_table[logX].extension ), "a" ) ) != NULL ) {
						fprintf ( fp, "\t%s : %s\n", the_time, Format ( "Continuing new logfile from previous yday: %d", last_yday ) );
						fprintf ( fp, "\tEngine Version: %s\n", getVersion() );
					}
				}
				fflush ( fp );
				fclose ( fp );
			}

			// -- now we check to see if we are going to be logging to th extreme or not!
			if ( ( IS_SET ( logFlag, log_table[logX].logtype ) || IS_SET ( logFlag, LOG_ALL ) ) ) {
				FILE *fp = NULL;
				std::string logOutStr = logStr;

				// -- no matter what only debug messages end up in the runtime.debug file.
				if ( log_table[logX].logtype == LOG_DEBUG ) {
					int maxDebug = 500;
					if ( System.mVerboseLogging )
					{ maxDebug = 1500; }
					// -- We only keep the last maxDebug debug messages before we wipe the file and
					// -- create a new one.  Purpose behind this is to eliminate massive debug logs
					// -- and by all account, no debug file should be empty.  This is an excellent
					// -- way to log things leading up to a crash, or simply a great way to log
					// -- the current stack leading up to a shutdown/crash/etc.
					if ( ++debugCounter > maxDebug ) {
						unlink ( Format ( "%s%s/%d/%s", LOG_DIR, the_date, getpid(), DEBUG_FILE ) );
						debugCounter = 1;
					}
					if ( ( fp = fopen ( Format ( "%s%s/%d/%s", LOG_DIR, the_date, getpid(), DEBUG_FILE ), "a" ) ) != NULL ) {

						// -- always ensure our Engine Version is associated with all new files
						if ( debugCounter == 1 ) {
							fprintf ( fp, "Engine Version: %s\n", getVersion() );
						}
						fprintf ( fp, "\t(%d)%s : %s : %s : %d : %s\n", debugCounter, the_time, mFile, mFunction, mLine, C_STR ( logStr ) );
					}
					// -- sitrep our log out to those who can listen to it.
					// -- sitrep ( log_table[logX].crFlagSitrep, Format ( "\a[F350](\a[F541]%s\a[F350]) (%d) %s%s : %s\an", log_table[logX].broadcast, debugCounter, log_table[logX].colour, the_time, C_STR ( logOutStr ) ) );

				} else {
					bool exists = true;
					if ( !file_exists ( Format ( "%s%s/%d/%s.log", LOG_DIR, the_date, getpid(), log_table[logX].extension ) ) ) {
						exists = false;
					}
					// -- not a debug message? Log it to its appropriate log.
					if ( ( fp = fopen ( Format ( "%s%s/%d/%s.log", LOG_DIR, the_date, getpid(), log_table[logX].extension ), "a" ) ) != NULL ) {
						// -- if we didn't exist, we put the engine version at the top of the log file
						if ( !exists ) {
							fprintf ( fp, "Engine Version: %s\n", getVersion() );
						}
						if ( System.mVerboseLogging )
						{ fprintf ( fp, "\t%s : %s\n", the_time, C_STR ( logStr ) ); }
						else
						{ fprintf ( fp, "\t%s : %s : %s : %d : %s\n", the_time, mFile, mFunction, mLine, C_STR ( logStr ) ); }
					}

					// -- sitrep our log out to those who can listen to it.
					// -- sitrep ( log_table[logX].crFlagSitrep, Format ( "\a[F350](\a[F541]%s\a[F350]) %s%s : %s\an", log_table[logX].broadcast, log_table[logX].colour, the_time, C_STR ( logOutStr ) ) );
				}

				fflush ( fp );             // -- flush the file pointer
				fclose ( fp );             // -- close the file pointer
			}
		}
		// -- finally change the last_yday to the current yday once we have processed everything
		// -- and our conditions are met appropriately.
		if ( last_yday != current_yday ) {
			last_yday = current_yday;   // -- make us the current yday (julian date)
		}
	} catch ( ... ) {
		// -- at least log where the log was called from
		std::cout << "Unknown error encountered during log_hd processing (" << mFile << " : " << mFunction << " : " << mLine << ")" << std::endl;
	}

	// -- Force a suicide after the logging is completed!
	if ( IS_SET ( logFlag, LOG_SUICIDE ) ) {
		kill(getpid(), SIGSEGV);
	}

	return;
}
