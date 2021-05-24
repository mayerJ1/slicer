//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.
#pragma once
#include "macros.h"

#include "ZigzagConnectorProcessor.h"

namespace cura
{

    class Polygons;

    /*!
     * This processor adds no connection. This is for line infill pattern.
     */
    class NoZigZagConnectorProcessor : public ZigzagConnectorProcessor
    {
    public:
        NoZigZagConnectorProcessor(const PointMatrix& rotation_matrix, Polygons& result)
            : ZigzagConnectorProcessor(rotation_matrix, result,
                false, false) // settings for skipping some zags, no use here
        {
        }

        void registerVertex(const Point& vertex);
        void registerScanlineSegmentIntersection(const Point& intersection, int scanline_index);
        void registerPolyFinished();
    };


}


