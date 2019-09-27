#include "CurlTask.hpp"

CurlTask::CurlTask(DownloadJobPtr& downloadjob,
                EventEnginePtr events):
      m_download_job(downloadjob) 
      , m_event_engine(events)
{
      m_easy_handler=curl_easy_init();
      if (!m_easy_handler){
            throw std::runtime_error("curl_easy_init() failed");
      }

      //pass this as the private data
      curl_easy_setopt(m_easy_handler, CURLOPT_PRIVATE, this);

      //handle http redirect, max follow 5
      curl_easy_setopt(m_easy_handler, CURLOPT_FOLLOWLOCATION, 1);
      curl_easy_setopt(m_easy_handler,CURLOPT_MAXREDIRS, 5);

      //No download should take more than  1 minutes
      curl_easy_setopt(m_easy_handler,CURLOPT_TIMEOUT, 60);

      //Not reuse the conneciton open
      curl_easy_setopt(m_easy_handler, CURLOPT_FORBID_REUSE, 1);

      //Callback to open socket
      curl_easy_setopt(m_easy_handler, CURLOPT_OPENSOCKETFUNCTION, &CurlTask::open_socket);
      curl_easy_setopt(m_easy_handler, CURLOPT_OPENSOCKETDATA, this);

      //callback to close socket
      curl_easy_setopt(m_easy_handler, CURLOPT_CLOSESOCKETFUNCTION, &CurlTask::close_socket);
      curl_easy_setopt(m_easy_handler, CURLOPT_CLOSESOCKETDATA, this);

      //callback to write to the download job
      curl_easy_setopt(m_easy_handler, CURLOPT_WRITEFUNCTION, &CurlTask::write_function);
      curl_easy_setopt(m_easy_handler, CURLOPT_WRITEDATA, this);

      //set the url to download
      curl_easy_setopt(m_easy_handler, CURLOPT_URL, downloadjob->get_url().c_str());

#ifdef __DEBUG__
      curl_easy_setopt(m_easy_handler, CURLOPT_VERBOSE, 1L);
#endif

}

CurlTask::~CurlTask()
{
     if (m_easy_handler)
     {
        curl_easy_setopt(m_easy_handler, CURLOPT_PRIVATE, 0);
        curl_easy_setopt(m_easy_handler, CURLOPT_OPENSOCKETDATA, 0);
        curl_easy_cleanup(m_easy_handler);
        m_easy_handler = 0;
     }
}


CURL* CurlTask::get_easy_handler() const 
{
      return m_easy_handler;
}

void CurlTask::on_job_complete(CURLcode result)
{
     m_download_job->handle_download_completed(m_easy_handler, result);
}

SocketTrackerPtr CurlTask::get_socket_tracker() const
{
    return m_tracker;
}

curl_socket_t CurlTask::open_socket(void* clientp, curlsocktype purpose, struct curl_sockaddr* address)
{
     CurlTask* self = static_cast<CurlTask*>(clientp);
     if (!self)
          return CURL_SOCKET_BAD;

     switch (purpose){
          case CURLSOCKTYPE_IPCXN:
               switch (address->socktype){
                     case SOCK_STREAM: { //TODO: only handle TCP so far
                          SocketTrackerPtr socket=self->m_event_engine->open_socket();
                          if (socket == NULL) 
                              return CURL_SOCKET_BAD;
                          self->m_tracker=socket;
                          return socket->socket->native_handle();
                     }
                     default:
                          return CURL_SOCKET_BAD;
                }
                break;
          default:
                return CURL_SOCKET_BAD;
     }
}

int CurlTask::close_socket(void* clientp, curl_socket_t item)
{
     CurlTask* self = static_cast<CurlTask*>(clientp);
     if (!self)
        return 1;
     
     self->m_event_engine->close_socket(self->m_tracker);
     return 0;
}

//write the http response body to the download job
size_t CurlTask::write_function(char* ptr, size_t size, size_t nmemb, void* userdata)
{
        LOG_TRACE("write " <<size<<"*"<<nmemb);
        CurlTask* self = static_cast<CurlTask*>(userdata);
        size_t actual_size = size * nmemb;
        if (!self)
                return 0;

        if (!actual_size)
                return 0;

        size_t write_size=self->m_download_job->write_to_sink(ptr,actual_size);
        if (write_size> 0){
                return write_size;
        } else {
                LOG_INFO("Cannot read the data, close the downloader connection");
                self->on_job_complete(CURLE_ABORTED_BY_CALLBACK);
        }
        return 0;
}

     
