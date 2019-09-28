#if !defined(__IEVENTENGINE_HPP__)
#define __IEVENTENGINE_HPP__

#include <boost/shared_ptr.hpp>
#include "Common.hpp"
#include "SocketTracker.hpp"

/**
 * The interface class to detect the events for the curl
 */
class IEventEngine {
public:
    /**
     * Detect read event for the curl
     * @param st, store all socket related information and will also need to be pass to
     * the aysnc operation handler. 
     * @param handler, the handler will be called when event detected
     * @return true if the detection is issued 
     */
    virtual bool async_detect_read_event(SocketTrackerPtr st, AsyncEventHandler handler) = 0;

    /**
     * Detect write event for the curl
     * @param st, store all socket related information and will also need to be pass to
     * the aysnc operation handler. 
     * @param handler, the handler will be called when event detected
     * @return true if the detection is issued 
     */
    virtual bool async_detect_write_event(SocketTrackerPtr st, AsyncEventHandler handler) = 0;


    /**
     * The timeout event required by CURLMOPT_TIMERFUNCTION. 
     * @param timeout_ms, timeout in number of ms
     * @param handler, the handler will be called when timeout reached
     * @return true if the detection is issued
     */
    virtual bool async_detect_timeout(long timeout_ms, TimeoutHandler handler) = 0;

    /**
     * Cancel the timeout timer 
     */
    virtual void cancel_timer() = 0;

    /**
     * run the handler in the context of the event engine
     */
    virtual void async_run(AsyncRunHandler handler) = 0;

    /**
     * Create the socket and store it into the SocketTrackerPtr
     * @return the new SocketTracker is created
     */
    virtual SocketTrackerPtr open_socket()=0;

    /**
     * Close the socket managed by low level event engine
     * @param st, the socket to close
     */
    virtual void close_socket(SocketTrackerPtr st)=0;
    virtual ~IEventEngine () {} 
};

typedef boost::shared_ptr<IEventEngine> EventEnginePtr;

#endif
