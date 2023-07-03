/*****************
 * project:MQTTClient
 * source:common.cpp
 * author:FEX
 * time:2021-09-15
 * description:
 * 公共处理类实现
 * copyright:
 *
 *
 * ***************/

#include "./Common.h"

namespace MQTTCLIENT
{
	//静态成员初始化
	MQTTCLIENT::enum_Status Common::m_RunStatus = MQTTCLIENT::e_None;

    //退出信号
    bool Common::SIGQUITFLAG = false;

    
    //MQTT消息返回主题
    const char Common::top_Rcv[STR_LEN] = "/AIService/All/Recv";

    //消息队列操作对象
    struct mosquitto* Common::mosq = nullptr;

    
    //存储所有目标信息
    std::vector<MQTTCLIENT::MapMarkerInfo> Common::m_markers;


    //遍历filePath目录，针对每个png和jpg文件，使用目标识别算法处理每一个图片文件，将识别结果返回
    vector<string> Common::files;

    
    map<string, string> Common::TargetTypeDic{{"person", "人员"}, {"soldier","士兵"}, {"vehicle", "车辆"}, {"millitary vehicle", "装甲车"}};


    map<string, string> Common::TargetTypeMap{{ "人", "person"}, { "人员", "person"}, {"士兵", "soldier"}, {"车", "vehicle"}, {"车辆", "vehicle"}, {"装甲车", "millitary vehicle"}};


	
	Common::Common()
	{

	}

	//读取配置文件中的配置信息，
	//key-要查询的关键字，value-查询到的值，length-保存值的字符串缓冲区最大长度
	//配置文件中每行的信息格式为：key=value
	const int Common::GetValueByKeyFromConfig(const char* key, char* value, const unsigned int length)
	{
		if(NULL == key || NULL == value || length <= 0)
		{
			return -1;
		}
		
		int ret=0;
		int FD = 0;
		char* ConfigContent = new char[2048];
		char *findStr = NULL;
		

		//打开配置文件，读写模式
		FD  = open("./Config.ini",O_RDWR|O_CREAT,0644);

        if(FD < 0)
        {
            std::cout<<"./Config.ini配置文件打开错误！"<<std::endl;
            return -1;
        }

		//读取文件中的内容到ConfigContent缓冲区，缓冲区最多容纳2048字节
		ret = read(FD,ConfigContent,2048);
		//关闭打开的文件
		close(FD);
		//从文件中读取到的字符数大于等于1时，进行处理
		if(1 <= ret)
		{	
			//字符串查找函数，在ConfigContent中查找key字符串，并返回找到的位置，如果没找到，则返回NULL
			findStr = strstr(ConfigContent,key);
			//如果没找到子字符串
			if(findStr == NULL)
			{
				return -1;
			}
			
			const int keyLen = strlen(key);
			int i=0 ,j=0;
			for( ; i< strlen(findStr)-1-keyLen && j<length-1; i++)
			{
				if(findStr[keyLen+1+i] == ' ' || findStr[keyLen+1+i] == '\n' || findStr[keyLen+1+i] == '\r'  
					|| findStr[keyLen+1+i] == '\t'  || findStr[keyLen+1+i] == '#'   || findStr[keyLen+1+i] == EOF)
				{
					break;
				}
				else
				{
					value[j++] = findStr[keyLen+1+i];
				}
			}

			printf("key-%s\r\nvalue-%s\r\n", key, value);
			//返回找到的字符串value的实际长度
			return j;
		}
		else//从配置文件中读取到的字符数小于1，则直接返回
		{
            std::cout<<"配置文件读取错误！"<<std::endl;
			return -1;
		}
	}

	//read from  fd
	//输入参数：fd-文件描述符，buffer-存放数据的缓冲区，maxSize-最大读取数据量，timeout-超时时长（ms）
	//输出参数：size-本次读取数据量
	const int64_t Common::Read(const int fd, uint8_t* buffer, const uint64_t maxSize, uint64_t* size, const uint32_t timeout)
	{
		if(buffer == NULL || size == NULL || fd < 3)
		{
			return -1;
		}

		fd_set rd;
		struct timeval tv;
		
		int64_t ret = -1;
		
		//读取的数据总量
		uint64_t pos = 0;
		
		while(pos < maxSize)
		{
		
			FD_ZERO(&rd);
			FD_SET(fd, &rd);

			tv.tv_sec = timeout/1000;
			tv.tv_usec = (timeout%1000) * 1000;

			ret = select(fd+1,&rd,NULL,NULL,&tv);
			
			if(0 > ret)
			{
				printf("select failed where reading!\n");
				break;
			}
			else if(0 == ret)
			{
				printf("select timeout where reading!\n");
				break;
			}
			else
			{
				//如果当前监控描述符有变化，则进行处理
				if(FD_ISSET(fd,&rd))
				{
					ret = read(fd, buffer, maxSize);
					if(0 > ret)
					{
						printf("read data failed\n");
						break;
					}
					else if(0 == ret)
					{
						printf("port disconnect or endfile where reading!\n");
						break;
					}
					else
					{
						pos += ret;
						
						//*size = ret;
					}
				}
			}
		}
		
		//输出参数：size值设定
		memcpy(size, &pos, sizeof(pos));
		
		if(pos != maxSize)
		{
			printf("read max-size %ld bytes，but real read size %ld bytes\n", maxSize, size);
		}

		return pos;
	}

