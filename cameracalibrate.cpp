#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <ctime>
#include <cctype>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"

// Predefined dictionary helper info
void printDictionaries() {
    std::cout << "Available predefined dictionaries (OpenCV IDs):\n"
              << "  0: DICT_4X4_50\n"
              << "  1: DICT_4X4_100\n"
              << "  2: DICT_4X4_250\n"
              << "  3: DICT_4X4_1000\n"
              << "  4: DICT_5X5_50\n"
              << "  5: DICT_5X5_100\n"
              << "  6: DICT_5X5_250\n"
              << "  7: DICT_5X5_1000\n"
              << "  8: DICT_6X6_50\n"
              << "  9: DICT_6X6_100\n"
              << "  10: DICT_6X6_250\n"
              << "  11: DICT_6X6_1000\n"
              << "  12: DICT_7X7_50\n"
              << "  13: DICT_7X7_100\n"
              << "  14: DICT_7X7_250\n"
              << "  15: DICT_7X7_1000\n"
              << "  16: DICT_ARUCO_MIP_36h12 (Default)\n";
}

int main(int argc, char** argv) {
    const char* keys =
        "{w width         | 9     | Board width (number of markers in X) }"
        "{h height        | 5     | Board height (number of markers in Y) }"
        "{l marker_length | 1.0   | Marker length in physical units (e.g. meters) }"
        "{d dictionary    | 16    | Predefined dictionary ID (e.g. 16 for DICT_ARUCO_MIP_36h12) }"
        "{s start_id      | 0     | First marker ID on the board (e.g., 0 or 1) }"
        "{o output        | camera_calibration.yml | Output calibration file }"
        "{@input_dir      |       | Input directory containing calibration images }"
        "{help ?          | false | Print this help message }";

    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Camera Calibration using CharucoBoard2 from a folder of images");

    if (parser.get<bool>("help") || !parser.has("@input_dir")) {
        parser.printMessage();
        printDictionaries();
        std::cout << "\nExample usage:\n"
                  << "  " << argv[0] << " --width=9 --height=5 --marker_length=0.026 --start_id=1 /path/to/image_folder\n\n";
        return 0;
    }

    int width = parser.get<int>("width");
    int height = parser.get<int>("height");
    float markerLength = parser.get<float>("marker_length");
    int dictId = parser.get<int>("dictionary");
    std::string outputFile = parser.get<std::string>("output");
    std::string inputDir = parser.get<std::string>("@input_dir");

    if (!parser.check()) {
        parser.printErrors();
        return 1;
    }

    if (width <= 0 || height <= 0 || markerLength <= 0.0f) {
        std::cerr << "Error: Board width, height, and marker length must be positive values.\n";
        return 1;
    }

    // Verify input directory
    if (!std::filesystem::exists(inputDir) || !std::filesystem::is_directory(inputDir)) {
        std::cerr << "Error: Input directory does not exist or is not a directory: " << inputDir << "\n";
        return 1;
    }

    // Collect list of image files from the folder
    std::vector<std::string> imageFiles;
    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tiff" || ext == ".webp") {
                imageFiles.push_back(entry.path().string());
            }
        }
    }

    if (imageFiles.empty()) {
        std::cerr << "Error: No calibration images found in directory: " << inputDir << "\n";
        return 1;
    }

    std::sort(imageFiles.begin(), imageFiles.end());
    std::cout << "Found " << imageFiles.size() << " images in " << inputDir << "\n";

    // Set up CharucoBoard2
    cv::Size boardSize(width, height);
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(static_cast<cv::aruco::PredefinedDictionaryType>(dictId));
    
    // Create marker IDs vector
    int startId = parser.get<int>("start_id");
    std::vector<int> ids;
    for (int i = 0; i < boardSize.area(); ++i) {
        ids.push_back(i + startId);
    }
    cv::aruco::CharucoBoard2 board(boardSize, markerLength, 1.0f, dictionary, ids);
    cv::aruco::CharucoDetector2 detector(board);

    std::vector<std::vector<cv::Point3f>> allObjectPoints;
    std::vector<std::vector<cv::Point2f>> allImagePoints;
    cv::Size imageSize;

    // Image list processing
    std::cout << "Processing images...\n";
    for (const auto& imgPath : imageFiles) {
        cv::Mat image = cv::imread(imgPath);
        if (image.empty()) {
            std::cerr << "Could not read image: " << imgPath << "\n";
            continue;
        }
        if (imageSize.area() == 0) {
            imageSize = image.size();
        } else if (imageSize != image.size()) {
            std::cerr << "Skipping image with different resolution: " << imgPath << "\n";
            continue;
        }

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        std::vector<cv::Point2f> charucoCorners;
        std::vector<int> charucoIds;

        detector.detectBoard(image, charucoCorners, charucoIds, markerCorners, markerIds);

        if (charucoCorners.size() >= 4) {
            std::vector<cv::Point3f> objPoints;
            std::vector<cv::Point2f> imgPoints;
            board.matchImagePoints(charucoCorners, charucoIds, objPoints, imgPoints);

            allImagePoints.push_back(imgPoints);
            allObjectPoints.push_back(objPoints);
            std::cout << "Image: " << imgPath << " - Detected " << charucoCorners.size() << " corners.\n";

            // Draw and display
            cv::Mat viewImg = image.clone();
            cv::aruco::drawDetectedMarkers(viewImg, markerCorners, markerIds);
            cv::aruco::drawDetectedCornersCharuco(viewImg, charucoCorners, charucoIds);
            cv::imshow("Calibration Corner Detection", viewImg);
            cv::waitKey(150); // short delay to show result
        } else {
            std::cout << "Image: " << imgPath << " - Board NOT detected (less than 4 corners found).\n";
        }
    }
    cv::destroyWindow("Calibration Corner Detection");

    if (allImagePoints.size() < 3) {
        std::cerr << "Error: Too few valid views for calibration (minimum is 3, recommended is 10+).\n";
        return 1;
    }

    std::cout << "\nCalibrating camera using " << allImagePoints.size() << " views...\n";
    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
    std::vector<cv::Mat> rvecs, tvecs;

    double rms = cv::calibrateCamera(
        allObjectPoints,
        allImagePoints,
        imageSize,
        cameraMatrix,
        distCoeffs,
        rvecs,
        tvecs
    );

    std::cout << "\n=======================================================\n"
              << "                 Calibration Results                   \n"
              << "=======================================================\n"
              << "RMS Re-projection Error: " << rms << " pixels\n"
              << "Camera Matrix:\n" << cameraMatrix << "\n\n"
              << "Distortion Coefficients:\n" << distCoeffs.t() << "\n"
              << "=======================================================\n\n";

    // Save calibration
    cv::FileStorage fs(outputFile, cv::FileStorage::WRITE);
    if (fs.isOpened()) {
        std::time_t rawtime;
        std::time(&rawtime);
        fs << "calibration_time" << std::asctime(std::localtime(&rawtime));
        fs << "image_width" << imageSize.width;
        fs << "image_height" << imageSize.height;
        fs << "board_width" << boardSize.width;
        fs << "board_height" << boardSize.height;
        fs << "marker_length" << markerLength;
        fs << "camera_matrix" << cameraMatrix;
        fs << "distortion_coefficients" << distCoeffs;
        fs << "avg_reprojection_error" << rms;
        fs.release();
        std::cout << "Calibration results successfully saved to: " << outputFile << "\n";
    } else {
        std::cerr << "Error: Could not open output file for writing: " << outputFile << "\n";
    }

    cv::destroyAllWindows();

    return 0;
}
