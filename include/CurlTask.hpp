#if !defined(__CURL_TASK_HPP__)
#define __CURL_TASK_HPP__

#include <boost/asio.hpp>
#include <curl/curl.h>
#include "IDownloadJob.hpp"
#include "Common.hpp"
#include "IEventEngine.hpp"
#include "SocketTracker.hpp"

/**
 * The CurlTask manages the low level curl easy handler and download jobs
 * In current implementation, CurlTask can only download one URL and will not be reused.
 */
class CurlTask {
public:
    /**
     * Constructor
     * Initial the easy curl using curl_easy_setopt
     * @param downloadjob, the download job for this easy curl
     * @param events, the event engine used to open/close the socket
     */
    CurlTask(DownloadJobPtr& downloadjob, EventEnginePtr events);

    /**
     * Destructor
     * free the low level resource managed by the easy curl
     */
    ~CurlTask();

    /**
     * @return the easy handler
     */
    CURL* get_easy_handler() const;

    /**
     * This function will be called when the download job is completed or have errors
     * It will notify the downloadjob for that.
     * @param result, the result returned by curl
     */
    void on_job_complete(CURLcode result);

    /**
     * Get the socket tracker managed by the CurlTask
     * @return the socket tracker
     */
    SocketTrackerPtr get_socket_tracker() const;

private:

    //callback for CURLOPT_OPENSOCKETFUNCTION 
    static curl_socket_t open_socket(void* clientp, curlsocktype purpose, struct curl_sockaddr* address);
    //callback for CURLOPT_CLOSESOCKETFUNCTION 
    static int close_socket(void* clientp, curl_socket_t item);

    //callback for CURLOPT_WRITEFUNCTION
    static size_t write_function(char* ptr, size_t size, size_t nmemb, void* userdata);

    DownloadJobPtr m_download_job;
    EventEnginePtr m_event_engine;
    SocketTrackerPtr m_tracker;
    CURL* m_easy_handler;

};

typedef boost::shared_ptr<CurlTask> CurlTaskPtr;

#endif
