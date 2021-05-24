#ifndef PATH_PLANNING_COMB_PATH_H
#define PATH_PLANNING_COMB_PATH_H

#include "IntPoint.h"

namespace cura
{

class CombPath : public  std::vector<Point> //!< A single path either inside or outise the parts
{
public:
    bool cross_boundary = false; //!< Whether the path crosses a boundary.
};

}//namespace cura

#endif