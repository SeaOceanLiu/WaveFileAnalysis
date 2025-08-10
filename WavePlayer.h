#ifndef WavePlayerH
#define WavePlayerH

#include <algorithm>
#include "AudioStream.h"

class WavePlayer : public AudioStream
{
private:
    char *m_pData;
    size_t m_length;
    size_t m_offset;
protected:
    virtual int feedAudioData(char *pBuffer, size_t bytesToFeeded) override;
public:
    WavePlayer(char *pData, size_t length);
    virtual ~WavePlayer(){}
};

#endif  // WavePlayerH
