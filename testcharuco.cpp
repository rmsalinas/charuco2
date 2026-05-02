#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"
using namespace cv;
void drawDetectedCornersCharuco(InputOutputArray _image, InputArray _charucoCorners,
                                InputArray _charucoIds, Scalar cornerColor) {
    CV_Assert(!_image.getMat().empty() &&
              (_image.getMat().channels() == 1 || _image.getMat().channels() == 3));
    CV_Assert((_charucoCorners.total() == _charucoIds.total()) ||
              _charucoIds.total() == 0);
    CV_Assert(_charucoCorners.channels() == 2);

    Mat charucoCorners = _charucoCorners.getMat();
    if (charucoCorners.type() != CV_32SC2)
        charucoCorners.convertTo(charucoCorners, CV_32SC2);
    Mat charucoIds;
    if (!_charucoIds.empty())
        charucoIds = _charucoIds.getMat();
    size_t nCorners = charucoCorners.total();

    for(size_t i = 0; i < nCorners; i++) {
        Point corner = charucoCorners.at<Point>((int)i);

        // draw first corner mark
        rectangle(_image, corner - Point(3, 3), corner + Point(3, 3), cornerColor, 1, LINE_AA);

        // draw ID
        if(!_charucoIds.empty()) {
            int id = charucoIds.at<int>((int)i);
            std::stringstream s;
            s << "id=" << id;

            // Variables de diseño ajustadas
            double fontScale = 0.75; // 50% más grande que 0.5
            Point offset(-35, -15);  // Movido un poco más arriba y a la izquierda para evitar solapamiento

            // 1. Contorno negro más grueso (grosor 4)
            putText(_image, s.str(), corner + offset, FONT_HERSHEY_SIMPLEX, fontScale,
                    Scalar(0, 0, 0), 4, LINE_AA);

            // 2. Texto principal (grosor 2)
            putText(_image, s.str(), corner + offset, FONT_HERSHEY_SIMPLEX, fontScale,
                    cornerColor, 2, LINE_AA);
        }
    }
}

void drawDetectedDiamonds(InputOutputArray _image, InputArrayOfArrays _corners, InputArray _ids=cv::noArray(), Scalar borderColor = cv::Scalar(0, 0, 255)) {
    CV_Assert(_image.getMat().total() != 0 &&
              (_image.getMat().channels() == 1 || _image.getMat().channels() == 3));
    CV_Assert((_corners.total() == _ids.total()) || _ids.total() == 0);

    // calculate colors
    Scalar textColor, cornerColor;
    textColor = cornerColor = borderColor;
    swap(textColor.val[0], textColor.val[1]);     // text color just sawp G and R
    swap(cornerColor.val[1], cornerColor.val[2]); // corner color just sawp G and B

    int nMarkers = (int)_corners.total();
    for(int i = 0; i < nMarkers; i++) {
        Mat currentMarker = _corners.getMat(i);
        CV_Assert( (currentMarker.total() == 4 || currentMarker.total() == 9 ) && currentMarker.channels() == 2);
        if (currentMarker.type() != CV_32SC2)
            currentMarker.convertTo(currentMarker, CV_32SC2);

        // draw marker sides
        if(currentMarker.total() == 4){
            for(size_t j = 0; j < currentMarker.total(); j++) {
                Point p0, p1;
                p0 = currentMarker.at<Point>(j);
                p1 = currentMarker.at<Point>((j + 1) % currentMarker.total());
                line(_image, p0, p1, borderColor, 1);
            }
        }
        else{//9 corners, draw the inner square
            line(_image, currentMarker.at<Point>(0), currentMarker.at<Point>(2), borderColor, 1);
            line(_image, currentMarker.at<Point>(2), currentMarker.at<Point>(8), borderColor, 1);
            line(_image, currentMarker.at<Point>(8), currentMarker.at<Point>(6), borderColor, 1);
            line(_image, currentMarker.at<Point>(6), currentMarker.at<Point>(0), borderColor, 1);
        }

        // draw first corner mark
        rectangle(_image, currentMarker.at<Point>(0) - Point(3, 3),
                  currentMarker.at<Point>(0) + Point(3, 3), cornerColor, 1, LINE_AA);

        // draw id composed by four numbers
        if(_ids.total() != 0) {
            Point cent(0, 0);
            for(size_t p = 0; p < currentMarker.total(); p++)
                cent += currentMarker.at<Point>(p);
            cent = cent / float(currentMarker.total());
            std::stringstream s;
            s << "id=" << _ids.getMat().at< Vec4i >(i);
            putText(_image, s.str(), cent, FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);
        }
    }
}

