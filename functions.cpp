#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include "functions.h"

using namespace std;

bool outOf2DBounds(int col, int row, int maxCol, int maxRow) {
  if (col < 0 || row < 0) {
    return true;
  }
  else if (col >= maxCol || row >= maxRow) {
    return true;
  }
  else {
    return false;
  }
}

bool validPixelArray(const Pixel *const*image, int width, int height) {

  for (int col = 0; col < width; ++col) {

    if (image[col] == nullptr) {
      return false;
    }
    
    for (int row = 0; row < height; ++row) {
      if (&image[col][row] == nullptr) {
        return false;
      }
    }
  }

  return true;
}


int energy(const Pixel *const*image, int col, int row, int width, int height) {

  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Image width and/or height cannot be 0.");
  }

  if (outOf2DBounds(col, row, width, height)) {
    throw std::invalid_argument("Pixel column and/or row is out of bounds.");
  }

  if (!validPixelArray(image, width, height)) {
    throw std::invalid_argument("Image array cannot contain null pointers.");
  }

  int x_gradient, rx, gx, bx;
  int y_gradient, ry, gy, by;

  // X RBG ENERGY DEFINITION
  if (width == 1) {
    rx = 0;
    gx = 0;
    bx = 0;
  }
  else if (col == 0) {
    rx = abs(image[width - 1][row].r - image[col + 1][row].r);
    gx = abs(image[width - 1][row].g - image[col + 1][row].g);
    bx = abs(image[width - 1][row].b - image[col + 1][row].b);
  }
  else if (col == (width - 1)) {
    rx = abs(image[col - 1][row].r - image[0][row].r);
    gx = abs(image[col - 1][row].g - image[0][row].g);
    bx = abs(image[col - 1][row].b - image[0][row].b);
  }
  else {
    rx = abs(image[col - 1][row].r - image[col + 1][row].r);
    gx = abs(image[col - 1][row].g - image[col + 1][row].g);
    bx = abs(image[col - 1][row].b - image[col + 1][row].b);
  }

  // Y RGB ENERGY DEFINITION
  if (height == 1) {
    ry = 0;
    gy = 0;
    by = 0;
  }
  else if (row == 0) {
    ry = abs(image[col][height - 1].r - image[col][row + 1].r);
    gy = abs(image[col][height - 1].g - image[col][row + 1].g);
    by = abs(image[col][height - 1].b - image[col][row + 1].b);
  }
  else if (row == (height - 1)) {
    ry = abs(image[col][row - 1].r - image[col][0].r);
    gy = abs(image[col][row - 1].g - image[col][0].g);
    by = abs(image[col][row - 1].b - image[col][0].b);
  }
  else {
    ry = abs(image[col][row - 1].r - image[col][row + 1].r);
    gy = abs(image[col][row - 1].g - image[col][row + 1].g);
    by = abs(image[col][row - 1].b - image[col][row + 1].b);
  }

  x_gradient = rx*rx + gx*gx + bx*bx;

  y_gradient = ry*ry + gy*gy + by*by;

  int energy = x_gradient + y_gradient;

  return energy;
}


int getVerticalSeam(const Pixel *const*image, int start_col, int width, int height, int* seam) {

  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("Image width and/or height cannot be 0.");
  }

  if (start_col < 0 || start_col >= width) {
    throw std::invalid_argument("Starting column must be within array bounds.");
  }

  if (!validPixelArray(image, width, height)) {
    throw std::invalid_argument("Image array cannot contain null pointers.");
  }

  // Assume len(seam) == height
  seam[0] = start_col;

  // initialize to energy of first pixel
  int total_minimal_energy = energy(image, start_col, 0, width, height);

  int current_col = start_col;
  for (int row = 1; row < height; ++row) {
    // find appropriate minimal energy pixel
    // if on edge, do not consider any pixels over bounds

    // These directions are in relation to the direction the path is moving
    // i.e. facing downward
    int forward = energy(image, current_col, row, width, height);
    // Must determine these below, might be on column bound
    int left;
    int right;


    if (width == 1) {
      seam[row] = current_col;
    }
    else if (current_col == 0) { // can only go "left" (right) or forward
      left = energy(image, current_col + 1, row, width, height);

      if (forward <= left) {
        seam[row] = current_col;
      } else { // Minimum is 'left'
        seam[row] = current_col + 1;
      }
    }
    else if (current_col == width - 1) { // can only go "right"(left) or forward
      right = energy(image, current_col - 1, row, width, height);
      
      if (forward <= right) {
        seam[row] = current_col;
      }
      else { // minimum is 'right'
        seam[row] = current_col - 1;
      }
    }
    else { // general case: not on column bound
      left = energy(image, current_col + 1, row, width, height);
      right = energy(image, current_col - 1, row, width, height);

      if (forward <= right && forward <= left) { // forward is leq both left & right
        seam[row] = current_col;
      }
      else if (left <= right) { // minimum is left
        seam[row] = current_col + 1;
      }
      else { // minimum is right
        seam[row] = current_col - 1;
      }
    }

    // make sure the current column is set to whatever the next column
    // in the path was determined to be
    current_col = seam[row]; 
    total_minimal_energy += energy(image, seam[row], row, width, height);
  }

  return total_minimal_energy;
}


