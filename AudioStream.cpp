#include "AudioStream.h"

// 参考Microsoft WASAPI官方文档：https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi

const GUID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const GUID IID_IAudioClient = __uuidof(IAudioClient);
const GUID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

DWORD __stdcall dataFeedingThread(void *obj)
{
    HRESULT hr;
    BYTE *pData;
    UINT32 paddingLength;
    int filled;
    UINT32 frames;
    UINT32 bytes;

    AudioStream *audioStreamInstance = (AudioStream *)obj;

    bool isContinue = true;
    while (isContinue) {
        DWORD r = WaitForMultipleObjects(2, audioStreamInstance->m_hEvents, FALSE, INFINITE);
        switch (r - WAIT_OBJECT_0) {
            case 0: // Feeding
                // Step1: 获取当前音频缓冲区中剩余的帧数（待播放帧数）
                hr = audioStreamInstance->m_audioClient->GetCurrentPadding(&paddingLength);
                if (FAILED(hr)) {
                    DEBUG_STRING("[ERROR] GetCurrentPadding failed: " + audioStreamInstance->getErrorString(hr));
                    isContinue = false;
                    break;
                }
                // Step2: 计算需要填充的帧数（缓冲区最大帧数-缓冲区中剩余帧数）
                frames = audioStreamInstance->m_blockSize - paddingLength; // calculate the frames to fill
                // Step3: 再换算出需要填充的字节数
                bytes = frames * audioStreamInstance->m_blockAlign;  // calculate the bytes to fill
                // Step4: 获取音频缓冲区指针
                hr = audioStreamInstance->m_audioRenderClient->GetBuffer(frames, &pData);
                if (FAILED(hr)) {
                    DEBUG_STRING("[ERROR] GetBuffer failed: " + audioStreamInstance->getErrorString(hr));
                    isContinue = false;
                    break;
                }

                filled = 0;
                if (pData) {
                    // 调用虚函数，由子类实现填充音频数据
					filled = audioStreamInstance->feedAudioData((char *)pData, bytes);
                }
                hr = audioStreamInstance->m_audioRenderClient->ReleaseBuffer(frames, 0);
                if (FAILED(hr)) {
                    DEBUG_STRING("[ERROR] ReleaseBuffer failed: " + audioStreamInstance->getErrorString(hr));
                    isContinue = false;
                    break;
                }

                if (filled < 0) {
                    DEBUG_STRING("[DEBUG] feeding ended because filled = " + std::to_string(filled));
                    isContinue = false;
                }
                break;
            case 1: // Stopping
                DEBUG_STRING("[INFO] Set stop");
                isContinue = false;
                break;
            default:
                DEBUG_PRINT("[ERROR] WaitForMultipleObjects return unknown or failed: %x", r);
                isContinue = false;
                break;
        }
    }
    audioStreamInstance->stop();
    DEBUG_STRING("[DEBUG] Thread end");
    return 0;
}

