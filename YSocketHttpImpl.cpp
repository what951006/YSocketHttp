#include "YSocketHttpImpl.h"
#include <ws2tcpip.h>
#include "YCommon.h"


YSocketHttpImpl::CHttpHeader::CHttpHeader(const char* pHeader)
	:m_uReturnValue(0)
{
	Revolse(std::string(pHeader));
}

YSocketHttpImpl::CHttpHeader::CHttpHeader(const std::string& strHeader)
	: m_uReturnValue(0)
{
	Revolse(strHeader);
}

YSocketHttpImpl::CHttpHeader::~CHttpHeader(void)
{
}

std::string YSocketHttpImpl::CHttpHeader::GetValue(const std::string& strKey)
{
	std::string strResult;
	std::map<std::string, std::string>::const_iterator itor;
	itor = m_ValueMap.find(strKey);
	if (itor != m_ValueMap.end())
		strResult = itor->second;
	return strResult;
}

bool YSocketHttpImpl::CHttpHeader::Revolse(const std::string& strHeader)
{
	int nStartPos = 0, nFindPos = 0, nLineIndex = 0;
	std::string strLine, strKey, strValue;
	do
	{
		nFindPos = strHeader.find("\r\n", nStartPos);
		if (-1 == nFindPos)
			strLine = strHeader.substr(nStartPos, strHeader.size() - nStartPos);
		else
		{
			strLine = strHeader.substr(nStartPos, nFindPos - nStartPos);
			nStartPos = nFindPos + 2;
		}
		if (0 == nLineIndex)//第一行
		{
			strncpy_s(m_szHttpVersion, strLine.c_str(), 8);
			m_szHttpVersion[8] = '\0';
			if (strcmp(m_szHttpVersion, "HTTP/1.1") != 0)
				return false;
			int nSpace1 = strLine.find(" ");
			int nSpace2 = strLine.find(" ", nSpace1 + 1);
			m_uReturnValue = atoi(strLine.substr(nSpace1 + 1, nSpace2 - nSpace1 - 1).c_str());
			m_strContent = strLine.substr(nSpace2 + 1, strLine.size() - nSpace2 - 1);
			nLineIndex++;
			continue;
		}
		int nSplit = strLine.find(": ");
		strKey = strLine.substr(0, nSplit);
		strValue = strLine.substr(nSplit + 2, strLine.size() - nSplit - 2);
		std::pair<std::string, std::string> data;
		data.first = strKey;
		data.second = strValue;
		m_ValueMap.insert(data);
		nLineIndex++;
	} while (nFindPos != -1);
	return true;
}




YSocketRequestImp::YSocketRequestImp()
{
	
}

void YSocketRequestImp::reset()
{
	headers_.swap(std::vector<std::string>());

}

void YSocketRequestImp::setRequestType(RequestType type /*= RT_GET*/)
{
	request_type_ = type;
}

void YSocketRequestImp::setHeader(const char * header)
{
	if(header)
		headers_.push_back(header);
}

std::string YSocketRequestImp::toString(const std::string & query_str, const std::string &domain)
{
	std::string header_string;
	//Get /Post
	switch (request_type_)
	{
	case YSocketRequestImp::RT_GET:
	{
		header_string = "GET ";
	}
		break;
	case YSocketRequestImp::RT_POST:
	{
		header_string = "POST ";
	}
		break;
	default:
		break;
	}

	header_string += query_str;
	header_string += " HTTP/1.1\r\n";

	//Accept
	header_string += "Accept: */*\r\n";

	//host
	header_string += "Host: ";
	header_string += domain;
	header_string += "\r\n";

	//user
	for (auto item : headers_)
	{
		header_string += item;
	}

	//Connection
	header_string += "Connection: Keep-Alive\r\n";

	//end flag
	header_string += "\r\n";
	return header_string;
}

YSocketRequestImp::RequestType YSocketRequestImp::getRequestType()
{
	return request_type_;
}

void YSocketRequestImp::setPostData(const std::string &post_data)
{
	post_data_ = post_data;
}

std::string YSocketRequestImp::getPostData()
{
	return post_data_;
}

///////////////////////////////////////////////////


void YSocketReplyImp::reset()
{
	error_ = YRE_UNKNOWN;
	data_ = "";
}

YSocketReplyError YSocketReplyImp::getError()
{
	return error_;
}