void removeVerticalSeam(Pixel **image, int& width, int height, int *verticalSeam) {

  for (int row = 0; row < height; ++row) {
    for (int col = verticalSeam[row]; col < width - 1; ++col) {
      image[col][row] = image[col + 1][row];
    }
  }

  width--;
}

/*
// TODO Write this function for extra credit
int getHorizontalSeam(const Pixel *const*image, int start_row, int width, int height, int* seam)
{
  return 0;
}

// TODO Write this function for extra credit
void removeHorizontalSeam(Pixel **image, int width, int& height, int *horizontalSeam)
{
  
}
*/

int *findMinVerticalSeam(const Pixel *const*image, int width, int height)
{
  // initialize minSeam and minDistance to seam starting at first col (index 0)
  int *minSeam = new int[height]{0};
  int minDist = getVerticalSeam(image, 0, width, height, minSeam);

  int *candidateSeam = new int[height]{0};
  int candidateDistance = -1; // invalid distance

  // start at second col (index 1) since we initialized with first col (index 0)
  for (int col = 1; col < width; ++col)
  {
    candidateDistance = getVerticalSeam(image, col, width, height, candidateSeam);

    if (candidateDistance < minDist)
    { // new min
      //  swap min & candidate
      minDist = candidateDistance;
      int* temp = candidateSeam;
      candidateSeam = minSeam;
      minSeam = temp;
    }
  }

  // clean up 
  delete [] candidateSeam;

  return minSeam;
}
/*
int *findMinHorizontalSeam(const Pixel *const*image, int width, int height)
{
  // initialize minSeam and minDistance to seam starting at first row (index 0)
  int *minSeam = new int[width]{0};
  int minDistance = getHorizontalSeam(image, 0, width, height, minSeam);

  int *candidateSeam = new int[width]{0};
  int candidateDistance = -1; // invalid distance

  // start at second row (index 1) since we initialized with first row (index 0)
  for (int row = 1; row < height; ++row)
  {
    candidateDistance = getHorizontalSeam(image, row, width, height, candidateSeam);

    if (candidateDistance < minDistance)
    { // new minimum
      //  swap min and candidate seams
      minDistance = candidateDistance;
      int* temp = candidateSeam;
      candidateSeam = minSeam;
      minSeam = temp;
    }
  }

    // clean up 
  delete [] candidateSeam;

  return minSeam;
}
*/
Pixel **createImage(int width, int height)
{
  cout << "Start createImage... " << endl;

  // Create a one dimensional array on the heap of pointers to Pixels
  //    that has width elements (i.e. the number of columns)
  Pixel **image = new Pixel *[width] {}; // initializes to nullptr

  for (int col = 0; col < width; ++col)
  { // loop through each column
    // assign that column to a one dimensional array on the heap of Pixels
    //  that has height elements (i.e. the number of rows)
    try
    {
      image[col] = new Pixel[height];
    }
    catch (std::bad_alloc &e)
    {
      // clean up already allocated arrays
      for (int i = 0; i < col; ++i)
      {
        delete[] image[i];
      }
      delete[] image;
      // rethrow
      throw e;
    }
  }

  // initialize cells
  // cout << "Initializing cells..." << endl;
  for (int row = 0; row < height; ++row)
  {
    for (int col = 0; col < width; ++col)
    {
      // cout << "(" << col << ", " << row << ")" << endl;
      image[col][row] = {0, 0, 0};
    }
  }
  cout << "End createImage... " << endl;
  return image;
}

void deleteImage(Pixel **image, int width)
{
  cout << "Start deleteImage..." << endl;
  // avoid memory leak by deleting the array
  for (int i = 0; i < width; ++i)
  {
    delete[] image[i];
  }
  delete[] image;
  image = nullptr;
  cout << "End deleteImage..." << endl;
}

