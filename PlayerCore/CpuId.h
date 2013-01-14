#ifndef _PLAYERCORE_DSUTIL_CPUID_H_
#define _PLAYERCORE_DSUTIL_CPUID_H_

class CCpuId
{
public:

	typedef enum {
		PROCESSOR_AMD,
		PROCESSOR_INTEL,
		PROCESSOR_UNKNOWN
	} PROCESSOR_TYPE;

	// Enum codes identical to FFmpeg cpu features define
	typedef enum {
		MPC_MM_MMX    = 0x0001, /* standard MMX */
		MPC_MM_3DNOW  = 0x0004, /* AMD 3DNOW */
		MPC_MM_MMXEXT = 0x0002, /* SSE integer functions or AMD MMX ext */
		MPC_MM_SSE    = 0x0008, /* SSE functions */
		MPC_MM_SSE2   = 0x0010, /* PIV SSE2 functions */
		MPC_MM_SSE3   = 0x0040, /* AMD64 & PIV SSE3 functions */
		MPC_MM_SSSE3  = 0x0080  /* PIV Core 2 SSSE3 functions */
	} PROCESSOR_FEATURES;

	CCpuId();

	int GetFeatures() const { return m_nCPUFeatures; };
	PROCESSOR_TYPE GetType() const { return m_nType; };
	int GetProcessorNumber();

private:
	int m_nCPUFeatures;
	PROCESSOR_TYPE m_nType;
};

#endif
