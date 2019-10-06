#if !defined(__SOCKET_TRACER_HPP)
#define __SOCKET_TRACER_HPP

#include <boost/shared_ptr.hpp>
#include "Common.hpp"


/*The SocketTracker used to track the events from multi curl*/
struct SocketTracker
{
        SocketTracker(TCPSocketPtr s);

        //Below two parameters is used to prevent multiple asyc_read and async_write at the same time
        bool pending_read;      //asio is waitting for asyc_read result
        bool pending_write;     //asio is waitting for asyc_write result

        //Below two parameters track the status returned from curl, 
        //so that we will not issue the async_read/write if the status changed.
        bool read_ready;    //the curl marks the socket is ready to read
        bool write_ready;    //the curl marks the socket is ready to write

        TCPSocketPtr socket; //the asio socket
};

inline SocketTracker::SocketTracker(TCPSocketPtr s):
       pending_read(false),
       pending_write(false),
       read_ready(false),
       write_ready(false),
       socket(s)
{}

typedef boost::shared_ptr<SocketTracker> SocketTrackerPtr;

#endif
