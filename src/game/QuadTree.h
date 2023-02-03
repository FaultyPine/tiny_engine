#pragma once

#include "tiny_engine/pch.h"
#include "tiny_engine/tiny_types.h"
#include <functional>

template<typename T>
struct Node {
    glm::vec2 point = glm::vec2(0);
    T data;
    bool hasData = false;
    Node(){}
    Node(glm::vec2 pos, T _data) {
        point = pos;
        data = _data;
        hasData = true;
    }
};

template<typename T>
struct QuadTree {
    QuadTree(){}
    QuadTree(glm::vec2 min, glm::vec2 max) : QuadTree() {
        this->bounds = BoundingBox2D(min, max);
    }
    QuadTree(BoundingBox2D bounds) : QuadTree() {
        this->bounds = bounds;
    }
    QuadTree(BoundingBox2D bounds, Node<T> n) : QuadTree(bounds) {
        node = n;
    }
    void Clear() {
        neighbors.clear();
        bounds = {};
        node = {};
    }
    void Draw();
    void insert(const Node<T>& node);
    Node<T>* search(glm::vec2 point);
    bool inBoundary(glm::vec2 point);
    void Transform(std::function<void(QuadTree<T>)> func);
    u32 GetSize() {
        u32 s = node.hasData ? 1 : 0;
        for (QuadTree<T>& child : neighbors) {
            if (!child.neighbors.empty()) {
                s += child.GetSize();
            }
        }
        return s;
    }

    BoundingBox2D bounds = {};
    Node<T> node = {};
    std::vector<QuadTree> neighbors;


};


template <typename T>
void QuadTree<T>::Transform(std::function<void(QuadTree<T>)> func) {
    func(*this);
    for (auto& child : neighbors) {
        child.Transform(func);
    }
}

template <typename T>
void QuadTree<T>::insert(const Node<T>& nodeToInsert) {
    if (neighbors.empty()) {
        neighbors.resize(4);
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

    glm::vec2 topLeft = bounds.min;
    glm::vec2 botRight = bounds.max;

    constexpr f32 minimumQuadTreeCellSize = 1.0f;
    bool isSizeOfBoundsLessThanOne = abs(topLeft.x - botRight.x) <= minimumQuadTreeCellSize
                                  && abs(topLeft.y - botRight.y) <= minimumQuadTreeCellSize;
    // cannot subdivide further
    if (isSizeOfBoundsLessThanOne) {
        if (!this->node.hasData) {
            //std::cout << "INSERT\n";
            this->node = nodeToInsert;
        }
        else {
            std::cout << "FAILED TO INSERT???\n";
        }
        return;
    }

    // topLeft = neighbors[0]
    // botLeft = neighbors[1]
    // topRight = neighbors[2]
    // botRight = neighbors[4]

    // indicates LEFT
    if ((topLeft.x + botRight.x) / 2 >= nodeToInsert.point.x) {

		// Indicates top left
		if ((topLeft.y + botRight.y) / 2 >= nodeToInsert.point.y) {
			if (!neighbors[0].node.hasData)
				neighbors[0] = QuadTree<T>(
					glm::vec2(topLeft.x, topLeft.y),
					glm::vec2((topLeft.x + botRight.x) / 2,
						(topLeft.y + botRight.y) / 2));
			neighbors[0].insert(nodeToInsert);
		}

		// Indicates bottom left
		else {
			if (!neighbors[1].node.hasData)
				neighbors[1] = QuadTree<T>(
					glm::vec2(topLeft.x,
						(topLeft.y + botRight.y) / 2),
					glm::vec2((topLeft.x + botRight.x) / 2,
						botRight.y));
			neighbors[1].insert(nodeToInsert);
		}

	}
    // indicates RIGHT
	else {

		// Indicates topRightTree
		if ((topLeft.y + botRight.y) / 2 >= nodeToInsert.point.y) {
			if (!neighbors[2].node.hasData)
				neighbors[2] = QuadTree<T>(
					glm::vec2((topLeft.x + botRight.x) / 2,
						topLeft.y),
					glm::vec2(botRight.x,
						(topLeft.y + botRight.y) / 2));
			neighbors[2].insert(nodeToInsert);
		}

		// Indicates botRightTree
		else {
			if (!neighbors[3].node.hasData)
				neighbors[3] = QuadTree<T>(
					glm::vec2((topLeft.x + botRight.x) / 2,
						(topLeft.y + botRight.y) / 2),
					glm::vec2(botRight.x, botRight.y));
			neighbors[3].insert(nodeToInsert);
		}

	}
}
template <typename T>
Node<T>* QuadTree<T>::search(glm::vec2 p) {
    // Current quad cannot contain it
    if (!inBoundary(p))
        return NULL;
 
    // We are at a quad of unit length
    // We cannot subdivide this quad further
    if (node.hasData)
        return &node;

    glm::vec2 topLeft = bounds.min;
    glm::vec2 botRight = bounds.max;
    QuadTree<T>* topLeftTree = &neighbors[0];
    QuadTree<T>* botLeftTree = &neighbors[1];
    QuadTree<T>* topRightTree = &neighbors[2];
    QuadTree<T>* botRightTree = &neighbors[3];
 
    if ((topLeft.x + botRight.x) / 2 >= p.x) {
        // Indicates topLeftTree
        if ((topLeft.y + botRight.y) / 2 >= p.y) {
            if (!topLeftTree->node.hasData)
                return NULL;
            return topLeftTree->search(p);
        }
 
        // Indicates botLeftTree
        else {
            if (!botLeftTree->node.hasData)
                return NULL;
            return botLeftTree->search(p);
        }
    }
    else {
        // Indicates topRightTree
        if ((topLeft.y + botRight.y) / 2 >= p.y) {
            if (!topRightTree->node.hasData)
                return NULL;
            return topRightTree->search(p);
        }
 
        // Indicates botRightTree
        else {
            if (!botRightTree->node.hasData)
                return NULL;
            return botRightTree->search(p);
        }
    }
}
template<typename T>
bool QuadTree<T>::inBoundary(glm::vec2 p) {
    return (p.x >= bounds.min.x && p.x <= bounds.max.x
            && p.y >= bounds.min.y && p.y <= bounds.max.y);
}

void DrawWireframeSquare(BoundingBox2D bounds, glm::vec4 color, f32 width) {
    Shapes2D::DrawLine(bounds.min, glm::vec2(bounds.max.x, bounds.min.y), color, width);
    Shapes2D::DrawLine(bounds.min, glm::vec2(bounds.min.x, bounds.max.y), color, width);

    Shapes2D::DrawLine(bounds.max, glm::vec2(bounds.max.x, bounds.min.y), color, width);
    Shapes2D::DrawLine(bounds.max, glm::vec2(bounds.min.x, bounds.max.y), color, width);
}

template<typename T>
void QuadTree<T>::Draw() {
    if (bounds.min == glm::vec2(0) && bounds.max == glm::vec2(0)) return;
    //std::cout << glm::to_string(bounds.min) << " -> " << glm::to_string(bounds.max) << "\n"; 
    if (node.hasData) {
        //std::cout << "Node pos: " << glm::to_string(node.point) << "\n";
        //std::cout << " real ent pos: " << glm::to_string(node.data->tf.position) << "\n";
    }

    DrawWireframeSquare(bounds, glm::vec4(1), 1.0f);
    for (auto& child : neighbors) {
        child.Draw();
    }
}