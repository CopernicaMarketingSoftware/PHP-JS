/**
 *  NewPlatform.h
 *
 *  The global platform in which all operations run. A platform holds the
 *  Javascript environment, on top of which multiple isolates can be running.
 *
 *  Think of a platform as the entire browser, and isolates as the environments
 *  in each tab. The platform has to be initialized only once.
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
#include <v8.h>
#include <libplatform/libplatform.h>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Platform
{
private:
    /**
     *  Pointer to the actual platform
     *  @var std::unique_ptr<v8::Platform>
     */
    std::unique_ptr<v8::Platform> _platform;

    /**
     *  The single one instance
     *  @var Platform
     */
    static Platform *_instance;


private:
    /**
     *  Private constructor (as this is a singleton)
     */
    Platform();
    
    /**
     *  Destructor
     */
    virtual ~Platform();

public:
    /**
     *  Get the one and only instance
     *  @return Platform
     */
    static Platform *instance();
    
    /**
     *  Cleanup the platform instance
     */
    static void shutdown();
};

/**
 *  End of namespace
 */
}

