#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"
using namespace cv;

void drawDetectedDiamonds(cv::InputOutputArray _image, cv::InputArrayOfArrays _corners, cv::InputArray _ids=cv::noArray(), cv::Scalar borderColor = cv::Scalar(0, 0, 255)) ;



int main(int argc, char **argv){

    if(argc == 1) {
        std::cout << "Usage: " << argv[0] << " <image>" << std::endl;
        return 0;
    }
    //assume argv [1] is a folder, read all jpg files and put them in imagePaths
    cv::Mat image=cv::imread(argv[1]);
    cv::aruco::CharucoBoard2 dboard(cv::Size(2, 2), 0.04f, 0.0f, cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_MIP_36h12));
    cv::aruco::CharucoDetector2 ddetector(dboard);
    std::vector<std::vector<cv::Point2f>> dCorners;
    std::vector<cv::Vec4i>                dIds;
    std::vector<std::vector<cv::Point2f>> mCorners;
    std::vector<int>                      mIds;
    ddetector.detectDiamonds(image, dCorners, dIds, mCorners, mIds);
    drawDetectedDiamonds(image, dCorners, dIds);
    cv::imshow("Diamonds", image);
    cv::waitKey(0);
}


void drawDetectedDiamonds(InputOutputArray _image, InputArrayOfArrays _corners, InputArray _ids, Scalar borderColor ) {
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