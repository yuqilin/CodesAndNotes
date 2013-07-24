#ifndef __NTLOG_H_2013_6_24__
#define __NTLOG_H_2013_6_24__

#ifdef NTLOG_EXPORTS
#define NTLOG_API __declspec(dllexport)
#else
#define NTLOG_API __declspec(dllimport)
#pragma comment(lib, "NTLog.lib")
#endif

/************************************************************************
	ntlog大小不变!	定义版本,每次更改加1,版本号用来判断预留空间的使用情况
************************************************************************/
#define NTLOG_VERSION           0x00026200          //@@@@version

#define NTLOG_GROUP_DEFAULT "NTLOG"

// 默认宏定义
#ifndef NTLOG_GROUP
#define NTLOG_GROUP         "NTLOG"
#pragma message("warning: undefined macro 'NTLOG_GROUP', default : [#define NTLOG_GROUP \"" NTLOG_GROUP "\"]")
#endif
// 默认配置目录
#ifndef NTLOG_CONFIG_DIR
#define NTLOG_CONFIG_DIR    "C:\\NTLOG_CONFIG\\"
//#pragma message("warning: undefined macro 'NTLOG_CONFIG_DIR', default : [#define NTLOG_CONFIG_DIR \"" NTLOG_CONFIG_DIR "\"]")
#endif

//Private Data
#define __TINI_SUFFIX _T(".ini")
#define __TTXT_SUFFIX _T(".txt")
#define __TNTLOG_CONFIG_FILE_PATH           __TNTLOG_CONFIG_DIR __TNTLOG_GROUP __TINI_SUFFIX
#define __TNTLOG_CONFIG_FILE_PATH2          __TNTLOG_CONFIG_DIR __TNTLOG_GROUP __TTXT_SUFFIX
#define __TNTLOG_CONFIG_FILE_PATH_DEFAULT   __TNTLOG_CONFIG_DIR __TNTLOG_GROUP_DEFAULT __TINI_SUFFIX
#define __TNTLOG_CONFIG_FILE_PATH_DEFAULT2  __TNTLOG_CONFIG_DIR __TNTLOG_GROUP_DEFAULT __TTXT_SUFFIX


#if (defined(UNICODE) || defined(_UNICODE))
#define TFUNCTION2(x)           L ## x
#define TFUNCTION(x)            TFUNCTION2(x)
#define __TFUNCTION__           TFUNCTION(__FUNCTION__)
#define TFUNCSIG2(x)            L ## x
#define TFUNCSIG(x)             TFUNCSIG2(x)
#define __TFUNCSIG__            TFUNCSIG(__FUNCSIG__)
#define TTIMESTAMP2(x)          L ## x
#define TTIMESTAMP(x)           TTIMESTAMP2(x)
#define __TTIMESTAMP__          TTIMESTAMP(__TIMESTAMP__)
#define TNTLOG_CONFIG_DIR2(x)   L ## x
#define TNTLOG_CONFIG_DIR(x)    TNTLOG_CONFIG_DIR2(x)
#define __TNTLOG_CONFIG_DIR     TNTLOG_CONFIG_DIR(NTLOG_CONFIG_DIR)
#define TNTLOG_GROUP2(x)        L ## x
#define TNTLOG_GROUP(x)         TNTLOG_GROUP2(x)
#define __TNTLOG_GROUP          TNTLOG_GROUP(NTLOG_GROUP)
#define __TNTLOG_GROUP_DEFAULT  TNTLOG_GROUP(NTLOG_GROUP_DEFAULT)
#else
#define __TFUNCTION__           __FUNCTION__
#define __TFUNCSIG__            __FUNCSIG__
#define __TTIMESTAMP__          __TIMESTAMP__
#define __TNTLOG_CONFIG_DIR     NTLOG_CONFIG_DIR
#define __TNTLOG_GROUP          NTLOG_GROUP
#define __TNTLOG_GROUP_DEFAULT  NTLOG_GROUP_DEFAULT
#endif

#define ENTER_FUNCTION_FLAG _T("→")
#define LEAVE_FUNCTION_FLAG _T("←")

