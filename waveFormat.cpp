#include "waveFormat.h"


WaveFile::WaveFile(fs::path filePath):
    m_filePath(filePath),
    m_fileBuffer(nullptr),
    m_chunkSize(0),
    m_pRootChunk(nullptr),
    m_riffStructTable{
        {"RIFF", std::bind(&WaveFile::RIFForRF64ChunkHandle, this, std::placeholders::_1)},
        {"RF64", std::bind(&WaveFile::RIFForRF64ChunkHandle, this, std::placeholders::_1)},
        {"fmt ", std::bind(&WaveFile::FormatChunkHandle, this, std::placeholders::_1)},
        {"data", std::bind(&WaveFile::DataChunkHandle, this, std::placeholders::_1)},
        {"fact", std::bind(&WaveFile::FactChunkHandle, this, std::placeholders::_1)},
        {"LIST", std::bind(&WaveFile::ListChunkHandle, this, std::placeholders::_1)},
        {"JUNK", std::bind(&WaveFile::JUNKChunkHandle, this, std::placeholders::_1)},
        {"ds64", std::bind(&WaveFile::DS64ChunkHandle, this, std::placeholders::_1)},
        {"bext", std::bind(&WaveFile::BroadcastWaveFormatExtensionHandle, this, std::placeholders::_1)},
        {"PAD ", std::bind(&WaveFile::PadChunkHandle, this, std::placeholders::_1)},
        {"cue ", std::bind(&WaveFile::CueChunkHandle, this, std::placeholders::_1),},
        {"plst", std::bind(&WaveFile::PlaylistChunkHandle, this, std::placeholders::_1)}
    },
    m_riffExplainTable{
        {"RIFF", std::bind(&WaveFile::RIFForRF64ChunkExplain, this, std::placeholders::_1)},
        {"RF64", std::bind(&WaveFile::RIFForRF64ChunkExplain, this, std::placeholders::_1)},
        {"fmt ", std::bind(&WaveFile::FormatChunkExplain, this, std::placeholders::_1)},
        {"data", std::bind(&WaveFile::DataChunkExplain, this, std::placeholders::_1)},
        {"fact", std::bind(&WaveFile::FactChunkExplain, this, std::placeholders::_1)},
        {"LIST", std::bind(&WaveFile::ListChunkExplain, this, std::placeholders::_1)},
        {"JUNK", std::bind(&WaveFile::JUNKChunkExplain, this, std::placeholders::_1)},
        {"ds64", std::bind(&WaveFile::DS64ChunkExplain, this, std::placeholders::_1)},
        {"bext", std::bind(&WaveFile::BroadcastWaveFormatExtensionExplain, this, std::placeholders::_1)},
        {"PAD ", std::bind(&WaveFile::PadChunkExplain, this, std::placeholders::_1)},
        {"cue ", std::bind(&WaveFile::CueChunkExplain, this, std::placeholders::_1)},
        {"plst", std::bind(&WaveFile::PlaylistChunkExplain, this, std::placeholders::_1)}
    },
    m_waveFormatExplainTable{
        {WaveFormat::UNKNOWN             , "Unknown(0)"                },
        {WaveFormat::PCM                 , "PCM"                       },
        {WaveFormat::ADPCM               , "ADPCM"                     },
        {WaveFormat::IEEE_FLOAT          , "IEEE float"                },
        {WaveFormat::IBM_CVSD            , "IBM_CVSD"                  },
        {WaveFormat::ALAW                , "8-bit ITU-T G.711 A-law"   },
        {WaveFormat::MULAW               , "8-bit ITU-T G.711 µ-law"   },
        {WaveFormat::OKI_ADPCM           , "OKI_ADPCM"                 },
        {WaveFormat::IMA_ADPCM           , "IMA_ADPCM"                 },
        {WaveFormat::DVI_ADPCM           , "DVI_ADPCM"                 },
        {WaveFormat::MEDIASPACE_ADPCM    , "MEDIASPACE_ADPCM"          },
        {WaveFormat::SIERRA_ADPCM        , "SIERRA_ADPCM"              },
        {WaveFormat::G723_ADPCM          , "G723_ADPCM"                },
        {WaveFormat::DIGISTD             , "DIGISTD"                   },
        {WaveFormat::DIGIFIX             , "DIGIFIX"                   },
        {WaveFormat::YAMAHA_ADPCM        , "YAMAHA_ADPCM"              },
        {WaveFormat::SONARC              , "SONARC"                    },
        {WaveFormat::DSPGROUP_TRUESPEECH , "DSPGROUP_TRUESPEECH"       },
        {WaveFormat::ECHOSC1             , "ECHOSC1"                   },
        {WaveFormat::AUDIOFILE_AF36      , "AUDIOFILE_AF36"            },
        {WaveFormat::APTX                , "APTX"                      },
        {WaveFormat::AUDIOFILE_AF10      , "AUDIOFILE_AF10"            },
        {WaveFormat::DOLBY_AC2           , "DOLBY_AC2"                 },
        {WaveFormat::GSM610              , "GSM610"                    },
        {WaveFormat::ANTEX_ADPCME        , "ANTEX_ADPCME"              },
        {WaveFormat::CONTROL_RES_VQLPC   , "CONTROL_RES_VQLPC"         },
        {WaveFormat::DIGIREAL            , "DIGIREAL"                  },
        {WaveFormat::DIGIADPCM           , "DIGIADPCM"                 },
        {WaveFormat::CONTROL_RES_CR10    , "CONTROL_RES_CR10"          },
        {WaveFormat::NMS_VBXADPCM        , "NMS_VBXADPCM"              },
        {WaveFormat::G721_ADPCM          , "G721_ADPCM"                },
        {WaveFormat::MPEG                , "MPEG"                      },
        {WaveFormat::CREATIVE_ADPCM      , "CREATIVE_ADPCM"            },
        {WaveFormat::FM_TOWNS_SND        , "FM_TOWNS_SND"              },
        {WaveFormat::OLIGSM              , "OLIGSM"                    },
        {WaveFormat::OLIADPCM            , "OLIADPCM"                  },
        {WaveFormat::OLICELP             , "OLICELP"                   },
        {WaveFormat::OLISBC              , "OLISBC"                    },
        {WaveFormat::OLIOPR              , "OLIOPR"                    },
        {WaveFormat::EXTENSIBLE          , "Determined by SubFormat"   }
    },
    m_channelMaskExplainTable{
        "FL", "FR", "FC", "LF", "BL", "BR", "FLC", "FRC", "BC", "SL", "SR", "TC", "TFL", "TFC", "TFR", "TBL", "TBC", "TBR", "RS"
    }
{
    ifstream inFile(filePath, ios::binary);
    if(!inFile.is_open()){
        throw runtime_error("Could not open file");
    }
    m_chunkSize = fs::file_size(filePath);

    m_fileBuffer = shared_ptr<char[]>(new char[m_chunkSize]);
    inFile.read(m_fileBuffer.get(), m_chunkSize);
    inFile.close();

    m_pRootChunk = (ChunkHeader *)(m_fileBuffer.get());
    string chunkID = string(m_pRootChunk->ChunkID, CHUNK_ID_SIZE);

    if(chunkID != "RIFF" && chunkID != "RF64")
    {
        throw runtime_error("Not a valid RIFF file");
    }
}