bool isValidColor(int colorVal)
{
  if (colorVal < 0 || colorVal > 255)
  {
    return false;
  }
  return true;
}

Pixel ** loadImage(string filename, int &width, int &height)
{
  cout << "Start loadImage..." << endl;
  // remove
  ifstream ifs(filename);
  if (!ifs.is_open())
  {
    throw std::invalid_argument("Failed to open input file (" + filename + ")");
  }

  string type;
  ifs >> type; // should be P3
  if (toupper(type.at(0)) != 'P' || type.at(1) != '3')
  {
    throw std::domain_error("Not PPM type P3 (" + type + ")");
  }
  ifs >> width;
  // cout << "w and h: " << w << " " << h << endl;
  if (ifs.fail())
  {
    throw std::domain_error("Read non-integer value for width");
  }
  if (width <= 0)
  {
    ostringstream oss;
    oss << "Width in file must be greater than 0 (" << width << ")";
    throw std::domain_error(oss.str());
  }

  ifs >> height;
  if (ifs.fail())
  {
    cout << "Read non-integer value for height" << endl;
  }
  if (height <= 0)
  {
    ostringstream oss;
    oss << "Height in file must be greater than 0 (" << height << ")";
    throw std::domain_error(oss.str());
  }

  int colorMax = 0;
  ifs >> colorMax;
  if (ifs.fail())
  {
    throw std::domain_error("Read non-integer value for max color value");
  }
  if (colorMax != 255)
  {
    ostringstream oss;
    oss << "Max color value must be 255 (" << colorMax << ")";
    throw std::domain_error(oss.str());
  }

  // load image throws exceptions but we will let them pass through
  Pixel **image = createImage(width, height);

  for (int row = 0; row < height; ++row)
  {
    for (int col = 0; col < width; ++col)
    {
      // cout << "Pixel(" << col << ", " << row << ")" << endl;
      ifs >> image[col][row].r;
      if (ifs.fail() && !ifs.eof())
      { // bad input that is not caused by being at the end of the file
        throw std::domain_error("Read non-integer value for red");
      }
      if (ifs.eof()) {
        throw std::domain_error("Expected value for red, but at end of file");
      }
      if (!isValidColor(image[col][row].r))
      {
        ostringstream oss;
        oss << "Invalid color value for red (" << image[col][row].r << ")";
        throw std::domain_error(oss.str());
      }

      ifs >> image[col][row].g;
      if (ifs.fail() && !ifs.eof())
      { // bad input that is not caused by being at the end of the file
        throw std::domain_error("Read non-integer value for green");
      }
      if (ifs.eof()) {
        throw std::domain_error("Expected value for green, but at end of file");
      }
      if (!isValidColor(image[col][row].r))
      {
        ostringstream oss;
        oss << "Invalid color value for green (" << image[col][row].r + ")";
        throw std::domain_error(oss.str());
      }

      ifs >> image[col][row].b;
      if (ifs.fail() && !ifs.eof())
      { // bad input that is not caused by being at the end of the file
        throw std::domain_error("Read non-integer value for blue");
      }
      if (ifs.eof()) {
        throw std::domain_error("Expected value for blue, but at end of file");
      }
      if (!isValidColor(image[col][row].r))
      {
        ostringstream oss;
        oss << "Invalid color value for blue (" << image[col][row].r + ")";
        throw std::domain_error(oss.str());
      }
    }
  }


  string nextline;
  ifs >> nextline;

  if (!ifs.fail()) {
    throw std::domain_error("Too many color values in file");
  }

  cout << "End loadImage..." << endl;
  return image;
}

void outputImage(string filename, const Pixel *const *image, int width, int height)
{
  cout << "Start outputImage..." << endl;
  // remove code
  // declare/define and open output file stream with filename
  ofstream ofs(filename);
  // ensure file is open
  if (!ofs.is_open())
  {
    throw std::invalid_argument("Error: failed to open output file - " + filename);
  }
  ofs << "P3" << endl;
  ofs << width << " " << height << endl;
  ofs << 255 << endl;
  for (int row = 0; row < height; ++row)
  {
    for (int col = 0; col < width; ++col)
    {
      ofs << image[col][row].r << " ";
      ofs << image[col][row].g << " ";
      ofs << image[col][row].b << " ";
    }
    ofs << endl;
  }
  cout << "End outputImage..." << endl;
}