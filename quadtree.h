/**
 * @file quadtree.h
 * Quadtree class definition.
 */

#ifndef QUADTREE_H
#define QUADTREE_H

#include "png.h"

/**
 * A tree structure that is used to compress PNG images.
 */
class Quadtree
{
  public:
    /**
     * The no parameters constructor takes no arguments, and produces
     * an empty Quadtree object, i.e. one which has no associated
     * QuadtreeNode objects, and in which root is NULL.
     */
    Quadtree();

    /**
     * This constructor's purpose is to build a Quadtree representing
     * the upper-left resolution by resolution block of the source image. This
     * effectively crops the source image into a resolution by resolution 
     * square.
     *
     * You may assume that resolution is a power of two, and that the width and
     * height of source are each at least resolution.
     *
     * Perhaps, to implement this, you could leverage the functionality
     * of another function you have written.
     *
     * @param source The source image to base this Quadtree on
     * @param resolution The width and height of the sides of the image to
     *  be represented
     */
    Quadtree(PNG const& source, int resolution);

    /**
     * Copy constructor. Simply sets this Quadtree to be a copy of the
     * parameter.
     * @param other The Quadtree to make a copy of
     */
    Quadtree(Quadtree const& other);

    /**
     * Destructor; frees all memory associated with this Quadtree.
     */
    ~Quadtree();

    /**
     * Assignment operator; frees memory associated with this Quadtree
     * and sets its contents to be equal to the parameter's.
     *
     * @param other The Quadtree to make a copy of
     * @return A constant reference to the Quadtree value that was copied
     */
    Quadtree const& operator=(Quadtree const& other);

    /**
     * Deletes the current contents of this Quadtree object, then turns
     * it into a Quadtree object representing the upper-left resolution 
     * by resolution block of source. You may assume that resolution 
     * is a power of two, and that the width and height of source 
     * are each at least resolution.
     *
     * @param source The source image to base this Quadtree on
     * @param resolution The width and height of the sides of the image to
     *  be represented
     */
    void buildTree(PNG const& source, int resolution);

    /**
     * Gets the RGBAPixel corresponding to the pixel at coordinates (x,
     * y) in the bitmap image which the Quadtree represents.
     *
     * Note that the Quadtree may not contain a node specifically
     * corresponding to this pixel (due, for instance, to pruning - see
     * below). In this case, getPixel will retrieve the pixel (i.e. the
     * color) of the square region within which the smaller query grid
     * cell would lie. (That is, it will return the element of the
     * nonexistent leaf's deepest surviving ancestor.) If the supplied
     * coordinates fall outside of the bounds of the underlying bitmap,
     * or if the current Quadtree is "empty" (i.e., it was created by
     * the default constructor) then the returned RGBAPixel should be
     * the one which is created by the default RGBAPixel constructor.
     *
     * @param x The x coordinate of the pixel to be retrieved
     * @param y The y coordinate of the pixel to be retrieved
     * @return The pixel at the given (x, y) location
     */
    RGBAPixel getPixel(int x, int y) const;

    /**
     * Returns the underlying PNG object represented by the Quadtree.
     *
     * If the current Quadtree is "empty" (i.e., it was created by the
     * default constructor) then the returned PNG should be the one
     * which is created by the default PNG constructor.  This function
     * effectively "decompresses" the Quadtree. A Quadtree object, in
     * memory, may take up less space than the underlying bitmap image,
     * but we cannot simply look at the Quadtree and tell what image it
     * represents. By converting the Quadtree back into a bitmap image,
     * we lose the compression, but gain the ability to view the image
     * directly.
     *
     * @return The decompressed PNG image this Quadtree represents
     */
    PNG decompress() const;

    /**
     * Rotates the Quadtree object's underlying image clockwise by 90
     * degrees. (Note that this should be done using pointer
     * manipulation, not by attempting to swap the element fields of
     * QuadtreeNodes.  Trust us; it's easier this way.)
     */
    void clockwiseRotate();

// PA 4 FUNCTIONS

