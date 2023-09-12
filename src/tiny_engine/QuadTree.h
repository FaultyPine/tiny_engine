#pragma once

#include "pch.h"
#include "tiny_engine/tiny_types.h"
#include <functional>

template<typename T>
struct QuadTreeNode {
    glm::vec2 point = glm::vec2(0);
    T data;
    bool hasData = false;
    QuadTreeNode() = default;
    QuadTreeNode(glm::vec2 pos, T _data) {
        point = pos;
        data = _data;
        hasData = true;
    }
};

enum QuadTreeChildDirection {
    NORTHWEST,
    SOUTHWEST,
    NORTHEAST,
    SOUTHEAST,
};

template<typename T>
struct QuadTree {

    QuadTree() = default;
    QuadTree(glm::vec2 min, glm::vec2 max) : QuadTree() {
        this->bounds = BoundingBox2D(min, max);
    }
    QuadTree(BoundingBox2D bounds) : QuadTree() {
        this->bounds = bounds;
    }
    ~QuadTree() {
        Clear();
    }
    void Clear() {
        children.clear();
        node = {};
    }
    void Draw();
    void insert(const QuadTreeNode<T>& node);
    void search(BoundingBox2D bounds, std::vector<T>& results);
    void Transform(std::function<void(QuadTreeNode<T>*)> func);
    u32 GetSize();

    BoundingBox2D bounds = {};
    QuadTreeNode<T> node = {};
    // TODO: (?) access array of children with array-based tree lookups
    std::vector<QuadTree> children = {};

private:
    bool inBoundary(glm::vec2 point);
    static QuadTree<T> GenerateChildQuadTree(BoundingBox2D bounds, QuadTreeChildDirection direction);
};

template <typename T>
u32 QuadTree<T>::GetSize()  {
    u32 s = node.hasData ? 1 : 0;
    for (QuadTree<T>& child : children) {
        if (!child.children.empty()) {
            s += child.GetSize();
        }
    }
    return s;
}

template <typename T>
void QuadTree<T>::Transform(std::function<void(QuadTreeNode<T>*)> func) {
    func(this);
    for (auto& child : neighbors) {
        child.Transform(func);
    }
}

template <typename T>
QuadTree<T> QuadTree<T>::GenerateChildQuadTree(BoundingBox2D bounds, QuadTreeChildDirection direction) {
    glm::vec2 topLeft = bounds.min;
    glm::vec2 botRight = bounds.max;
    switch (direction) {
        case NORTHWEST:
        {
            return QuadTree<T>(
                        glm::vec2(topLeft.x, topLeft.y),
                        glm::vec2((topLeft.x + botRight.x) / 2,
                            (topLeft.y + botRight.y) / 2));
        } break;
        case SOUTHWEST:
        {
            return QuadTree<T>(
                        glm::vec2(topLeft.x,
                            (topLeft.y + botRight.y) / 2),
                        glm::vec2((topLeft.x + botRight.x) / 2,
                            botRight.y));
        } break;
        case NORTHEAST:
        {
            return QuadTree<T>(
                        glm::vec2((topLeft.x + botRight.x) / 2,
                            topLeft.y),
                        glm::vec2(botRight.x,
                            (topLeft.y + botRight.y) / 2));
        } break;
        case SOUTHEAST:
        {
            return QuadTree<T>(
                        glm::vec2((topLeft.x + botRight.x) / 2,
                            (topLeft.y + botRight.y) / 2),
                        glm::vec2(botRight.x, botRight.y));
        } break;
        default:
        {
            ASSERT(false && "Invalid QuadTreeChildDirection passed to GenerateChildQuadTree");
            return QuadTree<T>();
        } break;
    }
}