ChunkHeader *WaveFile::getFirstChunk(void){
    m_chunkTable[string(m_pRootChunk->ChunkID, CHUNK_ID_SIZE)] = m_pRootChunk;
    return m_pRootChunk;
}

ChunkHeader *WaveFile::getNextBrotherChunk(ChunkHeader *parentChunk, ChunkHeader *currentChunk){
    ChunkHeader *nextChunk = (ChunkHeader *)((char *)currentChunk + CHUNK_HEADER_SIZE + currentChunk->ChunkSize);
    uint64_t a = (uint64_t)nextChunk;
    a -= (uint64_t)parentChunk;
    a -= CHUNK_HEADER_SIZE;

    if ((char *)nextChunk - (char *)parentChunk - CHUNK_HEADER_SIZE >= parentChunk->ChunkSize
        || (char *)nextChunk + nextChunk->ChunkSize > (char *)parentChunk + parentChunk->ChunkSize){
        return nullptr;
    }

    string chunkID = string(nextChunk->ChunkID, CHUNK_ID_SIZE);
    m_chunkTable[chunkID] = nextChunk;

    return nextChunk;
}

ChunkHeader *WaveFile::getFirstSubChunk(ChunkHeader *parentChunk){
    string chunkID = string(parentChunk->ChunkID, CHUNK_ID_SIZE);
    auto it = m_riffStructTable.find(chunkID);
    if(it == m_riffStructTable.end()){
        return UnknownChunkHandle(parentChunk);
    }
    ChunkHeader *pSubChunk = it->second(parentChunk);
    if(isKnownChunk(pSubChunk)){
        m_chunkTable[string(pSubChunk->ChunkID, CHUNK_ID_SIZE)] = pSubChunk;
    }
    return pSubChunk;
}

