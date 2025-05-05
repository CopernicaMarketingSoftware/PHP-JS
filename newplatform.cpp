/**
 *  NewPlatform.cpp
 * 
 *  Implementation file for the NewPlatform class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "newplatform.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  The single one instance
 *  @var Platform
 */
Platform *Platform::_instance = nullptr;

/**
 *  Private constructor (as this is a singleton)
 */
Platform::Platform() : _platform(v8::platform::NewSingleThreadedDefaultPlatform())
{
    // initialize all platform-stuff
    v8::V8::InitializeICUDefaultLocation(nullptr);
    v8::V8::InitializeExternalStartupData("/usr/share/v8/");
    v8::V8::InitializePlatform(_platform.get());
    v8::V8::Initialize();
}

/**
 *  Destructor
 */
Platform::~Platform()
{
    // cleanup stuff
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
}

/**
 *  Get the one and only instance
 *  @return Platform
 */
Platform *Platform::instance()
{
    // already set?
    if (_instance != nullptr) return _instance;
    
    // cerate the one and only instance right now
    return _instance = new Platform();
}

/**
 *  Cleanup the platform instance
 */
void Platform::shutdown()
{
    // if not even initialized
    if (_instance == nullptr) return;
    
    // destruct the one and only instance
    delete _instance;
    
    // set back to nullptr
    _instance = nullptr;
}


/**
 *  End of namespace
 */
}
