#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include "charuco2.h"
#include <filesystem>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

// Helper function to get sorted image paths from a directory
std::vector<fs::path> getImagePaths(const std::string& folderPath) {
    std::vector<fs::path> paths;
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cerr << "Directory does not exist: " << folderPath << "\n";
        return paths;
    }
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" || ext == ".tiff" || ext == ".webp") {
                paths.push_back(entry.path());
            }
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

int main(int argc, char** argv) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " <traditional_charuco_folder> <charuco2_folder>\n";
            return 1;
        }

        std::string charucoFolder = argv[1];
        std::string charuco2Folder = argv[2];

        // 9x5 board elements setup
        cv::Size boardSize(9, 5);
        cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_MIP_36h12);

        // 1. Traditional ChArUco setup
        cv::aruco::CharucoBoard traditionalBoard(boardSize, 1.0f, 0.8f, dictionary);
        cv::aruco::CharucoDetector traditionalDetector(traditionalBoard);

        // 2. ChArUco2 setup
        std::vector<int> ids;
        for (int id = 0; id < boardSize.area(); id++) {
            ids.push_back(id + 1);
        }
        cv::aruco::CharucoBoard2 board2(boardSize, 1.0f, 1.0f, dictionary, ids);
        cv::aruco::CharucoDetector2 detector2(board2);

        // Get image lists
        auto traditionalImages = getImagePaths(charucoFolder);
        auto charuco2Images = getImagePaths(charuco2Folder);

        std::cout << "--------------------------------------------------------\n";
        std::cout << "Comparing speeds of ChArUco vs ChArUco2 board detection\n";
        std::cout << "Board Size: 9x5\n";
        std::cout << "--------------------------------------------------------\n\n";

        // --- Benchmark Traditional ChArUco ---
        std::cout << "Benchmarking Traditional ChArUco on folder: " << charucoFolder << "\n";
        double totalTraditionalTimeMs = 0.0;
        int traditionalSuccessCount = 0;
        
        for (const auto& path : traditionalImages) {
            cv::Mat img = cv::imread(path.string());
            if (img.empty()) {
                std::cerr << "Failed to read image: " << path.string() << "\n";
                continue;
            }

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners;
            std::vector<cv::Point2f> charucoCorners;
            std::vector<int> charucoIds;

            auto start = std::chrono::high_resolution_clock::now();
            traditionalDetector.detectBoard(img, charucoCorners, charucoIds, markerCorners, markerIds);
            auto end = std::chrono::high_resolution_clock::now();

            double durationMs = std::chrono::duration<double, std::milli>(end - start).count();
            totalTraditionalTimeMs += durationMs;
            traditionalSuccessCount++;

            std::cout << "  " << path.filename().string() << ": " 
                      << durationMs << " ms (detected " << charucoCorners.size() << " corners)\n";
        }
        
        double avgTraditionalTimeMs = traditionalSuccessCount > 0 ? (totalTraditionalTimeMs / traditionalSuccessCount) : 0.0;
        std::cout << "Finished Traditional ChArUco: " << traditionalSuccessCount << " images processed, Average: "
                  << avgTraditionalTimeMs << " ms\n\n";

        // --- Benchmark ChArUco2 ---
        std::cout << "Benchmarking ChArUco2 on folder: " << charuco2Folder << "\n";
        double totalCharuco2TimeMs = 0.0;
        int charuco2SuccessCount = 0;

        for (const auto& path : charuco2Images) {
            cv::Mat img = cv::imread(path.string());
            if (img.empty()) {
                std::cerr << "Failed to read image: " << path.string() << "\n";
                continue;
            }

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners;
            std::vector<cv::Point2f> charucoCorners;
            std::vector<int> charucoIds;

            auto start = std::chrono::high_resolution_clock::now();
            detector2.detectBoard(img, charucoCorners, charucoIds, markerCorners, markerIds);
            auto end = std::chrono::high_resolution_clock::now();

            double durationMs = std::chrono::duration<double, std::milli>(end - start).count();
            totalCharuco2TimeMs += durationMs;
            charuco2SuccessCount++;

            std::cout << "  " << path.filename().string() << ": " 
                      << durationMs << " ms (detected " << charucoCorners.size() << " corners)\n";
        }

        double avgCharuco2TimeMs = charuco2SuccessCount > 0 ? (totalCharuco2TimeMs / charuco2SuccessCount) : 0.0;
        std::cout << "Finished ChArUco2: " << charuco2SuccessCount << " images processed, Average: "
                  << avgCharuco2TimeMs << " ms\n\n";

        // --- Summary Comparison ---
        std::cout << "========================================================\n";
        std::cout << "SUMMARY RESULTS:\n";
        std::cout << "========================================================\n";
        std::cout << "Traditional ChArUco:\n";
        std::cout << "  Images: " << traditionalSuccessCount << "\n";
        std::cout << "  Total Time: " << totalTraditionalTimeMs << " ms\n";
        std::cout << "  Average Time: " << avgTraditionalTimeMs << " ms\n";
        std::cout << "ChArUco2:\n";
        std::cout << "  Images: " << charuco2SuccessCount << "\n";
        std::cout << "  Total Time: " << totalCharuco2TimeMs << " ms\n";
        std::cout << "  Average Time: " << avgCharuco2TimeMs << " ms\n";
        std::cout << "========================================================\n";

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}