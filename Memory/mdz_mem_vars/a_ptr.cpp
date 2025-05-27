#include "a_ptr.h"
#include <inttypes.h>
#include <mdz_thr_mutex/lock_shared.h>

#include <stdexcept>      // std::invalid_argument
using namespace Mantids::Memory::Abstract;

PTR::PTR()
{
    value = nullptr;
    setVarType(TYPE_PTR);
}

PTR::PTR(void *_value)
{
    setVarType(TYPE_PTR);
    this->value = _value;
}

void * PTR::getValue()
{
    Threads::Sync::Lock_RD lock(mutex);
    return value;
}

bool PTR::setValue(void * _value)
{
    Threads::Sync::Lock_RW lock(mutex);

    this->value = _value;
    return true;
}

std::string PTR::toString()
{
    Threads::Sync::Lock_RD lock(mutex);

    char ovalue[256];
    void * ptr = value;
    snprintf(ovalue,sizeof(ovalue),"%.8lX", (uintptr_t)ptr);
    return ovalue;
}

bool PTR::fromString(const std::string &_value)
{
    Threads::Sync::Lock_RW lock(mutex);

    if (_value.empty())
    {
        this->value = nullptr;
        return true;
    }
    this->value = (void *)(strtol( _value.c_str(), nullptr, 16 ));
    return true;
}

Var *PTR::protectedCopy()
{
    Threads::Sync::Lock_RD lock(mutex);

    PTR * var = new PTR;
    if (var) *var = this->value;
    return var;
}
