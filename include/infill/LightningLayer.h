// Copyright (c) 2024 UltiMaker
// CuraEngine is released under the terms of the AGPLv3 or higher

#ifndef LIGHTNING_LAYER_H
#define LIGHTNING_LAYER_H

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "geometry/LinesSet.h"
#include "geometry/OpenLinesSet.h"
#include "infill/LightningTreeNode.h"
#include "utils/SquareGrid.h"
#include "utils/polygonUtils.h"

namespace cura
{
using LightningTreeNodeSPtr = std::shared_ptr<LightningTreeNode>;
using SparseLightningTreeNodeGrid = SparsePointGridInclusive<std::weak_ptr<LightningTreeNode>>;

struct GroundingLocation
{
    LightningTreeNodeSPtr tree_node; //!< not null if the gounding location is on a tree
    std::optional<ClosestPointPolygon> boundary_location; //!< in case the gounding location is on the boundary
    Point2LL p() const;
};

/*!
 * A layer of the lightning fill.
 *
 * Contains the trees to be printed and propagated to the next layer below.
 */
class LightningLayer
{
public:
    std::vector<LightningTreeNodeSPtr> tree_roots;

    void generateNewTrees(
        const Shape& current_overhang,
        const Shape& current_outlines,
        const LocToLineGrid& outline_locator,
        const coord_t supporting_radius,
        const coord_t wall_supporting_radius);

    /*! Determine & connect to connection point in tree/outline.
     * \param min_dist_from_boundary_for_tree If the unsupported point is closer to the boundary than this then don't consider connecting it to a tree
     */
    GroundingLocation getBestGroundingLocation(
        const Point2LL& unsupported_location,
        const Shape& current_outlines,
        const LocToLineGrid& outline_locator,
        const coord_t supporting_radius,
        const coord_t wall_supporting_radius,
        const SparseLightningTreeNodeGrid& tree_node_locator,
        const LightningTreeNodeSPtr& exclude_tree = nullptr);

    /*!
     * \param[out] new_child The new child node introduced
     * \param[out] new_root The new root node if one had been made
     * \return Whether a new root was added
     */
    bool attach(const Point2LL& unsupported_location, const GroundingLocation& ground, LightningTreeNodeSPtr& new_child, LightningTreeNodeSPtr& new_root);

    void reconnectRoots(
        std::vector<LightningTreeNodeSPtr>& to_be_reconnected_tree_roots,
        const Shape& current_outlines,
        const LocToLineGrid& outline_locator,
        const coord_t supporting_radius,
        const coord_t wall_supporting_radius);

    OpenLinesSet convertToLines(const Shape& limit_to_outline, const coord_t line_width) const;

    coord_t getWeightedDistance(const Point2LL& boundary_loc, const Point2LL& unsupported_location);

    void fillLocator(SparseLightningTreeNodeGrid& tree_node_locator);
};
} // namespace cura

#endif // LIGHTNING_LAYER_H
