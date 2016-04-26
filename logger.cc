#include "logger.h"
#include "xlogwriter.h"
#include <stdarg.h>

namespace xlog
{

int init(const xlog::InitInfo& ini)
{
	return XLogWriter::Instance()->init(ini);
}

int release()
{
	XLogWriter::Release();
	return xlog::RES_NO_ERROR;
}

int log(const char *format, ...)
{
	char buf[4096];
	memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
#if defined(LOG_OS_WIN)
	p += _vsnprintf(p,sizeof(buf)-10,format,args);
#else
	p += vsnprintf(p,sizeof(buf)-10,format,args);
#endif
	va_end(args);
//#if defined(LOG_OS_WIN)
	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';
//#endif
	std::string logStr;
	logStr.assign(buf,sizeof(buf));
	return XLogWriter::Instance()->write_log("INFO", logStr.c_str());
}

int logi(const char* format, ...)
{
	char buf[4096];
	memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
#if defined(LOG_OS_WIN)
	p += _vsnprintf(p,sizeof(buf)-10,format,args);
#else
	p += vsnprintf(p,sizeof(buf)-10,format,args);
#endif
	va_end(args);
//#if defined(LOG_OS_WIN)
	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';
//#endif
	std::string logStr;
	logStr.assign(buf,sizeof(buf));
	return XLogWriter::Instance()->write_log("INFO",logStr.c_str());
}

int logw(const char* format, ...)
{
	char buf[4096];
	memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
#if defined(LOG_OS_WIN)
	p += _vsnprintf(p,sizeof(buf)-10,format,args);
#else
	p += vsnprintf(p,sizeof(buf)-10,format,args);
#endif
	va_end(args);
//#if defined(LOG_OS_WIN)
	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';
//#endif
	std::string logStr;
	logStr.assign(buf,sizeof(buf));
	return XLogWriter::Instance()->write_log("WARN",logStr.c_str());
}

int loge(const char* format, ...)
{
	char buf[4096];
	memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
#if defined(LOG_OS_WIN)
	p += _vsnprintf(p,sizeof(buf)-10,format,args);
#else
	p += vsnprintf(p,sizeof(buf)-10,format,args);
#endif
	va_end(args);
//#if defined(LOG_OS_WIN)
	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';
//#endif
	std::string logStr;
	logStr.assign(buf,sizeof(buf));
	return XLogWriter::Instance()->write_log("ERROR",logStr.c_str());
}

}
