/*****************
 * project:MQTTClient
 * source:common.h
 * author:FEX
 * time:2022-08-20
 * description:
 * 公共处理类定义
 * copyright:
 *
 *
 * ***************/

#ifndef COMMON_H
#define COMMON_H

#include "../model/Model.h"

using namespace std;
using namespace MQTTCLIENT;

namespace MQTTCLIENT
{
	class Common
	{
	private:
			//状态机标志，表示当前执行状态，当前设备运行状态（无效、就绪、正在记录数据、正在回放数据、正在查询目录）
			static MQTTCLIENT::enum_Status m_RunStatus;
			
            //接收信号，程序退出。
            static bool SIGQUITFLAG;

	public:
		Common();
		//~Common();
		
		//读取配置文件中的配置信息，key-要查询的关键字，value-查询到的值，length-value能容纳的最大字符串长度
		static const int GetValueByKeyFromConfig(const char* key, char* value, const unsigned int length);
		
		//read from  fd
		//输入参数：fd-文件描述符，buffer-存放数据的缓冲区，maxSize-最大读取数据量，timeout-超时时长（ms）
		//输出参数：size-本次读取数据量
		static const int64_t Read(const int fd, uint8_t* buffer, const uint64_t maxSize, uint64_t* size, const uint32_t timeout);

		//write to fd
		//输入参数：fd-文件描述符，buffer-存放数据的缓冲区，maxSize-最大发送数据量，timeout-超时时长（ms）
		//输出参数：size-本次写入的数据量
		static const int64_t Write(const int fd, const uint8_t* buffer, const uint64_t maxSize, uint64_t* size, const uint32_t timeout);
		
		//判断system指令是否执行成功
		//输入参数：result调用system函数返回的值
		//返回值：0~成功，-1~失败
		static const int SystemCheck(int result);
		
		//异或校验，校验值占一个字节（8位）
		//输入参数：buffer-校验数据（含末尾校验值位置），len-需要校验的数据长度（不包括末尾校验值）
		//输出参数：校验值
		//返回值：异或校验结果
		static const uint8_t XOR(uint8_t* buffer, const uint32_t len);
		
		//异或校验检查
		//输入参数：buffer-校验检查数据（包含最后一个校验字节在内），len-数据长度（包含最后一个校验字节在内）
		//输出参数:无
		//返回值：0~成功，-1~失败
		static const int ChechXOR(const uint8_t* buffer, const uint32_t len);
		
		//校验计算
		//输入参数：buffer-校验数据（含末尾校验值位置），len-需要校验的数据长度（不包括末尾校验值）
		//输出参数：无
		//返回值：校验值
		static const int GetCheckSB(uint8_t* buffer, const int32_t len);
		
		//校验检查
		//输入参数：buffer-校验检查数据（包含最后四个校验字节在内），len-数据长度（包含最后四个校验字节在内）
		//输出参数：无
		//返回值：0~成功，-1~失败
		static const int CheckSB(const uint8_t* buffer, const int32_t len);
		
		
		//设置状态信息
		//输入参数：status状态值
		//输出参数：无
		//返回值：0~成功，-1~失败
		static const int SetStatus(const MQTTCLIENT::enum_Status status);
		
	
		//获取状态信息
		//输入参数：无
		//输出参数：无
		//返回值：状态标志值
		static const MQTTCLIENT::enum_Status GetStatus();

        
        //将TCP服务消息转换成字符串
        //msg-消息结构体，str-转换后的字符串保存位置，maxLen-str缓存的最大容量
        //返回str发送内容的总长度
        static int ST_ConvertTo_Char(const ST_MSG &msg, char* str, const uint32_t maxLen);


        //设置退出信号
        static const int SetSigFlag(const bool flag);
        //获取退出信号
        static const bool GetSigFlag(); 


        //加校验计算，将buffer的数据累加起来计算数值，最后对结果取反
        static const uint8_t CheckSum(const uint8_t *buffer, const uint32_t num);


        //将威胁等级返回的结构体数据转换成JSON数据
        static const int ThreatST_ConvertTo_JSON(const ST_THREATLEVEL_RECV &obj, char* str, const uint32_t len);
        //将情报融合返回的结构体数据转换成JSON数据
        static const int FusionST_ConvertTo_JSON(const ST_ANALYSISFUSION_RECV &obj, char* str, const uint32_t len);
        //将态势分析返回的结构体数据转换成JSON数据
        static const int AnalysisSendST_ConvertTo_JSON(const ST_ANALYSISSEND_RECV &obj, char* str, const uint32_t len);
        //将态势融合返回的结构体数据转换成JSON数据
        static const int TSSendST_ConvertTo_JSON(const ST_TSSEND_RECV &obj, char* str, const uint32_t len);
        


        //态势融合JSON转结构体
        static const int JSON_ConvertTo_TSSendST(const char* str, const uint32_t len, ST_TSSEND_RCV &obj);
        //态势分析JSON转结构体
        static const int JSON_ConvertTo_AnalysisSendST(const char* str, const uint32_t len, ST_ANALYSISSEND_RCV &obj);
        //情报融合JSON转结构体
        static const int JSON_ConvertTo_FusionST(const char* str, const uint32_t len, ST_ANALYSISFUSION_RCV &obj);

        //获取path目录下的文件
        static void getFiles(const string path, vector<string>& files);

        static struct mosquitto * GetMosquittoObj();


        	
        //全局对象定义
        //MQTT消息返回主题
        const static char top_Rcv[STR_LEN];

        //消息队列操作对象
        static struct mosquitto *mosq;

        //存储所有目标信息
        static std::vector<MQTTCLIENT::MapMarkerInfo> m_markers;

        
        //遍历filePath目录，针对每个png和jpg文件，使用目标识别算法处理每一个图片文件，将识别结果返回
        static vector<string> files;



        //
        static map<string, string> TargetTypeDic;

        //
        static map<string, string> TargetTypeMap;


	};//end class Common

}//end namespace MQTTCLIENT

#endif // COMMON_H