//As long as shared memory can be no change in ntlog
#include <tchar.h>	        // _T
#include <windows.h>
#include <string>	        // std::string strstr
#include <vector>	        // std:vector
//#include <stdio.h>		// _vsntprintf()
#include <algorithm>	    // sort
#include <time.h>		    // time
#include  <sys/stat.h>	    // stat
//#include <stdlib.h>		// _tsplitpath
#include <OAIdl.h>
#include <MLang.h>          // codepage

#pragma comment(lib, "Version.lib")     // VerQueryValue
#pragma comment(lib, "Winmm.lib")       // timeGetTime


/************************************************************************
    class CNTLog
************************************************************************/
#pragma pack(push, 1)
class NTLOG_API CNTLog
{
public:
    typedef enum
    {
        LEVEL_TRACE,
        LEVEL_DEBUG,
        LEVEL_INFO,
        LEVEL_WARN,
        LEVEL_ERROR,
        LEVEL_FATAL,
        LEVEL_PROMPT, //打印提示信息, 内部使用
        LEVEL_OFF
    } TLevel;

public:
    void            NTTrace(LPCTSTR pszFormat, ...);
    void            NTDebug(LPCTSTR pszFormat, ...);
    void            NTInfo(LPCTSTR pszFormat, ...);
    void            NTWarn(LPCTSTR pszFormat, ...);
    void            NTError(LPCTSTR pszFormat, ...);
    void            NTFatal(LPCTSTR pszFormat, ...);
    void            NTLog(LPCTSTR pszLogFilePath,LPCTSTR pszFormat, ...); //打印提示信息, 内部使用
    void            NTHexDump(LPCVOID p, ULONG uSize, LPCTSTR pszPrompt = NULL, TLevel level = LEVEL_TRACE);
    BOOL            InstallNTLogIni(LPCTSTR szSrcIniAbsolutePath);

    CNTLog&         operator << (const bool _Val);
    CNTLog&         operator << (const TCHAR _Val);

#if (defined(UNICODE) || defined(_UNICODE))
    CNTLog&         operator << (const char _Val);
#else
    CNTLog&         operator << (const wchar_t  _Val);
#endif

    CNTLog&         operator << (const signed char _Val);
    CNTLog&         operator << (const unsigned char _Val);
    CNTLog&         operator << (const signed short _Val);

#if (defined _WCHAR_T_DEFINED && defined _NATIVE_WCHAR_T_DEFINED)
    //与 operator <<(WCHAR)冲突
    CNTLog&         operator << (const unsigned short _Val);
#endif

    //四字节整数
    /*
//#if (defined _WIN64)
    CNTLog&         operator << (const  unsigned int _W64 _Val);
//#endif
    */
    CNTLog&         operator << (const signed int  _Val);
    CNTLog&         operator << (const unsigned int  _Val);
    CNTLog&         operator << (const long  _Val);
    CNTLog&         operator << (const unsigned long  _Val);
    CNTLog&         operator << (const signed __int64  _Val);
    CNTLog&         operator << (const unsigned __int64 _Val);
    CNTLog&         operator << (const float _Val);
    CNTLog&         operator << (const double _Val);
    CNTLog&         operator << (const long double _Val);
    CNTLog&         operator << (const void * _Val);
    CNTLog&         operator << (const TCHAR * _Val) throw();

#if (defined(UNICODE) || defined(_UNICODE))
    CNTLog&         operator << (const char * _Val) throw();
#else
    CNTLog&         operator << (const wchar_t * _Val) throw();
#endif

    CNTLog&         operator << (REFGUID _Val)  throw();
    //tagVARIANT
    CNTLog&         operator << (const tagVARIANT _Val)  throw();
    //std::string
    CNTLog&         operator << (const std::basic_string<WCHAR> _Val)  throw();
    CNTLog&         operator << (const std::basic_string<CHAR> _Val)  throw();
    //从缓冲到输出
    void            NTLog4CXX_Flush(const TLevel level);

public:
    static CNTLog&  GetInstance(LPCTSTR pszCallFunName = NULL, LPCVOID = 0);
    static BOOL     IsModuleUnloading(LPCTSTR szFunction);
    static BOOL     IsLogValid(TLevel l);
    static BOOL     IsDebugValid();
    static BOOL     IsInfoValid(); 
    static BOOL     IsWarnValid(); 
    static BOOL     IsErrorValid();
    static BOOL     IsFatalValid();
    static BOOL     IsTRACEValid();
    static UINT     GetStringCodePage(const char* szData);
    static LPCTSTR  TraceAuto(LPCSTR pszFuncSig, DWORD dwEBP, BOOL bIsStatic = FALSE) throw();

private:
    CNTLog();
    ~CNTLog();

private:
    static BOOL	    UnicodeToMultiByte(const std::basic_string<WCHAR> &strSrc, std::string &strDst);
    static BOOL     MultiByteToUnicode(const std::string& strSrc, std::basic_string<WCHAR> &strDst);

