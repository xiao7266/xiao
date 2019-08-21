// RotatingLog.cpp: implementation of the CRotatingLog class.
//
// Written by PJ Arends
//
//////////////////////////////////////////////////////////////////////

#include "RotatingLog.h"
#include <errno.h>
#include <assert.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Constructor
//    Constructs the CRotatingLog class, and opens the log file.
//
// Parameters
//    File       [in] - the name of the log file
//    MaxLines   [in] - the maximum number of lines the file is to contain
//    LineLength [in] - the length of each line in the file
//
// Returns
//    Nothing
//
// Note
//    Throws a CFileException if unable to open the file 'File'
//
//////////////////////////////////////////////////////////////////////

CRotatingLog::CRotatingLog(const char *File, long MaxLines, long LineLength)
{
        m_maxlines = MaxLines;
        if (m_maxlines < 1)
                m_maxlines = 1;
        if (m_maxlines > 9999)
                m_maxlines = 9999;
        m_linelength = LineLength;
        m_mutex = 0;
        m_mutex = SDL_CreateMutex();
        assert(m_mutex);
        buffer = new char[m_linelength + 8]; 
        m_file = 0;
        m_file = fopen(File,"r+b");
        if(m_file == 0){
                printf("open %s failed \n",File);
        }

}

bool CRotatingLog::FileStreamfflush(char* File){
        if(m_file){
                fflush(m_file);
                fclose(m_file);
                m_file = 0;
                m_file = fopen(File,"r+b");
                if(m_file == 0){
                        printf("open %s failed2 FileStreamfflush,errno =%d \n",File,errno);
                }
        }
        return true;
}

//////////////////////////////////////////////////////////////////////
//
// Destructor
//    Closes the file
//
// Parameters
//    None
//
// Returns
//    Nothing
//
//////////////////////////////////////////////////////////////////////

CRotatingLog::~CRotatingLog()
{
        //	m_file.Close();	
        SDL_DestroyMutex(m_mutex);
        m_mutex = 0;
        if(buffer)
                delete [] buffer;
        buffer = 0;		
        if(m_file)
                fclose(m_file);
        m_file = 0;	   

}

//////////////////////////////////////////////////////////////////////
//
// AddLine
//    Adds a line of text to the file
//
// Parameters
//    Line [in] - the text string to be added to the file
//
// Returns
//    TRUE on success, FALSE on failure. Use GetLastError() to get 
//    error information on failure.
//
// Note
//    If the 'Line' is longer than m_linelength, it will be truncated
//
//////////////////////////////////////////////////////////////////////

bool CRotatingLog::AddLine(const char* Line)
{
        SDL_LockMutex(m_mutex);
        if(m_file == 0){
                SDL_UnlockMutex(m_mutex);
                return false;
        }

        bool returnvalue = true;
        memset (buffer, ' ', m_linelength+6);
        buffer[m_linelength+6] = '\n';
        buffer[m_linelength + 7] = 0x00;
        fseek(m_file,0,SEEK_SET);
        char str[6]={0};
        long lastline = 0;
        int lock = 0;
        int read = 0;
        if (fgets(str,6,m_file))
        {
                lastline = atoi ((char*)str);
                if (lastline == 0){
                        returnvalue = false;
                        printf("str = %s \n",str);
                        SDL_UnlockMutex(m_mutex);
                        return returnvalue;
                }				
        }
        lastline++;
        if (lastline > m_maxlines)
                lastline = 1;
        fseek(m_file,0,SEEK_SET);
        long position  = (lastline - 1) * (m_linelength + 8) + 6;
        fseek(m_file,position,SEEK_CUR);
        fputs(buffer,m_file);
        fseek(m_file,0,SEEK_SET);
        fseek(m_file,position,SEEK_CUR);
        memset(buffer,0,m_linelength+6);
        sprintf(buffer,"%s  %02d %02d",Line,lock,read);
        fputs(buffer,m_file);
        fseek(m_file,0,SEEK_SET);
        memset(str,0,6);
        sprintf(str,"%04d\n",lastline);
        fputs(str,m_file);
        //int ret = fflush(m_file);
        //	printf("ret = %d ,ferror = %d,errno =%d\n",ret,ferror(m_file),errno);
        SDL_UnlockMutex(m_mutex);
        printf("add ok \n");
        return returnvalue;
}

//////////////////////////////////////////////////////////////////////
//
// GetDumpStart
//    Gets the line count of the oldest line in the file. Use this value
//    as the intial 'count' in GetDumpLine. bForward sets the direction 
//    GetDumpLine() goes to retrieve lines from the file
//
// Parameters
//    bForward [in] - the order to retrieve lines :
//                    TRUE  - oldest to newest
//                    FALSE - newest to oldest
//                                  
//
// Returns
//    The line count
//    zero if the file is empty
//    -1 if an error occurs, use GetLastError() to get error information.
//
//////////////////////////////////////////////////////////////////////

long CRotatingLog::GetDumpStart(bool bForward)
{
        if(m_file == 0)
                return false;
        m_bForward = bForward;
        long line = 0;
        fseek(m_file,0,SEEK_SET);
        char str[6]={0};
        if (fgets(str,6,m_file))
        {
                line = atoi((char*)str);
                if (line == 0){
                        line = -1;
                        return line;
                }
                if (m_bForward)
                {
                        line++;
                        if (line > m_maxlines)
                                line = 1;
                        else
                        {
                                long pos = line * (m_linelength + 8) + 6;
                                if (pos > GetFileLength())
                                        line = 1;
                        }
                }
        }

        return line;
}

