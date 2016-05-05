#include "logger.h"
#include "xlogwriter.h"
#include <stdarg.h>
#include <errno.h>

#define TMP_BUF_SIZE 4096

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
    char buf[TMP_BUF_SIZE];
	//memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
    int successCharacterNum = 0;
#if defined(LOG_OS_WIN)
    successCharacterNum = _vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#else
    successCharacterNum = vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#endif
	va_end(args);

    if ( successCharacterNum > 0 )
    {
        if ( successCharacterNum <= TMP_BUF_SIZE-10 )
            p += successCharacterNum;
        else
            p += TMP_BUF_SIZE-10;
    }
    else
    {
        memcpy(p, "logerror", 8);
        p += 8;
    }

	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';

	std::string logStr;
    logStr.assign(buf, TMP_BUF_SIZE);
	return XLogWriter::Instance()->write_log("INFO", logStr.c_str());
}

int logi(const char* format, ...)
{
    char buf[TMP_BUF_SIZE];
	//memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
    int successCharacterNum = 0;
#if defined(LOG_OS_WIN)
    successCharacterNum = _vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#else
    successCharacterNum = vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#endif
	va_end(args);

    if ( successCharacterNum > 0 )
    {
        if ( successCharacterNum <= TMP_BUF_SIZE-10 )
            p += successCharacterNum;
        else
            p += TMP_BUF_SIZE-10;
    }
    else
    {
        memcpy(p, "logerror", 8);
        p += 8;
    }

	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';

	std::string logStr;
    logStr.assign(buf, TMP_BUF_SIZE);
	return XLogWriter::Instance()->write_log("INFO",logStr.c_str());
}

int logw(const char* format, ...)
{
    char buf[TMP_BUF_SIZE];
	//memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
    int successCharacterNum = 0;
#if defined(LOG_OS_WIN)
    successCharacterNum = _vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#else
    successCharacterNum = vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#endif
	va_end(args);

    if ( successCharacterNum > 0 )
    {
        if ( successCharacterNum <= TMP_BUF_SIZE-10 )
            p += successCharacterNum;
        else
            p += TMP_BUF_SIZE-10;
    }
    else
    {
        memcpy(p, "logerror", 8);
        p += 8;
    }

	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';

	std::string logStr;
    logStr.assign(buf, TMP_BUF_SIZE);
	return XLogWriter::Instance()->write_log("WARN",logStr.c_str());
}

int loge(const char* format, ...)
{
    char buf[TMP_BUF_SIZE];
	//memset(buf,0,sizeof(buf));
	char *p = buf;
	va_list args;
	va_start(args,format);
    int successCharacterNum = 0;
#if defined(LOG_OS_WIN)
    successCharacterNum = _vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#else
    successCharacterNum = vsnprintf(p,TMP_BUF_SIZE-10,format,args);
#endif
	va_end(args);

    if ( successCharacterNum > 0 )
    {
        if ( successCharacterNum <= TMP_BUF_SIZE-10 )
            p += successCharacterNum;
        else
            p += TMP_BUF_SIZE-10;
    }
    else
    {
        memcpy(p, "logerror", 8);
        p += 8;
    }

	*p++ = '\r';
	*p++ = '\n';
	*p++ = '\0';

	std::string logStr;
    logStr.assign(buf, TMP_BUF_SIZE);
	return XLogWriter::Instance()->write_log("ERROR",logStr.c_str());
}

}