bool WaveFile::isKnownChunk(ChunkHeader *pChunkHeader){
    if(pChunkHeader == nullptr){
        return false;
    }
    string chunkID = string(pChunkHeader->ChunkID, CHUNK_ID_SIZE);
    auto it = m_riffStructTable.find(chunkID);
    return it != m_riffStructTable.end();
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>> WaveFile::getExplaination(ChunkHeader *pChunkHeader){
    string chunkID = string(pChunkHeader->ChunkID, CHUNK_ID_SIZE);
    auto it = m_riffExplainTable.find(chunkID);
    if(it == m_riffExplainTable.end()){
        return UnknownChunkExplain(pChunkHeader);
    }
    return it->second(pChunkHeader);
}

ChunkHeader *WaveFile::getChunk(string chunkID){
    auto it = m_chunkTable.find(chunkID);
    if(it == m_chunkTable.end()){
        return nullptr;
    }
    return it->second;
}

ChunkHeader *WaveFile::RIFForRF64ChunkHandle(ChunkHeader *pChunkHeader){
    RIFForRF64Chunk *pRIFForRF64Chunk = (RIFForRF64Chunk *)pChunkHeader;

    if (pRIFForRF64Chunk->pData - (char *)pRIFForRF64Chunk - CHUNK_HEADER_SIZE >= pRIFForRF64Chunk->header.ChunkSize){
        return nullptr;
    }
    return (ChunkHeader *)pRIFForRF64Chunk->pData;
}

ChunkHeader *WaveFile::FormatChunkHandle(ChunkHeader *pChunkHeader){
    FormatChunk *pFormatChunk = (FormatChunk *)pChunkHeader;

    if(pFormatChunk->pData - (char *)pFormatChunk - CHUNK_HEADER_SIZE >= pFormatChunk->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pFormatChunk->pData;
}

ChunkHeader *WaveFile::FactChunkHandle(ChunkHeader *pChunkHeader){
    FactChunk *pFactChunk = (FactChunk *)pChunkHeader;

	if(pFactChunk->pData - (char *)pFactChunk - CHUNK_HEADER_SIZE >= pFactChunk->header.ChunkSize){
		return nullptr;
	}

    return (ChunkHeader *)pFactChunk->pData;
}

ChunkHeader *WaveFile::DataChunkHandle(ChunkHeader *pChunkHeader){
    DataChunk *pDataChunk = (DataChunk *)pChunkHeader;
	uint32_t a = pDataChunk->pData - (char *)pDataChunk;
	a = a - CHUNK_HEADER_SIZE;
	if(pDataChunk->pData - (char *)pDataChunk - CHUNK_HEADER_SIZE >= pDataChunk->header.ChunkSize){
        return nullptr;
    }

    // 对于data chunk来说，没有子chunk，直接返回nullptr即可
	// return (ChunkHeader *)pDataChunk->pData;
    return nullptr;
}

ChunkHeader *WaveFile::DS64ChunkHandle(ChunkHeader *pChunkHeader){
    DS64Chunk *pDS64Chunk = (DS64Chunk *)pChunkHeader;

    if(pDS64Chunk->pData - (char *)pDS64Chunk - CHUNK_HEADER_SIZE >= pDS64Chunk->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pDS64Chunk->pData;
}

ChunkHeader *WaveFile::RIFFChunkHeader64Handle(ChunkHeader *pChunkHeader){
    RIFFChunkHeader64 *pRIFFChunkHeader64 = (RIFFChunkHeader64 *)pChunkHeader;

    if(pRIFFChunkHeader64->pData - (char *)pRIFFChunkHeader64 - CHUNK_HEADER_SIZE >= pRIFFChunkHeader64->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pRIFFChunkHeader64->pData;
}

ChunkHeader *WaveFile::JUNKChunkHandle(ChunkHeader *pChunkHeader){
    JUNKChunk *pJUNKChunk = (JUNKChunk *)pChunkHeader;

    if(pJUNKChunk->pData - (char *)pJUNKChunk - CHUNK_HEADER_SIZE >= pJUNKChunk->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pJUNKChunk->pData;
}

ChunkHeader *WaveFile::BroadcastWaveFormatExtensionHandle(ChunkHeader *pChunkHeader){
    BroadcastWaveFormatExtension *pBroadcastWaveFormatExtension = (BroadcastWaveFormatExtension *)pChunkHeader;

    // if(pBroadcastWaveFormatExtension->pData - (char *)pBroadcastWaveFormatExtension - CHUNK_HEADER_SIZE >= pBroadcastWaveFormatExtension->header.ChunkSize){
    //     return nullptr;
    // }

    return nullptr; // 保留字节目前无子chunk，直接返回nullptr即可
}

ChunkHeader *WaveFile::PadChunkHandle(ChunkHeader *pChunkHeader){
    PadChunk *pPadChunk = (PadChunk *)pChunkHeader;

    if(pPadChunk->pData - (char *)pPadChunk - CHUNK_HEADER_SIZE >= pPadChunk->header.ChunkSize){
        return nullptr;
    }

    return nullptr; // 保留字节目前无子chunk，直接返回nullptr即可
}

ChunkHeader *WaveFile::ListChunkHandle(ChunkHeader *pChunkHeader){
    LISTChunk *pListChunk = (LISTChunk *)pChunkHeader;

    if(pListChunk->pData - (char *)pListChunk - CHUNK_HEADER_SIZE >= pListChunk->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pListChunk->pData;
}

ChunkHeader *WaveFile::CueChunkHandle(ChunkHeader *pChunkHeader){
    CueChunk *pCueChunk = (CueChunk *)pChunkHeader;

    if(pCueChunk->pData - (char *)pCueChunk - CHUNK_HEADER_SIZE >= pCueChunk->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pCueChunk->pData;
}

ChunkHeader *WaveFile::PlaylistChunkHandle(ChunkHeader *pChunkHeader){
    PlaylistChunk *pPlaylistChunk = (PlaylistChunk *)pChunkHeader;

    if(pPlaylistChunk->pData - (char *)pPlaylistChunk - CHUNK_HEADER_SIZE >= pPlaylistChunk->header.ChunkSize){
        return nullptr;
    }

    return (ChunkHeader *)pPlaylistChunk->pData;
}

ChunkHeader *WaveFile::UnknownChunkHandle(ChunkHeader *pChunkHeader){
    UnknownChunk *pUnknownChunk = (UnknownChunk *)pChunkHeader;

    if(pUnknownChunk->pData - (char *)pUnknownChunk - CHUNK_HEADER_SIZE >= pUnknownChunk->header.ChunkSize){
        return nullptr;
    }
    return nullptr; // (ChunkHeader*)pUnknownChunk->pData;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::RIFForRF64ChunkExplain(ChunkHeader *pChunkHeader){
    RIFForRF64Chunk *pRIFForRF64Chunk = (RIFForRF64Chunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
										(char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pRIFForRF64Chunk->header.ChunkSize),
                                        (char*)&pRIFForRF64Chunk->header.ChunkSize,
                                        sizeof(pRIFForRF64Chunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("Format: '" + string(pRIFForRF64Chunk->Format, 4) + "'",
                                        (char*)pRIFForRF64Chunk->Format,
                                        sizeof(pRIFForRF64Chunk->Format));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::FormatChunkExplain(ChunkHeader *pChunkHeader){
    FormatChunk *pFormatChunk = (FormatChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
										(char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pFormatChunk->header.ChunkSize),
                                        (char*)&pFormatChunk->header.ChunkSize,
                                        sizeof(pFormatChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    string audioFormat;
    if(m_waveFormatExplainTable.find(static_cast<WaveFormat>(pFormatChunk->audioFormat)) != m_waveFormatExplainTable.end()){
        audioFormat = "AudioFormat: " + m_waveFormatExplainTable[static_cast<WaveFormat>(pFormatChunk->audioFormat)];
    }else{
        audioFormat = "AudioFormat: Unknown(" + to_string(pFormatChunk->audioFormat) + ")";
    }

    info = make_shared<ExplainationInfo>(audioFormat,
                                        (char*)&pFormatChunk->audioFormat,
                                        sizeof(pFormatChunk->audioFormat));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("NumChannels: " + to_string(pFormatChunk->numChannels),
                                        (char*)&pFormatChunk->numChannels,
                                        sizeof(pFormatChunk->numChannels));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("SampleRate: " + to_string(pFormatChunk->sampleRate),
                                        (char*)&pFormatChunk->sampleRate,
                                        sizeof(pFormatChunk->sampleRate));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("ByteRate: " + to_string(pFormatChunk->byteRate),
                                        (char*)&pFormatChunk->byteRate,
                                        sizeof(pFormatChunk->byteRate));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("BlockAlign: " + to_string(pFormatChunk->blockAlign),
                                        (char*)&pFormatChunk->blockAlign,
                                        sizeof(pFormatChunk->blockAlign));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("BitsPerSample: " + to_string(pFormatChunk->bitsPerSample),
                                        (char*)&pFormatChunk->bitsPerSample,
                                        sizeof(pFormatChunk->bitsPerSample));
    result->push_back(info);
	m_explainationList.push_back(info);

    if (pFormatChunk->header.ChunkSize > sizeof(FormatChunk) - CHUNK_HEADER_SIZE){
        Extension *pFormatExtension = (Extension *)pFormatChunk->pData;

        info = make_shared<ExplainationInfo>("ExtensionSize: " + to_string(pFormatExtension->extSize),
                                            (char*)&pFormatExtension->extSize,
                                            sizeof(pFormatExtension->extSize));
        result->push_back(info);
		m_explainationList.push_back(info);
        if (pFormatExtension->extSize == 22 && pFormatChunk->audioFormat == static_cast<uint16_t>(WaveFormat::EXTENSIBLE)){
            WaveFormatExt *pWaveFormatExt = (WaveFormatExt *)(pFormatExtension->pData);

			info = make_shared<ExplainationInfo>("    ValidBitsPerSample: " + to_string(pWaveFormatExt->validBitsPerSample),
                                                (char*)&pWaveFormatExt->validBitsPerSample,
												sizeof(pWaveFormatExt->validBitsPerSample));
			result->push_back(info);
			m_explainationList.push_back(info);
			std::stringstream ss;
			ss << std::hex << std::uppercase << pWaveFormatExt->channelMask;
            string channelString;
            for (int i = 0; i < MAX_CHANNEL_NUMBER; i++){
                if (pWaveFormatExt->channelMask & (1 << i)){
                    if(channelString.length() == 0){
						channelString = m_channelMaskExplainTable[i];
                    } else {
                        channelString += "|" + m_channelMaskExplainTable[i];
                    }
                }
            }
            if(channelString.length() == 0){
                channelString = "Undefined";
            }
			info = make_shared<ExplainationInfo>("    ChannelMask: 0x" + ss.str() + " (" + channelString + ")",
												(char*)&pWaveFormatExt->channelMask,
												sizeof(pWaveFormatExt->channelMask));
			result->push_back(info);
			m_explainationList.push_back(info);

            string subFormat = "{";
			ss.str(std::string());
			ss.clear();
			ss << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << pWaveFormatExt->subFormat.Data1;
            uint16_t subAudioFormat = pWaveFormatExt->subFormat.Data1 & 0xFFFF;
            if(m_waveFormatExplainTable.find(static_cast<WaveFormat>(subAudioFormat)) != m_waveFormatExplainTable.end()){
                audioFormat = m_waveFormatExplainTable[static_cast<WaveFormat>(subAudioFormat)];
            }else{
                audioFormat = "Unknown";
            }

			subFormat += ss.str() + "(" + audioFormat + ")";
			ss.str(std::string());
			ss.clear();
			ss << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << pWaveFormatExt->subFormat.Data2;
			subFormat += "-" + ss.str();
			ss.str(std::string());
			ss.clear();
			ss << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << pWaveFormatExt->subFormat.Data3;
			subFormat += "-" + ss.str() + "-";
			for (int j = 0; j < 8; j++){
				ss.str(std::string());
				ss.clear();
				ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int)pWaveFormatExt->subFormat.Data4[j];
				subFormat += ss.str();
			}
            subFormat += "}";
            info = make_shared<ExplainationInfo>("    SubFormat: " + subFormat,
                                                (char*)&pWaveFormatExt->subFormat,
                                                sizeof(pWaveFormatExt->subFormat));
            result->push_back(info);
			m_explainationList.push_back(info);
        } else {
            if(pFormatExtension->extSize > 0){
                info = make_shared<ExplainationInfo>("    ExtensionData: " + string((char*)pFormatExtension->pData, pFormatExtension->extSize),
                                                    (char*)pFormatExtension->pData,
                                                    pFormatExtension->extSize);
                result->push_back(info);
				m_explainationList.push_back(info);
            }
        }
    }
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::FactChunkExplain(ChunkHeader *pChunkHeader){
    FactChunk *pFactChunk = (FactChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;
    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
										(char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pFactChunk->header.ChunkSize),
                                        (char*)&pFactChunk->header.ChunkSize,
                                        sizeof(pFactChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("FactSize: " + to_string(pFactChunk->factSize),
                                        (char*)&pFactChunk->factSize,
                                        sizeof(pFactChunk->factSize));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::DataChunkExplain(ChunkHeader *pChunkHeader){
    DataChunk *pDataChunk = (DataChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
										(char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pDataChunk->header.ChunkSize),
                                        (char*)&pDataChunk->header.ChunkSize,
                                        sizeof(pDataChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::DS64ChunkExplain(ChunkHeader *pChunkHeader){
    DS64Chunk *pDS64Chunk = (DS64Chunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
										(char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pDS64Chunk->header.ChunkSize),
                                        (char*)&pDS64Chunk->header.ChunkSize,
                                        sizeof(pDS64Chunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("RIFFSize: " + to_string(pDS64Chunk->riffSize),
                                        (char*)&pDS64Chunk->riffSize,
                                        sizeof(pDS64Chunk->riffSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("DataSize: " + to_string(pDS64Chunk->dataSize),
                                        (char*)&pDS64Chunk->dataSize,
                                        sizeof(pDS64Chunk->dataSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("FactSize: " + to_string(pDS64Chunk->factSize),
                                        (char*)&pDS64Chunk->factSize,
                                        sizeof(pDS64Chunk->factSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("TableLength: " + to_string(pDS64Chunk->tableLength),
                                        (char*)&pDS64Chunk->tableLength,
                                        sizeof(pDS64Chunk->tableLength));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::RIFFChunkHeader64Explain(ChunkHeader *pChunkHeader){
    RIFFChunkHeader64 *pRIFFChunkHeader64 = (RIFFChunkHeader64 *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
										(char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pRIFFChunkHeader64->header.ChunkSize),
                                        (char*)&pRIFFChunkHeader64->header.ChunkSize,
                                        sizeof(pRIFFChunkHeader64->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::JUNKChunkExplain(ChunkHeader *pChunkHeader){
    JUNKChunk *pJUNKChunk = (JUNKChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pJUNKChunk->header.ChunkSize),
                                        (char*)&pJUNKChunk->header.ChunkSize,
                                        sizeof(pJUNKChunk->header.ChunkSize));
	result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::BroadcastWaveFormatExtensionExplain(ChunkHeader *pChunkHeader){
    BroadcastWaveFormatExtension *pBroadcastWaveFormatExtension = (BroadcastWaveFormatExtension *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("length: " + to_string(pBroadcastWaveFormatExtension->header.ChunkSize),
                                        (char*)&pBroadcastWaveFormatExtension->header.ChunkSize,
                                        sizeof(pBroadcastWaveFormatExtension->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("Description: " + string(pBroadcastWaveFormatExtension->description, 256),
                                        (char*)pBroadcastWaveFormatExtension->description,
                                        sizeof(pBroadcastWaveFormatExtension->description));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("Originator: " + string(pBroadcastWaveFormatExtension->originator, 32),
                                        (char*)&pBroadcastWaveFormatExtension->originator,
                                        sizeof(pBroadcastWaveFormatExtension->originator));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("OriginatorReference: " + string(pBroadcastWaveFormatExtension->originatorReference, 32),
                                        (char*)&pBroadcastWaveFormatExtension->originatorReference,
                                        sizeof(pBroadcastWaveFormatExtension->originatorReference));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("OriginationDate: " + string(pBroadcastWaveFormatExtension->originationDate, 10),
                                        (char*)&pBroadcastWaveFormatExtension->originationDate,
                                        sizeof(pBroadcastWaveFormatExtension->originationDate));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("OriginationTime: " + string(pBroadcastWaveFormatExtension->originationTime, 8),
                                        (char*)&pBroadcastWaveFormatExtension->originationTime,
                                        sizeof(pBroadcastWaveFormatExtension->originationTime));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("Align: " + to_string(pBroadcastWaveFormatExtension->align),
                                        (char*)&pBroadcastWaveFormatExtension->align,
                                        sizeof(pBroadcastWaveFormatExtension->align));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("TimeReference: " + to_string(pBroadcastWaveFormatExtension->timeReference),
                                        (char*)&pBroadcastWaveFormatExtension->timeReference,
                                        sizeof(pBroadcastWaveFormatExtension->timeReference));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("Version: " + to_string(pBroadcastWaveFormatExtension->version),
                                        (char*)&pBroadcastWaveFormatExtension->version,
                                        sizeof(pBroadcastWaveFormatExtension->version));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("UMID: " + string(pBroadcastWaveFormatExtension->UMID, 64),
                                        (char*)&pBroadcastWaveFormatExtension->UMID,
                                        sizeof(pBroadcastWaveFormatExtension->UMID));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("LoudnessValue: " + to_string(pBroadcastWaveFormatExtension->loudnessValue),
                                        (char*)&pBroadcastWaveFormatExtension->loudnessValue,
                                        sizeof(pBroadcastWaveFormatExtension->loudnessValue));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("LoudnessRange: " + to_string(pBroadcastWaveFormatExtension->loudnessRange),
                                        (char*)&pBroadcastWaveFormatExtension->loudnessRange,
                                        sizeof(pBroadcastWaveFormatExtension->loudnessRange));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("MaxTruePeakLevel: " + to_string(pBroadcastWaveFormatExtension->maxTruePeakLevel),
                                        (char*)&pBroadcastWaveFormatExtension->maxTruePeakLevel,
                                        sizeof(pBroadcastWaveFormatExtension->maxTruePeakLevel));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("MaxMomentaryLoudness: " + to_string(pBroadcastWaveFormatExtension->maxMomentaryLoudness),
                                        (char*)&pBroadcastWaveFormatExtension->maxMomentaryLoudness,
                                        sizeof(pBroadcastWaveFormatExtension->maxMomentaryLoudness));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("MaxShortTermLoudness: " + to_string(pBroadcastWaveFormatExtension->maxShortTermLoudness),
                                        (char*)&pBroadcastWaveFormatExtension->maxShortTermLoudness,
                                        sizeof(pBroadcastWaveFormatExtension->maxShortTermLoudness));
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("Reserved 180 bytes: " + string(pBroadcastWaveFormatExtension->reserved, 180),
                                        (char*)&pBroadcastWaveFormatExtension->reserved,
                                        sizeof(pBroadcastWaveFormatExtension->reserved));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::PadChunkExplain(ChunkHeader *pChunkHeader){
    PadChunk *pPadChunk = (PadChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pPadChunk->header.ChunkSize),
                                        (char*)&pPadChunk->header.ChunkSize,
                                        sizeof(pPadChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::ListChunkExplain(ChunkHeader *pChunkHeader){
    LISTChunk *pListChunk = (LISTChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;
    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pListChunk->header.ChunkSize),
                                        (char*)&pListChunk->header.ChunkSize,
                                        sizeof(pListChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::CueChunkExplain(ChunkHeader *pChunkHeader){
    CueChunk *pCueChunk = (CueChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;
    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("length: " + to_string(pCueChunk->header.ChunkSize),
                                        (char*)&pCueChunk->header.ChunkSize,
                                        sizeof(pCueChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("CuePoints: " + to_string(pCueChunk->cuePoints),
                                        (char*)&pCueChunk->cuePoints,
                                        sizeof(pCueChunk->cuePoints));
    result->push_back(info);
	m_explainationList.push_back(info);

    CuePoint *pCuePoint = (CuePoint *)pCueChunk->pData;
    for(int i = 0; i < pCueChunk->cuePoints; i++){
        info = make_shared<ExplainationInfo>("    Name" + to_string(i) + ": " + to_string(pCuePoint[i].name),
                                            (char*)&pCuePoint[i].name,
                                            sizeof(pCuePoint[i].name));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    Position" + to_string(i) + ": " + to_string(pCuePoint[i].position),
                                            (char*)&pCuePoint[i].position,
                                            sizeof(pCuePoint[i].position));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    FccChunk" + to_string(i) + ": " + string(pCuePoint[i].fccChunk, 4),
                                            (char*)pCuePoint[i].fccChunk,
                                            sizeof(pCuePoint[i].fccChunk));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    ChunkStart" + to_string(i) + ": " + to_string(pCuePoint[i].chunkStart),
                                            (char*)&pCuePoint[i].chunkStart,
                                            sizeof(pCuePoint[i].chunkStart));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    BlockStart" + to_string(i) + ": " + to_string(pCuePoint[i].blockStart),
                                            (char*)&pCuePoint[i].blockStart,
                                            sizeof(pCuePoint[i].blockStart));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    SampleOffset" + to_string(i) + ": " + to_string(pCuePoint[i].sampleOffset),
                                            (char*)&pCuePoint[i].sampleOffset,
                                            sizeof(pCuePoint[i].sampleOffset));
        result->push_back(info);
		m_explainationList.push_back(info);
    }
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::PlaylistChunkExplain(ChunkHeader *pChunkHeader){
    PlaylistChunk *pPlaylistChunk = (PlaylistChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;
    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);
    info = make_shared<ExplainationInfo>("length: " + to_string(pPlaylistChunk->header.ChunkSize),
                                        (char*)&pPlaylistChunk->header.ChunkSize,
                                        sizeof(pPlaylistChunk->header.ChunkSize));
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("Segments: " + to_string(pPlaylistChunk->segments),
                                        (char*)&pPlaylistChunk->segments,
                                        sizeof(pPlaylistChunk->segments));
    result->push_back(info);
	m_explainationList.push_back(info);

    PlaySegment *pPlaySegment = (PlaySegment *)pPlaylistChunk->pData;
    for (int i = 0; i < pPlaylistChunk->segments; i++){
        info = make_shared<ExplainationInfo>("    Name" + to_string(i) + ": " + to_string(pPlaySegment[i].name),
                                            (char*)&pPlaySegment[i].name,
                                            sizeof(pPlaySegment[i].name));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    Length" + to_string(i) + ": " + to_string(pPlaySegment[i].length),
                                            (char*)&pPlaySegment[i].length,
                                            sizeof(pPlaySegment[i].length));
        result->push_back(info);
		m_explainationList.push_back(info);

        info = make_shared<ExplainationInfo>("    Loops" + to_string(i) + ": " + to_string(pPlaySegment[i].loops),
                                            (char*)&pPlaySegment[i].loops,
                                            sizeof(pPlaySegment[i].loops));
        result->push_back(info);
		m_explainationList.push_back(info);
    }
    return result;
}

shared_ptr<vector<shared_ptr<ExplainationInfo>>>WaveFile::UnknownChunkExplain(ChunkHeader *pChunkHeader){
    UnknownChunk *pUnknownChunk = (UnknownChunk *)pChunkHeader;
    shared_ptr<vector<shared_ptr<ExplainationInfo>>> result = make_shared<vector<shared_ptr<ExplainationInfo>>>();
    shared_ptr<ExplainationInfo> info;

    info = make_shared<ExplainationInfo>("'" + string(pChunkHeader->ChunkID, 4) + "'",
                                        (char*)pChunkHeader,
                                        CHUNK_HEADER_SIZE + pChunkHeader->ChunkSize);
    result->push_back(info);
	m_explainationList.push_back(info);

    info = make_shared<ExplainationInfo>("length: " + to_string(pUnknownChunk->header.ChunkSize),
                                        (char*)&pUnknownChunk->header.ChunkSize,
                                        sizeof(pUnknownChunk->header.ChunkSize));
	result->push_back(info);
	m_explainationList.push_back(info);
    return result;
}