    /**
     * Compresses the image this Quadtree represents.
     *
     * If the color values of the leaves of a subquadtree don't vary by
     * much, we might as well represent the entire subtree by, say, the
     * average color value of those leaves. We may use this information
     * to effectively "compress" the image, by strategically trimming
     * the Quadtree.
     *
     * Consider a node \f$n\f$ and the subtree, \f$T_n\f$ rooted at
     * \f$n\f$, and let \f$avg\f$ denote the component-wise average
     * color value of all the leaves of \f$T_n\f$.  Component-wise
     * average means that every internal node in the tree calculates
     * its value by averaging its immediate children. This implies that
     * the average must be calculated in a "bottom-up" manner.
     *
     * Due to rounding errors, using the component-wise average is not
     * equivalent to using the true average of all leaves in a subtree.
     * If a node \f$n\f$ is pruned, the children of \f$n\f$ and the
     * subtrees for which they are the roots are removed from the
     * Quadtree. Node \f$n\f$ is pruned if the color value of no leaf
     * in \f$T_n\f$, differs from \f$avg\f$ by more than tolerance.
     * (Note: for all average calculations, just truncate the value to
     * integer.)
     *
     * We define the "difference" between two colors, \f$(r_1, g_1,
       b_1)\f$ and \f$(r_2, g_2, b_2)\f$, to be \f$(r_2 - r_1)^2 + (g_2
       - g_1)^2 + (b_2 - b_1)^2\f$.
     *
     * To be more complete, if the tolerance condition is met at a node
     * \f$n\f$, then the block of the underlying image which \f$n\f$
     * represents contains only pixels which are "nearly" the same
     * color. For each such node \f$n\f$, we remove from the Quadtree
     * all four children of \f$n\f$, and their respective subtrees (an
     * operation we call a pruning).  This means that all of the leaves
     * that were deleted, corresponding to pixels whose colors were
     * similar, are now replaced by a single leaf with color equal to
     * the average color over that square region.
     *
     * The prune function, given a tolerance value, prunes the Quadtree
     * as extensively as possible. (Note, however, that we do not want
     * the prune function to do an "iterative" prune. It is conceivable
     * that by pruning some mid-level node \f$n\f$, an ancestor \f$p\f$
     * of \f$n\f$ then becomes prunable, due to the fact that the prune
     * changed the leaves descended from \f$p\f$. Your prune function
     * should evaluate the prunability of each node based on the
     * presence of all nodes, and then delete the subtrees based at
     * nodes deemed prunable.)
     *
     * @note You should start pruning from the root of the Quadtree.
     *
     * @param tolerance The integer tolerance between two nodes that
     *  determines whether the subtree can be pruned.
     */
    void prune(int tolerance);

    /**
     * This function is similar to prune; however, it does not actually
     * prune the Quadtree. Rather, it returns a count of the total
     * number of leaves the Quadtree would have if it were pruned as in
     * the prune function.
     *
     * @param tolerance The integer tolerance between two nodes that
     *  determines whether the subtree can be pruned.
     * @return How many leaves this Quadtree would have if it were pruned
     *  with the given tolerance.
     */
    int pruneSize(int tolerance) const;

    /**
     * Calculates and returns the minimum tolerance necessary to
     * guarantee that upon pruning the tree, no more than numLeaves
     * leaves remain in the Quadtree.
     *
     * Essentially, this function is an inverse of pruneSize; for any
     * Quadtree object theTree, and for any positive integer tolerance
     * it should be true that
     *
     *      theTree.pruneSize(theTree.idealPrune(numLeaves)) <= numLeaves
     *
     * Once you understand what this function is supposed to do, you
     * will probably notice that there is an "obvious" implementation.
     * This is probably not the implementation you want to use! There
     * is a fast way to implement this function, and a slow way; you
     * will need to find the fast way. (If you doubt that it makes a
     * significant difference, the tests in the given main.cpp should
     * convince you.)
     *
     * @param numLeaves The number of leaves you want to remain in the tree
     *  after prune is called.
     * @return The minimum tolerance needed to guarantee that there are no
     *  more than numLeaves remaining in the tree.
     * @note The "obvious" implementation involves a sort of linear search over
     *  all possible tolerances. What if you tried a binary search instead?
     */
    int idealPrune(int numLeaves) const;

// END PA 4 FUNCTIONS

