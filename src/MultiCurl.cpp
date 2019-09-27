#include <boost/bind.hpp>
#include "Common.hpp"
#include "MultiCurl.hpp"

const int MAX_HOST_CONNECTIONS=5;

MultiCurl::MultiCurl(EventEnginePtr events):
    m_event_engine(events)
    , m_running_handles(0)
{
    m_multi=curl_multi_init();
    if (!m_multi){
       throw std::runtime_error("curl_multi_init() failed");
    }

    curl_multi_setopt(m_multi, CURLMOPT_SOCKETFUNCTION, &MultiCurl::socket_function);
    curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA, this);

    curl_multi_setopt(m_multi, CURLMOPT_TIMERFUNCTION, &MultiCurl::timer_function);
    curl_multi_setopt(m_multi, CURLMOPT_TIMERDATA, this);

    curl_multi_setopt(m_multi, CURLMOPT_MAX_HOST_CONNECTIONS, MAX_HOST_CONNECTIONS);
}

MultiCurl::~MultiCurl()
{
   try {
       curl_multi_cleanup(m_multi);
   } catch (std::exception& ex) {
       LOG_WARN("curl_multi_cleanup() error " << ex.what());
   } catch (...) {
       LOG_WARN("curl_multi_cleanup() error ");
   }
}

void MultiCurl::async_download(DownloadJobPtr& downloadjob)
{
   m_event_engine->async_run(boost::bind(&MultiCurl::async_download_impl, this, downloadjob));
}

void MultiCurl::async_download_impl(DownloadJobPtr& downloadjob)
{
    CurlTaskPtr ec(new CurlTask(downloadjob, m_event_engine));
    m_downloads.insert(std::pair<CURL*, CurlTaskPtr>(ec->get_easy_handler(),ec));
    curl_multi_add_handle(m_multi, ec->get_easy_handler());
}

int MultiCurl::timer_function(CURLM* native_multi, long timeout_ms, void* userp)
{
    MultiCurl* self = static_cast<MultiCurl*>(userp);
    if (timeout_ms > 0){
       self->m_event_engine->async_detect_timeout(timeout_ms, boost::bind(&MultiCurl::handle_timeout, self, boost::asio::placeholders::error));
    }else{ //Timeout
       self->m_event_engine->cancel_timer();
       self->handle_timeout(boost::system::error_code());
   }

   return 0;
} 

void MultiCurl::handle_timeout(const boost::system::error_code& err)
{
   if (err)
      return;

   socket_action(CURL_SOCKET_TIMEOUT, 0);
   if (m_running_handles <= 0) //no running curl and no need timeout handler
      m_event_engine->cancel_timer();
   check_multi_done();
}

void MultiCurl::socket_action(curl_socket_t s, int event_bitmask)
{
   curl_multi_socket_action(m_multi, s, event_bitmask, &m_running_handles);
   LOG_TRACE("detected "<<m_running_handles<<" running download job");
   if (!m_running_handles)
       m_event_engine->cancel_timer();
}

   
void MultiCurl::check_multi_done()
{
   CURLMsg *m;
   do {
       int msgq = 0;
       m = curl_multi_info_read(m_multi, &msgq);
       if(m && (m->msg == CURLMSG_DONE)) {
            CURL *e = m->easy_handle;
            CURLcode result = m->data.result;
            on_download_done(e, result);
       }
   }while (m);
}

void MultiCurl::on_download_done(CURL *e, CURLcode result)
{
   DownloadTaskMap::iterator ite=m_downloads.find(e);
   if (ite != m_downloads.end()) {
       CurlTaskPtr ecurl=ite->second;
       m_downloads.erase(ite);
       curl_multi_remove_handle(m_multi,e);
       ecurl->on_job_complete(result);
   }else {
       LOG_DEBUG("detected easy curl not managed by multicurl");
   }
}   
       
int MultiCurl::socket_function(CURL* native_easy, curl_socket_t s, int what, void* userp, void* socketp)
{
   LOG_TRACE("curl_socket_callback in "<<s<<" what:"<<what << " SocketTracker:"<<socketp <<" and multi:"<<userp);
   MultiCurl* self = static_cast<MultiCurl*>(userp);
   
   DownloadTaskMap::iterator ite=self->m_downloads.find(native_easy);     
   if (ite == self->m_downloads.end()) {
        LOG_DEBUG("detected easy curl not managed by multicurl");
        return 0;
   }

   CurlTaskPtr ecurl=ite->second;
   //close it as CURL will not use it anymore.
   if (what == CURL_POLL_REMOVE ){
      self->m_event_engine->close_socket(ecurl->get_socket_tracker());
      return 0;
   }

   SocketTrackerPtr st=ecurl->get_socket_tracker();
   st->read_ready = !!(what & CURL_POLL_IN);
   st->write_ready = !!(what & CURL_POLL_OUT);
   if (what == CURL_POLL_IN)
       self->m_event_engine->async_detect_event(st,boost::bind(&MultiCurl::on_read_event, self, boost::asio::placeholders::error, st)); 

   if (what == CURL_POLL_OUT)
       self->m_event_engine->async_detect_event(st,boost::bind(&MultiCurl::on_write_event, self, boost::asio::placeholders::error, st));

   return 0;
}

void MultiCurl::on_read_event(const boost::system::error_code& err, SocketTrackerPtr st)
{
     LOG_TRACE("on_read_event:pr,pw,rr,wr "<<st->pending_read<<" " <<st->pending_write<<" "<<st->read_ready<<" "<<st->write_ready);
     st->pending_read = false;
     if (!st->read_ready)
         return;

     if (!err){
        //tell curl is ready to read
        socket_action(st->socket->native_handle(), CURL_CSELECT_IN);
        if (st->read_ready)
            m_event_engine->async_detect_event(st,boost::bind(&MultiCurl::on_read_event, this, boost::asio::placeholders::error, st));
     } else if (err != boost::asio::error::operation_aborted){
        socket_action(st->socket->native_handle(), CURL_CSELECT_ERR);
        check_multi_done();
    }
}
     
void MultiCurl::on_write_event(const boost::system::error_code& err, SocketTrackerPtr st)
{
     LOG_TRACE("on_write_event:pr,pw,rr,wr "<<st->pending_read<<" " <<st->pending_write<<" "<<st->read_ready<<" "<<st->write_ready);
     st->pending_write = false;
     if (!st->write_ready)
        return;

     if (!err) {
        socket_action(st->socket->native_handle(), CURL_CSELECT_OUT);
        if (st->write_ready)
             m_event_engine->async_detect_event(st,boost::bind(&MultiCurl::on_write_event, this, boost::asio::placeholders::error, st));
     } else if (err != boost::asio::error::operation_aborted){
        socket_action(st->socket->native_handle(), CURL_CSELECT_ERR);
        check_multi_done();
    }
}
