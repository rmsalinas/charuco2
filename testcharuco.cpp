#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"



int main() {
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;

    // Basic ChArUco board setup
    int squaresX = 4;
    int squaresY = 6;
    float squareLength = 0.04f;
    float markerLength = 0.02f;
    cv::aruco::PredefinedDictionaryType dictionaryId = cv::aruco::DICT_ARUCO_MIP_36h12;

    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(dictionaryId);
    cv::aruco::CharucoBoard2 board(cv::Size(squaresX, squaresY),  dictionary);
    cv::aruco::CharucoDetector2 detector(board);

    if(0){
        cv::Mat outImg;
        board.generateImage(200,outImg);
        cv::imshow("board",outImg);
        //save to board.jpg
        cv::imwrite("board.jpg", outImg);


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
