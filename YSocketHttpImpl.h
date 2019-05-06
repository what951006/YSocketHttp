#pragma once
#include "YSocketHttp.h"

#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")


#include <vector>
#include <map>
#include <string>

class YSocketRequestImp : public YSocketRequest
{
public:
	enum RequestType
	{
		RT_GET,
		RT_POST
	};

	YSocketRequestImp();

	void reset();

	std::string toString(const std::string & query_str, const std::string &domain);

	void setRequestType(YSocketRequestImp::RequestType type = RT_GET);

	RequestType getRequestType();

	virtual void setHeader(const char * header) override;

	void setPostData(const std::string &post_data);

	std::string getPostData();

private:

	std::vector<std::string> headers_;
	RequestType request_type_ = RT_GET;
	std::string  post_data_;
};


class YSocketReplyImp : public YSocketReply
{
public:
	void reset();
	
	virtual YSocketReplyError getError() override;


	virtual const char* getData() override;


	virtual unsigned int getLength() override;

	std::string data_;
	YSocketReplyError error_ = YRE_UNKNOWN;
	int content_size_=0;
};


class YSocketHttpImpl : public YSocketHttp
{
public:
	YSocketHttpImpl();
	~YSocketHttpImpl();

	void closeSocket();

	virtual YSocketRequest * getRequest() override;

	virtual YSocketReply* syncGet(const char *url) override;

	virtual YSocketReply* syncPost(const char *url, const char* post_data ,int len) override;

protected:

	void downloadToReply(YSocketRequestImp &req , SOCKET &socket, YSocketReplyImp &reply);

	bool setNonBlockWin32(bool block);

	bool initSocket(const std::string& host_name, const short port);

	YSocketRequestImp request_;
	YSocketReplyImp reply_;
	
	SOCKET	socket_ = INVALID_SOCKET;
	//std::string domain_;

private:
	class CHttpHeader
	{
	public:
		CHttpHeader(const char* pHeader);
		CHttpHeader(const std::string& strHeader);
		virtual		~CHttpHeader(void);
		//********************************
		//�ⲿ�ӿ�
		//********************************
		//��ȡHTTP�汾
		const char*		GetHttpVersion()const { return m_szHttpVersion; }
		//��ȡHTTP����������ֵ
		const int		GetReturnValue()const { return m_uReturnValue; }
		//��ȡHTTP�����ַ�
		const char*		GetContent()const { return m_strContent.c_str(); }
		//��ȡĳһ������Ӧ��ֵ
		std::string		GetValue(const std::string& strKey);

	protected:
		//����HTTPͷ�ṹ
		bool	Revolse(const std::string& strHeader);
	private:
		//HTTP�������汾
		char		m_szHttpVersion[9];
		//����ֵ
		int		m_uReturnValue;
		//����˵���ַ���
		std::string	m_strContent;
		//���صļ�ֵ��
		std::map<std::string, std::string>	m_ValueMap;
	};
};

