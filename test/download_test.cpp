#define BOOST_TEST_MODULE MutliCurlTest

#include <boost/test/included/unit_test.hpp>
#include "MultiCurl.hpp"
#include "AsioEventEngine.hpp"
#include "FileDownloadJob.hpp"

class TestDownloadHandler : public IDownloadJob
{
public:
        TestDownloadHandler(const std::string& url):
                m_url(url), acutal_size(0), status_code(0) {}
        virtual size_t write_to_sink(char* ptr, size_t size)
        {
               acutal_size+=size;
               return size;
        }
        virtual void handle_download_completed(CURL* handler, int err )
        {
               curl_easy_getinfo (handler, CURLINFO_RESPONSE_CODE, &status_code);
        }
        virtual std::string get_url() const {return m_url;}

        std::string m_url;
        size_t acutal_size;
        long status_code; 
};

typedef boost::shared_ptr<TestDownloadHandler> TestDownloadHandlerPtr;

BOOST_AUTO_TEST_CASE(download_ok)
{
    boost::asio::io_service io_service;
    EventEnginePtr event(new AsioEventEngine(io_service));
    MultiCurl multi(event);

    TestDownloadHandlerPtr downloadJob(new TestDownloadHandler("google.com"));
    DownloadJobPtr dj=boost::dynamic_pointer_cast<IDownloadJob>(downloadJob);
    multi.async_download(dj);
    io_service.run();
    BOOST_CHECK( downloadJob->acutal_size > 0);
    BOOST_CHECK( downloadJob->status_code = 200);
}


BOOST_AUTO_TEST_CASE(download_bad)
{
    boost::asio::io_service io_service;
    EventEnginePtr event(new AsioEventEngine(io_service));
    MultiCurl multi(event);

    TestDownloadHandlerPtr downloadJob(new TestDownloadHandler("com.google"));
    DownloadJobPtr dj=boost::dynamic_pointer_cast<IDownloadJob>(downloadJob);
    multi.async_download(dj);
    io_service.run();
    BOOST_CHECK( downloadJob->acutal_size == 0);
    BOOST_CHECK( downloadJob->status_code == 0);
}

