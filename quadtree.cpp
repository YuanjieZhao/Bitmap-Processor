/**
 * @file quadtree.cpp
 * Quadtree class implementation.
 */

#include <cstddef>
#include <cstdlib>
#include <iostream>

using namespace std;

#include "quadtree.h"
#include "png.h"

const int MIN_TOLERANCE = 0;
const int MAX_TOLERANCE = 3 * (255 * 255);  // the "difference" between white and black color according to prune()

// Quadtree
//   - parameters: none
//   - constructor for the Quadtree class; makes an empty tree
Quadtree::Quadtree() : root(NULL), res(0) {}

// Quadtree
//   - parameters: PNG const & source - reference to a const PNG
//                    object, from which the Quadtree will be built
//                 int resolution - resolution of the portion of source
//                    from which this tree will be built
//   - constructor for the Quadtree class; creates a Quadtree representing
//        the resolution by resolution block in the upper-left corner of
//        source
Quadtree::Quadtree(PNG const& source, int resolution)
{
	root = NULL;
	buildTree(source, resolution);
}

// Quadtree
//   - parameters: Quadtree const & other - reference to a const Quadtree
//                    object, which the current Quadtree will be a copy of
//   - copy constructor for the Quadtree class
Quadtree::Quadtree(Quadtree const& other) 
{
	root = NULL;
	copyQuadtree(other);
}

// ~Quadtree
//   - parameters: none
//   - destructor for the Quadtree class
Quadtree::~Quadtree()
{
	deleteQuadtree(root);
}

// operator=
//   - parameters: Quadtree const & other - reference to a const Quadtree
//                    object, which the current Quadtree will be a copy of
//   - return value: a const reference to the current Quadtree
//   - assignment operator for the Quadtree class
Quadtree const& Quadtree::operator=(Quadtree const& other)
{
	copyQuadtree(other);
	return *this;
}

// helper function for deep delete
// Used by destructor and copy/assignment
// Deallocates Quadtree and its QuadtreeNode
void Quadtree::deleteQuadtree(){
	if (root != NULL) deleteQuadtree(root);
}

// helper function for deleteQuadtree() and pruneChildren()
void Quadtree::deleteQuadtree(QuadtreeNode*& node){
	if (node != NULL){
		deleteQuadtree(node->nwChild);
		deleteQuadtree(node->neChild);
		deleteQuadtree(node->swChild);
		deleteQuadtree(node->seChild);
		delete node;
		node = NULL;
	}
}

// helper function for deep copy
// Used by copy constructor and operator=
void Quadtree::copyQuadtree(Quadtree const& other){
	deleteQuadtree();
	res = other.res;
	copyQuadtree(root, other.root);
}

// helper function for copyQuadtree(Quadtree const& other)
void Quadtree::copyQuadtree(QuadtreeNode*& myNode, QuadtreeNode* const& otherNode){
	if (otherNode != NULL){
		myNode = new QuadtreeNode(otherNode->element);
		copyQuadtree(myNode->nwChild, otherNode->nwChild);
		copyQuadtree(myNode->neChild, otherNode->neChild);
		copyQuadtree(myNode->swChild, otherNode->swChild);
		copyQuadtree(myNode->seChild, otherNode->seChild);
	}
}


// buildTree (public interface)
//   - parameters: PNG const & source - reference to a const PNG
//                    object, from which the Quadtree will be built
//                 int resolution - resolution of the portion of source
//                    from which this tree will be built
//   - transforms the current Quadtree into a Quadtree representing
//        the resolution by resolution block in the upper-left corner of
//        source
void Quadtree::buildTree(PNG const& source, int resolution)
{
	deleteQuadtree();
	res = resolution;
	buildTree(source, resolution, 0, 0, root);
}


/** private helper function for buildTree(PNG const& source, int resolution)
  * @param
  * source - reference to a const PNG object
  * resolution - resolution of the portion of source from which this tree will be built
  * x - x-coordinate of top-left corner of the region represented by current node
  * y - y-coordinate of top-left corner of the region represented by current node
  * node - current node in Quadtree
  */
void Quadtree::buildTree(PNG const& source, int resolution, int x, int y, QuadtreeNode*& node){
	if (resolution == 1) {
		const RGBAPixel* pixel = source(x, y);
		node = new QuadtreeNode(*pixel);
	} else {
		node = new QuadtreeNode();
		int childResolution = resolution / 2;
		buildTree(source, childResolution, x, y, node->nwChild);
		buildTree(source, childResolution, x+childResolution, y, node->neChild);
		buildTree(source, childResolution, x, y+childResolution, node->swChild);
		buildTree(source, childResolution, x+childResolution, y+childResolution, node->seChild);
		getAvgPixelOfChildren(node);
	}
}

/* return the average of RGBAPixel of node's children
 * @param node - a non-leaf QuadtreeNode that has four children
 */
