#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"
#include <filesystem>
using namespace cv;


int main(int argc,char **argv) {
    try{
        if(argc<2){
            std::cerr<<"Usage: "<<argv[0]<<" <video/image>\n";
            return 1;
        }


        // Basic ChArUco board setup
        cv::Size boardSize(9, 5); // Number of squares in X and Y direction
        cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_MIP_36h12);
        cv::aruco::CharucoBoard2 board(boardSize, 1,1, dictionary);
        cv::aruco::CharucoDetector2 detector(board);

        //detect the board and draw the corners
        cv::VideoCapture inputVideo;
        cv::Mat image;
        //if extension is video, then open it
        if(std::filesystem::path(argv[1]).extension()==".mp4"){
            inputVideo.open(argv[1]);
        }
        else{//open the image
            image=cv::imread(argv[1]);
        }

        bool done=false;

        while (!done) {
            if(inputVideo.isOpened()){
                if(!inputVideo.grab())break;
                inputVideo.retrieve(image);
            }
            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners;
            std::vector<cv::Point2f> charucoCorners;
            std::vector<cv::Point3f> objPoints;
            std::vector<int> charucoIds;
            // Detect markers and interpolate charuco corners
            detector.detectBoard(image, charucoCorners, charucoIds,  markerCorners, markerIds);
            //draw markers
            cv::aruco::drawDetectedMarkers(image, markerCorners, markerIds);
            cv::aruco::drawDetectedCornersCharuco(image,charucoCorners, charucoIds);
            cv::imshow("charuco", image);
            done= ((char)cv::waitKey(0)==27);
        }


    }catch(std::exception &ex){
        std::cerr<<"Error: "<<ex.what()<<"\n";
    }
    return 0;
}