template <typename T>
void QuadTree<T>::insert(const QuadTreeNode<T>& nodeToInsert) {
    glm::vec2 topLeft = bounds.min;
    glm::vec2 botRight = bounds.max;
    if (children.empty()) {
        // populate all 4 directions
        for (u32 i = 0; i < 4; i++) {
            children.emplace_back(GenerateChildQuadTree(this->bounds, (QuadTreeChildDirection)i));
        }
    }
    // don't insert empty nodes
    if (!nodeToInsert.hasData) {
        std::cout << "Tried to insert node without data to quadtree\n";
        return;
    }
    // node not in bounds of this tree, can't insert
    if (!inBoundary(nodeToInsert.point)) {
        std::cout << "Tried to insert node out of bounds to quadtree\n";
        return;
    }


    constexpr f32 minimumQuadTreeCellSize = 1.0f;
    bool isSizeOfBoundsLessThanOne = abs(topLeft.x - botRight.x) <= minimumQuadTreeCellSize
                                  && abs(topLeft.y - botRight.y) <= minimumQuadTreeCellSize;
    // cannot subdivide further
    if (isSizeOfBoundsLessThanOne) {
        if (!this->node.hasData) {
            this->node = nodeToInsert;
        }
        else {
            // if this is being hit, there are too many entities too close together
            // TODO: it might be a good idea to change the QuadTree
            // implementation to store more than 1 piece of data per level
            // and allow users to specify the capacity of each level of the tree
            std::cout << "QuadTree max depth reached. Too many entities too close together!\n";
        }
        return;
    }

    // indicates LEFT
    if ((topLeft.x + botRight.x) / 2 >= nodeToInsert.point.x) {
		// Indicates top left
		if ((topLeft.y + botRight.y) / 2 >= nodeToInsert.point.y) {
            if (!node.hasData) {
                node = nodeToInsert;
            }
            else {
                children[NORTHWEST].insert(nodeToInsert);
            }
		}
		// Indicates bottom left
		else {
            if (!node.hasData) {
                node = nodeToInsert;
            }
            else {
                children[SOUTHWEST].insert(nodeToInsert);
            }
		}
	}
    // indicates RIGHT
	else {
		// Indicates topRightTree
		if ((topLeft.y + botRight.y) / 2 >= nodeToInsert.point.y) {
            if (!node.hasData) {
                node = nodeToInsert;
            }
            else {
                children[NORTHEAST].insert(nodeToInsert);
            }
		}
		// Indicates botRightTree
		else {
            if (!node.hasData) {
                node = nodeToInsert;
            }
            else {
                children[SOUTHEAST].insert(nodeToInsert);
            }
		}
	}
}

template <typename T>
void QuadTree<T>::search(BoundingBox2D boundsToSearch, std::vector<T>& results) {
    if (!node.hasData) {
        // if this node doesn't have data, we are a leaf
        // and have therefore exhausted the search
        return;
    }

    // if the range we are searching for is not within our
    // bounds, then we don't care about this quadtree
    if (!bounds.isIntersecting(boundsToSearch)) {
        return;
    }

    // if we get here, this *quadtree* is in range of the search bounds
    // however, that doesn't necessarily mean the node is

    // now check if the actual node's position is within the search
    // bounds. If so, we care about it
    if (boundsToSearch.isInBounds(node.point)) {
        results.push_back(node.data);
    }

    for (auto& child : children) {
        child.search(boundsToSearch, results);
    }
}
template<typename T>
bool QuadTree<T>::inBoundary(glm::vec2 p) {
    return bounds.isInBounds(p);
}

template<typename T>
void QuadTree<T>::Draw() {
    if (bounds.min == glm::vec2(0) && bounds.max == glm::vec2(0)) return;
    //std::cout << glm::to_string(bounds.min) << " -> " << glm::to_string(bounds.max) << "\n"; 
    if (node.hasData) {
        Shapes2D::DrawCircle(node.point, 1.0f);
        //std::cout << "QuadTreeNode pos: " << glm::to_string(node.point) << "\n";
        //std::cout << " real ent pos: " << glm::to_string(node.data->tf.position) << "\n";
    }

    Shapes2D::DrawWireframeSquare(bounds.min, bounds.max, glm::vec4(1), 0.5f);
    for (QuadTree<T>& child : children) {
        child.Draw();
    }
}