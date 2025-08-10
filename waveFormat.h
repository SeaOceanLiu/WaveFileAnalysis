#ifndef waveFormatH
#define waveFormatH

#include <cstdint>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#include "share.h"
#include <functional>

using namespace std;
namespace fs = std::filesystem;

struct alignas(2) SGUID{
    uint32_t    Data1;
    uint16_t    Data2;
    uint16_t    Data3;
    uint8_t     Data4[8];
};

const int CHUNK_ID_SIZE = 4;
struct alignas(2) ChunkHeader {
    char ChunkID[CHUNK_ID_SIZE];    // 4 bytes
    uint32_t ChunkSize;             // 4 bytes
};
const int CHUNK_HEADER_SIZE = sizeof(ChunkHeader);

struct alignas(2) RIFForRF64Chunk { // 使用alignas(2)来保证结构体对齐
    ChunkHeader header;             // 8 bytes，其中的ChunkID必须为"RIFF"或"RF64"，ChunkSize为文件大小减去8。当为RF64时，ChunkSize应为-1，此时使用DS64Chunk中的riffSize来获取大小
    char Format[CHUNK_ID_SIZE];     // "WAVE" 4 bytes
    char pData[0];                  // variable length
};

struct alignas(2) FormatChunk{  // 使用alignas(2)来保证结构体对齐。声音是交替存储的，所以每一个音频帧的大小是每一个采样的大小乘以声道数
    ChunkHeader header;         // 8 bytes，其中的ChunkID必须为"fmt "，ChunkSize为16时无扩展，大于16（例如18）时有扩展
    uint16_t audioFormat;       // 2 bytes，为1时表示PCM格式，为3时表示IEEE浮点格式，取值为0xFFFE时表示扩展格式后面有Extension，且Extension中的extSize必为22表示WaveFormatEx。取值参考WaveFormat枚举
    uint16_t numChannels;       // 2 bytes，通道数，一般为1或2，分别表示单声道和立体声
    uint32_t sampleRate;        // 4 bytes，采样率，一般采用8000，44100，48000等，
    uint32_t byteRate;          // 4 bytes，每秒播放的字节数，等于blockAlign * sampleRate
    uint16_t blockAlign;        // 2 bytes，每一个音频帧的字节数，等于bitPerSample * numChannels / 8
    uint16_t bitsPerSample;     // 2 bytes，每个采样的位数，一般为8，16，24，32，64等
    char pData[0];              // variable length，根据fmtHeader中的ChunkSize决定后面是否包含扩展Extension
};
enum class WaveFormat: uint16_t{
    UNKNOWN             = 0x0000,   // PCM Formate：标准的PCM文件类型  /*  Microsoft Corporation  */
    PCM                 = 0x0001,   /*  Microsoft Corporation  */
    ADPCM               = 0x0002,   /*  Microsoft Corporation  */
    IEEE_FLOAT          = 0x0003,   // Non-PCM Formats: 浮点编码
    IBM_CVSD            = 0x0005,   /*  IBM Corporation  */
    ALAW                = 0x0006,   // Non-PCM Formats: 压缩编码的PCM类型(A律) /*  Microsoft Corporation  */
    MULAW               = 0x0007,   // Non-PCM Formats: 压缩编码的PCM类型(U律) /*  Microsoft Corporation  */
    OKI_ADPCM           = 0x0010,   /*  OKI  */
    IMA_ADPCM           = 0x0011,   // (WAVE_FORMAT_DVI_ADPCM) /*  Intel Corporation  */
    DVI_ADPCM           = 0x0011,   /*  Intel Corporation  */
    MEDIASPACE_ADPCM    = 0x0012,   /*  Videologic  */
    SIERRA_ADPCM        = 0x0013,   /*  Sierra Semiconductor Corp  */
    G723_ADPCM          = 0x0014,   /*  Antex Electronics Corporation  */
    DIGISTD             = 0x0015,   /*  DSP Solutions, Inc.  */
    DIGIFIX             = 0x0016,   /*  DSP Solutions, Inc.  */
    YAMAHA_ADPCM        = 0x0020,   /*  Yamaha Corporation of America  */
    SONARC              = 0x0021,   /*  Speech Compression  */
    DSPGROUP_TRUESPEECH = 0x0022,   /*  DSP Group, Inc  */
    ECHOSC1             = 0x0023,   /*  Echo Speech Corporation  */
    AUDIOFILE_AF36      = 0x0024,   /*  Audiofile, Inc.  */
    APTX                = 0x0025,   /*  Audio Processing Technology  */
    AUDIOFILE_AF10      = 0x0026,   /*  Audiofile, Inc.  */
    DOLBY_AC2           = 0x0030,   /*  Dolby Laboratories  */
    GSM610              = 0x0031,   /*  Microsoft Corporation  */
    ANTEX_ADPCME        = 0x0033,   /*  Antex Electronics Corporation  */
    CONTROL_RES_VQLPC   = 0x0034,   /*  Control Resources Limited  */
    DIGIREAL            = 0x0035,   /*  DSP Solutions, Inc.  */
    DIGIADPCM           = 0x0036,   /*  DSP Solutions, Inc.  */
    CONTROL_RES_CR10    = 0x0037,   /*  Control Resources Limited  */
    NMS_VBXADPCM        = 0x0038,   /*  Natural MicroSystems  */
    G721_ADPCM          = 0x0040,   /*  Antex Electronics Corporation  */
    MPEG                = 0x0050,   /*  Microsoft Corporation  */
    CREATIVE_ADPCM      = 0x0200,   /*  Creative Labs, Inc  */
    FM_TOWNS_SND        = 0x0300,   /*  Fujitsu Corp.  */
    OLIGSM              = 0x1000,   /*  Ing C. Olivetti & C., S.p.A.  */
    OLIADPCM            = 0x1001,   /*  Ing C. Olivetti & C., S.p.A.  */
    OLICELP             = 0x1002,   /*  Ing C. Olivetti & C., S.p.A.  */
    OLISBC              = 0x1003,   /*  Ing C. Olivetti & C., S.p.A.  */
    OLIOPR              = 0x1004,   /*  Ing C. Olivetti & C., S.p.A.  */
    EXTENSIBLE          = 0xFFFE,   // Extensible Format：扩展格式，在fmt chunk中有其他sub-chunk时的类型
};

