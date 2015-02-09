#include "Engine.hpp"

void EventHandler::BootupEvents()
{
	// -- Event manager is now being tested with a repeating event every 15 minutes (60*15)
	// -- if all works out, this will announce the time every 15 minutes to all connected sockets.
	return;
}

int EventHandler::updateEvents ( void )
{
	std::list<Event *>::iterator iter, iter_next;
	try {
		// -- Execute our C++ Events
		for ( iter = mEventList.begin(); iter != mEventList.end(); iter = iter_next ) {
			Event *ev = ( *iter );
			iter_next = ++iter;
			// no fancy --/++ counters like in most systems; here we use
			// just a simple raw check to see how many seconds have passed
			// min of 0 seconds, max of, well, allot.  Either way, we can be
			// quite creative now with our events.
			if ( ev->hasElapsed() ) {

				// -- February 25, 2014
				// -- Written by: David Simmerson
				// -- if we crash during an event, the core-file will have
				// -- access to the System.pCrash.last_event crash pointer
				// -- so even if we do not receive the most accurate
				// -- core-file, we can at-least find out if we died in an
				// -- event.
				const char* mangled_name = typeid ( *ev ).name();

				int status;
				char* demangled_name = __cxxabiv1::__cxa_demangle ( mangled_name, 0, 0, &status );

				log_hd ( LOG_DEBUG, Format ( "Running Event: (%p) - %s", ev, demangled_name ) );
				if ( demangled_name ) {
					free ( demangled_name );
					demangled_name = NULL;
				}

				// -- add the stack here instead of in all the individual Execute calls..
				// -- of-course, this is a lazy way of doing it and it should probably be
				// -- updated at a later time to fit more appropriately.
				ev->Execute();  // run the event!
				// is the event scheduled to restart?
				if ( ev->isRestart() ) {
					ev->setInitTime ( time ( NULL ) );
					continue;
				}
				mEventList.remove ( ev );
				delete ev;
				ev = NULL;
			}
		}
	} catch ( ... ) {
		CATCH ( false );
	}

	// -- chain so that we don't break the lldb calls.
	tail_chain();

	// -- a typical action to ensure the debugger doesn't spaz out like a little hooker.
	return 1;
}

// destroy all events within the system!
void EventHandler::destroyEvents ( void )
{
	try {
		std::list<Event *>::iterator iter, iter_next;
		int destroyCounter = 0;

		for ( iter = mEventList.begin(); iter != mEventList.end(); iter = iter_next ) {
			Event *ev = ( *iter );
			iter_next = ++iter;
			mEventList.remove ( ev );

			const char* mangled_name = typeid ( *ev ).name();

			int status;
			char* demangled_name = __cxxabiv1::__cxa_demangle ( mangled_name, 0, 0, &status );

			log_hd ( LOG_DEBUG, Format ( "\tEvent: %s(%p) destroyed.", demangled_name, ev ) );

			if ( demangled_name ) { ; }
			{
				free ( demangled_name );
				demangled_name = NULL;
			}
			destroyCounter++;

			delete ev;
			ev = NULL;
		}

		log_hd ( LOG_DEBUG, Format ( "\t%d Events were destroyed.", destroyCounter ) );
		mEventList.clear(); // finally clear the event list
	} catch ( ... ) {
		CATCH ( false );
	}
	return;
}

void EventHandler::destroyEvent ( Event *ev )
{
	std::list<Event *>::iterator iter, iter_next;
	for ( iter = mEventList.begin(); iter != mEventList.end(); iter = iter_next ) {
		Event *e = ( *iter );
		iter_next = ++iter;
		if ( e == ev ) {
			log_hd ( LOG_DEBUG, Format ( "Destroying Event: %p", ev ) );
			mEventList.remove ( ev );
			delete ev;
			ev = NULL;
		}
	}
	return;
}

// report our Events to a selected character
// this will allow us to see all the important data!
void EventHandler::reportEvents ( Creature *c )
{
	std::list<Event *>::iterator iter, iter_next;
	int cnt = 0;
	double totSeconds = 0;

	if ( !c ) {
		log_hd ( LOG_ERROR, Format ( "Called %s without any Creature data.", __FUNCTION__ ) );
		return;
	}

	if ( !c->desc ) {
		log_hd ( LOG_ERROR, Format ( "Called %s with a Creature with no socket data", __FUNCTION__ ) );
		return;
	}

	writeBuffer ( Format ( "\n\r\a[F355]Events of %s\an\n\r", "The Infected City" ), c );
	for ( iter = mEventList.begin(); iter != mEventList.end(); iter = iter_next ) {
		Event *ev = ( *iter );
		iter_next = ++iter;
		double elapsed = difftime ( time ( NULL ), ev->getInitTime() );
		double ExecutesIn = ev->getSeconds() - elapsed;
		totSeconds += ExecutesIn;

		const char* mangled_name = typeid ( *ev ).name();

		int status;
		char* demangled_name = __cxxabiv1::__cxa_demangle ( mangled_name, 0, 0, &status );

		if ( ev->getType() == EV_LUA ) {
			LuaEvent *le = ( LuaEvent * ) ev;
			writeBuffer ( Format ( "\a[F504]Ev: \a[F152]%25s \a[F504] (\a[F531]%p\a[F504]) \a[F504]Repeats: \a[F152]%s - \a[F504]Start: \a[F152]%s - \a[F504]Executes in \a[F152]%f \a[F504]seconds \ay(\a[F500]%s\ay)\a[F504].\an\n\r",
								   status ? mangled_name : demangled_name, ev,
								   ev->isRestart() ? "Yes" : "No",
								   getDateTime ( ev->getInitTime() ), ExecutesIn, C_STR ( le->getScriptName() ) ), c );
		} else {
			writeBuffer ( Format ( "\a[F504]Ev: \a[F152]%25s \a[F504] (\a[F531]%p\a[F504]) \a[F504]Repeats: \a[F152]%s - \a[F504]Start: \a[F152]%s - \a[F504]Executes in \a[F152]%f \a[F504]seconds.\an\n\r",
								   status ? mangled_name : demangled_name, ev,
								   ev->isRestart() ? "Yes" : "No",
								   getDateTime ( ev->getInitTime() ), ExecutesIn ), c );
		}
		cnt++;

		if ( demangled_name ) { ; }
		{
			free ( demangled_name );
			demangled_name = NULL;
		}
	}
	writeBuffer ( Format ( "There are %d events in queue with a total of %f seconds.\an\n\r", cnt, totSeconds ), c );
	return;
}

Event *EventHandler::addEvent ( Event *ev, bool repeat, double seconds )
{
	std::list<Event *>::iterator iter, iter_next;

	for ( iter = mEventList.begin(); iter != mEventList.end(); iter = iter_next ) {
		Event *e = ( *iter );
		iter_next = ++iter;
		// already in the event list!
		if ( e == ev ) {
			return ev;
		}
	}
	// should we reach this point in time.
	mEventList.push_back ( ev );
	ev->setRestart ( repeat );
	ev->setSeconds ( seconds );
	ev->setInitTime ( time ( NULL ) );
	ev->setType ( EV_CPP );

	// -- ensure we log everything!
	log_hd ( LOG_DEBUG, Format ( "New Event: %p / Repeats: %s / Seconds till Execution: %ld", ev, repeat ? "yes" : "no", seconds ) );
	return ev;
}

Event::Event() : mInitTime ( 0 ), mSecondsToExecute ( 0 ), mWillRestart ( 0 )
{

}
Event::~Event() { }
