#pragma once

class CXTimer
{
public:
    CXTimer()
    {
        // Get CPU frequency
        __int64 nStart = GetCounter();
        Sleep(1000);
        __int64 nStop = GetCounter();
        m_nFrequency = (nStop - nStart);

        // Get call time
        const int LOOP_COUNT = 1000;
        __int64 nCallTime = 0;
        m_nCallTime = 0;
        for(int i=0; i<LOOP_COUNT; i++)
        {
            this->Start();
            nCallTime += this->Stop();
        }
        m_nCallTime = nCallTime / LOOP_COUNT;
    }

    ~CXTimer() {}

    void		Start()
    {
        m_nStart = GetCounter();
    }

    __int64		Stop()
    {
        m_nStop = GetCounter();
        __int64 nTime = (m_nStop-m_nStart-m_nCallTime) * 1000000000 / m_nFrequency;
        return (nTime >= 0) ? nTime : 0;
    }

protected:
    __int64		GetCounter()
    {
        __asm __emit 0x0F;
        __asm __emit 0x31;
    }

protected:
    __int64		m_nFrequency;
    __int64		m_nCallTime;
    __int64		m_nStart;
    __int64		m_nStop;
};