    static LPCTSTR	 GetCurrentModuleVersion(void);
    static HINSTANCE GetCurrentModuleHandle(void);

private:	
    typedef	struct _FileInfo { 
        std::basic_string<TCHAR>	strFilePath_;
        FILETIME stFileTime_;
    } FileInfo;
    typedef std::pair<INT, FileInfo> PAIRPID2FILEINFO;
    static bool		FileCmp(const PAIRPID2FILEINFO &val1, const PAIRPID2FILEINFO &val2);
private:
    void			PrintModuleInit(void);
    LPCTSTR			GetCurrentProcessName(void);
    static LPCTSTR	GetCurrentModuleName(void);
    BOOL			CheckConfigFileChanged(void);
    BOOL			MakeDirectoryA(LPCSTR lpszDestDirA);
    BOOL			MakeDirectory(LPCTSTR lpszDestPath);
    void			ResetParams(BOOL bModuleInit = FALSE);
    BOOL			GetConfig(BOOL bModuleInit = FALSE);
    void			OutputLog(TLevel level, LPCTSTR pszFormat, va_list ap = NULL, LPCTSTR pszLogFilePath = NULL, BOOL bCheckFileChanged = TRUE) throw();
    void			WriteToLogFile(TLevel level, LPCTSTR pszFileLogMsg, DWORD dwLen, LPCTSTR pszLogFilePath) throw();
    BOOL			GetNthFilePathName(INT nFileCnt, LPTSTR pOleFilePathName, LPTSTR pNewFilePathName);
    BOOL			DeleteHistoryLogFiles() throw();
    void			DeleteAllFilesOfPid(const INT nPid);
    int				FindPidFromLogFilePath(LPCTSTR pszFilePath);
    void			NTPrompt(LPCTSTR pszFormat, ...); //打印提示信息, 内部使用

private:
    typedef enum
    {
        CT_STDCALL = 0,
        CT_CDECL_CALL = 1,
        CT_THISCALL = 2,
        CT_FASTCALL = 3,
        CT_STDTHISCALL,
        CT_CDECLTHISCALL,
        CT_OTHERCALL = -1
    } CALLTYPE_NTLOG;

    typedef enum
    {
        PT_INVALID = -1,
        PT_VOID,
        PT_bool, PT_boolREF,
        PT_INT, PT_UINT, PT_INT64,  PT_UINT64,
        PT_CHAR, PT_WCHAR, PT_CHARPOINT, PT_WCHARPOINT, PT_CHARREF, 
        PT_INT1, PT_UINT1, /*PT_INT1REF,*/ PT_UINT1REF, PT_INT2, PT_UINT2, PT_INT2REF, PT_UINT2REF, PT_INTREF, PT_UINTREF,
        PT_CHARPOINTREF, PT_WCHARREF, PT_WCHARPOINTREF,
        PT_INT64REF, PT_UINT64REF,
        PT_FLOAT, PT_DOUBLE,
        PT_PENDPOINT, PT_PEND, PT_PENDREF,
        PT_VARIANT, PT_VARIANTREF,
        PT_GUID, PT_GUIDREF
    } PARAM_TYPE;

    struct stParamTypeInfo 
    {
        PARAM_TYPE pt;
        INT nValidCommaPos;
    };

