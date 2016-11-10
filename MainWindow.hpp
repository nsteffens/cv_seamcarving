#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileDialog>
#include <QPushButton>
#include <QBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QGroupBox>
#include <QStatusBar>

#include "ImageReader.hpp"
#include "QtOpencvCore.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    
    /* Konstruktor */
    explicit MainWindow(QWidget *parent = 0);
    
    /* Destruktor */
    ~MainWindow();
    
private slots:  
    
    /* Funktionen werden ausgelöst, wenn auf den entsprechenden Button geklickt wird */
    void on_pbOpenImage_clicked();
    void on_pbComputeSeams_clicked();
    void on_pbRemoveSeams_clicked();
    cv::Mat calculateEnergy(cv::Mat inputImage);
    int sobelX(cv::Point, cv::Mat inputImage);
    int sobelY(cv::Point, cv::Mat inputImage);
    std::vector<cv::Point> findSeamH();
    std::vector<cv::Point> findSeamV();
    cv::Mat removeSeamV(cv::Mat inputMat, std::vector<cv::Point> inputSeam);
    cv::Mat removeSeamH(cv::Mat inputMat, std::vector<cv::Point> inputSeam);
    cv::Mat drawSeam(std::vector<cv::Point> seam, cv::Mat Image);

private:

    
    /* GUI Komoneten */
    QWidget     *centralWidget;
    
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout;
    QVBoxLayout *verticalLayout_3;
    
    QPushButton *pbOpenImage;
    QPushButton *pbRemoveSeams;
    QPushButton *pbComputeSeams;
    
    QLabel      *lCaption;
    QLabel      *lCols;
    QLabel      *lRows;
    
    QSpinBox    *sbCols;
    QSpinBox    *sbRows;
    
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer;
    /*****************************************/
    
    /* Originalbild */
    cv::Mat         originalImage;
    /* Eventuel weitere Klassenattribute */
    //cv::Mat         workingCopy;
    cv::Mat         energyMap;

    std::vector<std::vector<cv::Point>> seamsV;
    std::vector<std::vector<cv::Point>> seamsH;
    
    /* Methode initialisiert die UI */
    void setupUi();
    
    /* Methoden aktivieren bzw. deaktivieren die UI */
    void enableGUI();
    void disableGUI();
};

#endif // MAINWINDOW_HPP