  private:
    /**
     * A simple class representing a single node of a Quadtree.
     * You may want to add to this class; in particular, it could
     * probably use a constructor or two.
     */
    class QuadtreeNode
    {
      public:
        QuadtreeNode* nwChild; /**< pointer to northwest child */
        QuadtreeNode* neChild; /**< pointer to northeast child */
        QuadtreeNode* swChild; /**< pointer to southwest child */
        QuadtreeNode* seChild; /**< pointer to southeast child */

        RGBAPixel element; /**< the pixel stored as this node's "data" */

        QuadtreeNode();
        QuadtreeNode(RGBAPixel const& elem);
    };

    QuadtreeNode* root; /**< pointer to root of quadtree */
    int res; // resolution of the underlying bitmap

    // helper function for deep delete
    // Used by destructor and copy/assignment
    // Deallocates Quadtree and its QuadtreeNode
    void deleteQuadtree();

    // helper function for deleteQuadtree() and pruneChildren()
    void deleteQuadtree(QuadtreeNode*& node);

    // helper function for deep copy
    // Used by copy constructor and operator=
    void copyQuadtree(Quadtree const& other);

    // helper function for copyQuadtree(Quadtree const& other)
    void copyQuadtree(QuadtreeNode*& myNode, QuadtreeNode* const& otherNode);

    /** private helper function for buildTree(PNG const& source, int resolution)
      * @param
      * source - reference to a const PNG object
      * resolution - resolution of the portion of source from which this tree will be built
      * x - x-coordinate of top-left corner of the region represented by current node
      * y - y-coordinate of top-left corner of the region represented by current node
      * node - current node in Quadtree
      */
    void buildTree(PNG const& source, int resolution, int x, int y, QuadtreeNode*& node);

    /* return the average of RGBAPixel of node's children
     * @param node - a non-leaf QuadtreeNode that has four children
     */
    void getAvgPixelOfChildren(QuadtreeNode* node);

    // return the average of the given four byte number
    uint8_t getAvg(uint8_t n1, uint8_t n2, uint8_t n3, uint8_t n4);

    // helper function for getPixel(int x, int y)
    RGBAPixel getPixel(int x, int y, QuadtreeNode* node, int resolution) const;

    // return true if given node has children (a node can have either zero or four children)
    bool hasChildren(QuadtreeNode* node) const ;

    // return true if the value of x or y is outside the bounds of underlying bitmap
    bool outOfBound(int x, int y) const;

    // helper function for clockwiseRotate()
    void clockwiseRotate(QuadtreeNode*& node);

    /** helper function of decompress()
     * transform a PNG img into the PNG image represented by this QuadTree
     * @param
     * source - the original PNG image to be transformed
     * resolution - the resolution of the region represented by current Quadtree node
     * x - x-coordinate of top-left corner of the region represented by current node
     * y - y-coordinate of top-left corner of the region represented by current node
     * node - current node in Quadtree
     */
    void transform (PNG& source, int resolution, int x, int y, QuadtreeNode* node) const ;

    // helper function of prune(int tolerance)
    // return true if all children (direct and indirect) of the given node are pruned
    void prune(int tolerance, QuadtreeNode*& node);

    // return true if all children (direct and indirect) of node are prunable
    // Pre-condition: rootNode must have children
    bool isChildrenPrunable(int tolerance, QuadtreeNode*& rootNode, QuadtreeNode*& node) const ;

    // return true if the color difference between avgPixel and nodePixel is no more than tolerance
    bool isPrunable(int tolerance, RGBAPixel avgPixel, RGBAPixel nodePixel) const ;

    // return square of number n
    int square (int n) const ;

    // delete all descendants of the given node
    void pruneChildren(QuadtreeNode*& node);

    // helper function of pruneSize(int tolerance)
    int pruneSize(int tolerance, QuadtreeNode* node) const;

    /** helper function of idealPrune()
      * binary search for the minimum tolerance given numLeaves
      * @param
      * numLeaves - number of leaves we wish the quadtree to have after pruning
      * minTolerance - minimum tolerance of current level of search
      * maxTolerance - maximum tolerance of current level of search
      */
    int searchTolerance(int numLeaves, int minTolerance, int maxTolerance) const ;



/**** Functions for testing/grading                      ****/
/**** Do not remove this line or copy its contents here! ****/
#include "quadtree_given.h"
};

#endif