//////////////////////////////////////////////////////////////////////
//
// GetDumpLine
//    Gets the line of text at the 'count', and updates count to point
//    to the next line.
//
// Parameters
//    count [in/out] - in - the line to get
//                     out - the next line to get
//    line      [in] - a CString to receive the line of text
//
// Returns
//    TRUE on success, FALSE on failure. Use GetLastError() to get error
//    information on failure
//
// Note
//    The count will loop back to the beginning, so you will have gone
//    through the file when count equals the value returned from GetDumpStart 
//
//////////////////////////////////////////////////////////////////////
static char *Util_rtrim(char *s) {

        char *t= s;
        while(*s) s++;
        while(*--s==' ' || *s=='\t' || *s=='\r' || *s=='\n') *s= 0;

        return t;

}


bool CRotatingLog::GetDumpLine(long &count, char *line,int* islock,int* isread)
{
        SDL_LockMutex(m_mutex);
        if(count == 0){
                SDL_UnlockMutex(m_mutex);
                return false;
        }
        if(m_file == 0){
                SDL_UnlockMutex(m_mutex);
                return false;
        }		 
        fseek(m_file,0,SEEK_SET);
        fseek(m_file,(count - 1) * (m_linelength + 8) + 6,SEEK_CUR);
        memset(buffer,0,m_linelength+4);
        fgets(buffer,m_linelength+4,m_file);
        buffer = Util_rtrim(buffer);
        sscanf(buffer,"%s  %02d %02d",line,islock,isread);
        if (m_bForward)
        {
                count++;
                if (count > m_maxlines)
                        count = 1;
                else
                {
                        long pos = count * (m_linelength + 8) + 6;
                        if (pos > GetFileLength()+1)
                                count = 1;
                }
        }
        else
        {
                count--;
                if (count == 0){
                        count = (GetFileLength() - 6) / (m_linelength + 8);
                        if((GetFileLength() - 6) % (m_linelength + 8))
                                count ++;
                }
        }
        SDL_UnlockMutex(m_mutex);
        return true;
}

long CRotatingLog::GetFileLength(){

        if(m_file){
                fseek(m_file,0,SEEK_SET);
                fseek(m_file,0,SEEK_END);
                return ftell(m_file);
        }
        return 0;
}

bool CRotatingLog::DelLine(long count){
        int lock=-1;
        int read = 0;
        SDL_LockMutex(m_mutex);
        if(m_file == 0){
                SDL_UnlockMutex(m_mutex);
                return false;
        }
        fseek(m_file,0,SEEK_SET);
        fseek(m_file,(count - 1) * (m_linelength + 8) + 6,SEEK_CUR);
        memset(buffer,'\0',m_linelength+6);
        fgets(buffer,m_linelength+6,m_file);
        buffer = Util_rtrim(buffer);
        sscanf(buffer,"%s  %02d %02d",buffer,&lock,&read);
        if(lock == 0){
                fseek(m_file,0,SEEK_SET);
                fseek(m_file,(count - 1) * (m_linelength + 8) + 6,SEEK_CUR);	    
                memset(buffer,'\0',m_linelength+6);
                buffer[m_linelength+6] = '\n';
                buffer[m_linelength + 7] = 0x00;
                fwrite(buffer,1,m_linelength+8,m_file);
        }
        SDL_UnlockMutex(m_mutex);
        return true;
}
bool CRotatingLog::LockLine(long count,int lock,int read){
        SDL_LockMutex(m_mutex);
        if(m_file == 0){
                SDL_UnlockMutex(m_mutex);
                return false;
        }
        fseek(m_file,0,SEEK_SET);
        fseek(m_file,(count - 1) * (m_linelength + 8) + 6,SEEK_CUR);
        memset(buffer,'\0',m_linelength+8);
        fgets(buffer,m_linelength+6,m_file);
        buffer = Util_rtrim(buffer);
        buffer[strlen(buffer)-6] = '\0';
        sprintf(buffer,"%s  %02d %02d",buffer,lock,read);
        fseek(m_file,0,SEEK_SET);
        fseek(m_file,(count - 1) * (m_linelength + 8) + 6,SEEK_CUR);
        fputs(buffer,m_file);
        SDL_UnlockMutex(m_mutex);
        return true;    
}

long CRotatingLog::GetValidLineCount(){

        long tmpcount =0;
        long tmp = 0;
        int lock = 0;
        int read = 0;
        SDL_LockMutex(m_mutex);
        tmp = GetDumpStart(false);
        if (tmp > 0)
        {
                long temp = tmp;
                do
                {
                        memset(buffer,0,m_linelength+8);
                        if (GetDumpLine(tmp, (char*)buffer,&lock,&read)){
                                if(strlen(buffer) !=0 )
                                        tmpcount++;
                        }else
                                break;
                } while (tmp != temp);
        }else
                tmpcount = 0;
        SDL_UnlockMutex(m_mutex);       
        return tmpcount;
}