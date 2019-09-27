#if !defined(__COMMON__HPP__)
#define __COMMON__HPP__

#include<iostream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> TCPSocketPtr;
typedef boost::function<void (const boost::system::error_code&) > TimeoutHandler;
typedef boost::function<void (const boost::system::error_code&, std::size_t) > AsyncEventHandler; 
typedef boost::function<void () > AsyncRunHandler;


//simple logger

#define WARN_LEVEL      0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03
#define TRACE_LEVEL     0x04

//#define LOG_LEVEL TRACE_LEVEL
#ifndef LOG_LEVEL
#define LOG_LEVEL   DEBUG_LEVEL
#endif

#if LOG_LEVEL >= TRACE_LEVEL
#define LOG_TRACE(msg)    std::cout<<msg<<std::endl
#else
#define LOG_TRACE(msg)    
#endif

#if LOG_LEVEL >= DEBUG_LEVEL
#define LOG_DEBUG(msg)    std::cout<<msg<<std::endl
#else
#define LOG_DEBUG(msg)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define LOG_INFO(msg)    std::cout<<msg<<std::endl
#else
#define LOG_INFO(msg)
#endif

#define LOG_WARN(msg)    std::cout<<msg<<std::endl



#endif
