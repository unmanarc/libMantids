#ifndef A_UINT8_H
#define A_UINT8_H

#include "a_var.h"
#include <stdint.h>
#include <mdz_thr_mutex/mutex_shared.h>

namespace Mantids { namespace Memory { namespace Abstract {

class UINT8: public Var
{
public:
    UINT8();
    UINT8(const uint8_t & value);
    UINT8& operator=(const uint8_t & _value)
    {
        setValue(_value);
        return *this;
    }

    uint8_t getValue();
    bool setValue(const uint8_t & value);

    void * getDirectMemory() override { return &value; }

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    Var * protectedCopy() override;
private:
    uint8_t value;
    Threads::Sync::Mutex_Shared mutex;

};

}}}

#endif // A_UINT8_H
