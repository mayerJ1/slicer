#ifndef INFILL_H
#define INFILL_H
#include "sliceProcess.h"
#include "sliceData.h"
#include "ZigZagConnectorProcessor.h"
#include "AngleDegrees.h"
#include "IntPoint.h"

namespace cura
{

    class AABB;
    class slicerProcess;
    class Infill
    {

        static constexpr int perimeter_gaps_extra_offset = 15; // extra offset so that the perimeter gaps aren't created everywhere due to rounding errors
        bool connect_polygons; //!< Whether to connect as much polygons together into a single path
        const Polygons& in_outline; //!< a reference polygon for getting the actual area within which to generate infill (see outline_offset)
        coord_t outline_offset; //!< Offset from Infill::in_outline to get the actual area within which to generate infill
        coord_t infill_line_width; //!< The line width of the infill lines to generate
        coord_t line_distance; //!< The distance between two infill lines / polygons
        AngleDegrees fill_angle; //!< for linear infill types: the angle of the infill lines (or the angle of the grid)
        coord_t z; //!< height of the layer for which we generate infill
        coord_t shift; //!< shift of the scanlines in the direction perpendicular to the fill_angle
        const Point infill_origin; //!< origin of the infill pattern
        bool connected_zigzags; //!< (ZigZag) Whether endpieces of zigzag infill should be connected to the nearest infill line on both sides of the zigzag connector
        bool use_endpieces; //!< (ZigZag) Whether to include endpieces: zigzag connector segments from one infill line to itself
        bool skip_some_zags;  //!< (ZigZag) Whether to skip some zags
        size_t zag_skip_count;  //!< (ZigZag) To skip one zag in every N if skip some zags is enabled
        bool mirror_offset; //!< Indication in which offset direction the extra infill lines are made

        static constexpr double one_over_sqrt_2 = 0.7071067811865475244008443621048490392848359376884740; //!< 1.0 / sqrt(2.0)
    public:
        /*!
         * \warning If \p perimeter_gaps is given, then the difference between the \p in_outline
         * and the polygons which result from offsetting it by the \p outline_offset
         * and then expanding it again by half the \p infill_line_width
         * is added to the \p perimeter_gaps
         *
         * \param[out] perimeter_gaps (optional output) The areas in between consecutive insets when Concentric infill is used.
         */
        Infill(bool connect_polygons
            , const Polygons& in_outline
            , coord_t outline_offset
            , coord_t infill_line_width
            , coord_t line_distance
            , AngleDegrees fill_angle
            , coord_t z
            , coord_t shift
            , const Point& infill_origin = Point()
            , bool connected_zigzags = true
            , bool use_endpieces = false
            , bool skip_some_zags = false
        ) :connect_polygons(connect_polygons)
            , in_outline(in_outline)
            , outline_offset(outline_offset)
            , infill_line_width(infill_line_width)
            , line_distance(line_distance)
            , fill_angle(fill_angle)
            , z(z)
            , shift(shift)
            , infill_origin(infill_origin)
            , connected_zigzags(connected_zigzags)
            , use_endpieces(use_endpieces)
            , skip_some_zags(skip_some_zags)
        {
        }

        /*!
         * Generate the infill.
         *
         * \param result_polygons (output) The resulting polygons (from concentric infill)
         * \param result_lines (output) The resulting line segments (from linear infill types)
         * \param mesh The mesh for which to generate infill (should only be used for non-helper objects)
         */
        void generate(Polygons& result_polygons, Polygons& result_lines);

    private:
        void _generate(Polygons& result_polygons, Polygons& result_lines);


        struct InfillLineSegment
        {
            /*!
             * Creates a new infill line segment.
             *
             * The previous and next line segments will not yet be connected. You
             * have to set those separately.
             * \param start Where the line segment starts.
             * \param end Where the line segment ends.
             */
            InfillLineSegment(const Point start, const size_t start_segment, const size_t start_polygon, const Point end, const size_t end_segment, const size_t end_polygon)
                : start(start)
                , start_segment(start_segment)
                , start_polygon(start_polygon)
                , end(end)
                , end_segment(end_segment)
                , end_polygon(end_polygon)
                , previous(nullptr)
                , next(nullptr)
            {
            }

            /*!
             * Where the line segment starts.
             */
            Point start;

            /*!
             * Which polygon line segment the start of this infill line belongs to.
             *
             * This is an index of a vertex in the PolygonRef that this infill line
             * is inside. It is used to disambiguate between the start and end of
             * the line segment.
             */
            size_t start_segment;

            /*!
             * Which polygon the start of this infill line belongs to.
             *
             * This is an index of a PolygonRef that this infill line
             * is inside. It is used to know which polygon the start segment belongs to.
             */
            size_t start_polygon;

            /*!
             * Where the line segment ends.
             */
            Point end;

            /*!
             * Which polygon line segment the end of this infill line belongs to.
             *
             * This is an index of a vertex in the PolygonRef that this infill line
             * is inside. It is used to disambiguate between the start and end of
             * the line segment.
             */
            size_t end_segment;