AudioStream::AudioStream(void):
    m_IMMDevice(NULL),
    m_audioClient(NULL),
    m_audioRenderClient(NULL),
    m_hThread(NULL),
    m_blockSize(0),
    m_blockAlign(0),
    m_isWorking(false),
    m_isPlaying(false),
    m_waveFormatEx(NULL),
    m_hEvents{NULL, NULL},
    m_audioErrorMapping{
        {0x88890001, "AUDCLNT_E_NOT_INITIALIZED"                    },
        {0x88890002, "AUDCLNT_E_ALREADY_INITIALIZED"                },
        {0x88890003, "AUDCLNT_E_WRONG_ENDPOINT_TYPE"                },
        {0x88890004, "AUDCLNT_E_DEVICE_INVALIDATED"                 },
        {0x88890005, "AUDCLNT_E_NOT_STOPPED"                        },
        {0x88890006, "AUDCLNT_E_BUFFER_TOO_LARGE"                   },
        {0x88890007, "AUDCLNT_E_OUT_OF_ORDER"                       },
        {0x88890008, "AUDCLNT_E_UNSUPPORTED_FORMAT"                 },
        {0x88890009, "AUDCLNT_E_INVALID_SIZE"                       },
        {0x8889000a, "AUDCLNT_E_DEVICE_IN_USE"                      },
        {0x8889000b, "AUDCLNT_E_BUFFER_OPERATION_PENDING"           },
        {0x8889000c, "AUDCLNT_E_THREAD_NOT_REGISTERED"              },
        {0x8889000d, "AUDCLNT_E_NO_SINGLE_PROCESS"                  },
        {0x8889000e, "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"         },
        {0x8889000f, "AUDCLNT_E_ENDPOINT_CREATE_FAILED"             },
        {0x88890010, "AUDCLNT_E_SERVICE_NOT_RUNNING"                },
        {0x88890011, "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"           },
        {0x88890012, "AUDCLNT_E_EXCLUSIVE_MODE_ONLY"                },
        {0x88890013, "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"       },
        {0x88890014, "AUDCLNT_E_EVENTHANDLE_NOT_SET"                },
        {0x88890015, "AUDCLNT_E_INCORRECT_BUFFER_SIZE"              },
        {0x88890016, "AUDCLNT_E_BUFFER_SIZE_ERROR"                  },
        {0x88890017, "AUDCLNT_E_CPUUSAGE_EXCEEDED"                  },
        {0x88890018, "AUDCLNT_E_BUFFER_ERROR"                       },
        {0x88890019, "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"            },
        {0x88890020, "AUDCLNT_E_INVALID_DEVICE_PERIOD"              },
        {0x88890021, "AUDCLNT_E_INVALID_STREAM_FLAG"                },
        {0x88890022, "AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE"       },
        {0x88890023, "AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES"           },
        {0x88890024, "AUDCLNT_E_OFFLOAD_MODE_ONLY"                  },
        {0x88890025, "AUDCLNT_E_NONOFFLOAD_MODE_ONLY"               },
        {0x88890026, "AUDCLNT_E_RESOURCES_INVALIDATED"              },
        {0x88890027, "AUDCLNT_E_RAW_MODE_UNSUPPORTED"               },
        {0x88890028, "AUDCLNT_E_ENGINE_PERIODICITY_LOCKED"          },
        {0x88890029, "AUDCLNT_E_ENGINE_FORMAT_LOCKED"               },
        {0x88890030, "AUDCLNT_E_HEADTRACKING_ENABLED"               },
        {0x88890040, "AUDCLNT_E_HEADTRACKING_UNSUPPORTED"           },
        {0x88890041, "AUDCLNT_E_EFFECT_NOT_AVAILABLE"               },
        {0x88890042, "AUDCLNT_E_EFFECT_STATE_READ_ONLY"             },
        {0x88890043, "AUDCLNT_E_POST_VOLUME_LOOPBACK_UNSUPPORTED"   }
    }
    {

    HRESULT hr;

    // Step1: 获取IMMDeviceEnumerator接口
    IMMDeviceEnumerator *enumerator = NULL;
    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void **)&enumerator);
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] CoCreateInstance failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }
    // Step2: 获取默认的音频渲染设备
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_IMMDevice); // use default
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] GetDefaultAudioEndpoint failed: " + getErrorString(hr));
        enumerator->Release();
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }
    enumerator->Release();
    // Step3: 获取设备ID，这里似乎只是为了调试时知道用的是哪个默认的音频渲染设备？
    LPWSTR strid;
    hr = m_IMMDevice->GetId(&strid);
    if( FAILED(hr)) {
        DEBUG_STRING("[ERROR] GetId failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }
    DEBUG_PRINT("[DEBUG] Use default device, the id is \"%ls\"", strid);
    // Step4：释放设备ID
    CoTaskMemFree(strid); // free by this

    // 创建两个事件，一个用于填充数据，一个用于停止填充数据
    m_hEvents[EnumHelp::toInt(AudioEvent::Feeding)]  = CreateEvent(NULL, false, false, NULL);
    m_hEvents[EnumHelp::toInt(AudioEvent::Stopping)] = CreateEvent(NULL, false, false, NULL);
    m_audioRenderClient = NULL;
    m_audioClient = NULL;
    m_blockAlign = 0;
    m_isWorking = false;
    m_isPlaying = false;
}

AudioStream::~AudioStream()
{
    stop();
    CloseHandle(m_hEvents[EnumHelp::toInt(AudioEvent::Feeding)]);
    CloseHandle(m_hEvents[EnumHelp::toInt(AudioEvent::Stopping)]);
    m_IMMDevice->Release();
    // CoUninitialize(); // final
}

string AudioStream::getErrorString(HRESULT hr)
{
    if (hr == S_OK) {
        return "S_OK";
    }

    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << hr;
    if (m_audioErrorMapping.find(hr) != m_audioErrorMapping.end()) {
        return m_audioErrorMapping[hr] + "(" + ss.str() + ")";
    }

    return "Unknown error(" + ss.str() + ")";
}

void AudioStream::setFormat(WAVEFORMATEXTENSIBLE *fmt)
{
    if (fmt) {
        m_waveFormatEx = fmt;
        m_blockAlign = m_waveFormatEx->Format.nBlockAlign;
    }
}

void AudioStream::play(void)
{
    if(m_waveFormatEx == nullptr) {
        DEBUG_STRING("[ERROR] No format");
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, 0));
        return;
    }

    if (m_isWorking) {
        stop();
    }

    HRESULT hr;
    // Step1: 获取IAudioClient接口
    hr = m_IMMDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void **)&m_audioClient);
    if(FAILED(hr)) {
        DEBUG_STRING("[ERROR] Activate failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }

    // Step2: 初始化IAudioClient接口
    // 如果指定的WAVEFORMATEX中的格式不支持，初始化IAudioClient时可能会触发错误
    hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,    // 使用共享模式，一般来说共享模式会通过自动插入MIXER来支持更多的格式
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK                       // 使用事件回调
        | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM                    // 自动转换PCM格式
        | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,              // 使用默认质量
        0, 0, &m_waveFormatEx->Format, NULL);
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] Initialize failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }

    // Step3: 获取设备音频缓冲区能容纳的音频帧数，该大小与m_audioClient->Initialize()的hnsBufferDuration参数有关，
    // 但有可能仍会不同，因此建议例行调用m_audioClient->GetBufferSize()，以确保缓存区帧数正确
    hr = m_audioClient->GetBufferSize(&m_blockSize);
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] GetBufferSize failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }
    DEBUG_PRINT("[DEBUG] m_blockSize = %d\n", m_blockSize);

    // Step4: 向IAudioClient注册事件，每当音频缓冲区需要填充数据时，事件就会被触发
    hr = m_audioClient->SetEventHandle(m_hEvents[EnumHelp::toInt(AudioEvent::Feeding)]);
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] SetEventHandle failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }

    // Step5: 获取IAudioRenderClient接口，如果是录音的话，这里应该使用IID_IAudioCaptureClient
    hr = m_audioClient->GetService(IID_IAudioRenderClient, (void **)&m_audioRenderClient);
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] GetService failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }

    // Step6: 根据音频缓冲区能容纳的音频帧数申请音频帧缓存空间。
    // 音频帧的字节大小是通过流中的通道数量乘以每个通道的样本大小来计算的。
    // 例如，对于具有16位采样的立体声（2声道）流，帧大小为4个字节。
    BYTE *pData;
    hr = m_audioRenderClient->GetBuffer(m_blockSize, &pData);   // 可以申请不超过前面m_audioClient->GetBufferSize()返回的帧数缓存空间
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] GetBuffer failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }
    // Step7: 调用虚函数用音频数据填充音频帧缓存空间
    feedAudioData((char *)pData, m_blockAlign * m_blockSize);

    // Step8: 释放音频帧缓存空间，从而将音频数据提交给音频引擎(audio engine)
    hr = m_audioRenderClient->ReleaseBuffer(m_blockSize, 0);
    if (FAILED(hr)) {
        DEBUG_STRING("[ERROR] ReleaseBuffer failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }

    // Step9: 创建一个线程来处理音频数据的填充，并将线程的优先级设置为时间关键型，以确保音频数据的及时填充
    m_hThread = CreateThread(0, 0, dataFeedingThread, this, 0, NULL);
    if (m_hThread == NULL) {
        DEBUG_STRING("[ERROR] CreateThread failed");
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, 0));
        return;
    }
    if(!SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL)){
        DEBUG_STRING("[WARNING] SetThreadPriority failed");
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, 0));
    }

    // Step10: 启动音频引擎，开始播放音频数据
    hr = m_audioClient->Start();
    if (FAILED(hr)) {
        DEBUG_STRING("[DEBUG] Start failed: " + getErrorString(hr));
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Error, hr));
        return;
    }
    m_isWorking = true;
    m_isPlaying = true;
    EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Playing, 0));
}

