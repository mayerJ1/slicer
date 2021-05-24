#ifndef SLICEPROCESS_H
#define SLICEPROCESS_H
#include "mesh.h"
#include "MeshGroup.h"
#include "slicer.h"
#include "FMatrix4x3.h"
#include "polygon.h"
#include "str.h"
#include "FMatrix4x3.h"

namespace cura{
class slicerProcess {
public:
  slicerProcess(const char* filename);
  
  ~slicerProcess() {}
  std::vector<SlicerLayer> layers;

  void load_stl_file();
  //for visulazation
  void has_z();
  void compute();
  //for visulazation
  void get_points();
  void get_lines();
  Polygons infill_process(const Polygons& in_outline, const coord_t line_distance, const double& infill_rotation, coord_t z);
  Mesh mesh;
  std::vector<coord_t> z_values;
  const char* filename;
  bool has_zvalue = false;
  FMatrix4x3 matrix;
  std::vector<cura::Point3> points;
  std::vector<cura::Point3> P;
  std::vector<Polygons> Results;
};

}


#endif