struct alignas(2) Extension{               // 使用alignas(2)来保证结构体对齐
    uint16_t extSize;           // 2 bytes，为0时无扩展，为22时使用WaveFormatEx，其它值时一般指向FactChunk
    char pData[0];              // variable length，根据extSize决定后面是否包含扩展长度，
};

enum class ChannelMask: uint32_t{
    SPEAKER_FRONT_LEFT            = 0x00000001, // 1
    SPEAKER_FRONT_RIGHT           = 0x00000002, // 2
    SPEAKER_FRONT_CENTER          = 0x00000004, // 4
    SPEAKER_LOW_FREQUENCY         = 0x00000008, // 8
    SPEAKER_BACK_LEFT             = 0x00000010, // 16
    SPEAKER_BACK_RIGHT            = 0x00000020, // 32
    SPEAKER_FRONT_LEFT_OF_CENTER  = 0x00000040, // 64
    SPEAKER_FRONT_RIGHT_OF_CENTER = 0x00000080, // 128
    SPEAKER_BACK_CENTER           = 0x00000100, // 256
    SPEAKER_SIDE_LEFT             = 0x00000200, // 512
    SPEAKER_SIDE_RIGHT            = 0x00000400, // 1024
    SPEAKER_TOP_CENTER            = 0x00000800, // 2048
    SPEAKER_TOP_FRONT_LEFT        = 0x00001000, // 4096
    SPEAKER_TOP_FRONT_CENTER      = 0x00002000, // 8192
    SPEAKER_TOP_FRONT_RIGHT       = 0x00004000, // 16384
    SPEAKER_TOP_BACK_LEFT         = 0x00008000, // 32768
    SPEAKER_TOP_BACK_CENTER       = 0x00010000, // 65536
    SPEAKER_TOP_BACK_RIGHT        = 0x00020000, // 131072
    SPEAKER_RESERVED              = 0x80000000  // 2147483648
};
const int MAX_CHANNEL_NUMBER = 19; // 目前只定义了19个声道，其中还含有一个保留声道
struct alignas(2) WaveFormatExt{  // 使用alignas(2)来保证结构体对齐，否则在读取时可能会出错
    uint16_t validBitsPerSample; // 2 bytes，实际采样位数，例如当bitPerSample为24时，该字段可以为17~24，意思是使用24位中的部分或全部比特
    uint32_t channelMask;        // 4 bytes，使用ChannelMask掩码标识出声道
    SGUID subFormat;             // 前2个字节（即一个uint16_t）表示原audioFormat的值，
};