            /*!
             * Which polygon the end of this infill line belongs to.
             *
             * This is an index of a PolygonRef that this infill line
             * is inside. It is used to know which polygon the end segment belongs to.
             */
            size_t end_polygon;

            /*!
             * The previous line segment that this line segment is connected to, if
             * any.
             */
            InfillLineSegment* previous;

            /*!
             * The next line segment that this line segment is connected to, if any.
             */
            InfillLineSegment* next;

            /*!
             * Compares two infill line segments for equality.
             *
             * This is necessary for putting line segments in a hash set.
             * \param other The line segment to compare this line segment with.
             */
            bool operator ==(const InfillLineSegment& other) const;
        };

        /*!
         * Stores the infill lines (a vector) for each line of a polygon (a vector)
         * for each polygon in a Polygons object that we create a zig-zaggified
         * infill pattern for.
         */
        std::vector<std::vector<std::vector<InfillLineSegment*>>> crossings_on_line;


        /*!
         * Convert a mapping from scanline to line_segment-scanline-intersections (\p cut_list) into line segments, using the even-odd rule
         * \param[out] result (output) The resulting lines
         * \param rotation_matrix The rotation matrix (un)applied to enforce the angle of the infill
         * \param scanline_min_idx The lowest index of all scanlines crossing the polygon
         * \param line_distance The distance between two lines which are in the same direction
         * \param boundary The axis aligned boundary box within which the polygon is
         * \param cut_list A mapping of each scanline to all y-coordinates (in the space transformed by rotation_matrix) where the polygons are crossing the scanline
         * \param total_shift total shift of the scanlines in the direction perpendicular to the fill_angle.
         */
        void addLineInfill(Polygons& result, const PointMatrix& rotation_matrix, const int scanline_min_idx, const int line_distance, const AABB boundary, std::vector<std::vector<coord_t>>& cut_list, coord_t total_shift);
        /*!
         * Function for creating linear based infill types (Lines, ZigZag).
         *
         * This function implements the basic functionality of Infill::generateLineInfill (see doc of that function),
         * but makes calls to a ZigzagConnectorProcessor which handles what to do with each line segment - scanline intersection.
         *
         * It is called only from Infill::generateLineinfill and Infill::generateZigZagInfill.
         *
         * \param outline_offset An offset from the reference polygon (Infill::in_outline) to get the actual outline within which to generate infill
         * \param[out] result (output) The resulting lines
         * \param line_distance The distance between two lines which are in the same direction
         * \param rotation_matrix The rotation matrix (un)applied to enforce the angle of the infill
         * \param zigzag_connector_processor The processor used to generate zigzag connectors
         * \param connected_zigzags Whether to connect the endpiece zigzag segments on both sides to the same infill line
         * \param extra_shift extra shift of the scanlines in the direction perpendicular to the fill_angle
         */
        void generateLinearBasedInfill(const int outline_offset, Polygons& result, const int line_distance, const PointMatrix& rotation_matrix, ZigzagConnectorProcessor& zigzag_connector_processor, const bool connected_zigzags, coord_t extra_shift);

        /*!
         *
         * generate lines within the area of [in_outline], at regular intervals of [line_distance]
         * idea:
         * intersect a regular grid of 'scanlines' with the area inside [in_outline] (see generateLineInfill)
         * zigzag:
         * include pieces of boundary, connecting the lines, forming an accordion like zigzag instead of separate lines    |_|^|_|
         *
         * Note that ZigZag consists of 3 types:
         * - without endpieces
         * - with disconnected endpieces
         * - with connected endpieces
         *
         *     <--
         *     ___
         *    |   |   |
         *    |   |   |
         *    |   |___|
         *         -->
         *
         *        ^ = even scanline
         *  ^            ^ no endpieces
         *
         * start boundary from even scanline! :D
         *
         *
         *                 v  disconnected end piece: leave out last line segment
         *          _____
         *   |     |     |  \                     .
         *   |     |     |  |
         *   |_____|     |__/
         *
         *   ^     ^     ^    scanlines
         *
         *
         *                 v  connected end piece
         *          ________
         *   |     |     |  \                      .
         *   |     |     |  |
         *   |_____|     |__/                       .
         *
         *   ^     ^     ^    scanlines
         *
         * \param[out] result (output) The resulting lines
         * \param line_distance The distance between two lines which are in the same direction
         * \param infill_rotation The angle of the generated lines
         */
        void generateZigZagInfill(Polygons& result, const coord_t line_distance, const double& infill_rotation);

        /*!
         * determine how far the infill pattern should be shifted based on the values of infill_origin and \p infill_rotation
         *
         * \param[in] infill_rotation the angle the infill pattern is rotated through
         *
         * \return the distance the infill pattern should be shifted
         */
        coord_t getShiftOffsetFromInfillOriginAndRotation(const double& infill_rotation);

        /*!
         * Connects infill lines together so that they form polylines.
         *
         * In most cases it will end up with only one long line that is more or less
         * optimal. The lines are connected on their ends by extruding along the
         * border of the infill area, similar to the zigzag pattern.
         * \param[in/out] result_lines The lines to connect together.
         */
        void connectLines(Polygons& result_lines);
    };

}//namespace cura
#endif

