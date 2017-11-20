// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "TCPLinker.h"
#include "MegaUtil.h"


#pragma pack(push)
#pragma pack(1)
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|X|  CC   |M|     PT      |       sequence number         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                           timestamp                           |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |           synchronization source (SSRC) identifier            |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  |            contributing source (CSRC) identifiers             |
//  |                             ....                              |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct RTPHEADER_FU {
    unsigned long _CSRCcount: 4;    // CSRC count (CC): 4 bits
    unsigned long _extension: 1;    // extension (X): 1 bit
    unsigned long _padding: 1;      // padding (P): 1 bit
    unsigned long _version: 2;      // version (V): 2 bits

    unsigned long _payloadtype: 7;  //payload type (PT): 7 bits
    unsigned long _marker: 1;       // marker (M): 1 bit
    unsigned long _seq: 16;         //sequence number: 16 bits
    unsigned long _timestamp;       //timestamp: 32 bits
    unsigned long _SSRC;            //synchronization source (SSRC) identifier

    // FU indicator
    // +---------------+
    // |0|1|2|3|4|5|6|7|
    // +-+-+-+-+-+-+-+-+
    // |F|NRI|  Type   |
    // +---------------+
    struct FU_indicator {
        unsigned char Type: 5;
        unsigned char NRI: 2;
        unsigned char F: 1;
    } fui;

    // FU header
    // +---------------+
    // |0|1|2|3|4|5|6|7|
    // +-+-+-+-+-+-+-+-+
    // |S|E|R|  Type   |
    // +---------------+
    struct FU_header {
        unsigned char Type: 5;
        unsigned char R: 1;
        unsigned char E: 1;
        unsigned char S: 1;
    } fuh;

    RTPHEADER_FU()
    {
        memset(this, 0, sizeof(RTPHEADER_FU));
    }
};

#pragma pack(pop)


class CRtpClient : public CUDPStreamT< CRtpClient >
{
    std::deque< std::pair< unsigned long, RTPHEADER_FU *> > m_deqStream;
    long m_idx;
    long m_lRetry;

    long m_lCountPack;  // 收到的包数
    long m_lCountRepeat;// 重复的包数
    long m_lCountThrow; // 丢弃的包数
    long m_lCountLose;  // 丢失的包数
    long m_lCountFrame; // 拼成的帧数