	//write to fd
	//输入参数：fd-文件描述符，buffer-存放数据的缓冲区，maxSize-最大发送数据量，timeout-超时时长（ms）
	//输出参数：size-本次写入的数据量
	const int64_t Common::Write(const int fd, const uint8_t* buffer, const uint64_t maxSize, uint64_t* size, const uint32_t timeout)
	{
		if(buffer == NULL || maxSize < 0 || size == NULL || fd < 3)
		{
			return -1;
		}

		fd_set wd;
		struct timeval tv;
		
		int64_t ret = -1;
		
		//写入的数据总量
		uint64_t pos = 0;
		
		//
		while(pos < maxSize)
		{
		
			FD_ZERO(&wd);
			FD_SET(fd, &wd);

			tv.tv_sec = timeout/1000;
			tv.tv_usec = (timeout%1000) * 1000;

			ret = select(fd+1,NULL,&wd,NULL,&tv);
			
			if(0 > ret)
			{
				printf("select failed where writing!\n");
				break;
			}
			else if(0 == ret)
			{
				printf("select timeout where writing!\n");
				break;
			}
			else
			{
				//如果当前监控描述符有变化，则进行处理
				if(FD_ISSET(fd,&wd))
				{
					ret = write(fd, buffer, maxSize);
					if(0 > ret)
					{
						printf("read data failed\n");
						break;
					}
					else if(0 == ret)
					{
						printf("port disconnect or endfile!\n");
						break;
					}
					else
					{
						pos += ret;
						
						//*size = ret;
					}
				}
			}
		}
		
		//输出参数：size值设定
		memcpy(size, &pos, sizeof(pos));
		
		if(pos != maxSize)
		{
			printf("write max-size %ld bytes，but real write size %ld bytes\n", maxSize, size);
		}

		return pos;
	}
	
	
	//判断system指令是否执行成功
	//输入参数：result调用system函数返回的值
	//返回值：0~成功，-1~失败
	const int Common::SystemCheck(int result)
	{
		if((-1 != result) && (WIFEXITED(result)) && (!(WEXITSTATUS(result))))
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	
	
	//异或校验，末尾四字节为校验值
	//输入参数：buffer-校验数据（含末尾四字节校验值位置），len-需要校验的数据长度（不包括末尾字节校验值）
	//输出参数：校验值
	//返回值：异或校验结果
	const uint8_t Common::XOR(uint8_t* buffer, const uint32_t len)
	{
		//要校验的数据为空，或者要校验的数据长度小于等于0，直接返回
		if(NULL == buffer || 0 >= len)
		{
			return 0;
		}
		uint8_t ret = buffer[0];
		
		//异或计算
		for(int i=0; i<len ;i++)
		{
			ret ^= buffer[i];
		}
		
		buffer[len] = ret;
		
		return ret;
	}
	
	
	//异或校验检查
	//输入参数：buffer-校验检查数据（包含最后一个校验字节在内），len-数据长度（包含最后一个校验字节在内）
	//输出参数:无
	//返回值：0~成功，-1~失败
	const int Common::ChechXOR(const uint8_t* buffer, const uint32_t len)
	{
		int ret=0;
		
		//要校验检查的数据为空，或者要校验的数据长度小于等于0，直接返回
		if(NULL == buffer || 0 >= len)
		{
			return 0;
		}
		
		uint8_t checkRet = buffer[0];
		
		//异或计算
		for(int i=0; i<len-1 ;i++)
		{
			checkRet ^= buffer[i];
		}
		
		//判断计算出的校验值是否和buffer最后一个字节值相等
		if(checkRet == buffer[len-1])
		{
			ret = 0;
		}
		else
		{
			ret = -1;
		}
		
		return ret;
	}
	
	
	//校验计算
	//输入参数：buffer-校验数据（含末尾校验值位置），len-需要校验的数据长度（不包括末尾校验值）
	//输出参数：无
	//返回值：校验值
	const int Common::GetCheckSB(uint8_t* buffer, const int32_t len)
	{
		//要校验的数据为空，或者要校验的数据长度小于等于0，直接返回
		if(NULL == buffer || 0 >= len)
		{
			return 0;
		}
		
		int ret = buffer[0];
		
		int iData = 0;
		//循环计算校验结果
		for(int i=1; i < len - 1 ;i++)
		{
			if(buffer[i] < 0)
			{
				iData = buffer[i] & 0xFF;
			}
			else
			{
				iData = buffer[i];
			}
			if(ret < 0)
			{
				ret = ret & 0xFF;
			}
			
			ret ^= iData;
		}
		
		return ret;
	}
	
	//校验检查
	//输入参数：buffer-校验检查数据（包含最后四个校验字节在内），len-数据长度（包含最后四个校验字节在内）
	//输出参数：无
	//返回值：0~成功，-1~失败
	const int Common::CheckSB(const uint8_t* buffer, const int32_t len)
	{
		//要校验的数据为空，或者要校验的数据长度小于等于0，直接返回
		if(NULL == buffer || 3 >= len)
		{
			return 0;
		}
		
		int ret = buffer[0];
		
		int iData = 0;
		
		for(int i=1; i < len - 4 ;i++)
		{
			if(buffer[i] < 0)
			{
				iData = buffer[i] & 0xFF;
			}
			else
			{
				iData = buffer[i];
			}
			if(ret < 0)
			{
				ret = ret & 0xFF;
			}
			
			ret ^= iData;
		}

        int recValue = (buffer[len-4] + buffer[len-3]*256 + buffer[len-2]*256*256 + buffer[len-1]*256*256*256);

        printf("cal value: %d\nrecv value %d\n", ret, recValue);
		
		if(ret == recValue)
		{
			ret = 0;
		}
		else
		{
			ret = -1;
		}
		
		return ret;
	}
	
	
	//设置状态信息
	//输入参数：status状态值
	//输出参数：无
	//返回值：0~成功，-1~失败
	const int Common::SetStatus(const MQTTCLIENT::enum_Status status)
	{
		Common::m_RunStatus = status;
		return 0;
	}
	

	//获取状态信息
	//输入参数：无
	//输出参数：无
	//返回值：状态标志值
	const MQTTCLIENT::enum_Status Common::GetStatus()
	{
		return Common::m_RunStatus;
	}

    

    //将消息转换成字符串
    //msg-消息结构体，str-转换后的字符串保存位置，maxLen-str缓存的最大容量
    //返回str发送内容的总长度
    int Common::ST_ConvertTo_Char(const ST_MSG &msg, char* str, const uint32_t maxLen)
    {
        if(NULL == str || nullptr == str || maxLen == 0)
        {
            return -1;
        }

        //先清空缓存
        memset(str, 0 ,maxLen);

        //拷贝消息头
        memcpy(str, (void*)&msg.st_head, sizeof(ST_HEADER));

        //再拷贝消息体，消息体数据不为空时才可以拷贝
        if(NULL != msg.st_msg || nullptr != msg.st_msg)
        {
            switch(msg.st_head.st_command)
            {
            //移动、缩放图像动作
            case MQTTCLIENT::enum_Command::e_Move:
            {
                MQTTCLIENT::ST_MOVE *s_Move = (MQTTCLIENT::ST_MOVE*)(msg.st_msg);
                std::cout<<"st_MoveType:"<<s_Move->st_MoveType<<endl
                        <<"st_MouseBtnType:"<<s_Move->st_MouseBtnType<<endl
                        <<"st_X:"<<s_Move->st_X<<endl
                        <<"st_Y:"<<s_Move->st_Y<<endl
                        <<"st_moveX:"<<s_Move->st_moveX<<endl
                        <<"st_moveY:"<<s_Move->st_moveY<<endl
                        <<"st_ScaleFactor:"<<s_Move->st_ScaleFactor<<endl;
                memcpy(str+sizeof(ST_HEADER),
                       &s_Move->st_MoveType, sizeof(s_Move->st_MoveType));
                memcpy(str+sizeof(ST_HEADER)+sizeof(s_Move->st_MoveType),
                       &s_Move->st_MouseBtnType, sizeof(s_Move->st_MouseBtnType));

                memcpy(str+sizeof(ST_HEADER)+sizeof(s_Move->st_MoveType)+sizeof(s_Move->st_MouseBtnType),
                       &s_Move->st_X, sizeof(s_Move->st_X));
                memcpy(str+sizeof(ST_HEADER)+sizeof(s_Move->st_MoveType)+sizeof(s_Move->st_MouseBtnType)+sizeof(s_Move->st_X),
                       &s_Move->st_Y, sizeof(s_Move->st_Y));

                memcpy(str+sizeof(ST_HEADER)+sizeof(s_Move->st_MoveType)+sizeof(s_Move->st_MouseBtnType)+sizeof(s_Move->st_X)+sizeof(s_Move->st_Y),
                       &s_Move->st_moveX, sizeof(s_Move->st_moveX));
                memcpy(str+sizeof(ST_HEADER)+sizeof(s_Move->st_MoveType)+sizeof(s_Move->st_MouseBtnType)+sizeof(s_Move->st_X)+sizeof(s_Move->st_Y)+sizeof(s_Move->st_moveX),
                       &s_Move->st_moveY, sizeof(s_Move->st_moveY));
                memcpy(str+sizeof(ST_HEADER)+sizeof(s_Move->st_MoveType)+sizeof(s_Move->st_MouseBtnType)+sizeof(s_Move->st_X)+sizeof(s_Move->st_Y)+sizeof(s_Move->st_moveX)+sizeof(s_Move->st_moveY),
                       &s_Move->st_ScaleFactor, sizeof(s_Move->st_ScaleFactor));

                break;
            }
            //开始建图动作
            case MQTTCLIENT::enum_Command::e_StartRTSP:
            {
                MQTTCLIENT::ST_START_RTSP *s_start = (MQTTCLIENT::ST_START_RTSP*)(msg.st_msg);
                std::cout<<"st_ViewpointType:"<<s_start->st_ViewpointType<<endl
                        <<"st_CoodinateSystemType:"<<s_start->st_CoodinateSystemType<<endl;
                memcpy(str+sizeof(ST_HEADER),
                       &s_start->st_ViewpointType, sizeof(s_start->st_ViewpointType));
                memcpy(str+sizeof(ST_HEADER)+sizeof(s_start->st_ViewpointType),
                       &s_start->st_CoodinateSystemType, sizeof(s_start->st_CoodinateSystemType));

                break;
            }
            //视角转换
            case MQTTCLIENT::enum_Command::e_ViewpointSwitch:
            {
                MQTTCLIENT::ST_VIEWPOINT_SWITCH *s_VP = (MQTTCLIENT::ST_VIEWPOINT_SWITCH*)(msg.st_msg);
                std::cout<<"st_ViewpointType:"<<s_VP->st_ViewpointType<<endl;
                memcpy(str+sizeof(ST_HEADER),
                       &s_VP->st_ViewpointType, sizeof(s_VP->st_ViewpointType));

                break;
            }
            //坐标系转换
            case MQTTCLIENT::enum_Command::e_CoordinateSystemSwitch:
            {
                MQTTCLIENT::ST_COORDINATESYSTEM_SWITCH *s_CS = (MQTTCLIENT::ST_COORDINATESYSTEM_SWITCH*)(msg.st_msg);
                std::cout<<"st_CoodinateSystemType:"<<s_CS->st_CoodinateSystemType<<endl;
                memcpy(str+sizeof(ST_HEADER),
                       &s_CS->st_CoodinateSystemType, sizeof(s_CS->st_CoodinateSystemType));

                break;
            }
            default:
                break;
            }
        }

        //最后拷贝消息尾
        memcpy(str+sizeof(ST_HEADER)+msg.st_head.st_len, (void*)&msg.st_tail, sizeof(ST_TAIL));

        return sizeof(msg.st_head)+sizeof(msg.st_tail)+msg.st_head.st_len;
    }

    
    //设置退出信号
    const int Common::SetSigFlag(const bool flag)
    {
        MQTTCLIENT::Common::SIGQUITFLAG = flag;
        return 0;
    }
    //获取退出信号
    const bool Common::GetSigFlag()
    {
        return MQTTCLIENT::Common::SIGQUITFLAG;
    }

    
    //加校验计算，将buffer的数据累加起来计算数值，最后对结果取反
    const uint8_t Common::CheckSum(const uint8_t *buffer, const uint32_t num)
    {
        uint8_t ret = 0;
        //计算累加和
        for(uint32_t i =0; i < num; i++)
        {
            ret += buffer[i];
        }
        //取反
        ret = ~ret;

        return ret;
    }


    
    //将威胁等级返回的结构体数据转换成JSON数据
    const int Common::ThreatST_ConvertTo_JSON(const ST_THREATLEVEL_RECV &obj, char* str, const uint32_t len)
    {
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }
        
        cJSON *js = cJSON_CreateObject();


        cJSON_AddNumberToObject(js,"type", obj.st_type);


        cJSON *Arr = cJSON_CreateArray();

        for(int i=0; i < obj.st_obj.vect.size();i++)
        {
            cJSON *tmp = cJSON_CreateObject();
            cJSON_AddStringToObject(tmp, "id", obj.st_obj.vect[i].id);
            cJSON_AddStringToObject(tmp, "targetName", obj.st_obj.vect[i].targetName);
            cJSON_AddStringToObject(tmp, "grade", obj.st_obj.vect[i].grade);

            cJSON_AddItemToArray(Arr, tmp);
        }

        cJSON_AddItemToObject(js, "data", Arr);

        cJSON_AddStringToObject(js, "api", obj.st_api);
        

        char *tmp = cJSON_Print(js);

        memcpy(str, tmp, strlen(tmp)<len?strlen(tmp):len);

        //std::cout<<"Threat_JSON:"<<str<<std::endl;

        cJSON_Delete(js);

        return 0;
    }

