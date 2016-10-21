#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    /* initialisiere die UI Komponeten */
    setupUi();
}

MainWindow::~MainWindow()
{
    /* loesche die UI Komponeten */
    delete centralWidget;    
    
    /* schliesse alle offenen Fenster */
    cv::destroyAllWindows();
}

/* Methode oeffnet ein Bild und zeigt es in einem separaten Fenster an */
void MainWindow::on_pbOpenImage_clicked()
{
    /* oeffne Bild mit Hilfe eines Dateidialogs */
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image...", QString(), QString("Images *.png *.jpg *.tiff *.tif"));
    
    /* wenn ein gueltiger Dateipfad angegeben worden ist... */
    if(!imagePath.isNull() && !imagePath.isEmpty())
    {
        /* ...lese das Bild ein */
        cv::Mat img = ImageReader::readImage(QtOpencvCore::qstr2str(imagePath));
        
        /* wenn das Bild erfolgreich eingelesen worden ist... */
        if(!img.empty())
        {
            /* ... merke das Originalbild ... */
            originalImage = img;
            
            /* ... aktiviere das UI ... */
            enableGUI();
            
            /* ... zeige das Originalbild in einem separaten Fenster an */
            cv::imshow("Original Image", originalImage); 
        }
        else
        {
            /* ...sonst deaktiviere das UI */
            disableGUI();
        }
    }
}

void MainWindow::on_pbComputeSeams_clicked()
{
    /* Anzahl der Spalten, die entfernt werden sollen */
    int colsToRemove = sbCols->value();
    
    /* Anzahl der Zeilen, die entfernt werden sollen */
    int rowsToRemove = sbRows->value();
    
    /* .............. */

    calculateEnergy(originalImage);

}

void MainWindow::on_pbRemoveSeams_clicked()
{
    /* .............. */
}


/* Helper Function to calculate energyMap*/
cv::Mat MainWindow::calculateEnergy(const cv::Mat& inputImage){

    energyMap = cv::Mat::zeros(inputImage.size(), inputImage.type());

    // Hier abfangen wenn Col/Row schon removed wurde
    workingCopy = originalImage.clone();

    int imageWidth = workingCopy.size().width;
    int imageHeight = workingCopy.size().height;

    // Iterate over the whole image
    for(int x = 0; x <= imageWidth; x++){
        for(int y = 0; y <= imageHeight; y++){

            cv::Point currentLocation = cv::Point(x,y);
            // May be removed in the future:
            // cv::Vec3b currentPixel = workingCopy.at<cv::Vec3b>(cv::Point(x,y));

            int pixelEnergy = abs(sobelX(currentLocation)) + abs(sobelY(currentLocation));

            // Set the value of the energypixel to the energymap in fancy green (BGR Notation in openCV)
            energyMap.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,pixelEnergy,0);

        }

    }

    cv::imshow("Energy Map", energyMap);

    return energyMap;
}


/*

    Helper Function to calculate the energy value in X-direction for an vector
*/
int MainWindow::sobelX(cv::Point pixelLocation){

    cv::Vec3b left_to_x;
    cv::Vec3b right_to_x;

    const int right_border = workingCopy.size().width-1;

    //Sonderbehandlung der Ränder
    if(pixelLocation.x == 0){
        right_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x+1,pixelLocation.y));
        left_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y));
    }else if(pixelLocation.x == right_border){
        right_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y));
        left_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x-1,pixelLocation.y));
    }else{
        right_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x+1,pixelLocation.y));
        left_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x-1,pixelLocation.y));
    }

    // vgl. paper von avidan et al
    // Berechnung für jeden Kanal und danach Addition der Energiewerte
    int pixelEnergyB = ((int)right_to_x.val[0] - (int)left_to_x.val[0]) / 2;
    int pixelEnergyG = ((int)right_to_x.val[1] - (int)left_to_x.val[1]) / 2;
    int pixelEnergyR = ((int)right_to_x.val[2] - (int)left_to_x.val[2]) / 2;

    // Wieder in den [0..255] Raum bringen..
    return int((pixelEnergyB + pixelEnergyG + pixelEnergyR)/3);
}

