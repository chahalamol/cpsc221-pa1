// File:        imglist.cpp
// Date:        2022-01-20 03:22
// Description: Contains partial implementation of ImgList class
//              for CPSC 221 2021W2 PA1
//              Function bodies to be completed by yourselves
//
// ADD YOUR FUNCTION IMPLEMENTATIONS IN THIS FILE
//

#include "imglist.h"
#include <vector>   // added
#include <iostream> // added

#include <math.h> // provides fmin and fabs functions

/**************************
 * MISCELLANEOUS FUNCTIONS *
 **************************/

/*
 * This function is NOT part of the ImgList class,
 * but will be useful for one of the ImgList functions.
 * Returns the "difference" between two hue values.
 * PRE: hue1 is a double between [0,360).
 * PRE: hue2 is a double between [0,360).
 *
 * The hue difference is the absolute difference between two hues,
 * but takes into account differences spanning the 360 value.
 * e.g. Two pixels with hues 90 and 110 differ by 20, but
 *      two pixels with hues 5 and 355 differ by 10.
 */
double HueDiff(double hue1, double hue2)
{
  return fmin(fabs(hue1 - hue2), fabs(360 + fmin(hue1, hue2) - fmax(hue1, hue2)));
}

/*********************
 * CONSTRUCTORS, ETC. *
 *********************/

/*
 * Default constructor. Makes an empty list
 */
ImgList::ImgList()
{
  // set appropriate values for all member attributes here
  northwest = NULL;
  southeast = NULL;
}

/*
 * Creates a list from image data
 * PRE: img has dimensions of at least 1x1
 */
ImgList::ImgList(PNG &img)
{
  // build the linked node structure and set the member attributes appropriately

  vector<vector<ImgNode *> > myVector;

  int m = img.width();
  int n = img.height();
  myVector.resize(m, vector<ImgNode *>(n));

  for (int y = 0; y < n; y++)
  {

    for (int x = 0; x < m; x++)
    {

      myVector[x][y] = new ImgNode();

      HSLAPixel *pixel = img.getPixel(x, y);

      myVector[x][y]->colour = *pixel;
    }
  }

  for (int y = 0; y < n; y++)
  {

    for (int x = 0; x < m; x++)
    {

      if (y != 0)
      {

        myVector[x][y]->north = myVector[x][y - 1];
      }

      if (y != n - 1)
      {

        myVector[x][y]->south = myVector[x][y + 1];
      }

      if (x != 0)
      {

        myVector[x][y]->west = myVector[x - 1][y];
      }

      if (x != m - 1)
      {

        myVector[x][y]->east = myVector[x + 1][y];
      }
    }
  }

  northwest = myVector[0][0];
  southeast = myVector[m - 1][n - 1];
}

/*
 * Copy constructor.
 * Creates this this to become a separate copy of the data in otherlist
 */
ImgList::ImgList(const ImgList &otherlist)
{
  // build the linked node structure using otherlist as a template
  Copy(otherlist);
}

/*
 * Assignment operator. Enables statements such as list1 = list2;
 *   where list1 and list2 are both variables of ImgList type.
 * POST: the contents of this list will be a physically separate copy of rhs
 */
ImgList &ImgList::operator=(const ImgList &rhs)
{
  // Re-build any existing structure using rhs as a template

  if (this != &rhs)
  { // if this list and rhs are different lists in memory
    // release all existing heap memory of this list
    Clear();

    // and then rebuild this list using rhs as a template
    Copy(rhs);
  }

  return *this;
}

/*
 * Destructor.
 * Releases any heap memory associated with this list.
 */
ImgList::~ImgList()
{
  // Ensure that any existing heap memory is deallocated
  Clear();
}

/************
 * ACCESSORS *
 ************/

/*
 * Returns the horizontal dimension of this list (counted in nodes)
 * Note that every row will contain the same number of nodes, whether or not
 *   the list has been carved.
 * We expect your solution to take linear time in the number of nodes in the
 *   x dimension.
 */
