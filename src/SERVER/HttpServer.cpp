/*******************
 * AUTHOR：FEX
 * DATE：2023-05-16
 * HTTP服务：HttpServer.cpp
 * Http服务Application
 * 实现Application
 * 
 * 
 * 
 * 
 * 
*********************/


#include "HttpServer.h"

void HttpApplication::initialize(Application& self) 
{
    loadConfiguration();
    ServerApplication::initialize(self);
}

void HttpApplication::uninitialize() 
{
    ServerApplication::uninitialize();
}

int HttpApplication::main(const vector<string>& args) 
{
    // HTTPServer HSer(new HTTPHandlerFactory, ServerSocket(8080), new HTTPServerParams);// ServerSocket是匿名对象
    // HSer.start();//启动http服务器
    // waitForTerminationRequest();//
    // std::cout << "关闭" << std::endl;
    // HSer.stop();
    // return Application::EXIT_OK;

    char m_ServerPort[PORT_LEN+1] = {0};
    MQTTCLIENT::Common::GetValueByKeyFromConfig("HTTP_SERVER_PORT", m_ServerPort, PORT_LEN);

    std::cout<<"HTTP Service Port："<<m_ServerPort<<std::endl;

    //可以指定固定的端口也可以从外部配置
    int port = atoi(m_ServerPort);
    ServerSocket svs(port);
    _httpSvr = new HTTPServer(new HTTPHandlerFactory(), svs, new HTTPServerParams);
    _httpSvr->start();
    //阻塞住进程
    waitForTerminationRequest();
    std::cout << "close http service" << std::endl;
    _httpSvr->stop();
    return Application::EXIT_OK;
}