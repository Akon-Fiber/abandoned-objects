//
//  world_object.cpp
//  abandoned-objects
//
//  Created by David Kelly on 19/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "world_object.hpp"
#include "utilities.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

/*** WorldObject ***/

WorldObject::WorldObject(){}

WorldObject::WorldObject(vector<Point> contour, int frameAppeared){
    this->contour = contour;
    this->frameAppeared = frameAppeared;
}

double WorldObject::getArea(){
    return contourArea(contour);
}

Point2f WorldObject::getCentre(){
    int sumX = 0, sumY = 0;
    int size = (int)contour.size();
    for(int i = 0; i < size; i++){
        sumX += contour[i].x;
        sumY += contour[i].y;
    }
    return Point2f(sumX / size, sumY / size);
}

Rect WorldObject::getRectRoi(){
    return boundingRect(contour);
}

bool WorldObject::checkOverlap(Rect region){
    Rect objectRegion = boundingRect(contour);
    Rect intersection = objectRegion & region;
    if(intersection.height != 0 && intersection.width != 0)
        return true;
    else return false;
}

bool WorldObject::checkAdjacency(cv::Rect region, int adjacencyDistance){
    region.x = region.x - adjacencyDistance;
    region.y = region.y - adjacencyDistance;
    region.height = region.height + (adjacencyDistance * 2);
    region.width = region.width + (adjacencyDistance * 2);
    return this->checkOverlap(region);
}

void WorldObject::updateRoiVector(){
    this->roiVector.push_back(this->getRectRoi());
}

vector<Rect> WorldObject::getRoiVector(){
    return this->roiVector;
}

cv::Mat WorldObject::getObjectImageRegion(){
    return this->objectImageRegion;
}

void WorldObject::setObjectImageRegion(cv::Mat objectImageRegion){
    this->objectImageRegion = objectImageRegion;
}

int WorldObject::getFrameAppeared(){
    return frameAppeared;
}

void WorldObject::setFrameAppeared(int frame){
    this->frameAppeared = frame;
}

int WorldObject::getFrameLargest(){
    return frameLargest;
}

void WorldObject::setFrameLargest(int frame){
    this->frameLargest = frame;
}

int WorldObject::getFrameDisappeared(){
    return frameDisappeared;
}

void WorldObject::setFrameDisappeared(int frame){
    this->frameDisappeared = frame;
}

vector<Point> WorldObject::getContour(){
    return contour;
}

void WorldObject::setContour(vector<Point> contour){
    this->contour = contour;
}

string WorldObject::toString(){
    stringstream str;
    str << "Top Left: " << this->getRectRoi().tl() << " Bottom Right: " << this->getRectRoi().br() << " Area: " << to_string(getArea());
    str << " FA: " << frameAppeared << " FL: " << frameLargest << " FD: " << frameDisappeared << " Type: " << type << endl;
    return str.str();
}

/*** WorldObject ***/

WorldObjectManager::WorldObjectManager(){}

WorldObjectManager::WorldObjectManager(Mat originalBackgroundImage){
    this->originalBackgroundImage = originalBackgroundImage;
}

