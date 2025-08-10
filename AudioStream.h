#ifndef AudioStreamH
#define AudioStreamH

#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include <Windows.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include "DebugTrace.h"
#include "EventQueue.h"

using namespace std;

#define AUDIOSTREAM_CALL(func) {\
                                HRESULT hr;\
                                hr = (func);\
                                if(FAILED(hr))\
                                {\
                                    DEBUG_PRINT("AUDIOSTREAM_CALL error = 0x%x, File:%s, Line:%d", hr, __FILE__, __LINE__);\
                                }\
                            }


DWORD WINAPI dataFeedingThread(void *obj);

enum class AudioEvent{
    Feeding     = 0,
    Stopping    = 1,
    Butt        = 2
};

class EnumHelp
{
public:
    static int toInt(AudioEvent e){return static_cast<int>(e);}
    static AudioEvent toAudioEvent(int i){return static_cast<AudioEvent>(i);}
};


class AudioStream
{
private:
    IMMDevice *m_IMMDevice;
    IAudioClient *m_audioClient;
    IAudioRenderClient *m_audioRenderClient;
    HANDLE m_hEvents[static_cast<int>(AudioEvent::Butt)];
    HANDLE m_hThread;
    // WAVEFORMATEX *m_waveFormatEx;
    WAVEFORMATEXTENSIBLE *m_waveFormatEx;
    uint32_t m_blockSize;  // alias frames
    WORD m_blockAlign; // size of one frame
    bool m_isWorking;
    bool m_isPlaying;

    unordered_map<uint32_t, string> m_audioErrorMapping;


protected:
    // 由子类实现该功能，返回值-1时表示数据结束
    virtual int feedAudioData(char *pBuffer, size_t requestBytes) = 0;    // return bytes feeded

public:
    friend DWORD WINAPI dataFeedingThread(void *obj);
    string getErrorString(HRESULT hr);

    AudioStream(void);
    ~AudioStream();
    void setFormat(WAVEFORMATEXTENSIBLE *fmt);
    void play(void);
    void stop(void);
    void pause(void);
    void resume(void);
    bool isPlaying(void);
};

#endif  // AudioStreamH
