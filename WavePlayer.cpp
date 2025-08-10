#include "WavePlayer.h"

WavePlayer::WavePlayer(char *pData, size_t length):
    AudioStream(),
    m_pData(pData),
    m_length(length),
    m_offset(0)
{

}

int WavePlayer::feedAudioData(char *pBuffer, size_t requestBytes){
    if(m_offset >= m_length){
        return -1;  // 使用负数表示EOF
    }

    int bytesToCopy = std::min(requestBytes, m_length - m_offset);
    memcpy_s(pBuffer, requestBytes, (char*)m_pData + m_offset, bytesToCopy);
    m_offset += bytesToCopy;

    return bytesToCopy;
}