unsigned int ImgList::GetDimensionX() const
{

  ImgNode *curr = northwest;
  int i = 1;

  if (northwest == NULL || southeast == NULL)
  {

    return 0;
  }

  if (northwest == southeast)
  {

    return 1;

  } // 1 pixel only

  while (curr->east != NULL)
  {

    curr = curr->east;
    i++;
  }

  return i;
}

/*
 * Returns the vertical dimension of the list (counted in nodes)
 * It is useful to know/assume that the grid will never have nodes removed
 *   from the first or last columns. The returned value will thus correspond
 *   to the height of the PNG image from which this list was constructed.
 * We expect your solution to take linear time in the number of nodes in the
 *   y dimension.
 */
unsigned int ImgList::GetDimensionY() const
{

  ImgNode *curr = northwest;

  int i = 1;

  if (northwest == NULL && southeast == NULL)
  {

    return 0;
  }

  if (northwest == southeast)
  {

    return 1;
  }

  while (curr->south != NULL)
  {

    curr = curr->south;
    i++;
  }

  return i;
}

/*
 * Returns the horizontal dimension of the list (counted in original pixels, pre-carving)
 * The returned value will thus correspond to the width of the PNG image from
 *   which this list was constructed.
 * We expect your solution to take linear time in the number of nodes in the
 *   x dimension.
 */
unsigned int ImgList::GetDimensionFullX() const
{

  ImgNode *curr = northwest;
  int i = 1;

  if (northwest == NULL || southeast == NULL)
  {

    return 0;
  }

  if (northwest == southeast)
  {

    return 1;

  } // 1 pixel only

  while (curr->east != NULL)
  {

    i++;
    i += curr->skipright;
    curr = curr->east;
  }

  return i;

  //  skipright and skipleft add to X..  //
  //  count how many pixels this represents
}

/*
 * Returns a pointer to the node which best satisfies the specified selection criteria.
 * The first and last nodes in the row cannot be returned.
 * PRE: rowstart points to a row with at least 3 physical nodes
 * PRE: selectionmode is an integer in the range [0,1]
 * PARAM: rowstart - pointer to the first node in a row
 * PARAM: selectionmode - criterion used for choosing the node to return
 *          0: minimum luminance across row, not including extreme left or right nodes
 *          1: node with minimum total of "hue difference" with its left neighbour and with its right neighbour.
 *        In the (likely) case of multiple candidates that best match the criterion,
 *        the left-most node satisfying the criterion (excluding the row's starting node)
 *        will be returned.
 * A note about "hue difference": For PA1, consider the hue value to be a double in the range [0, 360).
 * That is, a hue value of exactly 360 should be converted to 0.
 * The hue difference is the absolute difference between two hues,
 * but be careful about differences spanning the 360 value.
 * e.g. Two pixels with hues 90 and 110 differ by 20, but
 *      two pixels with hues 5 and 355 differ by 10.
 */
ImgNode *ImgList::SelectNode(ImgNode *rowstart, int selectionmode)
{
  ImgNode *curr = rowstart->east; // past the first
  ImgNode *temp = rowstart->east;

  if (selectionmode == 0)
  {

    while (curr->east != NULL) // avoid last
    {                          // iterate through row

      if (curr->colour.l < curr->east->colour.l && curr->colour.l < temp->colour.l) // smaller than temp as well..
      {                                                                             // if curr

        temp = curr; // store min luminance
      }

      curr = curr->east; // iterate curr past first
    }
  }
  else if (selectionmode == 1)
  {

    double sum = HueDiff(curr->colour.h, curr->east->colour.h) + HueDiff(curr->colour.h, curr->west->colour.h);

    while (curr->east != NULL)
    {

      if (HueDiff(curr->east->colour.h, curr->east->east->colour.h) + HueDiff(curr->east->colour.h, curr->colour.h) < sum && HueDiff(curr->east->colour.h, curr->east->east->colour.h) + HueDiff(curr->east->colour.h, curr->colour.h) < HueDiff(temp->east->colour.h, temp->east->east->colour.h) + HueDiff(temp->east->colour.h, temp->colour.h))
      {

        sum = HueDiff(curr->east->colour.h, curr->east->east->colour.h) + HueDiff(curr->east->colour.h, curr->colour.h);

        temp = curr->east;
      }

      curr = curr->east; // just iterate dont reassign temp
    }
  }

  return temp;
}

