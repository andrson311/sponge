#pragma once

#include <vector>
#include "bitmap.h"

int ConvertEquirectangularImageToCubemap(const Bitmap &b, std::vector<Bitmap> &Cubemap);