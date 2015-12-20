//
//  world_object.hpp
//  abandoned-objects
//
//  Created by David Kelly on 19/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef world_object_hpp
#define world_object_hpp

#include <stdio.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

enum WO_TYPE {OBJ_ABANDONED, OBJ_REMOVED, OBJ_UNKNOWN};
enum WO_STATUS {OBJ_GROWING, OBJ_SHRINKING, OBJ_SAME_SIZE, OBJ_GONE};

// class that represents a world object as it appears and disappears from a video sequence
class WorldObject{
private:
    std::vector<cv::Point> contour;
    int frameAppeared = 0;
    int frameLargest = 0;
    int frameDisappeared = 0;
public:
    WO_TYPE type = OBJ_UNKNOWN;
    WO_STATUS status = OBJ_SAME_SIZE;
    
    // default constructor
    WorldObject();
    
    // other constructor
    WorldObject(std::vector<cv::Point> contour, int frameAppeared=0);
    
    // returns object size
    double getArea();
    
    cv::Point2f getCentre();
    
    // returns a rect surrounding the object region
    cv::Rect getRectRoi();
    
    bool checkOverlap(cv::Rect region);
    
    // get and set
    
    int getFrameAppeared();
    
    void setFrameAppeared(int frame);
    
    int getFrameLargest();
    
    void setFrameLargest(int frame);
    
    int getFrameDisappeared();
    
    void setFrameDisappeared(int frame);
    
    std::vector<cv::Point> getContour();
    
    void setContour(std::vector<cv::Point> contour);
};

class WorldObjectManager{
private:
    std::vector<WorldObject> currentObjects;
    std::vector<WorldObject> processedObjects;
    
    // removes elements from currentObjects at the indices passed into the function
    void removeCurrentObjects(std::vector<int> indices, bool moveToProcessed=true);
    
    // removes extra merged objects. Keeps objects which occured first
    void pruneCurrentObjects();
public:
    WorldObjectManager();
    
    // updates all the world objects
    // It will add remove disapeared objects, add new objects, merge objects
    // It will also update whether an object is growing or shrinking
    void update(std::vector<std::vector <cv::Point>> contours, int referenceIndex);
    
    void drawCurrentObjectRegions(cv::Mat &image);
    
    std::vector<WorldObject> getCurrentObjects();
    
    std::vector<WorldObject> getProcessedObjects();
};

#endif /* world_object_hpp */
