/*******************
 * AUTHOR：FEX
 * DATE：2023-05-16
 * HTTP服务：HttpServer.h
 * Http服务Application
 * 定义Application
 * 
 * 
 * 
 * 
 * 
*********************/


#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Logger.h>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/ServerApplication.h>

#include "../FACTORY/HttpHandlerFactory.h"

using namespace Poco;
using Poco::Util::ServerApplication;
using Poco::Net::HTTPServer;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPServerParams;



#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace MQTTCLIENT;

class HttpApplication: public ServerApplication 
{
public:
    HttpApplication(){ }
    ~HttpApplication() { }
protected:
    //初始化操作
    void initialize(Application& self);
    //清理操作
    void uninitialize();
    //主函数入口
    int main(const vector<string>& args);
private:
    //http监听服务
    HTTPServer * _httpSvr;
};