    static BOOL     GetParamInfo(LPCSTR pszFuncSig, BOOL bIsStatic, CALLTYPE_NTLOG& ct, std::vector<stParamTypeInfo>& vecIndex2TypeName, BOOL& bRetClass);
    static BOOL     GetParamTypeAndLen(LPCSTR pszRaw, PARAM_TYPE& pt, INT& nLen);
    static BOOL     TranslateCodePage(char ** pszValA);
    static DWORD    GetSafeValidValue(
        const PARAM_TYPE &tslog_eParamType,
        LPSTR tslog_szRegValue,
        DWORD tslog_nRegValue,
        DWORD tslog_nRegValue2 = 0,
        LPINT tslog_pnRegValue = NULL,
        LPINT tslog_pnRegValue2 = NULL,
        LPSTR tslog_pRegValueA = NULL,
        LPWSTR  tslog_pRegValueW = NULL,
        LPINT pnRet = NULL) throw();

public:
    class NTLOG_API CEnterLeaveFunCallBack
    {
    public:
        CEnterLeaveFunCallBack(LPCTSTR pszCallFunName, LPCTSTR pszFunction, LPCTSTR pszFuncSig,
            LPCVOID pvThis = 0, BOOL bIsSimple = FALSE, DWORD dwEBP = 0, LPCSTR pszFuncSigA = NULL);
        ~CEnterLeaveFunCallBack();
    private:
        LPCVOID m_pvThis;
        LPCTSTR m_pszFunction;	  // function name					CNTGHeapVector::Add
        LPCTSTR m_pszFuncSig;	  // prototype +  param				BOOL CNTGHeapVector::Add(LPVOID)
        LPCSTR	m_pszFuncSigA;
        LPCTSTR m_pszCallFunName; // prototype +  param + value		BOOL CNTGHeapVector::Add(LPVOID = 0x00000000)
    private:
        static const size_t OFFSET_LEAVE_FUNCTION_FLAG;
        DWORD	m_dwEBP;
    };

private:
#pragma pack(push, 1)
    class NTLOG_API CRelease
    {
        friend class CNTLog;
        friend class CNTLog::CEnterLeaveFunCallBack;
    private:
        volatile LONG  m_bIsModuleUnloading;
        HMODULE m_hLang;
        SYSTEM_INFO m_si;
        DWORD m_dwMainThreadID;
    public:
        CRelease(INT);
        ~CRelease( ); // 使用静态成员的析构函数，实现单一实例的释放
    };
#pragma pack(pop)

private:
#pragma pack(push, 1)
    typedef struct _tagNTGHeapVector
    {
        LPSTR pszData;
        _tagNTGHeapVector* pNext;
    } NTGHeapVector ,*PNTGHeapVector;
#pragma pack(pop)

#pragma pack(push, 1)
    class NTLOG_API CNTGHeapVector
    {
    public:
        CNTGHeapVector();
        void Add(PNTGHeapVector pNTGHeapVector);
        void Add(LPCTSTR lpszFilter);
        void Clear();
        BOOL Find(LPCTSTR lpszFilter);
        BOOL SubFind(LPCTSTR lpszFilter);

    private:	//size don't be changed
        PNTGHeapVector m_pNTGHeapVector;
        DWORD m_dwReserve[20];
    };
#pragma pack(pop)

private:
    static HANDLE	    s_hMutex;			        // 只用于同步初始化和终结化
    static HANDLE	    s_hFileMap;			        // 文件影射句柄
    static CNTLog *     s_pThis;			        // 影射视图指针
    static HINSTANCE    s_hInst;			        // 模块句柄
    static TCHAR        s_szModuleFileName[_MAX_PATH];
    static CRITICAL_SECTION	s_cs;
    static DWORD        s_dwTlsIndex;
    static DWORD        s_dwTlsIndex_this;
    static CRelease     s_release;			        // 用它的析构函数释放单一实例
    static LONG32       s_lVer;
    static HANDLE       s_hLogFile;
    static DWORD        s_dwLogFileCnt;
    static INT32        s_nModuleID;
    static volatile LONG s_lOperatorStarted;
    /////////////////////variable must have same size/////////////////////////
private:
    LONG32				_lVer;			        // 版本,当前为
    LONG32				_cbThis;			    // sizeof TSLOG
    volatile LONG     	_lRef;			        // 模块计数
    CRITICAL_SECTION	_cs;			        // 用于同步对文件影射内容的访问
    HANDLE				_hConfigFileChanged;    // 配置文件变化监视句柄
private:
    BOOL			    m_bDebugViewLog;	    // 是否打印DebugView日志
    BOOL			    m_bFileLog;			    // 是否打印文件日志

    TLevel			    m_eLevel;			    // 日志级别
    CNTGHeapVector	    m_ghvInclude;		    // 过滤项
    CNTGHeapVector	    m_ghvExclude;		    // 滤除项

