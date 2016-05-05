#include "xlogwriter.h"
#include "logger.h"
#include "util/xmutex.h"
#include "util/utility.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <sys/stat.h>
#if defined(LOG_OS_WIN)
#include <time.h>
#include <sys/utime.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
#endif

Mutex k_mutex;
XLogWriter* XLogWriter::m_pIns = NULL;

#if defined(LOG_OS_WIN)
DWORD WINAPI _TaskThread(LPVOID pParam)
#else
void* _TaskThread(void* pParam)
#endif
{
	if ( NULL == pParam )
#if defined(LOG_OS_WIN)
		return 0;
#else
		return NULL;
#endif
	XLogWriter* pThread = (XLogWriter*)pParam;
	pThread->run();
#if defined(LOG_OS_WIN)
	return 0;
#else
	return NULL;
#endif
}

XLogWriter::XLogWriter()
: m_pLogFile(NULL)
, m_hThread(NULL)
, m_iLogSize(0)
{}

XLogWriter::~XLogWriter()
{
	this->stop();
	if (m_pLogFile)
	{
		fclose(m_pLogFile);
		m_pLogFile = NULL;
	}
}

XLogWriter* XLogWriter::Instance()
{
	if ( NULL == m_pIns )
	{
		k_mutex.lock();
		if ( NULL == m_pIns )
		{
			m_pIns = new XLogWriter();
		}
		k_mutex.unlock();
	}
	return m_pIns;
}

void XLogWriter::Release()
{
	k_mutex.lock();
	if (m_pIns)
	{
		delete m_pIns;
		m_pIns = NULL;
	}
	k_mutex.unlock();
}

int XLogWriter::init(const xlog::InitInfo& ini)
{
	m_ini = ini;
    delete_expire_log_files();
	int ret = create_log_file();
	if ( xlog::RES_NO_ERROR != ret )
		return ret;
	
	if ( xlog::InitInfo::LOG_MODE_ASYNC == ini.log_mode )
		return this->start();
	else
		return xlog::RES_NO_ERROR;
}

int XLogWriter::write_log(const std::string& levelstr, const std::string& log)
{
	std::string format_log;
	format_log = timestampstr() + " " + levelstr + " " + log;
	k_mutex.lock();
	if (!m_pLogFile)
	{
		k_mutex.unlock();
		return xlog::RES_ERROR_NO_LOGFILE;
	}

	if ( (m_iLogSize >> 20) >= m_ini.log_max_size_MB ) //too large for a single file
	{
		if (m_pLogFile)
		{
			fclose(m_pLogFile);
			m_pLogFile = NULL;
		}
		m_iLogSize = 0;
		create_log_file(); //create a new one to continue
	}
	
	if ( xlog::InitInfo::LOG_MODE_SYNC == m_ini.log_mode )
	{
		fwrite(format_log.c_str(), format_log.size(), 1, m_pLogFile);
		fflush(m_pLogFile);
		m_iLogSize += format_log.size();
		if ( m_ini.also_log_to_stderr ) //log to console for debug
		{
			#if defined(LOG_OS_WIN)
				OutputDebugStringA(format_log.c_str());
			#else
				fwrite(format_log.c_str(), format_log.size(), 1, stderr);
			#endif
		}
	}
	else
	{
		if ( m_logCache.size() < m_ini.log_cache_limit )
		{
			m_logCache.push_back(format_log);
		}
	}
	k_mutex.unlock();
	return xlog::RES_NO_ERROR;
}

void XLogWriter::run()
{
	while (true)
	{
		k_mutex.lock();
		if ( m_logCache.empty() )
		{
			k_mutex.unlock();
			this->sleep(1);
			continue;;
		}
		for ( log_cache_t::iterator io = m_logCache.begin(); io != m_logCache.end(); ++io )
		{
            fwrite(io->c_str(), io->size(), 1, m_pLogFile);
			m_iLogSize += io->size();
			if ( m_ini.also_log_to_stderr )
			{
				#if defined(LOG_OS_WIN)
					OutputDebugStringA(io->c_str());
				#else
					fwrite(io->c_str(), io->size(), 1, stderr);
				#endif
			}
		}
		m_logCache.clear();
		fflush(m_pLogFile);
		k_mutex.unlock();
	}
}

int XLogWriter::create_log_file()
{
	std::ostringstream logFileNameStream;
	logFileNameStream.fill('0');
	if ( !m_ini.log_dir.empty() )
		logFileNameStream << m_ini.log_dir << "/";

	time_t _nowsec = time(NULL);
	struct tm* pTm = localtime(&_nowsec);
	logFileNameStream << xutil_gethostname() << "." << xutil_getpid() << "."
	<< pTm->tm_year+1900
	<< std::setw(2) << pTm->tm_mon+1
	<< std::setw(2) << pTm->tm_mday
	<< "-"
	<< std::setw(2) << pTm->tm_hour
	<< std::setw(2) << pTm->tm_min
	<< std::setw(2) << pTm->tm_sec << ".log";
	//OutputDebugStringA(logFileNameStream.str().c_str());
	m_pLogFile = fopen(logFileNameStream.str().c_str(), "wb+");
	if (!m_pLogFile)
		return xlog::RES_ERROR_OPEN_LOG_FAIL;

	return xlog::RES_NO_ERROR;
}

