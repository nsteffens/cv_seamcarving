#include "QtOpencvCore.hpp"

namespace QtOpencvCore
{
    QImage img2qimg(cv::Mat& img)
    {
        QImage qimage;

         // convert the color to RGB (OpenCV uses BGR)
         switch (img.type()) {
         case CV_8UC1:
            cv::cvtColor(img, img, CV_GRAY2RGB);
            qimage = QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
            cv::cvtColor(img, img, CV_RGB2GRAY);
            break;
         case CV_8UC3:
            cv::cvtColor(img, img, CV_BGR2RGB);
            qimage = QImage((uchar*) img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
            cv::cvtColor(img, img, CV_RGB2BGR);
            break;
         }


         // return the QImage
         return qimage;
    }
    
    QPixmap img2qpix(cv::Mat& img)
    {
        return QPixmap::fromImage(img2qimg(img));
    }

    std::string qstr2str(QString const& qstr)
    {
        // return the converted QString (now std string)
        return qstr.toStdString();
    }

    QString str2qstr(std::string const& str)
    {
        // return the converted std string (now QString)
        return QString::fromStdString(str);
    }

    cv::Mat qimg2img(const QImage &qimg)
    {
        cv::Mat img;
        img = cv::Mat(qimg.height(), qimg.width(), CV_8UC4, const_cast<uchar*>(qimg.bits()), qimg.bytesPerLine());
        return img;
    }

} // namespace QtOpencvCore
