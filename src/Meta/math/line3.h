#pragma once

#include "line2.h"

namespace Engine {
namespace Math {

    struct Line3 {

        Vector3 mPointA;
        Vector3 mPointB;

        Line2 xy() const
        {
            return {
                mPointA.xy(),
                mPointB.xy()
            };
        }
    };

}
}