#pragma once

#include "../renderdata.h"

#include "sizeconstraints.h"

namespace Engine {
namespace Widgets {

    struct GridLayoutRenderData {
        template <typename HorizontalLayout, typename VerticalLayout>
        static std::pair<Vector2, Vector3> getCellContentDimensions(HorizontalLayout &hLayout, VerticalLayout &vLayout, size_t row, size_t col)
        {
            auto heightDimension = hLayout.getElementDimensions(row);
            auto widthDimension = vLayout.getElementDimensions(col);

            Vector2 cellPos = {
                widthDimension.first,
                heightDimension.first
            };
            Vector3 cellSize = {
                widthDimension.second,
                heightDimension.second,
                1.0f
            };

            return { cellPos, cellSize };
        }
    };

}
}