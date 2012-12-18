
#include "stdafx.h"
#include "BaseEngine.h"

CBaseEngine::CBaseEngine(CPlayerCore* pPlayerCore)
	: m_EngineType(ET_INVALID)
	, m_pPlayerCore(pPlayerCore)
	, m_pMediaInfo(NULL)
{
}

CBaseEngine::~CBaseEngine()
{
}

