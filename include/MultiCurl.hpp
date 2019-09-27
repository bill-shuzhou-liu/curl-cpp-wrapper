#if !defined(__MULTI_CURL_HPP__)
#define __MULTI_CURL_HPP__

#include <map>
#include <curl/curl.h>
#include <boost/asio.hpp>

#include "IDownloadJob.hpp"
#include "IEventEngine.hpp"
#include "SocketTracker.hpp"
#include "CurlTask.hpp"

/**
 * The MultiCurl managed a group of download tasks using the multi_curl interface.
 * The low level event detection is managed by the EventEngine:
 */
class MultiCurl {
public:
        //Map to store the list of the CurlTask
        typedef std::map<CURL*, CurlTaskPtr> DownloadTaskMap;

        //Constructor and destructor
        MultiCurl(EventEnginePtr events);
        ~MultiCurl();

        //kick off the async download
        void async_download(DownloadJobPtr& downloadjob);

private:
        void async_download_impl(DownloadJobPtr& downloadjob);

        //multi callback to set the socket fucntion and timer
        static int socket_function(CURL* native_easy, curl_socket_t s, int what, void* userp, void* socketp);
        static int timer_function(CURLM* native_multi, long timeout_ms, void* userp);

        //The callback to handle timeout.
        void handle_timeout(const boost::system::error_code& err);

        //check multi_info for complete download. This will be called whenever the call returned from CURL.
        void check_multi_done();
        void on_download_done(CURL *e, CURLcode result); 

        //callback for async_read and async_write
        void on_read_event(const boost::system::error_code& err, SocketTrackerPtr st);
        void on_write_event(const boost::system::error_code& err, SocketTrackerPtr st);

        //curl_multi_socket_action for Multi to dispatch job
        void socket_action(curl_socket_t s, int event_bitmask);

        EventEnginePtr m_event_engine; ///< the event engine
        CURLM *m_multi; ///< low level CURL multi interface
        int m_running_handles; ///< how many running easy handles
        DownloadTaskMap m_downloads; ///< store the download tasks
};

#endif