int main() {
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;

    // Basic ChArUco board setup
    int squaresX = 9;
    int squaresY = 5;
    cv::aruco::PredefinedDictionaryType dictionaryId = cv::aruco::DICT_ARUCO_MIP_36h12;

    std::vector<int> ids;
    int markerId=1;
    for(int y=0;y<squaresY;y++){
        for(int x= 0;x<squaresX;x++,markerId++){
            ids.push_back(markerId);
        }
    }

    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(dictionaryId);
    cv::aruco::CharucoBoard2 board(cv::Size(squaresX, squaresY), 1,1, dictionary,ids);
    cv::aruco::CharucoDetector2 detector(board);

    if(1){
        cv::aruco::CharucoBoard board2(cv::Size(3, 3), 0.1,0.05, dictionary);
        cv::Mat outImg;
        board2.generateImage(cv::Size(400,400), outImg);

        //detect the board and draw the corners
        std::vector<int> markerIds;
        std::vector<std::vector<Point2f>> markerCorners;
        std::vector<Point2f> charucoCorners;
        std::vector<int> charucoIds;
        cv::aruco::CharucoDetector detector2(board2);
        detector2.detectBoard(outImg, charucoCorners, charucoIds,  markerCorners, markerIds);
        cv::cvtColor(outImg,outImg,cv::COLOR_GRAY2BGR);
//        cv::aruco::drawDetectedMarkers(outImg, markerCorners, markerIds);
        for(auto p:charucoCorners)
            cv::circle(outImg,p,10,cv::Scalar(0,255,0),3);
        drawDetectedCornersCharuco(outImg, charucoCorners, charucoIds,cv::Scalar(0,255,0));
        //draw a circle around each charucoCorner
         cv::imshow("board1corners",outImg);

        //save to file
        cv::imwrite("diamon_standard.jpg", outImg);
         while(cv::waitKey(0)!=27) ;

    }

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
        drawDetectedDiamonds(output, diamondCorners, diamondIds);

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
        while (cv::waitKey(0) != 27){}


            exit(0);
    }

    if(0){
        cv::Mat outImg, outImg2;
        std::vector<int> ids1={1,2,3,4};
        std::vector<int> ids2={1,22,3,4};
        cv::aruco::CharucoBoard2 board(cv::Size(2, 2),1,1,  dictionary,ids1);
        board.generateImage(200,outImg);
        cv::aruco::CharucoBoard2 board2(cv::Size(2, 2), 1,1, dictionary,ids2);
        board2.generateImage(200,outImg2);

        //create one image with both
        cv::Mat canvas( outImg.rows, outImg.cols*2,CV_8UC1);
        outImg.copyTo(canvas(cv::Rect(0,0,outImg.cols,outImg.rows)));
        outImg2.copyTo(canvas(cv::Rect(outImg.cols,0,outImg2.cols,outImg2.rows)));
        //sav to file
        cv::imwrite("two_diamonds.png", canvas);
        cv::imshow("board",canvas);  while(cv::waitKey(0)!=27) ;


        std::vector<int> markerIds;
        std::vector<std::vector<Point2f>> diamondCorners, markerCorners;

        std::vector<cv::Point3f> objPoints;
        std::vector<Vec4i> diamonsIds;
        detector.detectDiamonds(canvas, diamondCorners, diamonsIds,  markerCorners, markerIds);

        for(auto dd:diamondCorners){
            std::cout<<"diamond corners "<<dd.size()<<"\n";
            for(auto p:dd)
                std::cout<<"  "<<p<<"\n";
        }
        cv::cvtColor(canvas,canvas,cv::COLOR_GRAY2BGR);
        aruco::drawDetectedMarkers(canvas, markerCorners, markerIds);
        drawDetectedDiamonds(canvas, diamondCorners, diamonsIds);


        cv::imshow("board",canvas);  while(cv::waitKey(0)!=27) ;

        exit(0);
    }

    if(1){
        cv::Mat outImg;
        board.generateImage(cv::Size(1800,1200),outImg);
        cv::imshow("board",outImg);
        //save to board.jpg
        //detec and draw
        std::vector<int> markerIds;
        std::vector<std::vector<Point2f>> markerCorners;
        std::vector<Point2f> charucoCorners;
        std::vector<int> charucoIds;
        detector.detectBoard(outImg, charucoCorners, charucoIds,  markerCorners, markerIds);
        cv::cvtColor(outImg,outImg,cv::COLOR_GRAY2BGR);
//        cv::aruco::drawDetectedMarkers(outImg, markerCorners, markerIds);
        for(auto p:charucoCorners)
            cv::circle(outImg,p,10,cv::Scalar(0,255,0),3);
        drawDetectedCornersCharuco(outImg, charucoCorners, charucoIds,cv::Scalar(0,255,0));
        cv::imshow("boardcorners",outImg);


         while(cv::waitKey(0)!=27) ;
         cv::imwrite("diamon_charuco2.jpg", outImg);

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
        for(auto m:markerCorners)
            for(auto p:m)
                std::cout<<"  "<<p<<"\n";


        cv::imshow("Detected Markers", image);
        cv::Mat im2=image.clone();
        cv::aruco::drawDetectedCornersCharuco(im2,charucoCorners, charucoIds);
        std::vector<cv::Point2f> cimgPoints;
        std::vector<cv::Point3f> cobjPoints;

        board.matchImagePoints(charucoCorners, charucoIds,cobjPoints,cimgPoints);
         std::cout<<"BOARDcimgPoints "<<cimgPoints.size()<<"\n";
         for(auto p:cimgPoints)
             std::cout<<"  "<<p<<"\n";
         //obj points
         std::cout<<"BOARDcobjPoints "<<cobjPoints.size()<<"\n";
         for(auto p:cobjPoints)
             std::cout<<"  "<<p<<"\n";
         std::cout<<std::flush;
        cv::imshow("charuco", im2);



        std::vector<std::vector<Point2f>> diamondCorners;
        std::vector<Vec4i> diamonsIds;
        detector.detectDiamonds(image, diamondCorners, diamonsIds,  markerCorners, markerIds);

        for(auto dd:diamondCorners){
            std::cout<<"diamond corners "<<dd.size()<<"\n";
            for(auto p:dd)
                std::cout<<"  "<<p<<"\n";
        }
        auto canvas=image.clone();
         aruco::drawDetectedMarkers(canvas, markerCorners, markerIds);
        drawDetectedDiamonds(canvas, diamondCorners, diamonsIds);


        if(diamondCorners.size()>0){
         aruco::CharucoBoard2 b(cv::Size(2,2),1,1,dictionary);
        b.matchImagePoints(diamondCorners[0],diamonsIds[0],cobjPoints,cimgPoints);
         std::cout<<"cimgPoints "<<cimgPoints.size()<<"\n";
         for(auto p:cimgPoints)
             std::cout<<"  "<<p<<"\n";
         //obj points
         std::cout<<"cobjPoints "<<cobjPoints.size()<<"\n";
         for(auto p:cobjPoints)
             std::cout<<"  "<<p<<"\n";
         std::cout<<std::flush;
        cv::imshow("diamons", canvas);
        }

        char key = (char)cv::waitKey(0);
        if(key==27) break;

    }

    return 0;
}