/*

    same for Y-Direction
*/
int MainWindow::sobelY(cv::Point pixelLocation){

    cv::Vec3b lower_to_x;
    cv::Vec3b upper_to_x;

    const int lower_border = workingCopy.size().height-1;

    //Sonderbehandlung der Ränder
    if(pixelLocation.y == 0){
        upper_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y));
        lower_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y+1));
    }else if(pixelLocation.y == lower_border){
        upper_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y-1));
        lower_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y));
    }else{
        upper_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y-1));
        lower_to_x = workingCopy.at<cv::Vec3b>(cv::Point(pixelLocation.x,pixelLocation.y+1));
    }

    // vgl. paper von avidan et al
    // Berechnung für jeden Kanal und danach Addition der Energiewerte
    int pixelEnergyB = ((int)lower_to_x.val[0] - (int)upper_to_x.val[0]) / 2;
    int pixelEnergyG = ((int)lower_to_x.val[1] - (int)upper_to_x.val[1]) / 2;
    int pixelEnergyR = ((int)lower_to_x.val[2] - (int)upper_to_x.val[2]) / 2;

    // Wieder in den [0..255] Raum bringen..
    return int((pixelEnergyB + pixelEnergyG + pixelEnergyR) / 3);
}


void MainWindow::setupUi()
{
    /* Boilerplate code */
    /*********************************************************************************************/
    resize(129, 211);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setMinimumSize(QSize(129, 211));
    setMaximumSize(QSize(129, 211));
    centralWidget = new QWidget(this);
    centralWidget->setObjectName(QString("centralWidget"));
    
    horizontalLayout = new QHBoxLayout(centralWidget);
    verticalLayout = new QVBoxLayout();
    
    pbOpenImage = new QPushButton(QString("Open Image"), centralWidget);
    verticalLayout->addWidget(pbOpenImage);
    
    
    verticalLayout_3 = new QVBoxLayout();
    lCaption = new QLabel(QString("Remove"), centralWidget);
    lCaption->setEnabled(false);
    verticalLayout_3->addWidget(lCaption);
    
    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString("horizontalLayout_3"));
    lCols = new QLabel(QString("Cols"), centralWidget);
    lCols->setEnabled(false);
    lRows = new QLabel(QString("Rows"), centralWidget);
    lRows->setEnabled(false);
    horizontalLayout_3->addWidget(lCols);
    horizontalLayout_3->addWidget(lRows);
    verticalLayout_3->addLayout(horizontalLayout_3);
    
    horizontalLayout_2 = new QHBoxLayout();
    sbCols = new QSpinBox(centralWidget);
    sbCols->setEnabled(false);
    horizontalLayout_2->addWidget(sbCols);
    sbRows = new QSpinBox(centralWidget);
    sbRows->setEnabled(false);
    horizontalLayout_2->addWidget(sbRows);
    verticalLayout_3->addLayout(horizontalLayout_2);
    verticalLayout->addLayout(verticalLayout_3);
    
    pbComputeSeams = new QPushButton(QString("Compute Seams"), centralWidget);
    pbComputeSeams->setEnabled(false);
    verticalLayout->addWidget(pbComputeSeams);
    
    pbRemoveSeams = new QPushButton(QString("Remove Seams"), centralWidget);
    pbRemoveSeams->setEnabled(false);
    verticalLayout->addWidget(pbRemoveSeams);
    
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(verticalSpacer);
    horizontalLayout->addLayout(verticalLayout);
    
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);
    setCentralWidget(centralWidget);
    /*********************************************************************************************/
    
    
    /* Verbindung zwischen den Buttonklicks und den Methoden, die beim jeweiligen Buttonklick ausgefuehrt werden sollen */
    connect(pbOpenImage,    &QPushButton::clicked, this, &MainWindow::on_pbOpenImage_clicked);  
    connect(pbComputeSeams, &QPushButton::clicked, this, &MainWindow::on_pbComputeSeams_clicked); 
    connect(pbRemoveSeams,  &QPushButton::clicked, this, &MainWindow::on_pbRemoveSeams_clicked);
}

void MainWindow::enableGUI()
{
    lCaption->setEnabled(true);
    
    lCols->setEnabled(true);
    lRows->setEnabled(true);
    
    sbCols->setEnabled(true);
    sbRows->setEnabled(true);
    
    pbComputeSeams->setEnabled(true);
    pbRemoveSeams->setEnabled(true);
    
    sbRows->setMinimum(0);
    sbRows->setMaximum(originalImage.rows);
    sbRows->setValue(2);
    
    sbCols->setMinimum(0);
    sbCols->setMaximum(originalImage.cols);
    sbCols->setValue(2);
}

void MainWindow::disableGUI()
{
    lCaption->setEnabled(false);
    
    lCols->setEnabled(false);
    lRows->setEnabled(false);
    
    sbCols->setEnabled(false);
    sbRows->setEnabled(false);
    
    pbComputeSeams->setEnabled(false);
    pbRemoveSeams->setEnabled(false);
}
