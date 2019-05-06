#pragma once
/************************************************************************/
/*@brief:this is only supported lightweight HTTP request						*/
/*	     if u need 2 download heavy stuff , please choose YNetwork		*/
/*		 https://github.com/what951006/Libcurl-Wrapper					*/
/*@author:yantao                                                        */
/*@contact:what951006@163.com											*/		
/************************************************************************/
	
enum YSocketReplyError
{
	YRE_NOERROR,			//No error
	YRE_UNKNOWN,			//default error
	YRE_QUERY_IP,			//getaddrinfo ,parse domain to IP error
	YRE_SOCKET_INIT,		//Socket initialize error
	YRE_SOCKET_BLOCK,		//Set Socket block or non-block error
	YRE_SOCKET_CONNECT,		//Socket connect API error
	YRE_SOCKET_SENDRECV,	//Socket Send Received Error
	YRE_REDIRECTION,		//Redirection error
	YRE_TRANS_ENCODE,		//Transfer-Encoding: chunked ,sorry, did not support this way now
	YRE_400,				//reply 400
	YRE_404,				//reply 400
	YRE_CONTENT,			//Content-Length:0
};
class YSocketReply
{
public:
	
	virtual YSocketReplyError getError() = 0 ;

	virtual const char* getData() = 0 ;

	virtual unsigned int getLength() = 0 ;

};

class YSocketRequest
{
public:
	/*don't forget to add \r\n at the end*/
	virtual void setHeader(const char * header) = 0;

};


class YSocketHttp
{
public:
	virtual YSocketRequest *getRequest() = 0;

	virtual YSocketReply* syncGet(const char *url/*utf-8*/) = 0;

	virtual YSocketReply* syncPost(const char *url/*utf-8*/ , const char* post_data/*utf-8*/,int len) = 0;

};