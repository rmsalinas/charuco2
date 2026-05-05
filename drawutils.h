
void drawDetectedMarkers(InputOutputArray _image, InputArrayOfArrays _corners,
                         InputArray _ids, Scalar borderColor= Scalar(0, 255, 0)) {
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
        CV_Assert(currentMarker.total() == 4 && currentMarker.channels() == 2);
        if (currentMarker.type() != CV_32SC2)
            currentMarker.convertTo(currentMarker, CV_32SC2);

        // draw marker sides
        for(int j = 0; j < 4; j++) {
            Point p0, p1;
            p0 = currentMarker.ptr<Point>(0)[j];
            p1 = currentMarker.ptr<Point>(0)[(j + 1) % 4];
            line(_image, p0, p1, borderColor, 2);
        }
        // draw first corner mark
        rectangle(_image, currentMarker.ptr<Point>(0)[0] - Point(3, 3),
                  currentMarker.ptr<Point>(0)[0] + Point(3, 3), cornerColor, 2, LINE_AA);

        // draw ID
        if(_ids.total() != 0) {
            Point cent(0, 0);
            for(int p = 0; p < 4; p++)
                cent += currentMarker.ptr<Point>(0)[p];
            cent = cent / 4.;
            std::stringstream s;
            s << _ids.getMat().ptr<int>(0)[i];
           // putText(_image, s.str(), cent, FONT_HERSHEY_SIMPLEX, 0.5, textColor, 2);

            double fontScale = 0.75; // 50% más grande que 0.5
            Point offset(-5, -4);  // Movido un poco más arriba y a la izquierda para evitar solapamiento

            // 1. Contorno negro más grueso (grosor 4)
            putText(_image, s.str(), cent + offset, FONT_HERSHEY_SIMPLEX, fontScale,
                    Scalar(0, 0, 0), 4, LINE_AA);

            // 2. Texto principal (grosor 2)
            putText(_image, s.str(), cent + offset, FONT_HERSHEY_SIMPLEX, fontScale,
                    cornerColor, 2, LINE_AA);

        }
    }
}
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

