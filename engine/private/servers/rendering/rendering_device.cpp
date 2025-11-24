#include "servers/rendering/rendering_device.h"


golias::RID golias::RIDAllocator::allocate_rid() {
    return next_rid++;
}
