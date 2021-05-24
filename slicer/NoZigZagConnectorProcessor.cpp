//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef INFILL_NO_ZIGZAG_CONNECTOR_PROCESSOR_H
#define INFILL_NO_ZIGZAG_CONNECTOR_PROCESSOR_H

#include "NoZigZagConnectorProcessor.h"

namespace cura
{

    void NoZigZagConnectorProcessor::registerVertex(const Point&)
    {
        //No need to add anything.
    }

    void NoZigZagConnectorProcessor::registerScanlineSegmentIntersection(const Point&, int)
    {
        //No need to add anything.
    }

    void NoZigZagConnectorProcessor::registerPolyFinished()
    {
        //No need to add anything.
    }

/*!
 * This processor adds no connection. This is for line infill pattern.
 */


} // namespace cura


#endif // INFILL_NO_ZIGZAG_CONNECTOR_PROCESSOR_H
