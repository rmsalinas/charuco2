#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"
using namespace cv;

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


    if(0){
        // ---------- 1. Setup ----------
        aruco::Dictionary dictionary =
            aruco::getPredefinedDictionary(aruco::DICT_6X6_250);

        const float squareLength = 0.040f;   // 4 cm
        const float markerLength = 0.024f;   // 2.4 cm

        // Dos diamantes con conjuntos de ids DIFERENTES
        const std::vector<int> ids1 = {45, 68, 28, 74};
        const std::vector<int> ids2 = {10, 22, 33, 44};

        // Un CharucoBoard por diamante (sólo cambian los ids).
        aruco::CharucoBoard board1(Size(3, 3), squareLength, markerLength,
                                   dictionary, ids1);
        aruco::CharucoBoard board2(Size(3, 3), squareLength, markerLength,
                                   dictionary, ids2);

        // ---------- 2. Renderizar cada diamante por separado ----------
        const int diamondPx = 400;           // 400x400 px cada uno
        Mat diamond1Img, diamond2Img;
        board1.generateImage(Size(diamondPx, diamondPx), diamond1Img, 0, 1);
        //rotate the image 90deg
        cv::rotate(diamond1Img, diamond1Img, cv::ROTATE_90_CLOCKWISE);


        board2.generateImage(Size(diamondPx, diamondPx), diamond2Img, 0, 1);

        // ---------- 3. Componer ambos en un único lienzo blanco ----------
        const int margin  = 80;
        const int canvasW = 2 * diamondPx + 3 * margin;
        const int canvasH = diamondPx + 2 * margin;
        Mat canvas(canvasH, canvasW, CV_8UC1, Scalar(255));

        Rect roi1(margin,                 margin, diamondPx, diamondPx);
        Rect roi2(2 * margin + diamondPx, margin, diamondPx, diamondPx);
        diamond1Img.copyTo(canvas(roi1));
        diamond2Img.copyTo(canvas(roi2));

        imwrite("two_diamonds.png", canvas);
        std::cout << "[+] Guardado two_diamonds.png\n";

        // ---------- 4. Calibración de cámara (placeholder) ----------
        Mat camMatrix = (Mat_<double>(3, 3)<<
                         700.0,   0.0, canvasW * 0.5,
                         0.0, 700.0, canvasH * 0.5,
                         0.0,   0.0,           1.0);
        Mat distCoeffs = Mat::zeros(1, 5, CV_64F);

        // ---------- 5. Detectar AMBOS diamantes con un único detector ----------
        // detectDiamonds es geométrico: encuentra cualquier diamante formado por
        // 4 marcadores del diccionario, no le importan los ids concretos del board.
        aruco::CharucoDetector detector(board1);

        std::vector<std::vector<Point2f>> diamondCorners, markerCorners;
        std::vector<Vec4i> diamondIds;
        std::vector<int>   markerIds;

        detector.detectDiamonds(canvas,
                                diamondCorners, diamondIds,
                                markerCorners,  markerIds);

        std::cout << "[+] Marcadores detectados: " << markerIds.size()  << "\n";
        std::cout << "[+] Diamantes detectados:  " << diamondIds.size() << "\n";
        for (size_t i = 0; i < diamondIds.size(); ++i)
            std::cout << "    diamond[" << i << "] ids = " << diamondIds[i] << "\n";

        if (diamondIds.size() < 2) {
            std::cerr << "[!] Se esperaban 2 diamantes.\n";
            return -1;
        }

        // ---------- 6. Pose de cada diamante ----------
        Mat output;
        cvtColor(canvas, output, COLOR_GRAY2BGR);
        aruco::drawDetectedMarkers(output, markerCorners, markerIds);
        aruco::drawDetectedDiamonds(output, diamondCorners, diamondIds);

        // 3x3 board => 4 esquinas internas con ids {0,1,2,3}.
        const std::vector<int> charucoIds = {0, 1, 2, 3};

        for (size_t i = 0; i < diamondIds.size(); ++i) {
            // Cualquiera de los dos boards vale: matchImagePoints sólo usa el
            // squareLength para construir los puntos 3D de las esquinas internas.
            Mat objPoints, imgPoints;
            board1.matchImagePoints(diamondCorners[i], charucoIds,
                                    objPoints, imgPoints);

            std::cout<<"OBJPO="<<objPoints.size()<<"  IMGPO="<<imgPoints.size()<<"\n";
            Vec3d rvec, tvec;
            bool ok = solvePnP(objPoints, imgPoints,
                               camMatrix, distCoeffs,
                               rvec, tvec,
                               /*useExtrinsicGuess=*/false,
                               SOLVEPNP_IPPE);
            if (!ok) {
                std::cerr << "[!] solvePnP falló para el diamante " << i << "\n";
                continue;
            }

            std::cout << "Diamante " << i << "  ids=" << diamondIds[i] << "\n"
                      << "  rvec = " << rvec << "\n"
                      << "  tvec = " << tvec << "  (metros)\n";

            drawFrameAxes(output, camMatrix, distCoeffs,
                          rvec, tvec, squareLength * 1.5f, 2);
        }

        imwrite("two_diamonds_pose.png", output);
        std::cout << "[+] Guardado two_diamonds_pose.png\n";




        cv::imshow("Detected Diamond", output);
        while (cv::waitKey(0) != 27) ;


            exit(0);
    }

    if(1){
        cv::Mat outImg, outImg2;
        std::vector<int> ids1={1,2,3,4};
        std::vector<int> ids2={1,22,3,4};
        cv::aruco::CharucoBoard2 board(cv::Size(2, 2),  dictionary,ids1);
        board.generateImage(200,outImg);
        cv::aruco::CharucoBoard2 board2(cv::Size(2, 2),  dictionary,ids2);
        board2.generateImage(200,outImg2);

        //create one image with both
        cv::Mat canvas( outImg.rows, outImg.cols*2,CV_8UC1);
        outImg.copyTo(canvas(cv::Rect(0,0,outImg.cols,outImg.rows)));
        outImg2.copyTo(canvas(cv::Rect(outImg.cols,0,outImg2.cols,outImg2.rows)));


        cv::imshow("board",canvas);
        while(cv::waitKey(0)!=27) ;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        std::vector<cv::Point2f> diamonsCorners;
        std::vector<cv::Point3f> objPoints;
        std::vector<int> diamonsIds;
        detector.detectDiamonds(canvas, diamonsCorners, diamonsIds,  markerCorners, markerIds);
        while (cv::waitKey(0) != 27) ;

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
