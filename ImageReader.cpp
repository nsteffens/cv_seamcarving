#include "ImageReader.hpp"

cv::Mat ImageReader::readImage(const std::string& filePath)
{
    return cv::imread(filePath);
}
