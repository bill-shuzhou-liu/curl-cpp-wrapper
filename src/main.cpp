#include <vector>
#include <string>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string.hpp>
#include "MultiCurl.hpp"
#include "FileDownloadJob.hpp"
#include "AsioEventEngine.hpp"

using namespace std;
int main(int argc, char *argv[]) {
    
    if (argc == 1){
        std::cerr<<"Usage: "<<argv[0]<<" url_file" <<std::endl;
        std::cerr<<"Each line of the file is in 'url download_file_name' format"<<std::endl;
        exit(0);
    }

    std::vector< std::pair<std::string,std::string> > urls;
    std::ifstream infile(argv[1]);
    if (!infile) {
         std::cerr<<"Cannot open file "<<argv[1]<<std::endl;
         exit(0);
    }

    std::string line;
    int count=0;
    while (std::getline(infile, line))
    {
       count++;
       std::vector<string> tokens;
       boost::split(tokens, line, boost::algorithm::is_any_of(" "), boost::algorithm::token_compress_on );
       if (tokens.size()==1){
	  urls.push_back(make_pair(tokens[0], to_string(count)+".download"));
       }else{
            urls.push_back(make_pair(tokens[0], tokens[1]));
       }

    }
    infile.close();

    boost::asio::io_service io_service;
    EventEnginePtr event(new AsioEventEngine(io_service));    
    
    MultiCurl multi(event);
    
    //create dummy worker to hold off the thread before the job is added
    boost::shared_ptr<boost::asio::io_service::work> worker(new boost::asio::io_service::work(io_service));
    boost::thread run_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    for (size_t i=0 ; i< urls.size(); i++){
         DownloadJobPtr  downloadJob(new FileDownloadJob(urls[i].first,urls[i].second));
         multi.async_download(downloadJob);
    }
    //release the dummy worker
    worker.reset();
    
    run_thread.join();
    
}
