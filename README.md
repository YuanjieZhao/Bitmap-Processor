# Bitmap-Processing-Library

A C++ library for processing PNG, capable of decompressing, rotating, pruning (compressing), and saving image.

## Public API

`buildTree`: transform a given PNG image into internal representation for future processing

`decompress`: transform internal representation into a PNG image

`prune`: compress a given PNG image using a specified tolerance value

`clockwiseRotate`: rotate a given PNG image clockwise

`getPixel`: get pixel value at a specified location

## Internal Representation of Image

A PNG image is first transformed into a quadtree before processing. Suppose we have an image of 128x128 pixels. In the following figure, the node at the green level of the tree corresponds to the entire 128x128 image; the nodes at the teal level of the tree correspond to the 64x64 partitions of the image; the nodes at the red level of the tree correspond to the 32x32 partitions of the image; the nodes at the black level of the tree correspond to the 16x16 partitions of the image; and so on. Each parent node can have either four or zero children.

![represent bitmap as quadtree](https://github.com/YuanjieZhao/Bitmap-Processor/blob/master/represent_bitmap_as_quadtree.svg)

## Sidenote

This library is an example of how recursion can greatly simplify the code and improve the overally readability.