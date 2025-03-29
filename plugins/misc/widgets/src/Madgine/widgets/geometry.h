#pragma once

namespace Engine {
namespace Widgets {

    struct Geometry {
        Matrix3 mPos;
        Matrix3 mSize;
    };

    struct GeometrySourceInfo {
        uint16_t mPos[9] = { 0 };
        uint16_t mSize[9] = { 0 };
    };

}
}