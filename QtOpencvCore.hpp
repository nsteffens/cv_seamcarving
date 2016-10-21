#ifndef QTOPENCVCORE_HPP
#define QTOPENCVCORE_HPP

#include <string>

#include <QImage>
#include <QPixmap>
#include <QString>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"


namespace QtOpencvCore
{
    /**
     * @brief This function takes a cv::Mat image and converts it to a QImage
     * @param  img is a cv::Mat image which will be converted to a QImage
     * @return QImage of the cv::Mat image img
     */
    QImage img2qimg(cv::Mat& img);

    QPixmap img2qpix(cv::Mat& img);
    
    /**
     * @brief This function takes a QImage image and converts it to a cv::Mat
     * @param  qimg is a QImage image which will be converted to a cv::Mat
     * @return cv::Mat of the QImage img
     */
    cv::Mat qimg2img(QImage const &qimg);

    /**
     * @brief Converts a QString to a std string
     * @param QString to be converted
     * @return  Std string created from the QString
     */
    std::string qstr2str(QString const& qstr);


    /**
     * @brief Converts a std string to a QString
     * @param Std string to be converted
     * @return QString created from the std string
     */
    QString str2qstr(std::string const& str);
}

#endif // QTOPENCVCORE_HPP