void WorldObjectManager::update(std::vector<std::vector <cv::Point>> contours, Mat currentFrameImage, int referenceIndex){
    int numberCurrentObjects = (int)currentObjects.size();
    int numberContours = (int)contours.size();
    // check which objects have overlapping contours and which contours overlap on objects
    int objectMatchingContour[numberCurrentObjects]; // points to the index of the contour that matches the object
    bool contourMatchesObject[numberContours]; // indicates whether a contour overlaps any object
    fill_n(contourMatchesObject, numberContours, false);
    for(int i = 0; i < numberCurrentObjects; i++){
        objectMatchingContour[i] = -1;
        for(int j = 0; j < numberContours; j++)
            if(currentObjects[i].checkOverlap(boundingRect(contours[j]))){
                objectMatchingContour[i] = j;
                contourMatchesObject[j] = true;
            }
    }
    vector<int> removeObjects;
    // update the status of existing objects
    for(int i = 0; i < numberCurrentObjects; i++){
        if(objectMatchingContour[i] == -1){
            // mark object as gone
            currentObjects[i].status = OBJ_GONE;
            currentObjects[i].setFrameDisappeared(referenceIndex);
            currentObjects[i].updateRoiVector();
            processObject(currentObjects[i], currentFrameImage);
            removeObjects.push_back(i);
        }
        else{
            vector<Point> matchingContour = contours[objectMatchingContour[i]];
            if(currentObjects[i].getArea() > contourArea(matchingContour)){
                // object is getting smaller
                currentObjects[i].status = OBJ_SHRINKING;
            }else if(currentObjects[i].getArea() < contourArea(matchingContour)){
                // object is growing
                currentObjects[i].status = OBJ_GROWING;
                currentObjects[i].setContour(matchingContour);
                currentObjects[i].setFrameLargest(referenceIndex);
                currentObjects[i].setObjectImageRegion(currentFrameImage(increaseRectSize(currentObjects[i].getRectRoi(), EXPANDED_ROI_EDGE_DETECTION_DISTANCE)));
            }else{
                // object is staying the same size
                currentObjects[i].status = OBJ_SAME_SIZE;
            }
        }
    }
    // remove objects that disappeared
    removeCurrentObjects(removeObjects);
    // remove objects that merged
    pruneCurrentObjects();
    // merge any objects that appear adjacent to each other
    mergeAdjacentCurrentObjects(currentFrameImage);
    // create new objects for contours that didn't overlap an existing object
    for(int i = 0; i < numberContours; i++){
        if(!contourMatchesObject[i]){
            currentObjects.push_back(*new WorldObject(contours[i], referenceIndex));
        }
    }
    // update the area vector of each world object
    for(int i = 0; i < (int)currentObjects.size(); i++){
        currentObjects[i].updateRoiVector();
    }
}

void WorldObjectManager::removeCurrentObjects(vector<int> indices){
    sort(indices.begin(), indices.end());
    for(int i = (int)indices.size() - 1; i >= 0; i--){
        currentObjects.erase(currentObjects.begin() + indices[i]);
    }
}

void WorldObjectManager::pruneCurrentObjects(){
    int numberCurrentObjects = (int)currentObjects.size();
    for(int i = 0; i < numberCurrentObjects - 1; i++){
        vector<int> mergedObjectIndices;
        mergedObjectIndices.push_back(i);
        int earliestAppearedIndex = (int)mergedObjectIndices.size() - 1;
        for(int j = i + 1; j < numberCurrentObjects; j++){
            if(currentObjects[i].getCentre() == currentObjects[j].getCentre()){
                mergedObjectIndices.push_back(j);
                if(currentObjects[j].getFrameAppeared() <= currentObjects[mergedObjectIndices[earliestAppearedIndex]].getFrameAppeared()){
                    earliestAppearedIndex = (int)mergedObjectIndices.size() - 1;
                }
            }
        }
        mergedObjectIndices.erase(mergedObjectIndices.begin() + earliestAppearedIndex);
        removeCurrentObjects(mergedObjectIndices);
        numberCurrentObjects = (int)currentObjects.size();
    }
}

void WorldObjectManager::mergeAdjacentCurrentObjects(Mat currentFrameImage){
    int numberCurrentObjects = (int)currentObjects.size();
    for(int i = 0; i < numberCurrentObjects; i++){
        vector<int> mergeObjectIndices; // vector to keep track of all the objects that are to be merged
        mergeObjectIndices.push_back(i);
        int mergeListFirstAppearedIndex = 0;
        for(int j = i + 1; j < numberCurrentObjects; j++){
            // check if the current object i is adjacent to the current object j
            if(currentObjects[i].checkAdjacency(currentObjects[j].getRectRoi())){
                mergeObjectIndices.push_back(j);
                if(currentObjects[j].getFrameAppeared() < currentObjects[mergeObjectIndices[mergeListFirstAppearedIndex]].getFrameAppeared()){
                    mergeListFirstAppearedIndex = (int)mergeObjectIndices.size() - 1;
                }
            }
        }
        // check if there are objects to merge before merging
        if((int)mergeObjectIndices.size() > 1){
            vector<Point> points; // a vector of all the points to merge into a convex shape
            for(int i = 0; i < (int)mergeObjectIndices.size(); i++){
                // get convex hull of each object to be merged
                vector<Point> convexContourPoints;
                convexHull(currentObjects[i].getContour(), convexContourPoints, true);
                points.insert(points.end(), convexContourPoints.begin(), convexContourPoints.end());
            }
            // merge adjacent contours into a new convex polygon contour
            vector<Point> newContour;
            convexHull(Mat(points), newContour, true);
            // update the first appeared object with a new contour and remove the merges objects
            WorldObject *mergedObject = &currentObjects[mergeObjectIndices[mergeListFirstAppearedIndex]];
            mergedObject->setContour(newContour);
            // updates the object image
            mergedObject->setObjectImageRegion(currentFrameImage(increaseRectSize(mergedObject->getRectRoi(), EXPANDED_ROI_EDGE_DETECTION_DISTANCE)));
            mergeObjectIndices.erase(mergeObjectIndices.begin() + mergeListFirstAppearedIndex);
            removeCurrentObjects(mergeObjectIndices);
            numberCurrentObjects = (int)currentObjects.size();
        }
    }
}

