#ifndef A_INT16_H
#define A_INT16_H

#include "a_var.h"
#include <stdint.h>
#include <mdz_thr_mutex/mutex_shared.h>

namespace Mantids { namespace Memory { namespace Abstract {

class INT16: public Var
{
public:
    INT16();
    INT16(const int16_t & value);
    INT16& operator=(const int16_t & _value)
    {
        setValue(_value);
        return *this;
    }

    int16_t getValue();
    bool setValue(const int16_t & value);

    void * getDirectMemory() override { return &value; }

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    Var * protectedCopy() override;

private:
    int16_t value;
    Threads::Sync::Mutex_Shared mutex;

};
}}}

#endif // A_INT16_H