/*
 * Renders this list's pixel data to a PNG, with or without filling gaps caused by carving.
 * PRE: fillmode is an integer in the range of [0,2]
 * PARAM: fillgaps - whether or not to fill gaps caused by carving
 *          false: render one pixel per node, ignores fillmode
 *          true: render the full width of the original image,
 *                filling in missing nodes using fillmode
 * PARAM: fillmode - specifies how to fill gaps
 *          0: solid, uses the same colour as the node at the left of the gap
 *          1: solid, using the averaged values (all channels) of the nodes at the left and right of the gap
 *             Note that "average" for hue will use the closer of the angular distances,
 *             e.g. average of 10 and 350 will be 0, instead of 180.
 *             Average of diametric hue values will use the smaller of the two averages
 *             e.g. average of 30 and 210 will be 120, and not 280
 *                  average of 170 and 350 will be 80, and not 260
 *          2: *** OPTIONAL - FOR BONUS ***
 *             linear gradient between the colour (all channels) of the nodes at the left and right of the gap
 *             e.g. a gap of width 1 will be coloured with 1/2 of the difference between the left and right nodes
 *             a gap of width 2 will be coloured with 1/3 and 2/3 of the difference
 *             a gap of width 3 will be coloured with 1/4, 2/4, 3/4 of the difference, etc.
 *             Like fillmode 1, use the smaller difference interval for hue,
 *             and the smaller-valued average for diametric hues
 */
PNG ImgList::Render(bool fillgaps, int fillmode) const
{

  PNG outpng; // this will be returned later. Might be a good idea to resize it at some point.

  // hash is wrong from test  so pointers are not allocatd right

  int k = this->GetDimensionFullX();
  int j = this->GetDimensionY();
  outpng.resize(k, j);

  if (fillgaps == false)
  {

    ImgNode *curr = northwest;
    ImgNode *row = northwest;

    int x = 0; // x
    int y = 0; // y

    while (row != NULL)
    {

      while (curr != NULL)
      { // iterate left to right

        HSLAPixel *pixel = outpng.getPixel(x, y); // start

        *pixel = curr->colour;
        curr = curr->east; // iterate along to end of row

        x++;
      }

      curr = row->south;
      row = row->south;

      y++;
      x = 0; // reset x
    }
  }
  else if (fillgaps == true)
  {

    if (fillmode == 0)
    {

      ImgNode *curr = northwest;
      ImgNode *row = northwest;

      int x = 0; // x
      int y = 0; // y
      int z = 0; // z

      while (row != NULL)
      {

        while (curr != NULL)
        { // iterate left to right

          HSLAPixel *pixel = outpng.getPixel(x, y);

          *pixel = curr->colour;

          while (z < curr->skipright)
          {

            z++;

            x++; // iterate x for every carved node

            HSLAPixel *pixel = outpng.getPixel(x, y); // pixel to right..

            *pixel = curr->colour; // set x+1 pixel to same as curr (which is west)
          }

          curr = curr->east; // move curr to next node

          x++; // since we moved again..

          z = 0; // restart for next loop
        }

        row = row->south;
        curr = row;

        y++;
        x = 0; // reset x
      }
    }
  }

  return outpng;
}

/************
 * MODIFIERS *
 ************/