/*All (compressed) non-PCM formats must have a fact chunk (Rev. 3 documentation).
    The chunk contains at least one value, the number of samples in the file.*/
struct alignas(2) FactChunk{
    ChunkHeader header;     // 8 bytes，其中的ChunkID必须为"fact"
    uint32_t factSize;          // 4 bytes，每个通道的采样总数
    char pData[0];              // variable length，根据dataSize决定后面是否包含扩展长度，
};
struct alignas(2) DataChunk{
    ChunkHeader header;     // 8 bytes，其中的ChunkID必须为"data"
    char pData[0];              // variable length，当ChunkSize为奇数时，后需要填充一个字节补齐为偶数
};


/**************************** 下面定义RF64 wave扩展文件结构 ****************************/
struct alignas(2) DS64Chunk{
    ChunkHeader header;     // 8 bytes，其中的ChunkID必须为"ds64"，ChunkSize一般为28
    uint64_t riffSize;          // 8 bytes，实际内容大小，通常为（文件大小-8）
    uint64_t dataSize;          // 8 bytes，实际数据块大小
    uint64_t factSize;          // 8 bytes，实际数据量（解压后）
    uint32_t tableLength;       // 4 bytes，一般为0，不为0时表示后面跟着的RIFFChunkHeader64个数
    char pData[0];              // variable length，根据tableLength决定后面是否有RIFFChunkHeader64，
};
struct alignas(2) RIFFChunkHeader64{
    ChunkHeader header;         // 8 bytes
    char pData[0];              // variable length
};

struct alignas(2) JUNKChunk{
    ChunkHeader header;         // 8 bytes，其中的ChunkID必须为"JUNK"
    char pData[0];              // variable length，一般长度为74字节且填充0
};
struct alignas(2) LISTChunk{
    ChunkHeader header;         // 8 bytes，其中的ChunkID必须为"LIST"
    char pData[0];              // variable length
};

struct alignas(2) BroadcastWaveFormatExtension{
    ChunkHeader header;             // 8 bytes
    char description[256];          //文件描述信息
    char originator[32];            //创作者信息
    char originatorReference[32];   //创作者关联信息
    char originationDate[10];       //创作日期
    char originationTime[8];        //创作时间 <<hh::mm::ss>>
    uint16_t align = 0;             //对齐占位数据
    uint64_t timeReference;         //参考时间
    uint16_t version;               //BWF版本号
    char UMID[64];                  //SMPTE UMID数据
    int16_t loudnessValue;          //集成响度值 LUFS
    int16_t loudnessRange;          //响度范围值 LU
    int16_t maxTruePeakLevel;       //TruePeak峰值 dBTP
    int16_t maxMomentaryLoudness;   //瞬时响度均值 LUFS
    int16_t maxShortTermLoudness;   //ShortTerm响度均值 LUFS
    char reserved[180];             //180字节的预留空间 现在全为0
};

struct alignas(2) PadChunk{
    ChunkHeader header;         // 8 bytes
    char pData[0];              // variable length
};

struct alignas(2) CueChunk{
    ChunkHeader header;         // 8 bytes，其中的ChunkID必须为"cue "
    uint32_t cuePoints;         // 4 bytes
    char pData[0];              // variable length，根据CuePoints决定后面CuePoint个数
};