int XLogWriter::delete_expire_log_files()
{
    std::vector<std::string> fileList;
#if defined(LOG_OS_WIN)
	_finddata_t dirFileInfo;
	std::string filePattern;
	filePattern.append(m_ini.log_dir).append("\\*.*");
	long handle = _findfirst(filePattern.c_str(), &dirFileInfo);
	if ( -1L == handle )
		return xlog::RES_NO_ERROR;
	while ( 0 == _findnext(handle, &dirFileInfo) )
	{
		if ( strstr(dirFileInfo.name, ".log") )
			fileList.push_back(dirFileInfo.name);
	}
#else
    DIR* pDir = opendir(m_ini.log_dir.c_str());
    if (!pDir)
        return xlog::RES_NO_ERROR;
    struct dirent *pDirEntry = NULL;
    while( pDirEntry = readdir(pDir) )
    {
        if ( strstr(pDirEntry->d_name, ".log") )
            fileList.push_back(pDirEntry->d_name);
    }
    closedir(pDir);
#endif
    std::map<time_t, std::string> ctime_2_filename_map;
    for ( std::vector<std::string>::iterator io = fileList.begin(); io != fileList.end(); ++io )
    {
        std::string filename;
        filename.append(m_ini.log_dir).append("/").append(io->c_str());
        struct stat fileinfo;
        if ( 0 != stat(filename.c_str(), &fileinfo) )
            continue;
#if defined(LOG_OS_APPLE)
        ctime_2_filename_map[fileinfo.st_birthtimespec.tv_sec] = filename;
#else
		ctime_2_filename_map[fileinfo.st_ctime] = filename;
#endif
    }
    if (ctime_2_filename_map.size() < m_ini.log_store_file_num)
        return xlog::RES_NO_ERROR;
    int idx = 0;
    for ( std::map<time_t, std::string>::reverse_iterator io = ctime_2_filename_map.rbegin(); io != ctime_2_filename_map.rend(); ++io )
    {
        if ( ++idx >= m_ini.log_store_file_num )
        {
            std::string filename = io->second;
            remove(filename.c_str());
        }
    }
    return xlog::RES_NO_ERROR;
}

int XLogWriter::start()
{
#if defined(LOG_OS_WIN)
	//m_hThread = (HANDLE)_beginthread(_TaskThread, 0, this); 
	m_hThread = CreateThread(NULL,0,_TaskThread,this,0,NULL);
	if (!m_hThread)
		return xlog::RES_START_THREAD_FAIL;
	SetThreadPriority(m_hThread,THREAD_PRIORITY_ABOVE_NORMAL);
	OutputDebugStringA("start a thread to write log file");
#else
    int ret = pthread_create(&m_hThread,NULL,&_TaskThread,this);
	if (ret != 0)
		return xlog::RES_START_THREAD_FAIL;
#endif
	return xlog::RES_NO_ERROR;
}

int XLogWriter::stop()
{
	if ( !m_hThread )
		return xlog::RES_NO_ERROR;

#if defined(LOG_OS_WIN)
	DWORD ret = WaitForSingleObject(m_hThread,2000);
	switch (ret)
	{
	case WAIT_OBJECT_0:
		break;
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		{
			//LOG("Thread::stop, wait thread failed, error=%d, id=%u",GetLastError(),GetThreadId(m_hThread));
		}
		break;
	}
	CloseHandle(m_hThread);
	m_hThread = NULL;
#elif defined(LOG_OS_APPLE)
	pthread_cancel(m_hThread);
#elif defined(LOG_OS_ANDROID)
    pthread_join(m_hThread, NULL);
#endif
	return xlog::RES_NO_ERROR;
}

void XLogWriter::sleep(int millsecs)
{
#if defined(LOG_OS_WIN)
	::Sleep(millsecs);
#else
	usleep(millsecs*1000);
#endif
}

std::string XLogWriter::timestampstr()
{
	char chTmp[100];
	struct tm* p;
	long _tvsec = 0, _tvusec = 0;
	xutil_systime(_tvsec, _tvusec);
	time_t _nowsec = _tvsec;
	//time_t _nowsec = time(NULL);
	p = localtime(&_nowsec);
	//p->tm_year = p->tm_year + 1900;
	p->tm_mon = p->tm_mon + 1;
#if defined(LOG_OS_WIN)
	_snprintf(chTmp,sizeof(chTmp),"%02d-%02d %02d:%02d:%02d.%06d",p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,_tvusec);
#else
	snprintf(chTmp,sizeof(chTmp),"%02d-%02d %02d:%02d:%02d.%06d",p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,_tvusec);
#endif
	return chTmp;
}

