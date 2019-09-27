#if !defined(_ASIO_EVENT_ENGINE_HPP)
#define _ASIO_EVENT_ENGINE_HPP

#include <boost/asio.hpp>
#include "IEventEngine.hpp"

/**
 * The Event layer managed by the Boost Asio: 
 * The CURLMOPT_SOCKETFUNCTION callback will inform the Asio which event the curl is waitting for:
 *   If it is CURL_POLL_IN, the Asio will async_read_some(null_buffers()...)
 *   If it is CURL_POLL_OUT, the Asio will async_write_some(null_buffers()...)
 * When the async ops returns, the curl_multi_socket_action will be called to set event bit
 * and then, multi curl can send request or read response using the socket. 
 */     
class AsioEventEngine : public IEventEngine
{
public:
    /**
     * Constructor
     * @param io_service, the io service to manage the events
     */
    AsioEventEngine(boost::asio::io_service& io_service);
               
    /**
     * Request to detect an event, which may call async_read or async_write based on the 
     * SocketTrackerPtr.
     * @param st, store all socket related information and will also need to be pass to
     * the aysnc operation handler. 
     * @param handler, the handler will be called when event detected
     * @return true if the event is issued 
     */
    virtual bool  async_detect_event(SocketTrackerPtr st, AsyncEventHandler handler);

    /**
     * The timeout event required by CURLMOPT_TIMERFUNCTION. The boost::asio::deadline_timer
     * is used to geneate such event
     * @param timeout_ms, timeout in number of ms
     * @param handler, the handler will be called when timeout reached
     * @return true if the event is issued
     */ 
    virtual bool  async_detect_timeout(long timeout_ms, TimeoutHandler handler); 

    /**
     * Cancel the timeout timer 
     */
    virtual void  cancel_timer();


    /**
     * run the handler in the thread managed by the event engine to make it thread safe
     */
    virtual void async_run(AsyncRunHandler handler);

    /**
     * Create the socket and store it into the SocketTrackerPtr
     * @return the new SocketTracker is created
     */
    virtual SocketTrackerPtr open_socket();

    /**
     * Close the socket managed by low level event engine
     * @param st, the socket to close
     */
    virtual void close_socket(SocketTrackerPtr st);

private:
    boost::asio::io_service& m_io_service; //< the io service required by the boost
    boost::asio::deadline_timer m_timer; //< the timeout timer
};

#endif
