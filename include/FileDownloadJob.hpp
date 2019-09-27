#if !defined(__FILE_DOWNLOAD_JOB_HPP__)
#define __FILE_DOWNLOAD_JOB_HPP__
#include <string>
#include <fstream>
#include "IDownloadJob.hpp"

class FileDownloadJob: public IDownloadJob
{
public:
        FileDownloadJob(const std::string& url, const std::string& filename);
        virtual size_t write_to_sink(char* ptr, size_t size);
        virtual void handle_download_completed(CURL* handler, int err );
        virtual std::string get_url() const;
private:
        std::string m_url;
        std::string m_filename;
        std::ofstream m_file;
};

#endif
