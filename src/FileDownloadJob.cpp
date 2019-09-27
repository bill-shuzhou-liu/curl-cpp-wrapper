#include <curl/curl.h>
#include "Common.hpp"
#include "FileDownloadJob.hpp"

FileDownloadJob::FileDownloadJob(const std::string& url,
                const std::string& filename):
        m_url(url)
        ,m_filename(filename)
        ,m_file(filename.c_str())
{
}

size_t FileDownloadJob::write_to_sink(char* ptr, size_t size)
{
        LOG_TRACE("FileDownloadJob::write_to_sink in:"<<size);

        if (size == 0) return size;
        m_file.write(ptr, size);

        return size;
}

void FileDownloadJob::handle_download_completed(CURL* handler, int err)
{
        LOG_TRACE("download "<<m_url);
        long status_code = 0;
        curl_easy_getinfo (handler, CURLINFO_RESPONSE_CODE, &status_code);

        if (!err ) //No error 
        {
                LOG_INFO("Download " << m_url << " to file "<< m_filename<<", status:"<<status_code);
                m_file.flush();
        }
        else
        {
                LOG_WARN("Fail to download "<< m_url << " to file "<< m_filename<<", reason:" << curl_easy_strerror(CURLcode(err)));
        }

}

std::string FileDownloadJob::get_url() const
{
        return m_url;
}
