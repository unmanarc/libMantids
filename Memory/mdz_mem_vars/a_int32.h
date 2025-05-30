#ifndef A_INT32_H
#define A_INT32_H

#include "a_var.h"
#include <stdint.h>
#include <mdz_thr_mutex/mutex_shared.h>

namespace Mantids { namespace Memory { namespace Abstract {

class INT32: public Var
{
public:
    INT32();
    INT32(const int32_t &value);
    INT32& operator=(const int32_t & _value)
    {
        setValue(_value);
        return *this;
    }

    int32_t getValue();
    bool setValue(const int32_t & value);

    void * getDirectMemory() override { return &value; }

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    Var * protectedCopy() override;

private:
    int32_t value;
    Threads::Sync::Mutex_Shared mutex;

};

}}}

#endif // A_INT32_H
