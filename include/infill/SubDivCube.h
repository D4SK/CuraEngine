// Copyright (c) 2023 UltiMaker
// CuraEngine is released under the terms of the AGPLv3 or higher

#ifndef INFILL_SUBDIVCUBE_H
#define INFILL_SUBDIVCUBE_H

#include "geometry/OpenLinesSet.h"
#include "geometry/Point2LL.h"
#include "geometry/Point3LL.h"
#include "geometry/Point3Matrix.h"
#include "geometry/PointMatrix.h"
#include "settings/types/LayerIndex.h"
#include "settings/types/Ratio.h"

namespace cura
{

class Polygon;
class SliceMeshStorage;

class SubDivCube
{
public:
    /*!
     * Constructor for SubDivCube. Recursively calls itself eight times to flesh out the octree.
     * \param mesh contains infill layer data and settings
     * \param my_center the center of the cube
     * \param depth the recursion depth of the cube (0 is most recursed)
     */
    SubDivCube(SliceMeshStorage& mesh, Point3LL& center, size_t depth);

    /*!
     * Precompute the octree of subdivided cubes
     * \param mesh contains infill layer data and settings
     */
    static void precomputeOctree(SliceMeshStorage& mesh, const Point2LL& infill_origin);

    /*!
     * Generates the lines of subdivision of the specific cube at the specific layer. It recursively calls itself, so it ends up drawing all the subdivision lines of sub-cubes too.
     * \param z the specified layer height
     * \param result (output) The resulting lines
     */
    void generateSubdivisionLines(const coord_t z, OpenLinesSet& result);

private:
    /*!
     * Generates the lines of subdivision of the specific cube at the specific layer. It recursively calls itself, so it ends up drawing all the subdivision lines of sub-cubes too.
     * \param z the specified layer height
     * \param result (output) The resulting lines
     * \param directional_line_groups Array of 3 times a polylines. Used to keep track of line segments that are all pointing the same direction for line segment combining
     */
    void generateSubdivisionLines(const coord_t z, OpenLinesSet (&directional_line_groups)[3]);

    struct CubeProperties
    {
        coord_t side_length; //!< side length of cubes
        coord_t height; //!< height of cubes based. This is the distance from one point of a cube to its 3d opposite.
        coord_t square_height; //!< square cut across lengths. This is the diagonal distance across a face of the cube.
        coord_t max_draw_z_diff; //!< maximum draw z differences. This is the maximum difference in z at which lines need to be drawn.
        coord_t max_line_offset; //!< maximum line offsets. This is the maximum distance at which subdivision lines should be drawn from the 2d cube center.
    };

    /*!
     * Rotates a point 120 degrees about the origin.
     * \param target the point to rotate.
     */
    static void rotatePoint120(Point2LL& target);

    /*!
     * Rotates a point to align it with the orientation of the infill.
     * \param target the point to rotate.
     */
    static void rotatePointInitial(Point2LL& target);

    /*!
     * Determines if a described theoretical cube should be subdivided based on if a sphere that encloses the cube touches the infill mesh.
     * \param mesh contains infill layer data and settings
     * \param center the center of the described cube
     * \param radius the radius of the enclosing sphere
     * \return the described cube should be subdivided
     */
    static bool isValidSubdivision(SliceMeshStorage& mesh, Point3LL& center, coord_t radius);

    /*!
     * Finds the distance to the infill border at the specified layer from the specified point.
     * \param mesh contains infill layer data and settings
     * \param layer_nr the number of the specified layer
     * \param location the location of the specified point
     * \param[out] distance2 the squared distance to the infill border
     * \return Code 0: outside, 1: inside, 2: boundary does not exist at specified layer
     */
    static coord_t distanceFromPointToMesh(SliceMeshStorage& mesh, const LayerIndex layer_nr, Point2LL& location, coord_t* distance2);

    /*!
     * Adds the defined line to the specified polygons. It assumes that the specified polygons are all parallel lines. Combines line segments with touching ends closer than
     * epsilon. \param[out] group the polygons to add the line to \param from the first endpoint of the line \param to the second endpoint of the line
     */
    void addLineAndCombine(OpenLinesSet& group, Point2LL from, Point2LL to);

    size_t depth_; //!< the recursion depth of the cube (0 is most recursed)
    Point3LL center_; //!< center location of the cube in absolute coordinates
    std::array<std::shared_ptr<SubDivCube>, 8> children_; //!< pointers to this cube's eight octree children
    static std::vector<CubeProperties> cube_properties_per_recursion_step_; //!< precomputed array of basic properties of cubes based on recursion depth.
    static Ratio radius_multiplier_; //!< multiplier for the bounding radius when determining if a cube should be subdivided
    static Point3Matrix rotation_matrix_; //!< The rotation matrix to get from axis aligned cubes to cubes standing on a corner point aligned with the infill_angle
    static PointMatrix infill_rotation_matrix_; //!< Horizontal rotation applied to infill
    static coord_t radius_addition_; //!< addition to the bounding radius when determining if a cube should be subdivided
};

} // namespace cura
#endif // INFILL_SUBDIVCUBE_H
