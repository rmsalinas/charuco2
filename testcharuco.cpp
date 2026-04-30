#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"



int getGlobalCornerID(cv::aruco::CharucoBoard2 board,int marker_id, int corner_id)
{
    //obtain the row, col of the marker_id
    auto row_col=board.getIdPos(marker_id);
    if(corner_id<=1){
        return (board.bSize.width+1) *row_col.first +  row_col.second+  corner_id;
    }
    else if(corner_id==2){
        return (board.bSize.width+1) *(row_col.first+1) +  row_col.second+ 1;
    }
    else  {
        return (board.bSize.width+1) *(row_col.first+1) +  row_col.second;

    }
}
//opposite of getGlobalCornerID, given a global corner id, return the marker ids and corner ids of that corner

std::vector<std::pair<int,int>>  getMarkerCornersFromGlobalCornerID( cv::aruco::CharucoBoard2 board,int gid)
{
    std::vector<std::pair<int,int>> result;
    const int W = board.bSize.width;

    // Decompose gid into (cr, cc) in the (H+1) x (W+1) corner grid.
    // Inverse of  gid = (W+1) * cr + cc  used in getGlobalCornerID.
    const int cr = gid / (W + 1);
    const int cc = gid % (W + 1);

    // Up to 4 markers share a global corner. Sort order: 0=TL, 1=TR, 2=BR, 3=BL.
    // board.getId() returns -1 for out-of-range (row,col), so it doubles as a bounds check.
    int id;

    // Marker at (cr, cc) sees this point as its top-left (0)
    id = board.getId(cr,     cc);
    if (id != -1) result.emplace_back(id, 0);

    // Marker at (cr, cc-1) sees this point as its top-right (1)
    id = board.getId(cr,     cc - 1);
    if (id != -1) result.emplace_back(id, 1);

    // Marker at (cr-1, cc-1) sees this point as its bottom-right (2)
    id = board.getId(cr - 1, cc - 1);
    if (id != -1) result.emplace_back(id, 2);

    // Marker at (cr-1, cc) sees this point as its bottom-left (3)
    id = board.getId(cr - 1, cc);
    if (id != -1) result.emplace_back(id, 3);

    return result;
}
int main() {
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;

    // Basic ChArUco board setup
    int squaresX = 4;
    int squaresY = 6;
    float squareLength = 0.04f;
    float markerLength = 0.02f;
    cv::aruco::PredefinedDictionaryType dictionaryId = cv::aruco::DICT_ARUCO_MIP_36h12;

    std::vector<int> ids;
    int markerId=1;
    for(int y=0;y<squaresY;y++){
        for(int x= 0;x<squaresX;x++,markerId++){
            ids.push_back(markerId);
        }
    }

    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(dictionaryId);
    cv::aruco::CharucoBoard2 board(cv::Size(squaresX, squaresY),  dictionary,ids);
    cv::aruco::CharucoDetector2 detector(board);

    if(1){
        cv::Mat outImg;
        board.generateImage(200,outImg);
        cv::imshow("board",outImg);
        //save to board.jpg
        cv::imwrite("board.jpg", outImg);

        // for(auto id:board.ids){
        //     auto row_col=board.getIdPos(id);
        //     std::cout<<"Marker "<<id<<" pos="<<row_col.first<<" "<<  row_col.second<< " --> "<<board.getId(row_col.first,row_col.second)<<std::endl;
        //     for(int c=0;c<4;c++){
        //         std::cout<<"Marker "<<id<<" corner "<<c<<" has global corner id "<< getGlobalCornerID(board,id,c)<<std::endl;
        //         auto gid=getGlobalCornerID(board,id,c);
        //     }
        // }
        int idx=0;
        for(int y=0;y<board.bSize.height;y++){
            for(int x=0;x<board.bSize.width;x++,idx++){
                if( board.getIdPos(board.ids[idx]) != std::make_pair(y,x))
                {
                    std::cout<<"[ERROR: marker "<<board.ids[idx]<<" is not at the expected position "<<y<<" "<<x<<" ]";
                }
            }
        }

        for(int y=0;y<board.bSize.height;y++){
            for(int x=0;x<board.bSize.width+1;x++){
                    int gid= (y*(board.bSize.width+1)+x);
                    std::cout<<"Global corner id "<<gid<<" is shared by marker corners: ";
                    auto marker_corners=getMarkerCornersFromGlobalCornerID(board,gid);
                    for(auto mc:marker_corners){
                        std::cout<<"(marker id "<<mc.first<<" corner "<<mc.second<<") " ;
                        if(getGlobalCornerID(board,mc.first,mc.second)!=gid){
                            std::cout<<"[ERROR: getGlobalCornerID(board,"<<mc.first<<","<<mc.second<<") should return "<<gid<<" but returned "<<getGlobalCornerID(board,mc.first,mc.second)<<"]";

                        }
                    }
                    std::cout<<std::endl;
                }
            }

         while(cv::waitKey(0)!=27) ;

    }


    cv::VideoCapture inputVideo;
    inputVideo.open("/home/salinas/Descargas/board2.mp4");

    // if (!inputVideo.isOpened()) {
    //     std::cerr << "Could not open video capture device 0." << std::endl;
    //     return -1;
    // }

    // std::cout << "Press 'q' to exit, 's' to capture a frame for calibration, 'c' to run calibration." << std::endl;

    // std::vector<std::vector<cv::Point2f>> allImgPoints;
    // std::vector<std::vector<cv::Point3f>> allObjPoints;
    // cv::Size imageSize;

    //move 240 frames
    int f=0;
  //for(int i=0;i<285;i++) {inputVideo.grab();f++;}

    while (inputVideo.grab()) {
        std::cout<<"frame "<<f++<<std::endl;
        cv::Mat image;
        inputVideo.retrieve(image);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        std::vector<cv::Point2f> charucoCorners;
        std::vector<cv::Point3f> objPoints;
        std::vector<int> charucoIds;

        // Detect markers and interpolate charuco corners
        detector.detectBoard(image, markerCorners, charucoIds,   markerIds);
        //draw markers
        cv::aruco::drawDetectedMarkers(image, markerCorners, markerIds);
        cv::imshow("Detected Markers", image);
        char key = (char)cv::waitKey(0);
        if(key==27) break;

    }

    return 0;
}
