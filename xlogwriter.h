#pragma once
#include "logger.h"
#include <string>
#include <vector>

#if defined(LOG_OS_WIN)
#include <Windows.h>
#else
#include <pthread.h>
#endif

class XLogWriter
{
public:
	static XLogWriter* Instance();
	static void Release();

public:
	int init(const xlog::InitInfo& ini);
	int write_log(const std::string& levelstr, const std::string& log);

	void run(); //Async mode, thread

private:
	XLogWriter();
	~XLogWriter();

	int create_log_file();
	int start(); //Async mode, thread
	int stop(); //Async mode, thread
	void sleep(int millsecs);
	std::string timestampstr();

private:
	typedef std::vector<std::string>	log_cache_t;
	static XLogWriter*	m_pIns;
	xlog::InitInfo	m_ini;
	log_cache_t	m_logCache;
	FILE*		m_pLogFile;
	size_t		m_iLogSize;

#if defined(LOG_OS_WIN)
	HANDLE		m_hThread;
#else
	pthread_t	m_hThread;
#endif
};
