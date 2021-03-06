#include "occupancy_grid.h"
#include "occupancy_grid.inl" 
#include "robot_configuration.h" // Data structures and configuration data shared with Arduino controller
#include "error_handling.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <boost/range/size.hpp>
#include <boost/range/iterator_range.hpp>
#include <opencv2/imgproc.hpp>

COccupancyGrid::COccupancyGrid()
:   m_matnMapObstacle(c_nMapExtent, c_nMapExtent, CV_8UC1, cv::Scalar(128))
{}

COccupancyGrid::COccupancyGrid(COccupancyGrid const& occgrid) 
:   COccupancyGridBaseT(occgrid), m_matnMapObstacle(occgrid.m_matnMapObstacle.clone())
{}

COccupancyGrid& COccupancyGrid::operator=(COccupancyGrid const& occgrid) {
    COccupancyGridBaseT::operator=(occgrid); 
    m_matnMapObstacle=occgrid.m_matnMapObstacle.clone();
    return *this;
}

void COccupancyGrid::updateGrid(rbt::point<int> const& pt, double fOdds) {
    // Calculating the greyscale map is pretty expensive
    // If we ever need a non-binary version, a lookup table
    // would be useful instead of this:
    // auto const nColor = rbt::numeric_cast<std::uint8_t>(1.0 / ( 1.0 + std::exp( fOdds )) * 255);
    assert(is_inside(pt));
    m_matnMapObstacle.at<std::uint8_t>(pt.y, pt.x) = 0 < fOdds ? 0 : 255;
}

void COccupancyGrid::updateGridPoly(std::vector<rbt::point<int>> const& rngpt, double fOdds) {
    std::vector<cv::Point> vecpt(boost::begin(rngpt), boost::end(rngpt));
    cv::fillConvexPoly(m_matnMapObstacle, vecpt.data(), vecpt.size(), 0 < fOdds ? 0 : 255);
}

cv::Mat ObstacleMapWithPoses(cv::Mat const& m, std::vector<rbt::pose<double>> const& vecpose) {
    rbt::point<int> ptnPrev = ToGridCoordinate(vecpose.front().m_pt);
    boost::for_each(vecpose, [&](rbt::pose<double> const& pose) {
        auto const ptnGrid = ToGridCoordinate(pose.m_pt);
        cv::line(m, ptnPrev, ptnGrid, cv::Scalar(0));
        ptnPrev = ptnGrid;
    });
    return m;
}

std::vector<rbt::point<int>> RenderRobotPose(cv::Mat& mat, rbt::pose<double> const& pose, cv::Scalar color) {
    rbt::size<double> const szfHalfSize(c_nRobotWidth/2.0, c_nRobotHeight/2.0);
    std::vector<rbt::point<int>> vecpt = {
        ToGridCoordinate(pose.m_pt - szfHalfSize.rotated(pose.m_fYaw)),
        ToGridCoordinate(pose.m_pt + rbt::size<double>(szfHalfSize.x, -szfHalfSize.y).rotated(pose.m_fYaw)),
        ToGridCoordinate(pose.m_pt + szfHalfSize.rotated(pose.m_fYaw)),
        ToGridCoordinate(pose.m_pt + rbt::size<double>(-szfHalfSize.x, szfHalfSize.y).rotated(pose.m_fYaw))
    };
    cv::fillConvexPoly(mat, reinterpret_cast<cv::Point*>(vecpt.data()), vecpt.size(), color);
    return vecpt;
}

cv::Mat COccupancyGrid::ObstacleMapWithPoses(std::vector<rbt::pose<double>> const& vecpose) const {
    return ::ObstacleMapWithPoses(ObstacleMap(), vecpose);
}