    DWORD			    m_dwMaxLogFileSize;	    // 日志文件最大尺寸
    INT32			    m_nMaxLogFileCnt;	    // 日志文件数目限制
    INT32			    m_nMaxHistroyProcessCnt;// 历史日志进程数目限制

    HANDLE			    m_hLogFile;			    // 日志文件句柄
    INT32			    m_nLogFileCnt;		    // 第几个日志文件
    INT32			    m_nFileLogLineCnt;	    // 文件日志中的行号	
    DWORD			    m_dwCurFileSize;
    CHAR			    m_szLogFileDirA[MAX_PATH];	
    //DWORD			    m_dwReserve[20];		// V1.0 保留80个字节
    DWORD			    m_dwReserve[16];
    DWORD			    m_dwAllLogFileCnt;
    LPTSTR			    m_pszLogDataA;		    // 指向全局堆 当成char大小来看以防multi/unicode模块合在一起使用
    DWORD			    m_dwMaxLogDataSize;     // 全局堆大小,字符数! 非字节大小
    DWORD			    m_dwUsedSize;		    // 全局堆使用大小,字符数! 非字节大小

};
#pragma pack(pop)


//////////日志宏//////////

#ifdef NTLOG_ENABLE

#define _NTTRACE \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTTrace } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTTrace }
#define _NTDEBUG \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTDebug } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTDebug }
#define _NTINFO \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTInfo } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTInfo }
#define _NTWARN \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTWarn } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTWarn }
#define _NTERROR \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTError } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTError }
#define _NTFATAL \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTFatal } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTFatal }
#define _NTPROMPT \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTLog } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTLog  }
#define _NTHEXDUMP \
    __if_exists(this)       { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__, (LPVOID)this).NTHexDump } \
    __if_not_exists(this)   { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) CNTLog::GetInstance(__TFUNCTION__).NTHexDump }

#define _INSTALLNTLOGINI     CNTLog::GetInstance(NULL).InstallNTLogIni

//////////NTTRACEAUTO//////////

#pragma warning( disable : 4731 ) //"pointer": 框架指针寄存器"register"被内联程序集代码修改
#define  _NTTRACEAUTO() \
    DWORD ntlog_dwEBP = 0; \
    LPCTSTR ntlog_lpctszFuncSig = NULL; \
    { __asm mov dword ptr[ntlog_dwEBP], ebp } \
    __if_exists(this) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, FALSE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig, ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__ , (LPCVOID)this); } \
    __if_not_exists(this) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, TRUE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig, ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__ , NULL); } \
    if(!ntlog_lpctszFuncSig) {{__asm push ebp }{__asm pop ebp  }} // no reach, but absolutely necessarily. 

#define  _NTTRACEAUTO2() \
    DWORD ntlog_dwEBP; \
    LPCTSTR ntlog_lpctszFuncSig; \
    { __asm mov dword ptr[ntlog_dwEBP], ebp } \
    __if_exists(this) { if(CNTLog::IsTRACEValid()) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, FALSE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig, ENTER_FUNCTION_FLAG __TFUNCTION__ , __TFUNCSIG__,(LPCVOID)this); } } \
    __if_not_exists(this) { if(CNTLog::IsTRACEValid()) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, TRUE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig, ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__, NULL); } } \
    if(!ntlog_lpctszFuncSig) {{__asm push ebp }{__asm pop ebp  }} // no reach, but absolutely necessarily. 

#define _NTTRACEAUTO_SIMPLE() \
    __if_exists(this) { CNTLog::CEnterLeaveFunCallBack callback( __TFUNCSIG__ , ENTER_FUNCTION_FLAG __TFUNCTION__ , __TFUNCSIG__, (LPCVOID)this, TRUE); } \
    __if_not_exists(this){ CNTLog::CEnterLeaveFunCallBack callback(__TFUNCSIG__ , ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__, NULL, TRUE); } 

#define _NTTRACEAUTO_SIMPLE2() \
    __if_exists(this) { if(CNTLog::IsTRACEValid()) { CNTLog::CEnterLeaveFunCallBack callback( __TFUNCSIG__ , ENTER_FUNCTION_FLAG __TFUNCTION__ , __TFUNCSIG__, (LPCVOID)this, TRUE); } } \
    __if_not_exists(this){ if(CNTLog::IsTRACEValid()) { CNTLog::CEnterLeaveFunCallBack callback(__TFUNCSIG__ , ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__, NULL, TRUE); } }

