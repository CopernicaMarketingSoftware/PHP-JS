/**
 *  Isolate.cpp
 * 
 *  Implementation for the Isolate class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2026 Copernica BV
 */

/**
 *  Dependencies
 */
#include "isolate.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  The create-params
 *  v8::Isolate::CreateParams
 */
v8::Isolate::CreateParams Isolate::_params;

/**
 *  The underlying isolate
 *  @var    v8::Isolate*
 */
v8::Isolate *Isolate::_isolate;

/**
 *  Templates for wrapping objects
 *  @var std::vector
 */
std::vector<Template> Isolate::_templates;

/**
 *  Total number of instances
 *  @var size_t
 */
size_t Isolate::_instances = 0;

/**
 *  End of namespace
 */
}
