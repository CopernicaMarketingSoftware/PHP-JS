/**
 *  Timeout.h
 * 
 *  Class that terminates a running script when it runs for too long
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <chrono>
#include <condition_variable>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Timeout
{
private:
    /**
     *  Mutex to protect resources
     *  @var std::mutex
     */
    std::mutex _mutex;
    
    /**
     *  The isolate that must be terminated
     *  @var v8::Isolate
     */
    v8::Isolate *_isolate;

    /**
     *  Point in time when to expire
     *  @var std::chrono::time_point
     */
    std::chrono::steady_clock::time_point _expire;

    /**
     *  Condition variable to wake up the other thread
     *  @var std::condition_variable
     */
    std::condition_variable _cv;
    
    /**
     *  The thread that is running
     *  @var std::thread
     */
    std::thread _thread;
    
    
    /**
     *  Run the thread
     */
    void run()
    {
        // obtain a lock to access the shared resource
        std::unique_lock<std::mutex> lock(_mutex);
        
        // while the isolate has not yet been killed
        while (_isolate != nullptr)
        {
            // wait for the conditio variable
            auto result = _cv.wait_until(lock, _expire);
            
            // proceed if the timeout has not yet been reached
            if (result != std::cv_status::timeout) continue;
            
            // double-check
            if (_isolate == nullptr) break;
            
            // terminate execution
            _isolate->TerminateExecution();
            
            // forget the isolate, we no longer have to terminate
            _isolate = nullptr;
        }
    }
    
public:
    /**
     *  Constructor
     *  @param  isolate
     *  @param  timeout
     */
    Timeout(v8::Isolate *isolate, time_t timeout) : _isolate(isolate), _expire(std::chrono::steady_clock::now() + std::chrono::seconds(timeout))
    {
        // if there is no timeout
        if (timeout == 0) return;
        
        // start a thread
        _thread = std::thread(std::bind(&Timeout::run, this));
    }
    
    /**
     *  No copying
     *  @param  that
     */
    Timeout(const Timeout &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Timeout()
    {
        // if never started in the first place
        if (!_thread.joinable()) return;
        
        // obtain a lock to access the shared resource
        std::unique_lock<std::mutex> lock(_mutex);
        
        // reset the isolate so that the thread can no longer terminate execution
        _isolate = nullptr;
        
        // wake up the other thread
        _cv.notify_one();
        
        // join the thread
        _thread.join();
    }
};
    
/**
 *  End of namespace
 */
}