void WorldObjectManager::processObject(WorldObject object, Mat currentFrameImage){
    Mat originalImageRegion = originalBackgroundImage(increaseRectSize(object.getRectRoi(), EXPANDED_ROI_EDGE_DETECTION_DISTANCE));
    Mat objectImageRegion = object.getObjectImageRegion();
    cvtColor(originalImageRegion, originalImageRegion, CV_BGR2GRAY);
//    imshow("Original Grayscale", originalImageRegion);
    cvtColor(objectImageRegion, objectImageRegion, CV_BGR2GRAY);
//    imshow("Object GrayScale", objectImageRegion);
    Canny(originalImageRegion, originalImageRegion, EDGE_DETECTION_LOW_THRESHOLD, EDGE_DETECTION_HIGH_THRESHOLD);
//    imshow("Original Edges", originalImageRegion);
    Canny(objectImageRegion, objectImageRegion, EDGE_DETECTION_LOW_THRESHOLD, EDGE_DETECTION_HIGH_THRESHOLD);
//    imshow("Object Edges", objectImageRegion);
//    cvWaitKey(0);
//    cvDestroyAllWindows();
    // get connected components first image
    vector<vector <Point>> contoursOriginalImage;
    vector<Vec4i> hierarchyOriginalImage;
    findContours(originalImageRegion, contoursOriginalImage, hierarchyOriginalImage, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    // get connected components second image
    vector<vector <Point>> contoursObjectImage;
    vector<Vec4i> hierarchyObjectImage;
    findContours(objectImageRegion, contoursObjectImage, hierarchyObjectImage, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    int originalImageEdges = (int)contoursOriginalImage.size();
    int objectImageEdges = (int)contoursObjectImage.size();
    if(originalImageEdges > objectImageEdges){
        object.type = OBJ_REMOVED;
    }else{
        object.type = OBJ_ABANDONED;
    }
    // updates the world object manager background image after the object becomes part of the background
    processedObjects.push_back(object);
    this->originalBackgroundImage.release();
    this->originalBackgroundImage = currentFrameImage;
}

void WorldObjectManager::drawCurrentObjectRegions(cv::Mat &image){
    for(int i = 0; i < (int)currentObjects.size(); i++){
        Rect roi = currentObjects[i].getRectRoi();
        rectangle(image, roi, Scalar(0, 0, 255));
    }
}

vector<WorldObject> WorldObjectManager::getCurrentObjects(){
    return currentObjects;
}

vector<WorldObject> WorldObjectManager::getProcessedObjects(){
    return processedObjects;
}

string WorldObjectManager::currentObjectsToString(){
    stringstream str;
    if((int)currentObjects.size() != 0){
        str << "Current Objects: " << endl;
        for(int i = 0; i < (int)currentObjects.size(); i++){
            str << currentObjects[i].toString();
        }
    }
    return str.str();
}

std::string WorldObjectManager::processedObjectsToString(){
    stringstream str;
    if((int) processedObjects.size()){
        str << "Processed Objects: " << endl;
        for(int i = 0; i < (int)processedObjects.size(); i++){
            str << processedObjects[i].toString();
        }
    }
    return str.str();
}

void WorldObjectManager::setOriginalBackgroundImage(cv::Mat originalBackgroundImage){
    this->originalBackgroundImage = originalBackgroundImage;
}