void Quadtree::getAvgPixelOfChildren(QuadtreeNode* node){
	RGBAPixel nwElem = node->nwChild->element;
	RGBAPixel neElem = node->neChild->element;
	RGBAPixel swElem = node->swChild->element;
	RGBAPixel seElem = node->seChild->element;

	node->element.red = getAvg(nwElem.red, neElem.red, swElem.red, seElem.red);
	node->element.green = getAvg(nwElem.green, neElem.green, swElem.green, seElem.green);
	node->element.blue = getAvg(nwElem.blue, neElem.blue, swElem.blue, seElem.blue);
	node->element.alpha = getAvg(nwElem.alpha, neElem.alpha, swElem.alpha, seElem.alpha);
}

// return the average of the given four byte number
uint8_t Quadtree::getAvg(uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4){
	return (n1 + n2 + n3 + n4) / 4;
}

// getPixel (public interface)
//   - parameters: int x, int y - coordinates of the pixel to be retrieved
//   - return value: an RGBAPixel representing the desired pixel of the
//        underlying bitmap
//   - retrieves and returns the pixel at coordinates (x, y) in the
//        underlying bitmap
RGBAPixel Quadtree::getPixel(int x, int y) const
{
	if (outOfBound(x, y) || root == NULL) { return RGBAPixel(); }
	return getPixel(x, y, root, res);
}

// helper function for getPixel(int x, int y)
RGBAPixel Quadtree::getPixel(int x, int y, QuadtreeNode* node, int resolution) const {
	if (!hasChildren(node)) { return node->element; }
	else {
		int r = resolution / 2; 	// r is the resolution of region represented by a node's child
		if (x < r && y < r){
			return getPixel(x, y, node->nwChild, r);
		} else if (x < r && y >= r){
			return getPixel(x, y, node->swChild, r);
		} else if (x >= r && y < r){
			return getPixel(x, y, node->neChild, r);
		} else {
			return getPixel(x, y, node->seChild, r);
		}
	}
}

// return true if given node has children (a node can have either zero or four children)
bool Quadtree::hasChildren(QuadtreeNode* node) const {
	return node->nwChild != NULL;
}

// return true if the value of x or y is outside the bounds of underlying bitmap
bool Quadtree::outOfBound(int x, int y) const {
	return x < 0 || y < 0 || x >= res || y >= res;
}

// decompress (public interface)
//   - parameters: none
//   - return value: a PNG object representing this quadtree's underlying
//        bitmap
//   - constructs and returns this quadtree's underlying bitmap
PNG Quadtree::decompress() const
{
	if (root == NULL) return PNG();
	PNG img(res, res);
	transform(img, res, 0, 0, root);
	return img;
}

/** helper function of decompress()
 * transform a PNG img into the PNG image represented by this QuadTree
 * @param
 * source - the original PNG image to be transformed
 * resolution - the resolution of the region represented by current Quadtree node
 * x - x-coordinate of top-left corner of the region represented by current node
 * y - y-coordinate of top-left corner of the region represented by current node
 * node - current node in Quadtree
 */
void Quadtree::transform (PNG& source, int resolution, int x, int y, QuadtreeNode* node) const {
	if (!hasChildren(node)){
		RGBAPixel newPixel = node->element;
		for (int i = 0; i < resolution; i++){
			for (int j = 0; j < resolution; j++){
				RGBAPixel* pixelPtr = source(x+i, y+j);
				*pixelPtr = newPixel;
			}
		}

	} else {
		int childResolution = resolution / 2;
		transform(source, childResolution, x, y, node->nwChild);
		transform(source, childResolution, x+childResolution, y, node->neChild);
		transform(source, childResolution, x, y+childResolution, node->swChild);
		transform(source, childResolution, x+childResolution, y+childResolution, node->seChild);
	}
}


// clockwiseRotate (public interface)
//   - parameters: none
//   - transforms this quadtree into a quadtree representing the same
//        bitmap, rotated 90 degrees clockwise
void Quadtree::clockwiseRotate() {
	if (root != NULL){
		clockwiseRotate(root);
	}
}

// helper function for clockwiseRotate()
void Quadtree::clockwiseRotate(QuadtreeNode*& node){
	if (hasChildren(node)){
		QuadtreeNode* tempNode = node->nwChild;
		node->nwChild = node->swChild;
		node->swChild = node->seChild;
		node->seChild = node->neChild;
		node->neChild = tempNode;
		clockwiseRotate(node->nwChild);
		clockwiseRotate(node->swChild);
		clockwiseRotate(node->seChild);
		clockwiseRotate(node->neChild);
	}
}

// prune (public interface)
//   - parameters: int tolerance - an integer representing the maximum
//                    "distance" which we will permit between a node's color
//                    (i.e. the average of its descendant leaves' colors)
//                    and the color of each of that node's descendant leaves
//   - for each node in the quadtree, if the "distance" between the average
//        of that node's descendant leaves' colors and the color of each of
//        that node's descendant leaves is at most tolerance, this function
//        deletes the subtrees beneath that node; we will let the node's
//        color "stand in for" the colors of all (deleted) leaves beneath it
void Quadtree::prune(int tolerance)
{
	if (root != NULL){
		prune(tolerance, root);
	}
}

