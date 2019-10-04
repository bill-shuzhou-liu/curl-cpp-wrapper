#include "Common.hpp"
#include "AsioEventEngine.hpp"

AsioEventEngine::AsioEventEngine(boost::asio::io_service& io_service):
       m_io_service(io_service)
       ,m_timer(io_service)
{
}

bool AsioEventEngine::async_detect_read_event(SocketTrackerPtr st, AsyncEventHandler handler)
{
        LOG_TRACE("check read event");
        st->socket->async_read_some(boost::asio::null_buffers(), handler);
        st->pending_read = true; //only one async_read allowd at the same time
        return true;
}

bool AsioEventEngine::async_detect_write_event(SocketTrackerPtr st, AsyncEventHandler handler)
{

        LOG_TRACE("check write event");
        st->socket->async_write_some(boost::asio::null_buffers(), handler);
        st->pending_write = true; //only one async_write allowd at the same time
        return true;

}

bool AsioEventEngine::async_detect_timeout(long timeout_ms, TimeoutHandler handler)
{
     if (timeout_ms < 0){
        return false;
     }

     m_timer.expires_from_now(boost::posix_time::millisec(timeout_ms));
     m_timer.async_wait(handler);
     return true;
}

void AsioEventEngine::async_run(AsyncRunHandler handler)
{
     m_io_service.post(handler);
}

void AsioEventEngine::cancel_timer()
{
    m_timer.cancel();
}

TCPSocketPtr AsioEventEngine::open_socket()
{
   LOG_TRACE("Open Socket");
   TCPSocketPtr tp(new boost::asio::ip::tcp::socket(m_io_service));
   boost::system::error_code ec;
   tp->open(boost::asio::ip::tcp::v4(), ec);
   if (ec){
       LOG_WARN("fail to open the connection:"<<ec.message());
       return TCPSocketPtr();
   }

   return tp;
}

void AsioEventEngine::close_socket(SocketTrackerPtr st)
{
   if (st->socket != NULL){
        st->socket->close();
   }
}
