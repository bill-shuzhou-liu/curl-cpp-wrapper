#if !defined(__IDOWNLOAD_JOB_HPP__)
#define __IDOWNLOAD_JOB_HPP__

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

typedef boost::asio::streambuf MsgBuffer;

class IDownloadJob
{
public:
    // Receive the download data from the curl and then write to memory
    virtual size_t write_to_sink(char* ptr, size_t size)=0;
    // It will be notified when the download complete 
    virtual void handle_download_completed(CURL* handler, int err)=0;
    // Get URL
    virtual std::string get_url() const = 0;
    virtual ~IDownloadJob() {}
};
typedef boost::shared_ptr<IDownloadJob> DownloadJobPtr;



#endif