    //将情报融合返回的结构体数据转换成JSON数据
    const int Common::FusionST_ConvertTo_JSON(const ST_ANALYSISFUSION_RECV &obj, char* str, const uint32_t len)
    {
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }
        
        cJSON *js = cJSON_CreateObject();

        cJSON_AddNumberToObject(js,"type", obj.st_type);



        cJSON *js1 = cJSON_CreateObject();
        cJSON_AddStringToObject(js1, "text", obj.st_obj.text);



        cJSON *Arr = cJSON_CreateArray();

        for(int i=0; i < obj.st_obj.vect.size();i++)
        {
            cJSON *tmp = cJSON_CreateObject();
            cJSON_AddNumberToObject(tmp, "id", obj.st_obj.vect[i].id);
            cJSON_AddStringToObject(tmp, "mapType", obj.st_obj.vect[i].mapType);
            cJSON_AddStringToObject(tmp, "camp", obj.st_obj.vect[i].camp);
            cJSON_AddBoolToObject(tmp, "isWeapon", obj.st_obj.vect[i].isWeapon);
            cJSON_AddStringToObject(tmp, "lon_lat", obj.st_obj.vect[i].lon_lat);
            cJSON_AddStringToObject(tmp, "obj_bbox", obj.st_obj.vect[i].obj_bbox);
            cJSON_AddStringToObject(tmp, "time", obj.st_obj.vect[i].time);
            cJSON_AddStringToObject(tmp, "threat_degree", obj.st_obj.vect[i].threat_degree);
            cJSON_AddStringToObject(tmp, "trend", obj.st_obj.vect[i].trend);

            cJSON_AddItemToArray(Arr, tmp);
        }



