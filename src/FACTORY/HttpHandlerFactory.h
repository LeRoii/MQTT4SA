/*******************
 * AUTHOR：FEX
 * DATE：2023-05-16
 * HTTP服务：HttpHandlerFactory.h
 * 工厂代码
 * 定义工厂模式
 * 
 * 
 * 
 * 
 * 
*********************/


#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>

#include "../BAL/BALHandler.h"
#include <Poco/URI.h>
using Poco::URI;

using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPRequestHandler;



#include <iostream>
#include <string>
#include <vector>

using namespace std;


class HTTPHandlerFactory : public HTTPRequestHandlerFactory 
{
public:
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request);
};
