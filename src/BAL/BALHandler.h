/*******************
 * AUTHOR：FEX
 * DATE：2023-05-16
 * HTTP服务：BALHandle.h
 * 业务层代码
 * 定义每一个http请求的实际处理对象
 * 
 * 
 * 
 * 
 * 
*********************/


#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/StreamCopier.h>

#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPServerRequestImpl.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPServerSession.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/DynamicStruct.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/Net/NameValueCollection.h>


#include <Poco/FileStream.h>



using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPMessage;
using Poco::Net::MessageHeader;
using Poco::StreamCopier;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Logger;
using Poco::Net::SocketStream;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPServerRequestImpl;
//using namespace Poco::Net;

using Poco::JSON::Parser;
using Poco::JSON::Object;
using Poco::Dynamic::Var;
using Poco::Net::HTTPServerSession;
using Poco::JSON::JSONException;
using Poco::DynamicStruct;
using Poco::FileOutputStream;
using Poco::Net::PartHandler;
using Poco::Net::NameValueCollection;



#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../common/Common.h"


using namespace std;
using namespace MQTTCLIENT;


class FilePartHandle : public Poco::Net::PartHandler
{
public:
    FilePartHandle();

    struct FilePart
    {
        std::string name;
        std::string file_name;
        std::shared_ptr< std::vector<std::uint8_t> >  content;
    };

    void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream);

    void Print();

    void SaveFile();

    std::list<FilePart>& file_parts();

private:

    std::list<FilePart> file_parts_;
    char  m_SaveDir[MQTTCLIENT::MAX_DIRPATH];

    void QBFusion();
};


//上传文件
class UploadRequestHandler: public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
	
	void sendResponseResult(HTTPServerRequest & request, HTTPServerResponse & response,std::string result_info);

};