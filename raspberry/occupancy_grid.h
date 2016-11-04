#pragma once

#include "rover.h"
#include "nonmoveable.h"
#include "geometry.h"

#include <opencv2/core.hpp>

// An implementation of an occupancy grid, as described e.g. 
// in Thrun et al, "Probabilistic Robotics"
struct COccupancyGrid {
    COccupancyGrid();        
    COccupancyGrid(COccupancyGrid const& occgrid);
    COccupancyGrid& operator=(COccupancyGrid const& occgrid);

    // Update the occupancy grid. 'pose' is the robot's pose. 
    // The obstacle is assumed to be a pixel at polar coordinates (fAngle, nDistance)
    // in robot's frame of reference.
    void update(rbt::pose<double> const& pose, double fAngle, int nDistance);

    // Update the occupancy grid with a sequence of points. The obstacle points
    // are in world coordinates
    void update(rbt::pose<double> const& pose, std::vector<rbt::point<double>> const& vecptf);

    cv::Mat const& LogOddsMap() const { return m_matfMapLogOdds; }
    cv::Mat const& ObstacleMap() const { return m_matnMapObstacle; }
    
    cv::Mat ObstacleMapWithPoses(std::vector<rbt::pose<double>> const& vecpose) const;
private:
    void internalUpdatePerObstacle(rbt::point<double> const& ptf, rbt::point<double> const& ptfObstacle);
    void internalUpdatePerPose(rbt::pose<double> const& pose);

    cv::Mat m_matfMapLogOdds;
    cv::Mat m_matnMapObstacle; // thresholded version of m_matfMapLogOdds
};
