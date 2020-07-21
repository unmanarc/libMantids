#ifndef NULLCONTAINER_H
#define NULLCONTAINER_H

#include <cx2_mem_streams/streamable.h>

namespace CX2 { namespace Memory { namespace Containers {

class NullContainer : public Streams::Streamable
{
public:
    NullContainer();

    bool streamTo(Memory::Streams::Streamable * out, Streams::Status & wrsStat) override;
    Memory::Streams::Status write(const void * buf, const size_t &count, Streams::Status & wrStatUpd) override;

    uint64_t size() const override { return bytes; }

private:
    uint64_t bytes;
};

}}}

#endif // NULLCONTAINER_H
