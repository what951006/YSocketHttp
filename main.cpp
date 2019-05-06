#include "YSocketHttpImpl.h"
#include "YCommon.h"


int main()
{

	YSocketHttpImpl impl;
	impl.getRequest()->setHeader("Accept-Language:en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7\r\n");
	impl.getRequest()->setHeader("Accept-Encoding:gzip, deflate, br\r\n");
	impl.getRequest()->setHeader("User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 Safari/537.36\r\n");
	auto reply = impl.syncGet("http://www.baidu.com/");

	if (reply->getError() == YRE_NOERROR)
	{
		unsigned int result =reply->getLength();
		std::string data(reply->getData(), result);
		std::wstring wstr =  Utf2U(data);

		std::string data2 = U2A(wstr);
	//	printf();
	}
	
	return 0;
}