        cJSON_AddItemToObject(js1, "map", Arr);



        cJSON_AddItemToObject(js, "data", js1);

        cJSON_AddStringToObject(js, "api", obj.st_api);


        char *tmp = cJSON_Print(js);

        memcpy(str, tmp, strlen(tmp)<len?strlen(tmp):len);

        //std::cout<<"Fusion_JSON:"<<str<<std::endl;

        cJSON_Delete(js);

        return 0;

    }

    //将态势分析返回的结构体数据转换成JSON数据
    const int Common::AnalysisSendST_ConvertTo_JSON(const ST_ANALYSISSEND_RECV &obj, char* str, const uint32_t len)
    {
        
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }
        
        cJSON *js = cJSON_CreateObject();

        cJSON_AddNumberToObject(js,"type", obj.st_type);




        cJSON *Arr = cJSON_CreateArray();

        for(int i=0; i < obj.st_obj.vect.size();i++)
        {
            cJSON *tmp = cJSON_CreateObject();
            cJSON_AddNumberToObject(tmp, "longitude", obj.st_obj.vect[i].longitude);
            cJSON_AddNumberToObject(tmp, "latitude", obj.st_obj.vect[i].latitude);
            cJSON_AddNumberToObject(tmp, "angle", obj.st_obj.vect[i].angle);

            cJSON_AddItemToArray(Arr, tmp);
        }



        cJSON_AddItemToObject(js, "data", Arr);


        cJSON_AddStringToObject(js, "api", obj.st_api);



        char *tmp = cJSON_Print(js);

        memcpy(str, tmp, strlen(tmp)<len?strlen(tmp):len);

        //std::cout<<"Analysis_JSON:"<<str<<std::endl;

        cJSON_Delete(js);

        return 0;
    }
    //将态势融合返回的结构体数据转换成JSON数据
    const int Common::TSSendST_ConvertTo_JSON(const ST_TSSEND_RECV &obj, char* str, const uint32_t len)
    {
        
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }
        
        cJSON *js = cJSON_CreateObject();


        cJSON_AddNumberToObject(js,"type", obj.st_type);



        cJSON *Arr = cJSON_CreateArray();

        for(int i=0; i < obj.st_obj.vect.size();i++)
        {
            cJSON *tmp = cJSON_CreateObject();
            cJSON_AddStringToObject(tmp, "timestampAndUserId", obj.st_obj.vect[i].timestampAndUserId);

            //处理markerinfo信息
            cJSON *mi = cJSON_CreateObject();

            cJSON_AddStringToObject(mi, "timestampAndUserId", obj.st_obj.vect[i].Info.timestampAndUserId);
            cJSON_AddNumberToObject(mi, "latitude", obj.st_obj.vect[i].Info.latitude);
            std::cout<<"struct2json : "<<obj.st_obj.vect[i].Info.latitude<<std::endl;
            cJSON_AddNumberToObject(mi, "longitude", obj.st_obj.vect[i].Info.longitude);
            cJSON_AddStringToObject(mi, "markerUrl", obj.st_obj.vect[i].Info.markerUrl);

            //markerdetailinfo信息
            cJSON *mdi = cJSON_CreateObject();
            
            cJSON_AddStringToObject(mdi, "camp", obj.st_obj.vect[i].Info.mdiInfo.camp);
            cJSON_AddStringToObject(mdi, "targetName", obj.st_obj.vect[i].Info.mdiInfo.targetName);
            cJSON_AddNumberToObject(mdi, "longitude", obj.st_obj.vect[i].Info.mdiInfo.longitude);
            cJSON_AddNumberToObject(mdi, "latitude", obj.st_obj.vect[i].Info.mdiInfo.latitude);
            cJSON_AddNumberToObject(mdi, "targetCount", obj.st_obj.vect[i].Info.mdiInfo.targetCount);
            cJSON_AddStringToObject(mdi, "targetDirection", obj.st_obj.vect[i].Info.mdiInfo.targetDirection);
            cJSON_AddNumberToObject(mdi, "targetSpeed", obj.st_obj.vect[i].Info.mdiInfo.targetSpeed);
            cJSON_AddNumberToObject(mdi, "targetState", obj.st_obj.vect[i].Info.mdiInfo.targetState);
            cJSON_AddNumberToObject(mdi, "isWeapon", obj.st_obj.vect[i].Info.mdiInfo.isWeapon);
            cJSON_AddNumberToObject(mdi, "mHitRadius", obj.st_obj.vect[i].Info.mdiInfo.mHitRadius);
            cJSON_AddStringToObject(mdi, "targetType", obj.st_obj.vect[i].Info.mdiInfo.targetType);


            cJSON_AddItemToObject(mi, "markerDetailInfo", mdi);


            cJSON_AddNumberToObject(mi, "jbMarkerCode", obj.st_obj.vect[i].Info.jbMarkerCode);
            cJSON_AddStringToObject(mi, "jbColor", obj.st_obj.vect[i].Info.jbColor);
            cJSON_AddStringToObject(mi, "setOption", obj.st_obj.vect[i].Info.setOption);
            cJSON_AddNumberToObject(mi, "addMarkerTime", obj.st_obj.vect[i].Info.addMarkerTime);
            cJSON_AddNumberToObject(mi, "delMarkerTime", obj.st_obj.vect[i].Info.delMarkerTime);
            cJSON_AddNumberToObject(mi, "updateMarkerTime", obj.st_obj.vect[i].Info.updateMarkerTime);
            cJSON_AddStringToObject(mi, "publisherUserId", obj.st_obj.vect[i].Info.publisherUserId);

            cJSON_AddItemToObject(tmp, "markerInfo", mi);


            cJSON_AddNumberToObject(tmp, "type", obj.st_obj.vect[i].type);
            cJSON_AddNumberToObject(tmp, "res", obj.st_obj.vect[i].res);

            //memcpy(obj.st_obj.vect[i].aiSource, "AI.Fusion", strlen("AI.Fusion"));
            cJSON_AddStringToObject(tmp, "aiSource", "AI.Fusion");

            cJSON_AddItemToArray(Arr, tmp);
        }



        cJSON_AddItemToObject(js, "data", Arr);

        cJSON_AddStringToObject(js, "api", obj.st_api);


        char *tmp = cJSON_Print(js);

        memcpy(str, tmp, strlen(tmp)<len?strlen(tmp):len);

        //std::cout<<"TSSend_JSON:"<<str<<std::endl;

        cJSON_Delete(js);

        return 0;
    }
    



    //态势融合JSON转结构体
    const int Common::JSON_ConvertTo_TSSendST(const char* str, const uint32_t len, ST_TSSEND_RCV &obj)
    {
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }
        


        
        const char * first_yinhao = strstr(str,"markerDetailInfo");
        const char * last_yinhao = strstr(str,"jbMarkerCode");
        char * new_str = new char[strlen(str)+1];

        int slow=0,fast=0;
        while(str[fast] !='\0'){
            if((str+fast)!=(first_yinhao+18) && (str+fast)!=(last_yinhao-3)&&str[fast]!='\\'){
                new_str[slow++]=str[fast];
            }
            fast++;
           }
        new_str[slow]='\0';
        std::cout<<"after del:"<<std::endl<<new_str<<std::endl;

        

        cJSON* js = cJSON_Parse(new_str);
        if(js == NULL || js == nullptr)
        {
            std::cout<<"JSON_ConvertTo_TSSendST failed where str is not json format!!!"<<std::endl;
            return -1;
        }
        else
        {
            printf("json print:\n");
            std::cout<<cJSON_Print(js)<<std::endl;
        }

        memset(&obj, 0, sizeof(obj));
        //解析字符串
        const char* tmp_Type = cJSON_GetObjectItem(js, "type")!=NULL?cJSON_GetObjectItem(js, "type")->valuestring:"";
        if(tmp_Type != NULL && tmp_Type != nullptr)
        {
            memset(obj.type, 0, sizeof(obj.type));
            memcpy(obj.type, tmp_Type, strlen(tmp_Type));
        }
        const char* tmp_userid = cJSON_GetObjectItem(js, "userid")!=NULL?cJSON_GetObjectItem(js, "userid")->valuestring:"";
        if(tmp_userid != NULL && tmp_userid != nullptr)
        {
            memset(obj.userid, 0, sizeof(obj.userid));
            memcpy(obj.userid, tmp_userid, strlen(tmp_userid));
        }

        //解析数组
        cJSON* arr_js = cJSON_GetObjectItem(js, "data");
        if(arr_js == NULL || arr_js == nullptr)
        {
            std::cout<<"array analysis error!"<<std::endl;
            return 0;
        }
        int arr_size = cJSON_GetArraySize(arr_js);//获取数组成员个数
        cJSON* arr_item = arr_js->child;//获取子对象
        if(arr_item == NULL || arr_item == nullptr)
        {
            std::cout<<"analysis array child error!"<<std::endl;
            return 0;
        }
        for(int i=0; i < arr_size ;i++)
        {
            
            MQTTCLIENT::MapMarkerInfoData info;
            memset(&info, 0, sizeof(info));

            const char* tmp_source = cJSON_GetObjectItem(arr_item, "source")!=NULL?cJSON_GetObjectItem(arr_item, "source")->valuestring:"";
            if(tmp_source != NULL && tmp_source != nullptr)
            {
                memset(info.source, 0, sizeof(info.source));
                memcpy(info.source, tmp_source, strlen(tmp_source));
            }
            info.type = cJSON_GetObjectItem(arr_item, "type")!=NULL?cJSON_GetObjectItem(arr_item, "type")->valueint:0;
            
            //map marker info  start
            cJSON* MapMarkerInfo_js = cJSON_GetObjectItem(arr_item, "markerInfo");
            if(MapMarkerInfo_js != NULL && MapMarkerInfo_js != nullptr)
            {
                const char* tmp_timestampAndUserId = cJSON_GetObjectItem(MapMarkerInfo_js, "timestampAndUserId")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "timestampAndUserId")->valuestring:"";
                if(tmp_timestampAndUserId != NULL && tmp_timestampAndUserId != nullptr)
                {
                    memset(info.info.timestampAndUserId, 0, sizeof(info.info.timestampAndUserId));
                    strcpy(info.info.timestampAndUserId, tmp_timestampAndUserId);
                }
                
                info.info.latitude = cJSON_GetObjectItem(MapMarkerInfo_js, "latitude")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "latitude")->valuedouble:0;
                info.info.longitude = cJSON_GetObjectItem(MapMarkerInfo_js, "longitude")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "longitude")->valuedouble:0;

                const char* tmp_markerUrl = cJSON_GetObjectItem(MapMarkerInfo_js, "markerUrl")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "markerUrl")->valuestring:"";
                if(tmp_markerUrl != NULL && tmp_markerUrl != nullptr)
                {
                    memset(info.info.markerUrl, 0, sizeof(info.info.markerUrl));
                    memcpy(info.info.markerUrl, tmp_markerUrl, sizeof(tmp_markerUrl));
                }

                //map marker detail info  start
                cJSON* MapMarkerInfoData_js = cJSON_GetObjectItem(MapMarkerInfo_js, "markerDetailInfo");
                if(MapMarkerInfoData_js != NULL && MapMarkerInfoData_js != nullptr)
                {
                    const char* tmp_camp = cJSON_GetObjectItem(MapMarkerInfoData_js, "camp")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "camp")->valuestring:"";
                    if(tmp_camp != NULL && tmp_camp != nullptr)
                    {
                        memset(info.info.mdiInfo.camp, 0, sizeof(info.info.mdiInfo.camp));
                        strcpy(info.info.mdiInfo.camp, tmp_camp);
                    }
                    const char* tmp_targetName = cJSON_GetObjectItem(MapMarkerInfoData_js, "targetName")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "targetName")->valuestring:"";
                    if(tmp_targetName != NULL && tmp_targetName != nullptr)
                    {
                        memset(info.info.mdiInfo.targetName, 0, sizeof(info.info.mdiInfo.targetName));
                        memcpy(info.info.mdiInfo.targetName, tmp_targetName, strlen(tmp_targetName));
                    }
                    
                    // info.info.mdiInfo.longitude = cJSON_GetObjectItem(MapMarkerInfoData_js, "longitude")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "longitude")->valuedouble:0;

                    info.info.mdiInfo.longitude = stod(cJSON_GetObjectItem(MapMarkerInfoData_js, "longitude")->valuestring);
                    std::cout<<"info.info.mdiInfo.longitude"<<info.info.mdiInfo.longitude<<std::endl;



                    info.info.mdiInfo.latitude = stod(cJSON_GetObjectItem(MapMarkerInfoData_js, "latitude")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "latitude")->valuestring:"");

                    info.info.mdiInfo.targetCount = cJSON_GetObjectItem(MapMarkerInfoData_js, "targetCount")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "targetCount")->valuedouble:0;

                    const char* tmp_targetDirection = cJSON_GetObjectItem(MapMarkerInfoData_js, "targetDirection")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "targetDirection")->valuestring:"";
                    if(tmp_targetDirection != NULL && tmp_targetDirection != nullptr)
                    {
                        memset(info.info.mdiInfo.targetDirection, 0, sizeof(info.info.mdiInfo.targetDirection));
                        memcpy(info.info.mdiInfo.targetDirection, tmp_targetDirection, strlen(tmp_targetDirection));
                    }
                    
                    
                    info.info.mdiInfo.targetSpeed = cJSON_GetObjectItem(MapMarkerInfoData_js, "targetSpeed")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "targetSpeed")->valuedouble:0;

                    info.info.mdiInfo.targetState = cJSON_GetObjectItem(MapMarkerInfoData_js, "targetState")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "targetState")->valueint:0;

                    info.info.mdiInfo.isWeapon = cJSON_GetObjectItem(MapMarkerInfoData_js, "isWeapon")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "isWeapon")->valueint:0;

                    info.info.mdiInfo.mHitRadius = cJSON_GetObjectItem(MapMarkerInfoData_js, "mHitRadius")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "mHitRadius")->valueint:0;

                    const char* tmp_targetType = cJSON_GetObjectItem(MapMarkerInfoData_js, "targetType")!=NULL?cJSON_GetObjectItem(MapMarkerInfoData_js, "targetType")->valuestring:"";
                    if(tmp_targetType != nullptr)
                    {
                        memset(info.info.mdiInfo.targetType, 0, sizeof(info.info.mdiInfo.targetType));
                        strcpy(info.info.mdiInfo.targetType, tmp_targetType);
                    }

                }

                //map marker detail info  end

                info.info.jbMarkerCode = stod(cJSON_GetObjectItem(MapMarkerInfo_js, "jbMarkerCode")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "jbMarkerCode")->valuestring:"");

                const char* tmp_jbColor = cJSON_GetObjectItem(MapMarkerInfo_js, "jbColor")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "jbColor")->valuestring:"";
                if(tmp_jbColor != NULL && tmp_jbColor != nullptr)
                {
                    memset(info.info.jbColor, 0, sizeof(info.info.jbColor));
                    memcpy(info.info.jbColor, tmp_jbColor, strlen(tmp_jbColor));
                }
                const char* tmp_setOption = cJSON_GetObjectItem(MapMarkerInfo_js, "setOption")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "setOption")->valuestring:"";
                if(tmp_setOption != NULL && tmp_setOption != nullptr)
                {
                    memset(info.info.setOption, 0, sizeof(info.info.setOption));
                    memcpy(info.info.setOption, tmp_setOption, strlen(tmp_setOption));
                }
                

                //info.info.addMarkerTime = cJSON_GetObjectItem(MapMarkerInfo_js, "addMarkerTime")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "addMarkerTime")->valueint:0;

                info.info.addMarkerTime = cJSON_GetObjectItem(MapMarkerInfo_js, "addMarkerTime")!=NULL?cJSON_GetNumberValue(cJSON_GetObjectItem(MapMarkerInfo_js, "addMarkerTime")):0;

                info.info.delMarkerTime = cJSON_GetObjectItem(MapMarkerInfo_js, "delMarkerTime")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "delMarkerTime")->valueint:0;

                info.info.updateMarkerTime = cJSON_GetObjectItem(MapMarkerInfo_js, "updateMarkerTime")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "updateMarkerTime")->valueint:0;

                const char* tmp_publisherUserId = cJSON_GetObjectItem(MapMarkerInfo_js, "publisherUserId")!=NULL?cJSON_GetObjectItem(MapMarkerInfo_js, "publisherUserId")->valuestring:"";
                if(tmp_publisherUserId != NULL && tmp_publisherUserId != nullptr)
                {
                    memset(info.info.publisherUserId, 0, sizeof(info.info.publisherUserId));
                    memcpy(info.info.publisherUserId, tmp_publisherUserId, strlen(tmp_publisherUserId));
                }
                
            }
            
            //map marker info  end
            obj.vect.push_back(info);
            arr_item = arr_item->next;//跳到下一个对象
        }

        
        //清楚字符串
        cJSON_Delete(js);

        return 0;
    }

    //态势分析JSON转结构体
    const int Common::JSON_ConvertTo_AnalysisSendST(const char* str, const uint32_t len, ST_ANALYSISSEND_RCV &obj)
    {
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }

        cJSON* js = cJSON_Parse(str);
        if(js == NULL || js == nullptr)
        {
            std::cout<<"JSON_ConvertTo_AnalysisSendST failed where str is not json format!!!"<<std::endl;
            return -1;
        }
        else
        {
            std::cout<<cJSON_Print(js)<<std::endl;
        }

        memset(&obj, 0, sizeof(obj));
        //解析字符串
        const char* tmp_Type = cJSON_GetObjectItem(js, "type")!=NULL?cJSON_GetObjectItem(js, "type")->valuestring:"";
        if(tmp_Type != NULL && tmp_Type != nullptr)
        {
            memset(obj.type, 0, sizeof(obj.type));
            memcpy(obj.type, tmp_Type, strlen(tmp_Type));
        }
        
        const char* tmp_userid = cJSON_GetObjectItem(js, "userid")!=NULL?cJSON_GetObjectItem(js, "userid")->valuestring:"";
        if(tmp_userid != NULL && tmp_userid != nullptr)
        {
            memset(obj.userid, 0, sizeof(obj.userid));
            memcpy(obj.userid, tmp_userid, strlen(tmp_userid));
        }


        ST_ANALYSISSEND_INFO obj_info;
        cJSON* data_js = cJSON_GetObjectItem(js, "data");
        if(data_js != NULL && data_js != nullptr)
        {

            const char* tmp_source = cJSON_GetObjectItem(data_js, "source")!=NULL?cJSON_GetObjectItem(data_js, "source")->valuestring:"";
            if(tmp_source != NULL && tmp_source != nullptr)
            {
                memset(obj_info.source, 0, sizeof(obj_info.source));
                memcpy(obj_info.source, tmp_source, strlen(tmp_source));
            }

            obj_info.time = cJSON_GetObjectItem(data_js, "time")!=NULL?cJSON_GetObjectItem(data_js, "time")->valueint:0;

        }

        obj.data = obj_info;
        
        //清楚字符串
        cJSON_Delete(js);

        return 0;
    }

    //情报融合JSON转结构体
    const int Common::JSON_ConvertTo_FusionST(const char* str, const uint32_t len, ST_ANALYSISFUSION_RCV &obj)
    {
        if(str == NULL || str == nullptr || len <= 0)
        {
            return -1;
        }

        cJSON* js = cJSON_Parse(str);
        if(js == NULL || js == nullptr)
        {
            std::cout<<"JSON_ConvertTo_AnalysisSendST failed where str is not json format!!!"<<std::endl;
            return -1;
        }
        else
        {
            std::cout<<cJSON_Print(js)<<std::endl;
        }

        memset(&obj, 0, sizeof(obj));

        const char* tmp_zip = cJSON_GetObjectItem(js, "zip")!=NULL?cJSON_GetObjectItem(js, "zip")->valuestring:"";
        if(tmp_zip != NULL && tmp_zip != nullptr)
        {
            memset(obj.zip, 0, sizeof(obj.zip));
            memcpy(obj.zip, tmp_zip, strlen(tmp_zip));
        }


        return 0;
    }


    
    //获目录遍历，获取当前目录下文件
    void Common::getFiles(const string path, vector<string>& files )  
    { 
        DIR* d = opendir(path.c_str());
        if(d == NULL)
        {
            std::cout<<"save file path is NULL"<<std::endl;
            return;
        }

        std::cout<<std::endl<<"path:"<<path<<std::endl;
        struct dirent* entry;
        while((entry=readdir(d)) != NULL)
        {
            std::string dPath(path);
            dPath.append(entry->d_name);

            std::cout<<"entry->d_name:"<<entry->d_name<<std::endl;

            //文件名称长度不小于4字节
            if(strlen(entry->d_name) >= 4)
            {
                //处理图片类型文件
                if((strncmp(entry->d_name+strlen(entry->d_name)-4, ".jpg", 4) == 0) 
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".JPG", 4) == 0) 
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".png", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".PNG", 4) == 0)
                    )
                {
                    std::cout<<"files vector add picture file:"<<dPath<<endl;
                    //添加图片文件到vector
                    files.push_back(dPath);
                }
                //处理视频类型文件
                else if((strncmp(entry->d_name+strlen(entry->d_name)-4, ".mp4", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".MP4", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".Mp4", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".mP4", 4) == 0)
                    )
                {
                    std::cout<<"files vector add video file:"<<dPath<<endl;
                    //添加图片文件到vector
                    files.push_back(dPath);

                }
                //音频文件
                else if((strncmp(entry->d_name+strlen(entry->d_name)-4, ".wav", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".WAV", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".mp3", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".MP3", 4) == 0)
                    )
                {
                    std::cout<<"files vector add audio file:"<<dPath<<endl;
                    //添加图片文件到vector
                    files.push_back(dPath);

                }
                //文本文件
                else if((strncmp(entry->d_name+strlen(entry->d_name)-4, ".txt", 4) == 0)
                    || (strncmp(entry->d_name+strlen(entry->d_name)-4, ".TXT", 4) == 0)
                    )
                {
                    std::cout<<"files vector add text file:"<<dPath<<endl;
                    //添加图片文件到vector
                    files.push_back(dPath);
                }
                else
                {
                    //删除非jpg、png文件
                    if(remove(dPath.c_str()) == 0)
                    {
                        std::cout<<"Remove file:"<<dPath<<"succeed!"<<std::endl;
                    }
                    else
                    {
                        std::cout<<"Remove file:"<<dPath<<"fail!"<<std::endl;
                    }
                }
            }
            else
            {
                //.和..文件跳过
                if((strncmp(entry->d_name+strlen(entry->d_name)-1, ".", 1) == 0) 
                    || (strncmp(entry->d_name+strlen(entry->d_name)-2, "..", 2) == 0))
                {
                    continue;
                }

                //删除非视频、图片、音频、文本文件
                if(remove(dPath.c_str()) == 0)
                {
                    std::cout<<"Remove file:"<<dPath<<"succeed!"<<std::endl;
                }
                else
                {
                    std::cout<<"Remove file:"<<dPath<<"fail!"<<std::endl;
                }
            }
        }

        closedir(d);
        
    }

    
    struct mosquitto * Common::GetMosquittoObj()
    {
        return Common::mosq;
    }

	

}//end namespace MQTTCLIENT