const char* YSocketReplyImp::getData()
{
	return data_.c_str();
}

unsigned int YSocketReplyImp::getLength()
{
	return data_.length();
}
/////////////////////////////////////////////////////////////////////


YSocketHttpImpl::YSocketHttpImpl()
{
	WSADATA data;
	WSAStartup(0x0202, &data);
}


YSocketHttpImpl::~YSocketHttpImpl()
{
	WSACleanup();
}

void YSocketHttpImpl::closeSocket()
{
	if (INVALID_SOCKET != socket_)
	{
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
	}
}

YSocketReply* YSocketHttpImpl::syncGet(const char *url)
{
	try
	{
		closeSocket();
		request_.setRequestType(YSocketRequestImp::RT_GET);
		std::string strHostName, strPage;
		u_short uPort = 80;
		MyParseUrlA(url, strHostName, strPage, uPort);
		if (initSocket(strHostName, uPort))
		{
			std::string && send_request = request_.toString(strPage, strHostName);
			//send_request = "GET /speed/download/QAQGAME_INSTALL_V3.2.1.94_official.exe HTTP/1.1\r\nAccept: */*\r\nHost: obs.qaqgame.com\r\nConnection: Keep-Alive\r\n\r\n";
			if (SOCKET_ERROR >= send(socket_, send_request.c_str(), send_request.size(), 0))
			{
				throw YRE_SOCKET_SENDRECV;
			}
			downloadToReply(request_,socket_, reply_);
		}
	}
	catch (YSocketReplyError error)
	{
		reply_.error_ = error;
	}
	closeSocket();
	return &reply_;
}

YSocketReply* YSocketHttpImpl::syncPost(const char *url, const char* post_data, int len)
{
	try
	{
		closeSocket();
		request_.setRequestType(YSocketRequestImp::RT_POST);
		request_.setPostData(std::string(post_data, len));
		std::string strHostName, strPage;
		u_short uPort = 80;
		MyParseUrlA(url, strHostName, strPage, uPort);
		if (initSocket(strHostName, uPort))
		{
			std::string && send_request = request_.toString(strPage, strHostName);
			//send_request = "GET /speed/download/QAQGAME_INSTALL_V3.2.1.94_official.exe HTTP/1.1\r\nAccept: */*\r\nHost: obs.qaqgame.com\r\nConnection: Keep-Alive\r\n\r\n";
			if (SOCKET_ERROR >= send(socket_, send_request.c_str(), send_request.size(), 0))
			{
				throw YRE_SOCKET_SENDRECV;
			}
			downloadToReply(request_, socket_, reply_);
		}
	}
	catch (YSocketReplyError error)
	{
		reply_.error_ = error;
	}
	closeSocket();
	return &reply_;
}

YSocketRequest * YSocketHttpImpl::getRequest()
{
	return &request_;
}

void YSocketHttpImpl::downloadToReply(YSocketRequestImp &req, SOCKET &socket, YSocketReplyImp &reply)
{
	/*first need 2 get header and get content size*/
	char buff[4096];
	int received_size = recv(socket, buff, sizeof(buff), 0);
	if (SOCKET_ERROR >= received_size)
	{
		throw YRE_SOCKET_SENDRECV;
	}

	std::string str(buff, received_size);
	int http_header_endpos = str.find("\r\n\r\n");

	CHttpHeader header(str.substr(0, http_header_endpos));
	int nHttpValue = header.GetReturnValue();

	if (400 == nHttpValue)
	{
		throw YRE_400;
	}
	else if (404 == nHttpValue)//文件不存在
	{
		throw YRE_404;
	}
	else if (nHttpValue > 300 && nHttpValue < 400)//重定向
	{
		string strReLoadUrl = header.GetValue("location");
		if (strReLoadUrl.find("http://") != std::string::npos)
		{
			reply.reset();
			switch (req.getRequestType())
			{
			case YSocketRequestImp::RT_GET:
			{
				syncGet(strReLoadUrl.c_str());
			}
			case YSocketRequestImp::RT_POST:
			{
				std::string &&post_data = request_.getPostData();
				syncPost(strReLoadUrl.c_str(), post_data.c_str(), post_data.size());
			}
			}
		}
		else
			throw YRE_REDIRECTION;

		return;
	}

	if (!header.GetValue("Transfer-Encoding").empty())
	{
		throw YRE_TRANS_ENCODE;
	}

	reply.content_size_ = atoi(header.GetValue("Content-Length").c_str());
	if (reply.content_size_ <= 0)
	{
		throw YRE_CONTENT;
	}
	reply.data_ = str.substr(http_header_endpos + 4);

	while (true)
	{
		if (0 == reply.content_size_ || reply.content_size_ == reply.data_.size())
		{
			break;
		}
		int received_size = recv(socket, buff, sizeof(buff), 0);
		if (SOCKET_ERROR >= received_size)
		{
			throw YRE_SOCKET_SENDRECV;
		}
		reply.data_.append(buff, received_size);
	}
	
	reply.error_ = YRE_NOERROR;
	closeSocket();
}