/*
 * Removes exactly one node from each row in this list, according to specified criteria.
 * The first and last nodes in any row cannot be carved.
 * PRE: this list has at least 3 nodes in each row
 * PRE: selectionmode is an integer in the range [0,1]
 * PARAM: selectionmode - see the documentation for the SelectNode function.
 * POST: this list has had one node removed from each row. Neighbours of the created
 *       gaps are linked appropriately, and their skip values are updated to reflect
 *       the size of the gap.
 */
void ImgList::Carve(int selectionmode)
{
  // add your implementation here

  // one node removed from each row
  // ImgNode *ImgList::SelectNode(ImgNode *rowstart, int selectionmode)

  // call selectNode helper
  // delete that node by pointing a pointer to the selected node
  // reassign its neighbours pts
  // update skipvalues of friends.. if curr->south = 1 / west 1 /  EACH ROW In list so every time

  ImgNode *carved = SelectNode(northwest, selectionmode); // same selection mode and starts at NW

  ImgNode *row = northwest;

  while (row != NULL)
  {

    ImgNode *leftneigh = carved->west;
    ImgNode *rightneigh = carved->east;
    ImgNode *northneigh = carved->north;
    ImgNode *southneigh = carved->south;

    leftneigh->east = rightneigh; // left node east ptr points to currs right neigh

    rightneigh->west = leftneigh; // right node west ptr ppints to currs west neigh

    leftneigh->skipright = carved->skipright + 1;

    rightneigh->skipleft = carved->skipleft + 1;

    // north and south bit trickier, add skipup and skipdown prev values if exist

    if (northneigh != NULL)
    { // if we're not at the top row
      northneigh->skipdown = carved->skipdown + 1;
    }

    if (southneigh != NULL) // if we're not at the bottom
    {
      southneigh->skipup = carved->skipup + 1;
    }

    delete carved; // delete carved

    row = row->south;

    carved = SelectNode(row, selectionmode); // reassign carved to next row to select new node
  }
}

// note that a node on the boundary will never be selected for removal
/*
 * Removes "rounds" number of nodes (up to a maximum of node width - 2) from each row,
 * based on specific selection criteria.
 * Note that this should remove one node from every row, repeated "rounds" times,
 * and NOT remove "rounds" nodes from one row before processing the next row.
 * PRE: selectionmode is an integer in the range [0,1]
 * PARAM: rounds - number of nodes to remove from each row
 *        If rounds exceeds node width - 2, then remove only node width - 2 nodes from each row.
 *        i.e. Ensure that the final list has at least two nodes in each row.
 * POST: this list has had "rounds" nodes removed from each row. Neighbours of the created
 *       gaps are linked appropriately, and their skip values are updated to reflect
 *       the size of the gap.
 */
