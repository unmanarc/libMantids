#ifndef A_BOOL_H
#define A_BOOL_H

#include "a_var.h"
#include <stdint.h>
#include <mdz_thr_mutex/mutex_shared.h>

namespace Mantids { namespace Memory { namespace Abstract {
class BOOL: public Var
{
public:
    BOOL();
    BOOL(const bool & value);
    BOOL& operator=(const bool & _value)
    {
        setValue(_value);
        return *this;
    }

    bool getValue();
    bool setValue(bool value);

    void * getDirectMemory() override { return &value; }

    std::string toString() override;
    bool fromString(const std::string & value) override;
protected:
    Var * protectedCopy() override;
private:
    bool value;
    Threads::Sync::Mutex_Shared mutex;

};
}}}
#endif // A_BOOL_H