    ICallBack *m_pCallBack;
public:
    CRtpClient() : m_lRetry(0), m_lCountPack(0), m_lCountThrow(0), m_lCountLose(0), m_lCountFrame(0), m_lCountRepeat(0), m_pCallBack(NULL), m_idx(0)
    {}
    ~CRtpClient()
    {
        std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::iterator pos;
        for (pos = m_deqStream.begin(); pos != m_deqStream.end(); pos++)
            delete pos->second;
    }
    bool Create(long idx, ICallBack *pCallBack, long lPort = 0)
    {
        m_idx = idx;
        m_pCallBack = pCallBack;
        return __super::Create(lPort);
    }
    long Take(char *sData, long lLength)
    {
        m_lCountPack++;
        RTPHEADER_FU *pRTPHEADER_FU = (RTPHEADER_FU *)sData;
        pRTPHEADER_FU->_seq = ntohs(pRTPHEADER_FU->_seq);
        pRTPHEADER_FU->_timestamp = ntohl(pRTPHEADER_FU->_timestamp);
        pRTPHEADER_FU->_SSRC = ntohl(pRTPHEADER_FU->_SSRC);

        // RTP包排序
        if (m_deqStream.size() == 0)
            m_deqStream.push_front(std::make_pair(lLength, pRTPHEADER_FU));
        else {
            bool bFoundPos = false;
            std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::iterator pos;
            for (pos = m_deqStream.begin(); pos != m_deqStream.end(); pos++) {
                if (pos->second->_seq < pRTPHEADER_FU->_seq) {
                    m_deqStream.insert(pos, std::make_pair(lLength, pRTPHEADER_FU));
                    bFoundPos = true;
                    break;
                } else if (pos->second->_seq == pRTPHEADER_FU->_seq) {
                    // 由于有重传机制,可能会收到重复的包,丢弃即可
                    m_lCountRepeat++;
                    bFoundPos = true;
                    break;
                }
            }

            if (!bFoundPos) {
                // 来迟的包,作为重复包处理
                m_lCountRepeat++;
            }
        }

        // 重传,每收到5个包,检查一次重传
        m_lRetry++;
        if (m_lRetry > 5 && m_deqStream.size() > 5) {
            long lLast = (m_deqStream.begin() + 4)->second->_seq; // 从第5个包开始检查
            std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::iterator pos;
            for (pos = m_deqStream.begin() + 5; pos != m_deqStream.end(); pos++) {
                if (pos->second->_seq + 1 != lLast) {
                    // 发送重传包
                    // Send();
                }
            }

            m_lRetry = 0;
        }

        // 组包
        if (m_deqStream.size() > 0) {
            std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::reverse_iterator rbegin = m_deqStream.rbegin();
            RTPHEADER_FU *pp = rbegin->second;  // 协助调试代码行
            if (rbegin->second->fui.Type == 28) {
                bool bHasFrame = false;
                std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::reverse_iterator pos = rbegin;
                for (unsigned long lSeq = pos->second->_seq; pos != m_deqStream.rend(); lSeq++) {
                    pp = pos->second;  // 协助调试代码行
                    if (lSeq != pos->second->_seq)
                        break;

                    if (pos->second->fuh.E == 1) {
                        bHasFrame = true;
                        break;
                    }

                    pos++;
                }

                if (bHasFrame) {
                    long s1 = pos->second->_seq;  // 协助调试代码行
                    long s2 = m_deqStream.rbegin()->second->_seq;  // 协助调试代码行
                    long lBuferSize = 2048 * (pos->second->_seq - m_deqStream.rbegin()->second->_seq + 1);
                    char *sFrame = new char[ lBuferSize ];
                    *(long *)sFrame = 0x01000000;
                    long lUsed = 4;

                    RTPHEADER_FU::FU_indicator *pfui = (RTPHEADER_FU::FU_indicator *)sFrame + lUsed;
                    *pfui = m_deqStream.rbegin()->second->fui;
                    pfui->Type = m_deqStream.rbegin()->second->fuh.Type;
                    lUsed += 1;

                    std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::reverse_iterator p;

                    long lPackCount = 0; // 协助调试代码行
                    for (p = m_deqStream.rbegin(); p != pos; p++) {
                        pp = p->second;  // 协助调试代码行
                        void *p2 = ((char *)p->second) + 14;
                        memcpy(sFrame + lUsed, p->second + 1, p->first - sizeof(RTPHEADER_FU));
                        lUsed += p->first - sizeof(RTPHEADER_FU);
                        delete[](char *)p->second;

                        lPackCount++;  // 协助调试代码行
                    }
                    memcpy(sFrame + lUsed, p->second + 1, p->first - sizeof(RTPHEADER_FU));
                    lUsed += p->first - sizeof(RTPHEADER_FU);
                    delete[](char *)p->second;
                    m_deqStream.erase(p.base() - 1, m_deqStream.rbegin().base());

                    lPackCount++;  // 协助调试代码行

                    if (m_pCallBack)
                        m_pCallBack->Notify(601, NULL, (unsigned char *)sFrame, lUsed);

                    // 组包成功
                    /*FILE * f = fopen( "vauwatcher.264", "a+b" );
                    fseek( f, 0, SEEK_END );
                    fwrite( sFrame, 1, lUsed, f );
                    fclose(f);*/

                    m_lCountFrame++;
                    delete [] sFrame;
                }
            } else {
                pp = rbegin->second;  // 协助调试代码行
                if (m_lCountFrame != 0) {
                    if (pp->fuh.Type == 0x65) {
                        int a = 0;
                    }
                    rbegin->second->_SSRC = 0x01000000;
                    if (m_pCallBack)
                        m_pCallBack->Notify(601, NULL, (unsigned char *)&rbegin->second->_SSRC, rbegin->first - 8);
                } else {
                    char sCount[128] = {0};
                    sprintf_s(sCount, "--- %d 开始接收视频数据\n", m_idx);
                    ::OutputDebugString(sCount);
                }

                // 组包成功
                //FILE * f = fopen( "vauwatcher.264", "a+b" );
                //fseek( f, 0, SEEK_END );
                //fwrite( (char*)&rbegin->second->_SSRC, 1, rbegin->first - 8, f );
                //fclose(f);

                m_lCountFrame++;
                delete[](char *)rbegin->second;
                m_deqStream.erase(rbegin.base() - 1);
            }
        }

        // 限制缓冲大小
        if (m_deqStream.size() > 10) {
            std::deque< std::pair< unsigned long, RTPHEADER_FU * > >::reverse_iterator rbegin = m_deqStream.rbegin();
            m_lCountLose += (rbegin + 1)->second->_seq - rbegin->second->_seq - 1;
            m_lCountThrow++;
            delete[](char *) rbegin->second;
            m_deqStream.erase(rbegin.base() - 1);
        }

        char sCount[128] = {0};
    //  sprintf_s( sCount, "缓%d 收%d 重%d 失%d 弃%d 帧%d\n", m_deqStream.size(), m_lCountPack, m_lCountRepeat, m_lCountThrow, m_lCountLose, m_lCountFrame );
        ::OutputDebugString(sCount);
        return lLength;
    }
};

