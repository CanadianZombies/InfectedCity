#ifndef _EventHandler_Hpp
#define _EventHandler_Hpp

// -- this will speed up the process ever so slightly
// -- for the overall development of our event structures.
#define DEFINE_EVENT(EventClass) \
	public: \
	EventClass() { }; \
	~EventClass() { }; \
	void Execute(void) 

typedef enum { EV_BEFORE_INPUT, EV_BEFORE_OUTPUT, EV_POST_OUTPUT } event_types;

class Event
{
	public:
		Event();
		virtual ~Event();

	protected:
		time_t  mInitTime;  // the initiation time for the event
		double  mSecondsToExecute; // suspect time of ending
		bool    mWillRestart;  // will it restart when it is done?
		int 	mEvType;
	public:
		virtual void Execute ( void ) = 0;
		void setType ( int t ) { mEvType = t; }
		int getType ( void ) { return mEvType; }

		void setInitTime ( time_t t )
		{
			mInitTime = t;
		}
		void setSeconds ( double t )
		{
			mSecondsToExecute = t;
		}

		double getSeconds ( void )
		{
			return mSecondsToExecute;
		}

		time_t getInitTime ( void )
		{
			return mInitTime;
		}

		bool hasElapsed ( void )   // has the event elapsed its seconds requirements?
		{
			double elapsed = difftime ( time ( NULL ), mInitTime );
			if ( elapsed >= mSecondsToExecute )
			{ return true; }
			return false;
		}
		bool isRestart ( void )
		{
			return mWillRestart;
		}
		void setRestart ( bool t )
		{
			mWillRestart = t;
		}
};

class EventHandler
{
	public:
		EventHandler() { }
		~EventHandler()
		{
			destroyEvents();
		}
	private:
		EventHandler ( const EventHandler& );
		EventHandler&operator= ( const EventHandler& );

		std::list<Event*>mEventList;
	public:
		std::list<Event *>::iterator getFirst() { return mEventList.begin(); }
		std::list<Event *>::iterator getLast() { return mEventList.end(); }
		void BootupEvents();
		int processEvents ( int ev_type );
		void destroyEvents ( void );
		void destroyEvent ( Event *ev );
		Event *addEvent ( Event *ev, bool repeat, double seconds ); // return the event so we can store certain events easily
		Event *addEvent ( Event *ev, bool repeat, double seconds, int ev_type );
};

#endif