#define _NTTRACEAUTO_COMPLEX() \
    DWORD ntlog_dwEBP; \
    LPCTSTR ntlog_lpctszFuncSig; \
    { __asm mov dword ptr[ntlog_dwEBP], ebp } \
    __if_exists(this) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, FALSE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig, ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__, (LPCVOID)this, FALSE, ntlog_dwEBP, __FUNCSIG__); } \
    __if_not_exists(this) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, TRUE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig, ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__, NULL, FALSE, ntlog_dwEBP, __FUNCSIG__); } \
    if(!ntlog_lpctszFuncSig) {{__asm push ebp }{__asm pop ebp  }} // no reach, but absolutely necessarily. 

#define _NTTRACEAUTO_COMPLEX2() \
    DWORD ntlog_dwEBP; \
    LPCTSTR ntlog_lpctszFuncSig; \
    { __asm mov dword ptr[ntlog_dwEBP], ebp } \
    __if_exists(this) { if(CNTLog::IsTRACEValid()) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, FALSE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__; CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig,ENTER_FUNCTION_FLAG __TFUNCTION__ , __TFUNCSIG__,(LPCVOID)this, FALSE, ntlog_dwEBP, __FUNCSIG__); } } \
    __if_not_exists(this) { if(CNTLog::IsTRACEValid()) { ntlog_lpctszFuncSig = CNTLog::TraceAuto(__FUNCSIG__, ntlog_dwEBP, TRUE); if(!ntlog_lpctszFuncSig) ntlog_lpctszFuncSig = __TFUNCSIG__;CNTLog::CEnterLeaveFunCallBack callback(ntlog_lpctszFuncSig,ENTER_FUNCTION_FLAG __TFUNCTION__, __TFUNCSIG__, NULL, FALSE, ntlog_dwEBP, __FUNCSIG__); } } \
    if(!ntlog_lpctszFuncSig) {{__asm push ebp }{__asm pop ebp  }} //no reach, but absolutely necessarily. 

//////////END NTTRACEAUTO//////////

//////////4CXX//////////

