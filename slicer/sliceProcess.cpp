#include "sliceProcess.h"
#include"iostream"
#include "MeshGroup.h"
#include "infill.h"

namespace cura {

    slicerProcess::slicerProcess(const char* filename_) {
        filename = filename_;
        load_stl_file();
    }

    void slicerProcess::load_stl_file() {
        int read_success;
        read_success = cura::loadMeshSTL_binary(&mesh, filename, matrix);
        if (read_success == 1) {
            std::cout << "read_successful" << std::endl;
        }
    }
    void slicerProcess::compute() {
        coord_t hight = mesh.Max().z - mesh.Min().z;
        const cura::coord_t thick = 300;
        const size_t num_layer = hight / thick;
        std::cout << num_layer;
        Slicer slicer(&mesh, thick, num_layer);
        //std::cout<<mesh.max();
        layers = slicer.layers;
        if (has_zvalue) {
            for (size_t i = 0; i < num_layer; i++) {
                z_values.push_back(thick + i * thick);
            }
        }
        for (size_t i = 0; i < layers.size(); ++i) {
            layers[i].makePolygons();
            Results.push_back(infill_process(layers[i].polygons, 1000, 0.0, z_values[i]));
        }


    }

    void slicerProcess::has_z() {
        this->has_zvalue = true;
    }

    void slicerProcess::get_points() {
        for (size_t i = 0; i < layers.size(); ++i) {
            for (size_t j = 0; j < layers[i].polygons.size(); ++j) {
                for (size_t k = 0; k < layers[i].polygons[j].size(); ++k) {
                    Point p = layers[i].polygons[j][k];
                    points.push_back(Point3(p.X, p.Y, z_values[i]));
                }
            }


        }
    }

    Polygons slicerProcess::infill_process(const Polygons& in_outline, const coord_t line_distance, const double& infill_rotation, coord_t z) {
        Polygons results;
        Polygons lines;
        bool connect_polygons = true;
        coord_t outline_offset = 10;
        coord_t infill_line_width = line_distance;
        AngleDegrees fill_angle;
        coord_t shift = 0;
        Infill infill(connect_polygons, in_outline, outline_offset, infill_line_width, line_distance, infill_rotation, z, shift);
        infill.generate(results, lines);
        return lines;
    }

    void slicerProcess::get_lines() {

        size_t I = Results.size();
        for (size_t i = 0; i < I; ++i) {
            size_t J = Results[i].size();
            //              std::vector<Point3> p;
            for (size_t j = 0; j < J; ++j) {
                for (size_t k = 0; k < 2; ++k) {
                    Point p = Results[i].paths[j][k];
                    P.push_back(cura::Point3(p.X, p.Y, z_values[i]));
                }
            }
            //              P.push_back(std::move(p));

        }


    }

}