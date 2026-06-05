#pragma once

namespace Engine {
namespace Widgets {

    struct Geometry {
        Math::Matrix3 mPos;
        Math::Matrix3 mSize;
    };

    struct GeometrySourceInfo {
        uint16_t mPos[9] = { 0 };
        uint16_t mSize[9] = { 0 };
    };

}
}