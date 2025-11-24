#include "servers/rendering/rendering_device.h"


RID RIDAllocator::allocate_rid() {
    return next_rid++;
}