void ImgList::Carve(unsigned int rounds, int selectionmode)
{

  //  mean that if rounds > (width - 2) then removes (width - 2) nodes each row so there are 2 nodes left which is the first and the last node?

  // update links and skipvalues

  int z = 0; // var to keep track of loops to match rounds
  ImgNode *row = northwest;

  if (rounds < GetDimensionFullX() - 2)
  {

    while (z < rounds)
    {

      ImgNode *carved = SelectNode(northwest, selectionmode); // same selection mode and starts at NW

      while (row != NULL)
      {

        ImgNode *leftneigh = carved->west;
        ImgNode *rightneigh = carved->east;
        ImgNode *northneigh = carved->north;
        ImgNode *southneigh = carved->south;

        leftneigh->east = rightneigh; // left node east ptr points to currs right neigh

        rightneigh->west = leftneigh; // right node west ptr ppints to currs west neigh

        leftneigh->skipright = carved->skipright + 1;

        rightneigh->skipleft = carved->skipleft + 1;

        // north and south bit trickier, add skipup and skipdown prev values if exist

        if (northneigh != NULL)
        { // if we're not at the top row
          northneigh->skipdown = carved->skipdown + 1;
        }

        if (southneigh != NULL) // if we're not at the bottom
        {
          southneigh->skipup = carved->skipup + 1;
        }

        delete carved; // delete carved

        row = row->south;

        carved = SelectNode(row, selectionmode); // reassign carved to next row to select new node
      }

      z++;
    }
  }
  else if (rounds > GetDimensionFullX() - 2)
  { // then remove width - 2 nodes to keep only first and last one remaining

    while (z < GetDimensionFullX() - 2)
    {

      ImgNode *carved = SelectNode(northwest, selectionmode); // same selection mode and starts at NW

      while (row != NULL)
      {

        ImgNode *leftneigh = carved->west;
        ImgNode *rightneigh = carved->east;
        ImgNode *northneigh = carved->north;
        ImgNode *southneigh = carved->south;

        leftneigh->east = rightneigh; // left node east ptr points to currs right neigh

        rightneigh->west = leftneigh; // right node west ptr ppints to currs west neigh

        leftneigh->skipright = carved->skipright + 1;

        rightneigh->skipleft = carved->skipleft + 1;

        // north and south bit trickier, add skipup and skipdown prev values if exist

        if (northneigh != NULL)
        { // if we're not at the top row
          northneigh->skipdown = carved->skipdown + 1;
        }

        if (southneigh != NULL) // if we're not at the bottom
        {
          southneigh->skipup = carved->skipup + 1;
        }

        delete carved; // delete carved

        row = row->south;

        carved = SelectNode(row, selectionmode); // reassign carved to next row to select new node
      }

      z++;
    }
  }
}

/*
 * Helper function deallocates all heap memory associated with this list,
 * puts this list into an "empty" state. Don't forget to set your member attributes!
 * POST: this list has no currently allocated nor leaking heap memory,
 *       member attributes have values consistent with an empty list.
 */
void ImgList::Clear()
{
  // memory leak

  if (northwest != NULL)
  {

    if (northwest == southeast)
    {

      delete northwest;
    }
    else
    {

      ImgNode *curr = northwest;
      ImgNode *row = northwest;

      while (row != NULL)
      {

        while (curr != NULL)
        {

          delete curr;

          curr = curr->east; // iterate end of row
        }

        curr = row->south;
        row = row->south;
      }
    }
  }

  northwest = NULL;
  southeast = NULL;
}
/* ************************
 *  * OPTIONAL - FOR BONUS *
 ** ************************
 * Helper function copies the contents of otherlist and sets this list's attributes appropriately
 * PRE: this list is empty
 * PARAM: otherlist - list whose contents will be copied
 * POST: this list has contents copied from by physically separate from otherlist
 */
void ImgList::Copy(const ImgList &otherlist)
{
  // add your implementation here
}

/*************************************************************************************************
 * IF YOU DEFINED YOUR OWN PRIVATE FUNCTIONS IN imglist.h, YOU MAY ADD YOUR IMPLEMENTATIONS BELOW *
 *************************************************************************************************/

ImgNode *ImgList::buildMatrix(vector<int> &vect, PNG &img, int i, int j, ImgNode *last)
{

  if (i >= img.width() || j >= img.height())
    return NULL;
  ; // base case

  ImgNode *curr = new ImgNode;

  HSLAPixel *pixel = img.getPixel(i, j); // getting pixel at this point
  curr->colour = *pixel;                 // setting colour

  if (j == 0)
    curr->east = NULL;
  else
    curr->east = last;

  if (i == 0)
    curr->north = NULL;
  else
    curr->north = last;

  if (curr->north == NULL && curr->west == NULL)
  {

    northwest = curr;
  }

  if (curr->east == NULL && curr->south == NULL)
  { // if its bottom one

    southeast = curr;
  } // if its LAST node

  // Recursive calls  for RIGHT and DOWN pointers

  curr->east = buildMatrix(vect, img, i, j + 1, curr);

  curr->south = buildMatrix(vect, img, i + 1, j, curr);

  return curr; // now it should be last one!

  // !!! How do i return the linked list structure back from helper function to the constructor..
  // how do i return points northwest and southeast back to Imglist
}