#define __NT4CXX(level, nop, LogData) \
    __if_exists(this) { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) (CNTLog::GetInstance(__TFUNCTION__, this)<<LogData).NTLog4CXX_Flush(CNTLog::LEVEL_##level)} \
    __if_not_exists(this) { if(!CNTLog::IsModuleUnloading(__TFUNCTION__)) (CNTLog::GetInstance(__TFUNCTION__, NULL)<<LogData).NTLog4CXX_Flush(CNTLog::LEVEL_##level)}

#define __NT4CXX2(level, nop, LogData) \
    __if_exists(this) { if(!CNTLog::IsModuleUnloading(__TFUNCTION__) && CNTLog::IsLogValid(CNTLog::LEVEL_##level)) (CNTLog::GetInstance(__TFUNCTION__, this)<<LogData).NTLog4CXX_Flush(CNTLog::LEVEL_##level)} \
    __if_not_exists(this) { if(!CNTLog::IsModuleUnloading(__TFUNCTION__) && CNTLog::IsLogValid(CNTLog::LEVEL_##level)) (CNTLog::GetInstance(__TFUNCTION__, NULL)<<LogData).NTLog4CXX_Flush(CNTLog::LEVEL_##level)}

//////////END 4CXX//////////

#define ISNTDEBUGVALID          CNTLog::IsDebugValid
#define ISNTINFOVALID           CNTLog::IsInfoValid
#define ISNTWARNVALID           CNTLog::IsWarnValid
#define ISNTERRORVALID          CNTLog::IsErrorValid
#define ISNTFATALVALID          CNTLog::IsFatalValid

#else // !defined(NTLOG_ENABLE)

#define _NTTRACE
#define _NTDEBUG
#define _NTINFO
#define _NTWARN
#define _NTERROR
#define _NTFATAL
#define _NTPROMPT
#define _NTHEXDUMP
#define _INSTALLNTLOGINI

#define __NT4CXX(level, nop, LogData)
#define __NT4CXX2(level, nop, LogData)

#define _NTTRACEAUTO
#define _NTTRACEAUTO2

#define ISNTDEBUGVALID
#define ISNTINFOVALID
#define ISNTWARNVALID
#define ISNTERRORVALID
#define ISNTFATALVALID


#endif // NTLOG_ENABLE

//////////////////////////////////////////////////////////////////////////
// 外部使用宏
#define NTTRACE                 _NTTRACE
#define NTDEBUG                 _NTDEBUG
#define NTINFO                  _NTINFO
#define NTWARN                  _NTWARN
#define NTERROR                 _NTERROR
#define NTFATAL                 _NTFATAL
#define NTPROMPT                _NTPROMPT
#define NTHEXDUMP               _NTHEXDUMP
#define NTTRACEAUTO             _NTTRACEAUTO
#define NTTRACEAUTO2            _NTTRACEAUTO2
#define INSTALLNTLOGINI         _INSTALLNTLOGINI

#define  _NTDEBUG4CXX(nop, LogData)     __NT4CXX(DEBUG, nop, LogData)
#define  NTDEBUG4CXX(LogData)           _NTDEBUG4CXX(0, LogData)
#define  _NTTRACE4CXX(nop, LogData)     __NT4CXX(TRACE, nop, LogData)
#define  NTTRACE4CXX(LogData)           _NTTRACE4CXX(0, LogData)
#define  _NTINFO4CXX(nop, LogData)      __NT4CXX(INFO, nop, LogData)
#define  NTINFO4CXX(LogData)            _NTINFO4CXX(0, LogData)
#define  _NTWARN4CXX(nop, LogData)	    __NT4CXX(WARN, nop, LogData)
#define  NTWARN4CXX(LogData)            _NTWARN4CXX(0, LogData)
#define  _NTERROR4CXX(nop, LogData)     __NT4CXX(ERROR, nop, LogData)
#define  NTERROR4CXX(LogData)           _NTERROR4CXX(0, LogData)
#define  _NTFATAL4CXX(nop, LogData)     __NT4CXX(FATAL, nop, LogData)
#define  NTFATAL4CXX(LogData)           _NTFATAL4CXX(0, LogData)
#define  _NTDEBUG4CXX2(nop, LogData)    __NT4CXX2(DEBUG, nop, LogData)
#define  NTDEBUG4CXX2(LogData)          _NTDEBUG4CXX2(0, LogData)
#define  _NTTRACE4CXX2(nop, LogData)    __NT4CXX2(TRACE, nop, LogData)
#define  NTTRACE4CXX2(LogData)          _NTTRACE4CXX2(0, LogData)
#define  _NTINFO4CXX2(nop, LogData)     __NT4CXX2(INFO, nop, LogData)
#define  NTINFO4CXX2(LogData)           _NTINFO4CXX2(0, LogData)
#define  _NTWARN4CXX2(nop, LogData)	    __NT4CXX2(WARN, nop, LogData)
#define  NTWARN4CXX2(LogData)           _NTWARN4CXX2(0, LogData)
#define  _NTERROR4CXX2(nop, LogData)    __NT4CXX2(ERROR, nop, LogData)
#define  NTERROR4CXX2(LogData)          _NTERROR4CXX2(0, LogData)
#define  _NTFATAL4CXX2(nop, LogData)    __NT4CXX2(FATAL, nop, LogData)
#define  NTFATAL4CXX2(LogData)          _NTFATAL4CXX2(0, LogData)

// 特殊函数名
#define NTINSTALL               INSTALLNTLOGINI     //安装日志配置文件
#define NTAUTO                  NTTRACEAUTO
#define NTAUTO2                 NTTRACEAUTO2
#define NTDUMP                  NTHEXDUMP

#define NTISDEBUGVALID          ISNTDEBUGVALID
#define NTDEBUGVALID            ISNTDEBUGVALID
#define NTISINFOVALID           ISNTINFOVALID
#define NTINFOVALID             ISNTINFOVALID
#define NTISWARNVALID           ISNTWARNVALID
#define NTWARNVALID             ISNTWARNVALID
#define NTISERRORVALID          ISNTERRORVALID
#define NTERRORVALID            ISNTERRORVALID
#define NTISFATALVALID          ISNTFATALVALID
#define NTFATALVALID            ISNTFATALVALID

// 兼容MFC ATL 的 TRACE
#ifdef TRACE
#undef TRACE
#endif
#define TRACE NTTRACE

#endif // __NTLOG_H_2013_6_24__
