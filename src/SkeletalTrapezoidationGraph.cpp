#include "SkeletalTrapezoidationGraph.h"
#include <unordered_map>

#include "utils/linearAlg2D.h"

namespace arachne
{

void SkeletalTrapezoidationGraph::fixNodeDuplication()
{
    for (auto node_it = nodes.begin(); node_it != nodes.end();)
    {
        node_t* replacing_node = nullptr;
        for (edge_t* outgoing = node_it->some_edge; outgoing != node_it->some_edge; outgoing = outgoing->twin->next)
        {
            assert(outgoing);
            if (outgoing->from != &*node_it)
            {
                replacing_node = outgoing->from;
            }
            if (outgoing->twin->to != &*node_it)
            {
                replacing_node = outgoing->twin->to;
            }
        }
        if (replacing_node)
        {
            for (edge_t* outgoing = node_it->some_edge; outgoing != node_it->some_edge; outgoing = outgoing->twin->next)
            {
                outgoing->twin->to = replacing_node;
                outgoing->from = replacing_node;
            }
            node_it = nodes.erase(node_it);
        }
        else
        {
            ++node_it;
        }
    }
}
    
void SkeletalTrapezoidationGraph::collapseSmallEdges(coord_t snap_dist)
{
    std::unordered_map<edge_t*, std::list<edge_t>::iterator> edge_locator;
    std::unordered_map<node_t*, std::list<node_t>::iterator> node_locator;
    
    for (auto edge_it = edges.begin(); edge_it != edges.end(); ++edge_it)
    {
        edge_locator.emplace(&*edge_it, edge_it);
    }
    
    for (auto node_it = nodes.begin(); node_it != nodes.end(); ++node_it)
    {
        node_locator.emplace(&*node_it, node_it);
    }
    
    auto safelyRemoveEdge = [this, &edge_locator](edge_t* to_be_removed, std::list<edge_t>::iterator& current_edge_it, bool& edge_it_is_updated)
    {
        if (current_edge_it != edges.end()
            && to_be_removed == &*current_edge_it)
        {
            current_edge_it = edges.erase(current_edge_it);
            edge_it_is_updated = true;
        }
        else
        {
            edges.erase(edge_locator[to_be_removed]);
        }
    };

    auto should_collapse = [snap_dist](node_t* a, node_t* b) 
    { 
        return shorterThen(a->p - b->p, snap_dist); 
    };
        
    for (auto edge_it = edges.begin(); edge_it != edges.end();)
    {
        if (edge_it->prev)
        {
            edge_it++;
            continue;
        }
        
        edge_t* quad_start = &*edge_it;
        edge_t* quad_end = quad_start; while (quad_end->next) quad_end = quad_end->next;
        edge_t* quad_mid = (quad_start->next == quad_end)? nullptr : quad_start->next;

        bool edge_it_is_updated = false;
        if (quad_mid && should_collapse(quad_mid->from, quad_mid->to))
        {
            assert(quad_mid->twin);
            int count = 0;
            for (edge_t* edge_from_3 = quad_end; edge_from_3 && edge_from_3 != quad_mid->twin; edge_from_3 = edge_from_3->twin->next)
            {
                edge_from_3->from = quad_mid->from;
                edge_from_3->twin->to = quad_mid->from;
                if (count > 50)
                {
                    std::cerr << edge_from_3->from->p << " - " << edge_from_3->to->p << '\n';
                }
                if (++count > 1000) 
                {
                    break;
                }
            }

            // o-o > collapse top
            // | |
            // | |
            // | |
            // o o
            if (quad_mid->from->some_edge == quad_mid)
            {
                if (quad_mid->twin->next)
                {
                    quad_mid->from->some_edge = quad_mid->twin->next;
                }
                else
                {
                    quad_mid->from->some_edge = quad_mid->prev->twin;
                }
            }
            
            nodes.erase(node_locator[quad_mid->to]);

            quad_mid->prev->next = quad_mid->next;
            quad_mid->next->prev = quad_mid->prev;
            quad_mid->twin->next->prev = quad_mid->twin->prev;
            quad_mid->twin->prev->next = quad_mid->twin->next;

            safelyRemoveEdge(quad_mid->twin, edge_it, edge_it_is_updated);
            safelyRemoveEdge(quad_mid, edge_it, edge_it_is_updated);
        }

        //  o-o
        //  | | > collapse sides
        //  o o
        if ( should_collapse(quad_start->from, quad_end->to) && should_collapse(quad_start->to, quad_end->from))
        { // Collapse start and end edges and remove whole cell

            quad_start->twin->to = quad_end->to;
            quad_end->to->some_edge = quad_end->twin;
            if (quad_end->from->some_edge == quad_end)
            {
                if (quad_end->twin->next)
                {
                    quad_end->from->some_edge = quad_end->twin->next;
                }
                else
                {
                    quad_end->from->some_edge = quad_end->prev->twin;
                }
            }
            nodes.erase(node_locator[quad_start->from]);

            quad_start->twin->twin = quad_end->twin;
            quad_end->twin->twin = quad_start->twin;
            safelyRemoveEdge(quad_start, edge_it, edge_it_is_updated);
            safelyRemoveEdge(quad_end, edge_it, edge_it_is_updated);
        }
        // If only one side had collapsable length then the cell on the other side of that edge has to collapse
        // if we would collapse that one edge then that would change the quad_start and/or quad_end of neighboring cells
        // this is to do with the constraint that !prev == !twin.next

        if (!edge_it_is_updated)
        {
            edge_it++;
        }
    }
}

void SkeletalTrapezoidationGraph::makeRib(edge_t*& prev_edge, Point start_source_point, Point end_source_point, bool is_next_to_start_or_end)
{
    Point p = LinearAlg2D::getClosestOnLine(prev_edge->to->p, start_source_point, end_source_point);
    coord_t dist = vSize(prev_edge->to->p - p);
    prev_edge->to->data.distance_to_boundary = dist;
    assert(dist >= 0);

    nodes.emplace_front(SkeletalTrapezoidationJoint(), p);
    node_t* node = &nodes.front();
    node->data.distance_to_boundary = 0;
    
    edges.emplace_front(SkeletalTrapezoidationEdge(SkeletalTrapezoidationEdge::EdgeType::EXTRA_VD));
    edge_t* forth_edge = &edges.front();
    edges.emplace_front(SkeletalTrapezoidationEdge(SkeletalTrapezoidationEdge::EdgeType::EXTRA_VD));
    edge_t* back_edge = &edges.front();
    
    prev_edge->next = forth_edge;
    forth_edge->prev = prev_edge;
    forth_edge->from = prev_edge->to;
    forth_edge->to = node;
    forth_edge->twin = back_edge;
    back_edge->twin = forth_edge;
    back_edge->from = node;
    back_edge->to = prev_edge->to;
    node->some_edge = back_edge;
    
    prev_edge = back_edge;
}

std::pair<SkeletalTrapezoidationGraph::edge_t*, SkeletalTrapezoidationGraph::edge_t*> SkeletalTrapezoidationGraph::insertRib(edge_t& edge, node_t* mid_node)
{
    edge_t* edge_before = edge.prev;
    edge_t* edge_after = edge.next;
    node_t* node_before = edge.from;
    node_t* node_after = edge.to;
    
    Point p = mid_node->p;

    std::pair<Point, Point> source_segment = getSource(edge);
    Point px = LinearAlg2D::getClosestOnLineSegment(p, source_segment.first, source_segment.second);
    coord_t dist = vSize(p - px);
    assert(dist > 0);
    mid_node->data.distance_to_boundary = dist;
    mid_node->data.transition_ratio = 0; // Both transition end should have rest = 0, because at the ends a whole number of beads fits without rest

    nodes.emplace_back(SkeletalTrapezoidationJoint(), px);
    node_t* source_node = &nodes.back();
    source_node->data.distance_to_boundary = 0;

    edge_t* first = &edge;
    edges.emplace_back(SkeletalTrapezoidationEdge());
    edge_t* second = &edges.back();
    edges.emplace_back(SkeletalTrapezoidationEdge(SkeletalTrapezoidationEdge::EdgeType::TRANSITION_END));
    edge_t* outward_edge = &edges.back();
    edges.emplace_back(SkeletalTrapezoidationEdge(SkeletalTrapezoidationEdge::EdgeType::TRANSITION_END));
    edge_t* inward_edge = &edges.back();

    if (edge_before)
    {
        edge_before->next = first;
    }
    first->next = outward_edge;
    outward_edge->next = nullptr;
    inward_edge->next = second;
    second->next = edge_after;

    if (edge_after) 
    {
        edge_after->prev = second;
    }
    second->prev = inward_edge;
    inward_edge->prev = nullptr;
    outward_edge->prev = first;
    first->prev = edge_before;

    first->to = mid_node;
    outward_edge->to = source_node;
    inward_edge->to = mid_node;
    second->to = node_after;

    first->from = node_before;
    outward_edge->from = mid_node;
    inward_edge->from = source_node;
    second->from = mid_node;

    node_before->some_edge = first;
    mid_node->some_edge = outward_edge;
    source_node->some_edge = inward_edge;
    if (edge_after) 
    {
        node_after->some_edge = edge_after;
    }

    first->data.setMarked(true);
    outward_edge->data.setMarked(false); // TODO verify this is always the case.
    inward_edge->data.setMarked(false);
    second->data.setMarked(true);

    outward_edge->twin = inward_edge;
    inward_edge->twin = outward_edge;

    first->twin = nullptr; // we don't know these yet!
    second->twin = nullptr;

    assert(second->prev->from->data.distance_to_boundary == 0);

    return std::make_pair(first, second);
}

SkeletalTrapezoidationGraph::edge_t* SkeletalTrapezoidationGraph::insertNode(edge_t* edge, Point mid, coord_t mide_node_bead_count)
{
    edge_t* last_edge_replacing_input = edge;

    nodes.emplace_back(SkeletalTrapezoidationJoint(), mid);
    node_t* mid_node = &nodes.back();

    edge_t* twin = last_edge_replacing_input->twin;
    last_edge_replacing_input->twin = nullptr;
    twin->twin = nullptr;
    std::pair<edge_t*, edge_t*> left_pair = insertRib(*last_edge_replacing_input, mid_node);
    std::pair<edge_t*, edge_t*> right_pair = insertRib(*twin, mid_node);
    edge_t* first_edge_replacing_input = left_pair.first;
    last_edge_replacing_input = left_pair.second;
    edge_t* first_edge_replacing_twin = right_pair.first;
    edge_t* last_edge_replacing_twin = right_pair.second;

    first_edge_replacing_input->twin = last_edge_replacing_twin;
    last_edge_replacing_twin->twin = first_edge_replacing_input;
    last_edge_replacing_input->twin = first_edge_replacing_twin;
    first_edge_replacing_twin->twin = last_edge_replacing_input;

    mid_node->data.bead_count = mide_node_bead_count;

    return last_edge_replacing_input;
}

std::pair<Point, Point> SkeletalTrapezoidationGraph::getSource(const edge_t& edge)
{
    const edge_t* from_edge;
    for (from_edge = &edge; from_edge->prev; from_edge = from_edge->prev) 
    {
    }
    
    const edge_t* to_edge;
    for (to_edge = &edge; to_edge->next; to_edge = to_edge->next) 
    {
    }
    
    return std::make_pair(from_edge->from->p, to_edge->to->p);
}

}
