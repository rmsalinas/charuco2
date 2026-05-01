#pragma once
#include <opencv2/objdetect/aruco_detector.hpp>
namespace cv::aruco{

class CharucoBoard2{
public:
    cv::Size bSize={-1,-1};
    Dictionary  dictionary;
    std::vector<int> ids;
    float markerLength, markerSeparation;
    CharucoBoard2();
    /**
     * @brief CharucoBoard2
     * @param bSize
     * @param markerLength
     * @param markerSeparation ignored
     * @param dictionary
     * @param ids
     */
    CharucoBoard2(cv::Size bSize, float markerLength, float markerSeparation,const Dictionary &dictionary, InputArray ids = noArray());
    void generateImage(float markerSizePix, Mat& outImage) const;
    //returns the row,col of a given marker id
    std::pair<int,int> getIdPos(int id)const;
    //returns the id at the row,col indicated
    int getId(int row,int col)const;

    /** @brief Given a board configuration and a set of detected markers, returns the corresponding
     * image points and object points, can be used in solvePnP()
     *
     * @param detectedCorners List of detected marker corners of the board.
     * For cv::Board and cv::GridBoard the method expects std::vector<std::vector<Point2f>> or std::vector<Mat> with Aruco marker corners.
     * For cv::CharucoBoard the method expects std::vector<Point2f> or Mat with ChAruco corners (chess board corners matched with Aruco markers).
     *
     * @param detectedIds List of identifiers for each marker or charuco corner.
     * For any Board class the method expects std::vector<int> or Mat.
     *
     * @param objPoints Vector of marker points in the board coordinate space.
     * For any Board class the method expects std::vector<cv::Point3f> objectPoints or cv::Mat
     *
     * @param imgPoints Vector of marker points in the image coordinate space.
     * For any Board class the method expects std::vector<cv::Point2f> objectPoints or cv::Mat
     *
     * @sa solvePnP
     */
    void matchImagePoints(InputArrayOfArrays detectedCorners, InputArray detectedIds,
                                  OutputArray objPoints, OutputArray imgPoints) const;

};

class     CharucoDetector2{
    CharucoBoard2 board;
    cv::aruco::ArucoDetector Adetector;
public:
    CharucoDetector2(const CharucoBoard2& board);
    //void detectBoard(InputArray image, OutputArray imgPoints,  OutputArray objPoints,OutputArray markerIds) const;

    void detectBoard(InputArray image, OutputArray charucoCorners, OutputArray charucoIds,
                     InputOutputArrayOfArrays markerCorners, InputOutputArray markerIds);
    void detectDiamonds(InputArray image, OutputArrayOfArrays _diamondCorners, OutputArray _diamondIds,
                        InputOutputArrayOfArrays inMarkerCorners, InputOutputArray inMarkerIds) ;
private:
    //given a marker id and one of its corners, return the global corner id of that corner, which is a unique id for that corner in the whole board,
    int getGlobalCornerID(int marker_id,int corner_id,const CharucoBoard2 &board) const;
    //opposite of getGlobalCornerID, given a global corner id, return the marker ids and corner ids of that corner
    std::vector<std::pair<int,int>>  getMarkerCornersFromGlobalCornerID( int gid,const CharucoBoard2 &board)const;

};

}