struct alignas(2) CuePoint{
    uint32_t name;              // 4 bytes, Specifies the cue point name. Each <cue-point> record must have a unique dwName field.
    uint32_t position;          // 8 bytes, Specifies the sample position of the cue point. This is the sequential sample number within the play order.
    char fccChunk[4];           // 4 bytes, Specifies the name or chunk ID of the chunk containing the cue point.
    uint32_t chunkStart;        // 4 bytes, Specifies the position of the start of the data chunk containing the cue point.
    uint32_t blockStart;        // 4 bytes, Specifies the position of the start of the block containing the position.
    uint32_t sampleOffset;      // 4 bytes, Specifies the sample offset of the cue point relative to the start of the block.
};

struct alignas(2) PlaylistChunk{
    ChunkHeader header;         // 8 bytes，其中的ChunkID必须为"plst"
    uint32_t segments;         // 4 bytes
    char pData[0];              // variable length，根据segments决定后面PlaySegment个数
};
struct alignas(2) PlaySegment{
    uint32_t name;              // 4 bytes, Specifies the cue point name
    uint32_t length;            // 4 bytes, Specifies the length of the section in samples
    uint32_t loops;             // 4 bytes, Specifies the number of times to play the section
};

// 一些不常用扩展Chunk
// Associated Data Chunk
// Instrument Chunk
// Sample Chunk

struct alignas(2) UnknownChunk{
    ChunkHeader header;         // 8 bytes
    char pData[0];              // variable length
};

class ExplainationInfo{
public:
    string text;
    char *pData;
    size_t dataSize;

	ExplainationInfo(string text, char *pData, size_t dataSize):text(text),pData(pData),dataSize(dataSize){}
};

class WaveFile{
    using ChunkStructHandle = std::function<ChunkHeader*(ChunkHeader *)>;               // 函数指针类型，用于指向不同类型的Chunk结构体
    using ChunkExplainHandle = std::function<shared_ptr<vector<shared_ptr<ExplainationInfo>>>(ChunkHeader *)>;// 函数指针类型，用于指向不同类型的Chunk解析函数
private:
    fs::path m_filePath;    // 文件路径
    size_t m_chunkSize;     // 文件大小

    shared_ptr<char[]>m_fileBuffer;
    ChunkHeader *m_pRootChunk;

    unordered_map<string, ChunkStructHandle> m_riffStructTable;
    unordered_map<string, ChunkExplainHandle> m_riffExplainTable;
    unordered_map<WaveFormat, string> m_waveFormatExplainTable;
    vector<string>m_channelMaskExplainTable;
    vector<shared_ptr<ExplainationInfo>> m_explainationList;

    unordered_map<string, ChunkHeader*> m_chunkTable;

public:
    WaveFile(fs::path filePath);
    ChunkHeader *getFirstChunk(void);
    ChunkHeader *getFirstSubChunk(ChunkHeader *parentChunk);
    ChunkHeader *getNextBrotherChunk(ChunkHeader *parentChunk, ChunkHeader *pChunkHeader);
    bool isKnownChunk(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> getExplaination(ChunkHeader *pChunkHeader);
    ChunkHeader *getChunk(string chunkID);

    ChunkHeader *RIFForRF64ChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *FormatChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *FactChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *DataChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *DS64ChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *RIFFChunkHeader64Handle(ChunkHeader *pChunkHeader);
    ChunkHeader *JUNKChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *BroadcastWaveFormatExtensionHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *PadChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *ListChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *CueChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *PlaylistChunkHandle(ChunkHeader *pChunkHeader);
    ChunkHeader *UnknownChunkHandle(ChunkHeader *pChunkHeader);

    shared_ptr<vector<shared_ptr<ExplainationInfo>>>RIFForRF64ChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>FormatChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>FactChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>DataChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>DS64ChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>RIFFChunkHeader64Explain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>JUNKChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>BroadcastWaveFormatExtensionExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>PadChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>ListChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>CueChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>PlaylistChunkExplain(ChunkHeader *pChunkHeader);
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>UnknownChunkExplain(ChunkHeader *pChunkHeader);
};

#endif // waveFormatH