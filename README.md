## quick start
'''
cmake CMakeLists.txt
make
'''

Then you can use the downloader to download multiple urls and save to file 1,2,3...:

'''
./bin/downloader google.com facebook.com
'''

## The design
The multi-curl uses the boost asio as the event engine: When it needs to write the request or read the response, it will ask the boost asio whether the socket is ready to read/write. 
![Class diagram](docs/mcurl_class.jpg?raw=true "Class diagram")

The IEventEngine is an interface class and the AsioEventEngine implemented the event detect mechanism required by the multi-curl using boost asio. 

The MultiCurl class will manage the multi curl interface, i.e. MCURL. It also stores a list of CurlTask. 

The CurlTask will manage the easy curl interface, i.e. CURL. It also hold the pointer to the downloadjob.

The IDownloadJob provides an interface to download job to file or in memory. 

The Sequence diagram shows how an async_download works. 
![Sequence diagram](docs/mcurl_sequence.jpg?raw=true "Sequence diagram")
1. the User sends async_download(job) request where the job contains the URL to download
1.1 The MutliCurl will create an CurlTask and add the easy curl handler to mutli curl. 
1.2 The EventEngine will timeout and call the handler in MutliCurl
1.3 The MutliCurl will call curl_multi_socket_action to kick off libcurl to handle this new request. 
1.4 The curl will open the socket. It will call the open_socket function in the EventEngine to get a socket managed by the asio
1.5 Internally, it will do the DNS query (using another thread or lookup cache).
1.6 After that, it is ready to write the request, so it call socket_action defined in MultiCurl to check whether the socket is ready to write.
1.7 The MutliCurl delegate that request to the EventEngine, which in turn will call async_write_some(null_buffer....) using boost asio
1.8 The write handler in the MutliCurl indicates the write is ready.
1.9 The MutliCurl will call socket_action(CURL_CSELECT_OUT) to let curl know he can write the request to the web server.
1.10 The curl writes the request to the web server.
1.11 The curl will call socket_action again to check whether the socket is ready to read.
1.12. Similar to 1.7, EventEngine will call async_read_some(null_buffer ...)
1.13 The read handler in the MutliCurl indicates the read is ready
1.14 The MutliCurl will call socket_action(CURL_CSELECT_IN) to let curl know he can read from the web server.
1.15 The curl start to read the data from web server
1.16 It will call the CURLOPT_WRITEFUNCTION callback set in the CurlTask to write the http body to it. 
1.17 The CurlTask in turn forwarded that to the DownloadJob::write_to_sink(...)
1.18 When all the body is downloaded, the libcurl will call the socket_function again to notify MutliCurl
1.19 Then the MutliCurl will call the on_job_complete(...) on CurlTask
1.20 The CurlTask will call handle_download_completed in the DownloadJob