bool YSocketHttpImpl::setNonBlockWin32(bool block)
{
	unsigned long ulEnable = block ? 1 : 0;
	int iRet = ioctlsocket(socket_, FIONBIO, &ulEnable);
	return (SOCKET_ERROR != iRet);
}


bool YSocketHttpImpl::initSocket(const std::string& host_name, const short port)
{
	bool bResult = false;
	try
	{
		char port_str[32];
		sprintf_s(port_str, sizeof(port_str), "%d", port);

		struct addrinfo  *ailist = nullptr;
		struct addrinfo  add_info;
		add_info.ai_flags = AI_CANONNAME;
		add_info.ai_family = AF_INET; //ipv4 AF_INET
		add_info.ai_socktype = SOCK_STREAM;//SOCK_STREAM
		add_info.ai_protocol = IPPROTO_TCP;//IPPROTO_TCP
		add_info.ai_addrlen = 0;
		add_info.ai_canonname = NULL;
		add_info.ai_addr = NULL;
		add_info.ai_next = NULL;
		if ((getaddrinfo(host_name.c_str(), port_str, &add_info, &ailist)) != 0)
			throw YRE_QUERY_IP;

		char ip_addr_buff[INET_ADDRSTRLEN];
		PCSTR  ip_addr = nullptr;
		sockaddr_in * dest_info = nullptr; //ipv4
		for (auto aip = ailist; aip != nullptr; aip = aip->ai_next)
		{
			if (aip->ai_family == AF_INET)
			{/*Get IPV4 address*/
				dest_info = (sockaddr_in *)ailist->ai_addr;
				ip_addr = inet_ntop(AF_INET, &dest_info->sin_addr, ip_addr_buff, sizeof(ip_addr_buff));
				break;
			}
		}

		if (!ip_addr)
			throw YRE_QUERY_IP;

		socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == socket_)
			throw YRE_SOCKET_INIT;
		int nSec = 1000 * 10;
		//10秒收发超时
		setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (char *)&nSec, sizeof(int));
		setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nSec, sizeof(int));

		if (!setNonBlockWin32(true))//设置非阻塞模式
			throw YRE_SOCKET_BLOCK;

		int iRet = connect(socket_, (SOCKADDR*)dest_info, sizeof(sockaddr_in));

		int iErrorNo = SOCKET_ERROR;
		int iLen = sizeof(int);
		// 如果getsockopt返回值不为0，则说明有错误出现  
		if (SOCKET_ERROR == iRet && 0 != getsockopt(socket_, SOL_SOCKET, SO_ERROR, (char*)&iErrorNo, &iLen))
		{
			throw YRE_SOCKET_CONNECT;
		}

		fd_set fsWrite;
		TIMEVAL tv;
		tv.tv_sec = 5; //select 5 seconds timeout
		tv.tv_usec = 0;

		FD_ZERO(&fsWrite);
		FD_SET(socket_, &fsWrite);
		// 如果集合fsWrite中的套接字有信号，则说明连接成功，此时iRet的返回值大于0  
		iRet = select(0, NULL, &fsWrite, NULL, &tv);
		if (iRet > 0)
		{
			if (!setNonBlockWin32(false))//设置非阻塞模式
				throw YRE_SOCKET_BLOCK;
		}
		else
		{
			throw YRE_SOCKET_CONNECT;
		}
		bResult = true;
	}
	catch (YSocketReplyError error)
	{
		reply_.error_ = error;
	}
	catch (...)
	{

	}
	return bResult;
}