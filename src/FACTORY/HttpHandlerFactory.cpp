/*******************
 * AUTHOR：FEX
 * DATE：2023-05-16
 * HTTP服务：HttpHandlerFactory.cpp
 * 工厂代码
 * 实现工厂模式
 * 
 * 
 * 
 * 
 * 
*********************/

#include "HttpHandlerFactory.h"

HTTPRequestHandler * HTTPHandlerFactory::createRequestHandler(const HTTPServerRequest & request)
{
    std::cout<<std::endl<<"Request URI："<<request.getURI()<<std::endl;
    std::cout<<"Request Method："<<request.getMethod()<<std::endl;
    std::cout<<"Request Host："<<request.getHost()<<std::endl;
    std::cout<<"Request Content Type："<<request.getContentType()<<endl;
    std::cout<<"Request Content Length："<<request.getContentLength()<<std::endl<<std::endl;

    if(request.getURI() == "/upload")
    {
        return new UploadRequestHandler;
    }
    else
    {
        return 0;

    }
}