// helper function of prune(int tolerance)
void Quadtree::prune(int tolerance, QuadtreeNode*& node){
	if (!hasChildren(node)){
		return;
	} else {
		if (isChildrenPrunable(tolerance, node, node)){
			pruneChildren(node);
		} else {
			prune(tolerance, node->nwChild);
			prune(tolerance, node->neChild);
			prune(tolerance, node->swChild);
			prune(tolerance, node->seChild);
		}
	}
}

/** return true if all children (direct and indirect) of node are prunable
  * Pre-condition: rootNode must have children
  * @param
  * tolerance - see prune(int tolerance)
  * rootNode - root of a subtree to be pruned
  * node - current node to be checked whether it is prunable
  */
bool Quadtree::isChildrenPrunable(int tolerance, QuadtreeNode*& rootNode, QuadtreeNode*& node) const {
	if(!hasChildren(node)){
		// node is a leaf
		return isPrunable(tolerance, rootNode->element, node->element);
	} else {
		bool p1 = isChildrenPrunable(tolerance, rootNode, node->nwChild);
		bool p2 = isChildrenPrunable(tolerance, rootNode, node->neChild);
		bool p3 = isChildrenPrunable(tolerance, rootNode, node->swChild);
		bool p4 = isChildrenPrunable(tolerance, rootNode, node->seChild);
		return p1 && p2 && p3 && p4;
	}	
}

// return true if the color difference between avgPixel and nodePixel is no more than tolerance
bool Quadtree::isPrunable(int tolerance, RGBAPixel avgPixel, RGBAPixel nodePixel) const {
	int diff = square((int) (avgPixel.red - nodePixel.red)) + 
			   square((int) (avgPixel.green - nodePixel.green)) + 
			   square((int) (avgPixel.blue - nodePixel.blue));
	return diff <= tolerance;
}

// return square of number n
int Quadtree::square (int n) const {
	return n * n;
}

// delete all descendants of the given node
void Quadtree::pruneChildren(QuadtreeNode*& node){
	deleteQuadtree(node->nwChild);
	deleteQuadtree(node->neChild);
	deleteQuadtree(node->swChild);
	deleteQuadtree(node->seChild);
}


// pruneSize (public interface)
//   - parameters: int tolerance - an integer representing the maximum
//                    "distance" which we will permit between a node's color
//                    (i.e. the average of its descendant leaves' colors)
//                    and the color of each of that node's descendant leaves
//   - returns the number of leaves which this quadtree would contain if it
//        was pruned using the given tolerance; does not actually modify the
//        tree
int Quadtree::pruneSize(int tolerance) const
{
	if (root == NULL) return 0;
	return pruneSize(tolerance, root);
}

// helper function of pruneSize(int tolerance)
int Quadtree::pruneSize(int tolerance, QuadtreeNode* node) const{
	if (!hasChildren(node)){
		return 1;
	} else {
		if (isChildrenPrunable(tolerance, node, node)){
			return 1;
		} else {
			return pruneSize(tolerance, node->nwChild) +
				   pruneSize(tolerance, node->neChild) +
				   pruneSize(tolerance, node->swChild) +
				   pruneSize(tolerance, node->seChild);
		}
	}
}


// idealPrune (public interface)
//   - parameters: int numLeaves - an integer representing the number of
//                    leaves we wish the quadtree to have, after pruning
//   - returns the minimum tolerance such that pruning with that tolerance
//        would yield a tree with at most numLeaves leaves
int Quadtree::idealPrune(int numLeaves) const
{
	if (root == NULL) return 0;
	return searchTolerance(numLeaves, MIN_TOLERANCE, MAX_TOLERANCE);
}


/** helper function of idealPrune()
  * binary search for the minimum tolerance given numLeaves
  * @param
  * numLeaves - number of leaves we wish the quadtree to have after pruning
  * minTolerance - minimum tolerance of current level of search
  * maxTolerance - maximum tolerance of current level of search
  */
int Quadtree::searchTolerance(int numLeaves, int minTolerance, int maxTolerance) const {
	int tryTolerance = (minTolerance + maxTolerance) / 2;
	int numLeavesAfterTry = pruneSize(tryTolerance);
	if (numLeavesAfterTry <= numLeaves){
		if (pruneSize(tryTolerance - 1) > numLeaves) {
			return tryTolerance;
		} else { 		// tryTolerance is too large
			return searchTolerance(numLeaves, minTolerance, tryTolerance);
		}
	} else {			// tryTolerance is too small
		return searchTolerance(numLeaves, tryTolerance, maxTolerance);
	}
}

// QuadtreeNode
//   - parameters: none
//   - constructor for the QuadtreeNode class; creates an empty
//        QuadtreeNode, with all child pointers NULL
Quadtree::QuadtreeNode::QuadtreeNode()
{
    neChild = seChild = nwChild = swChild = NULL;
}

// QuadtreeNode
//   - parameters: RGBAPixel const & elem - reference to a const
//        RGBAPixel which we want to store in this node
//   - constructor for the QuadtreeNode class; creates a QuadtreeNode
//        with element elem and all child pointers NULL
Quadtree::QuadtreeNode::QuadtreeNode(RGBAPixel const& elem)
{
    element = elem;
    neChild = seChild = nwChild = swChild = NULL;
}