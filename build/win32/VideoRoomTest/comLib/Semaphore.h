// Semaphore.h: interface for the Semaphore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEMAPHORE_H__BCFA5B9B_53AC_41E1_AEAB_F72E8898CC87__INCLUDED_)
#define AFX_SEMAPHORE_H__BCFA5B9B_53AC_41E1_AEAB_F72E8898CC87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "comLib/comdefine.h"

#ifndef WIN32
#include <semaphore.h>
#endif

#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX  INT_MAX
#endif

namespace comLib
{

class Semaphore
{
public:
	Semaphore(unsigned long initCount, unsigned long maxCount);
	~Semaphore();
	void Post();
	bool Wait();
	bool Wait(int nTimeout);//timeout return false,-- millisecond
protected:
#ifdef WIN32
	HANDLE m_Semaphore;
#else
	sem_t m_Semaphore;
#endif
};

inline Semaphore::Semaphore(unsigned long initCount, unsigned long maxCount)
{
#ifdef WIN32
	m_Semaphore = CreateSemaphore(NULL, initCount, maxCount, NULL);
#else
	sem_init(&m_Semaphore, 0, initCount);
#endif
}

inline Semaphore::~Semaphore()
{
#ifdef WIN32
	CloseHandle(m_Semaphore);
	m_Semaphore = NULL;
#else
	sem_destroy(&m_Semaphore);
#endif
}

//inline void Semaphore::Clear()
//{
//#ifdef WIN32
//	CloseHandle(m_Semaphore);
//	m_Semaphore = CreateSemaphore(NULL, 0, m_maxCount, NULL);
//#else
//	sem_init(&m_Semaphore, 0, 0);
//#endif
//}

inline void Semaphore::Post()
{
#ifdef WIN32
	ReleaseSemaphore(m_Semaphore, 1, NULL);
#else
	sem_post(&m_Semaphore);
#endif
}

inline bool Semaphore::Wait()
{
#ifdef WIN32
	return ( WAIT_OBJECT_0 == WaitForSingleObject(m_Semaphore, INFINITE));
#else
	return (0 == sem_wait(&m_Semaphore));
#endif
}

inline bool Semaphore::Wait(int nTimeout)
{
#ifdef WIN32
	return (WAIT_OBJECT_0 == ::WaitForSingleObject(m_Semaphore, nTimeout));
#else
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	int timeoutSec = nTimeout/1000;
	int timeoutMilliSec = nTimeout%1000;
	ts.tv_sec += timeoutSec;
	int tsMilliSec = ts.tv_nsec/1000/1000; //nanosecond to microsecond to millisecond  
	if ( tsMilliSec + timeoutMilliSec < 1000 )
	{
		ts.tv_nsec = ts.tv_nsec + timeoutMilliSec*1000*1000;
	}
	else
	{
		ts.tv_sec += 1;
		ts.tv_nsec =  (tsMilliSec + timeoutMilliSec -1000)*1000*1000;		
	}
	return ( 0 == sem_timedwait(&m_Semaphore, &ts));
#endif
}

} //namespace comLib

#endif // !defined(AFX_SEMAPHORE_H__BCFA5B9B_53AC_41E1_AEAB_F72E8898CC87__INCLUDED_)
