#ifndef EventQueueH
#define EventQueueH

#include <any>
#include <queue>
#include <mutex>

using namespace std;

enum class EventName: int{
    None,

    Playing,
    Paused,
    Resumed,
    Stop,
    Error,

    Exit
};

class Event{
public:
    EventName m_eventName;
    std::any m_eventParam;

    Event(EventName eventName, std::any param):
        m_eventName(eventName),
        m_eventParam(param)
    {
    }
    // 拷贝构造函数
    Event(const Event& event):
        m_eventName(event.m_eventName),
        m_eventParam(event.m_eventParam)
    {
    }
    // 移动构造函数
    Event(Event&& event):
        m_eventName(event.m_eventName),
        m_eventParam(event.m_eventParam)
    {
    }
    // 赋值运算符重载
    Event& operator=(const Event& event){
        m_eventName = event.m_eventName;
        m_eventParam = event.m_eventParam;
        return *this;
    }
    // 移动赋值运算符重载
    Event& operator=(Event&& event){
        m_eventName = event.m_eventName;
        m_eventParam = event.m_eventParam;
        return *this;
    }

    virtual ~Event(){};
};


class EventQueue{
private:
    static std::mutex m_mtxForEventQueue;
    std::queue<shared_ptr<Event>> m_eventQueue;
    EventQueue(){}
    ~EventQueue(){clear();}
public:
    static EventQueue* getInstance(void){
        static EventQueue instance; // 静态局部变量，程序运行期间只会被初始化一次
        return &instance;
    }
    void pushEventIntoQueue(shared_ptr<Event> event);
    shared_ptr<Event> popEventFromQueue(void);
    void clear(void);
};
#endif