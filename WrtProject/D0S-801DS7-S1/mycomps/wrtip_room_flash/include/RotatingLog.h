// RotatingLog.h: interface for the CRotatingLog class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_mutex.h>
class CRotatingLog  
{
        long GetFileLength();
public:
        CRotatingLog(const char* File, long MaxLines, long LineLength);
        virtual ~CRotatingLog();
        bool AddLine(const char* Line);
        bool GetDumpLine(long &count, char* line,int* islock,int* isread);
        long GetDumpStart(bool bForward);
        bool DelLine(long count);
        bool LockLine(long count,int lock,int read);
        long GetValidLineCount();
        bool FileStreamfflush(char* File);

protected:
        bool  m_bForward;
        FILE* m_file;
        long  m_linelength;
        long  m_maxlines;
        char *buffer;
        SDL_mutex* m_mutex;
};


