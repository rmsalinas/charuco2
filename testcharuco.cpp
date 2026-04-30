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
        cv::aruco::CharucoBoard2 board(cv::Size(2, 2),  dictionary);
        board.generateImage(200,outImg);
        cv::imshow("board",outImg);
        while(cv::waitKey(0)!=27) ;

        exit(0);
    }

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
    inputVideo.set(cv::CAP_PROP_POS_FRAMES, 10);


    //move 240 frames
    int f=0;
//   for(int i=0;i<380;i++) {inputVideo.grab();f++;}

    while (inputVideo.grab()) {
        std::cout<<"frame "<<f++<<std::endl;
        cv::Mat image;
        inputVideo.retrieve(image);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        std::vector<cv::Point2f> charucoCorners;
        std::vector<cv::Point3f> objPoints;
        std::vector<int> charucoIds;

        // void detectBoard(InputArray image, OutputArray charucoCorners, OutputArray charucoIds,
        //                  InputOutputArrayOfArrays markerCorners, InputOutputArray markerIds);



        // Detect markers and interpolate charuco corners
        detector.detectBoard(image, charucoCorners, charucoIds,  markerCorners, markerIds);
        //draw markers
        cv::aruco::drawDetectedMarkers(image, markerCorners, markerIds);
        cv::imshow("Detected Markers", image);
        cv::Mat im2=image.clone();
        cv::aruco::drawDetectedCornersCharuco(im2,charucoCorners, charucoIds);
        cv::imshow("charuco", im2);


        char key = (char)cv::waitKey(0);
        if(key==27) break;

    }

    return 0;
}