void AudioStream::stop(void)
{
    if (m_isWorking) {
        m_isWorking = false;
        m_isPlaying = false;
        SetEvent(m_hEvents[EnumHelp::toInt(AudioEvent::Stopping)]);  // 设置事件为已触发，这样dataFeedingThread的WaitForMultipleObjects就会迅速返回
        WaitForSingleObject(m_hThread, 1000);       // 然后等待线程结束，最多等1秒，结束后清理资源
        if (m_audioClient) {
            m_audioClient->Stop();
            m_audioClient->Release();
            m_audioClient = NULL;
        }
        if (m_audioRenderClient) {
            m_audioRenderClient->Release();
            m_audioRenderClient = NULL;
        }
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Stop, 0));
    }
}

void AudioStream::pause(void)
{
    if (m_isWorking && m_isPlaying && m_audioClient) {
        m_audioClient->Stop();
        m_isPlaying = false;
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Paused, 0));
    }
}

void AudioStream::resume(void)
{
    if (m_isWorking && !m_isPlaying && m_audioClient) {
        m_audioClient->Start();
        m_isPlaying = true;
        EventQueue::getInstance()->pushEventIntoQueue(make_shared<Event>(EventName::Resumed, 0));
    }
}

bool AudioStream::isPlaying(void){
    return m_isPlaying;
}