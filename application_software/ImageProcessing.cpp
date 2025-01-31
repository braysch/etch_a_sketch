#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <windows.h>
#include <fstream>

using namespace cv;
double distance(std::pair < double, double > a, std::pair < double, double > b) {
  double distance = (((a.first - b.first) * (a.first - b.first)) + ((a.second - b.second) * (a.second, →-b.second)));
  return distance;
}
int main(int arg, char * argv[]) {
  Mat source = imread(argv[1]);
  if (source.empty()) {
    std::cout << "Could not open or find image." << std::endl;
    return -1;
  }
  Mat source_gray, dest, detected_edges, new_edges;
  cvtColor(source, source_gray, COLOR_BGR2GRAY);
  imwrite("USU_bgray.png", source_gray);
  //Apply the Bilateral Filter
  bilateralFilter(source_gray, dest, 7, 50, 50);
  //Perform Edge Detection
  Canny(dest, detected_edges, 60, 180);
  namedWindow("Bilateral", WINDOW_NORMAL);
  imshow("Bilateral", detected_edges); //Display the edge detected Image for viewing
  std::ofstream edgeValuesFile("edgeValuesFile.txt");
  std::vector < std::pair < int, int >> edgeValues;
  //Loop through the pixel data, store the non black pixel coordinates in an array.
  for (int row = 0; row < detected_edges.rows; row++) {
    for (int col = 0; col < detected_edges.cols; col++) {
      uchar pixelGrayValue = detected_edges.at < uchar > (row, col);
      if (pixelGrayValue != NULL) {
        edgeValues.push_back(std::make_pair(row, col));
      }
    }
  }
  //The algorithm to order the points so that we can draw an appropriate picture.
  //Grab the first point to be the starting point
  std::vector < std::pair < int, int >> sortedEdgeValues;
  sortedEdgeValues.push_back(edgeValues[0]); //store the starting point
  std::vector < std::pair < int, int >> ::const_iterator current = , →(sortedEdgeValues.end() - 1); //save the value from the starting point
  edgeValues.erase(edgeValues.begin()); //remove the starting point from our list
  std::vector < std::pair < int, int >> ::const_iterator closest; //the next closest point
  double currentShortest = INT_MAX; //set the current shortest distance to a very large number
  //Loop through our vector of points until it is empty.
  while (edgeValues.size() != 0) {
    //loop through all of the points, or until an adjacent point is found
    for (iterator = edgeValues.begin();
      ((iterator != edgeValues.end()) && (currentShortest > , →1.0)); iterator++) {
      double newDistance = distance( * current, * iterator);
      if ((newDistance < currentShortest)) {
        currentShortest = newDistance; //set the value of the shortest distance found so
        , →far
        closest = iterator; //save the position of the closest coordinate
      }
    }
    sortedEdgeValues.push_back( * closest); //add the next closest coordinate to the sorted
    , →array
    current = (sortedEdgeValues.end() - 1);
    currentShortest = INT_MAX;
    edgeValues.erase(closest);
  }
  //output the sorted array values to a file
  std::ofstream sortedEdgeValuesFile("sortedEdgeValuesFile.txt");
  for (iterator = sortedEdgeValues.begin(); iterator != sortedEdgeValues.end(); iterator++) {
    sortedEdgeValuesFile << (iterator -> first) << ", " << -(iterator -> second) << std::endl;
  }
  sortedEdgeValuesFile.close();
  waitKey(0);
  return 0;
}