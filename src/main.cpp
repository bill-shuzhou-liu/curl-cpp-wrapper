#include <vector>
#include <string>
#include <sstream>
#include <boost/thread.hpp>
#include "MultiCurl.hpp"
#include "FileDownloadJob.hpp"
#include "AsioEventEngine.hpp"

int main(int argc, char *argv[]) {
    if (argc == 1){
        LOG_WARN("Usage: "<<argv[0]<<" \"url1\" \"url2\" ...");
        exit(0);
    }


    std::vector<std::string> urls;
    for (int i=1 ; i<argc; i++){
        urls.push_back(argv[i]);
    }


    boost::asio::io_service io_service;
    EventEnginePtr event(new AsioEventEngine(io_service));    
    
    MultiCurl multi(event);
    
    //create dummy worker to hold off the thread before the job is added
    boost::shared_ptr<boost::asio::io_service::work> worker(new boost::asio::io_service::work(io_service));
    boost::thread run_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    for (size_t i=0 ; i< urls.size(); i++){
         std::stringstream ss;
         ss<<i;
         DownloadJobPtr  downloadJob(new FileDownloadJob(urls[i],ss.str()));
         multi.async_download(downloadJob);
    }
    //release the dummy worker
    worker.reset();
    
    run_thread.join();
    
}