class CRtspClient : public CTCPLinkT<CRtspClient>
{
    enum ENUM_STATE { e_none, e_destribe, e_setup, e_play, e_stream, e_teardown  };

    ENUM_STATE m_rtsp_state;

    ICallBack *m_pCallBack;
    long m_lCSeq;
    long m_idx;
    std::string m_rtsp;
    std::string m_sSession;

    DWORD m_dwTickCount;

    CRtpClient m_rtpVideo, m_rtpAudio;

public:
    CRtspClient()
    {
        static long idx = 1;
        m_idx = idx++;

        m_rtsp_state = e_none;
        m_pCallBack = NULL;
        m_lCSeq = 1;
    }

    long Open(const char *sRtsp, ICallBack *pCallBack)
    {
        m_rtsp = sRtsp;

        if (0 != strncmp(sRtsp, "rtsp://", strlen("rtsp://"))) {
            ::OutputDebugString("rtsp connection error");
            return -1;
        }

        std::pair< std::string, long> IP_Port = GetIPAndPortFromUrl(sRtsp);
        m_sIP = IP_Port.first;
        m_lPort = IP_Port.second;
        m_pCallBack = pCallBack;
        return __super::Connect(m_sIP.c_str(), m_lPort);
    }

    long TearDown()
    {
        return 0;
    }

    long Take(char *sData, long lLength)
    {
        char c = sData[ lLength ];
        sData[ lLength ] = 0;

        char *sBeginLineEnd = strstr(sData, "\r\n");
        if (sBeginLineEnd == NULL) {
            sData[ lLength ] = c;
            return 0;
        }
        char *sHeadEnd = strstr(sBeginLineEnd + strlen("\r\n"), "\r\n\r\n");
        if (sHeadEnd == NULL) {
            sData[ lLength ] = c;
            return 0;
        }

        sBeginLineEnd[0] = 0;
        sHeadEnd[0] = 0;
        long lBodyLen = 0;

        CSplitVector spvStartLine(sData, " ");
        CSplitDeque spdHeadLine(sBeginLineEnd + strlen("\r\n"), ":", "\r\n");
        if (spvStartLine.size() == 0 || spdHeadLine.size() == 0) {
            sData[ lLength ] = c;
            return lLength;
        }
        CSplitDeque::iterator pos = spdHeadLine.find("Content-Length");
        if (pos != spdHeadLine.end()) {
            lBodyLen = atol(pos->second.c_str());
            if (sHeadEnd - sData + strlen("\r\n\r\n") + lBodyLen > (unsigned long)lLength) {
                ::OutputDebugString("The package body hasn't yet received done\n");
                sHeadEnd[0] = '\r';
                sBeginLineEnd[0] = '\r';
                sData[ lLength ] = c;
                return 0;
            }
        }

        if (spvStartLine.size() < 3) {
            sData[ lLength ] = c;
            return lLength;
        }

        if (spvStartLine[0] != "RTSP/1.0") {
            CSplitDeque::iterator pos = spdHeadLine.find("nCSeq");
            if (pos == spdHeadLine.end()) {
                sData[ lLength ] = c;
                return lLength;
            }
            const char *sFormat = "RTSP/1.0 405 Method Not Allowed\r\nCSeq:%s\r\n\r\n";
            char s[ 512 ] = {0};
            sprintf_s(s, 511, sFormat, pos->second.c_str());
            Send(s, strlen(s));
            return lLength;
        }

        if (spvStartLine[1] != "200") {
            ::OutputDebugString(m_rtsp.c_str());
            ::OutputDebugString("error：");
            ::OutputDebugString(sData);
            ::OutputDebugString("\n");

            Close();
            sData[ lLength ] = c;
            return lLength;
        }

        OnPackage(spdHeadLine, sHeadEnd + strlen("\r\n\r\n"), lBodyLen);

        sData[ lLength ] = c;
        return sHeadEnd - sData + strlen("\r\n\r\n") + lBodyLen;
    }

private:
    long GetCSeq()
    {
        return m_lCSeq++;
    }

    long Option(const char *rtsp, ICallBack *pCallBack)
    {
        return 0;
    }

    long Destribe()
    {
        const char *sFormat = "DESCRIBE %s RTSP/1.0\r\nCSeq: %d\r\nAccept: application/sdp\r\nUser-Agent: diversifEye-RTSP-Client\r\n\r\n";
        char sCmd[4096] = {0};
        sprintf_s(sCmd, 4095, sFormat, m_rtsp.c_str(), GetCSeq());
        Send(sCmd, strlen(sCmd));
        m_rtsp_state = e_destribe;
        m_dwTickCount = ::GetTickCount();
        return 0;
    }

    long Setup(const char *rtsp)
    {
        const char *sFormat = "SETUP %s RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\nUser-Agent: diversifEye-RTSP-Client\r\n\r\n";
        static long s_lPort = 7000;

//      long lPort = 7000+GetTickCount()%1000;
//      lPort -= lPort%2;
        m_rtpVideo.Create(m_idx, m_pCallBack, s_lPort++);
        m_rtpAudio.Create(m_idx, m_pCallBack, s_lPort++);
        char sCmd[4096] = {0};
        sprintf_s(sCmd, 4095, sFormat, rtsp, GetCSeq(), m_rtpVideo.GetPort(), m_rtpAudio.GetPort());
        Send(sCmd, strlen(sCmd));
        m_rtsp_state = e_setup;
        return 0;
    }

    long Play()
    {
        const char *sFormat = "PLAY %s RTSP/1.0\r\nCSeq: %d\r\nSession:%s\r\nRange: npt=0.000000-\r\nUser-Agent: diversifEye-RTSP-Client\r\n\r\n";

        char sCmd[4096] = {0};
        sprintf_s(sCmd, 4095, sFormat, m_rtsp.c_str(), GetCSeq(), m_sSession.c_str());
        Send(sCmd, strlen(sCmd));
        m_rtsp_state = e_stream;

        return 0;
    }

    void OnConnect(long nErrorCode)
    {
        if (nErrorCode != 0) {
            Close();
            ErrMsg(nErrorCode);
            m_state = e_close_by_net_error;
        } else
            m_state = e_connected;

        Destribe();
    }

    long OnPackage(CSplitDeque &spd, const char *sBody, long lBodyLen)
    {
        assert(atol(spd.find("CSeq")->second.c_str() + 1) == m_lCSeq - 1);
        if (m_rtsp_state == e_destribe) {
            DWORD dwTickCount = ::GetTickCount();
            assert(dwTickCount - m_dwTickCount < 3000);
            char ss[ 64 ] = {0};
            sprintf_s(ss, 63, "%d\n", dwTickCount - m_dwTickCount);
            CSplitDeque::iterator pos = spd.find("Content-Base");
            if (pos != spd.end()) {
                CSplitDeque spdSDP(sBody , ":", "\r\n");

                for (CSplitDeque::iterator posTrack = spdSDP.begin(); posTrack != spdSDP.end(); posTrack++) {
                    if (posTrack->first == "a=control") {
                        std::string sTrack = TrimBlank(posTrack->second.c_str());
                        if (sTrack != "*") {
                            Setup(TrimBlank((pos->second + sTrack).c_str()).c_str());
                            return 0;
                        }
                    }
                }
            } else
                Setup(m_rtsp.c_str());
        } else if (m_rtsp_state == e_setup) {
            CSplitDeque::iterator pos = spd.find("Session");
            assert(pos != spd.end());
            m_sSession = CSplitVector(pos->second.c_str(), ";")[0];
            Play();
        } else {
            assert(spd.find("Session")->second == m_sSession);
        }

        return 0;
    }
};

