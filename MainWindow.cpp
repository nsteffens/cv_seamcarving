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

    std::cout << "Processing..." << std::endl;

    workingCopy = originalImage.clone();

    // Loop for vertical seams
   for (int i = 0; i < colsToRemove; i++){

       // Energymap berechnen
       energyMap = calculateEnergy(workingCopy);
       // Seam berechnen
       std::vector<cv::Point> tmpSeamV = findSeamV();
       // löschen des Seams + Zusammenfügen
       workingCopy = removeSeamV(workingCopy, tmpSeamV);

   }

   // Prepared horizontal seam removal
   for (int i = 0; i < rowsToRemove; i++){
       // Energymap berechnen
       energyMap = calculateEnergy(workingCopy);

       // Seam berechnen
       std::vector<cv::Point> tmpSeamH = findSeamH();

       // Löschen des Seams + Zusammenfügen

       workingCopy = removeSeamH(workingCopy, tmpSeamH);
   }

   std::cout << "Calculation finished!" << std::endl;

}

void MainWindow::on_pbRemoveSeams_clicked()
{
    /* .............. */


    cv::imshow("Removed img", workingCopy);


}

/**
 * @brief MainWindow::removeSeamV
 *
 *      Removes a given vertical Seam and returns the new image
 *
 * @param inputMat      the image matrix where the seam should be removed
 * @param inputSeam     the seam that will be removed - Needs to be a vertical seam
 * @return outputMat    the stitched image matrix without the seam pixels
 */
cv::Mat MainWindow::removeSeamV(cv::Mat inputMat, std::vector<cv::Point> inputSeam){

    for (int y = 0; y < inputMat.size().height; ++y) {

        // Start at the Seam and move every pixel on the right side of the seam to the left
        // inputMat.size().width-1 because last column is empty (and will be removed..)!
        for (int x = inputSeam[y].x; x < inputMat.size().width-1; ++x){

            inputMat.at<cv::Vec3b>(cv::Point(x,y)) = inputMat.at<cv::Vec3b>(cv::Point(x+1, y));

        }
    }

    cv::Mat outputMat = inputMat(cv::Rect(0,0,inputMat.size().width-1,inputMat.size().height)).clone();

    return outputMat;
}

/**
 * @brief MainWindow::removeSeamH
 *
 *      Removes a given vertical Seam and returns the new image
 *
 * @param inputMat      the image matrix where the seam should be removed
 * @param inputSeam     the seam that will be removed - Needs to be a horizontal seam
 * @return outputMat    the stitched image matrix without the seam pixels
 */
cv::Mat MainWindow::removeSeamH(cv::Mat inputMat, std::vector<cv::Point> inputSeam){

    // Analog to removeSeamV - now moving everything below the seam upwards
    for(int x = 0; x < inputMat.size().width; ++x) {
        for (int y = inputSeam[x].y; y < inputMat.size().height-1; ++y){
            inputMat.at<cv::Vec3b>(cv::Point(x,y)) = inputMat.at<cv::Vec3b>(cv::Point(x, y+1));
        }
    }

    cv::Mat outputMat = inputMat(cv::Rect(0,0,inputMat.size().width,inputMat.size().height-1)).clone();

    return outputMat;
}


/**
 * @brief MainWindow::findSeamH
 *
 * Finds an horizontal seam based on the current energyMap
 *
 * @return horizontalSeam
 */
std::vector<cv::Point> MainWindow::findSeamH(){

    // Vector with points where our seam goes along
    std::vector<cv::Point> horizontalSeam;
    std::vector<cv::Point>::iterator it;

    // Finding the seam with dynamic programming:
    cv::Mat wayfindingMatrix = cv::Mat::zeros(energyMap.size(), energyMap.type());

    // Fill the first column with initial contour strength values
    for(int y = 0; y < energyMap.size().height; y++){
        int value = energyMap.at<cv::Vec3b>(cv::Point(0,y)).val[1];
        wayfindingMatrix.at<cv::Vec3b>(cv::Point(0,y)) = cv::Vec3b(0,value,0);
    }

    // Calculate the rest and fill the matrix..
    // We don't need calculate the first column, so x = 1;
    for(int x = 1; x < energyMap.size().width; x++){
        for(int y = 0; y < energyMap.size().height; y++){

           cv::Point currentLocation = cv::Point(x,y);

           //int self = energyMap.at<cv::Vec3b>(currentLocation).val[1];
           // Exception if there is no upper neighbour (upper border)
           if(y == 0){
               // Set value of x,y to min(lowerNeighbour, directNeighbour) + SELF

               int directNeighbour  = energyMap.at<cv::Vec3b>(cv::Point(x-1,y)).val[1];
               int lowerNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x-1,y+1)).val[1];

               int value = std::min(directNeighbour, lowerNeighbour) + (int)energyMap.at<cv::Vec3b>(currentLocation).val[1];
               wayfindingMatrix.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,value,0);

           // Exception if there is no lower neighbour  (lower border)
           }else if (y == wayfindingMatrix.size().height-1){
               // Set value of x,y to min(upperNeighbour, directNeighbour) + SELF

               int directNeighbour  = energyMap.at<cv::Vec3b>(cv::Point(x-1,y)).val[1];
               int upperNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x-1,y-1)).val[1];

               int value = std::min(directNeighbour, upperNeighbour) + (int)energyMap.at<cv::Vec3b>(currentLocation).val[1];
               wayfindingMatrix.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,value,0);
           }else{
               // catch normal case..

               int directNeighbour  = energyMap.at<cv::Vec3b>(cv::Point(x-1,y)).val[1];
               int lowerNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x-1,y+1)).val[1];
               int upperNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x-1,y-1)).val[1];

               int value = std::min(std::min(directNeighbour,lowerNeighbour),upperNeighbour) + (int)energyMap.at<cv::Vec3b>(currentLocation).val[1];
               wayfindingMatrix.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,value,0);
           }
        }
    }

    // Finished the wayfindingMatrix calculation. Now find the cheapest path heading back to the start

    // First find the minimum in the last column

    cv::Point min_pos;
    int min = 756; // Cant be higher than 255*3..

    for(int y = 0; y < wayfindingMatrix.size().height; y++){
        // Set X to right border column
        int x = wayfindingMatrix.size().width-1;

        if(min > wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y)).val[1]){
            min = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y)).val[1];
            min_pos = cv::Point(x,y);
        }
    }

    // Add the start of the seam;
    it = horizontalSeam.begin();
    horizontalSeam.insert(it, cv::Point(min_pos.x,min_pos.y));

    int y_adjust = min_pos.y;

    // Iterate over the wayfindingmatrix;
    for(int x = wayfindingMatrix.size().width-1; x > 0; x--){

       int y = y_adjust;
       cv::Point smallestNeighbour;

        // normal procedure if not on the top or bottom border
       if(y != 0 && y != wayfindingMatrix.size().height-1){

           // Look at the Elements at X-1/Y , X-1/Y+1 and X-1/Y-1 and select lowest
        cv::Vec3b upperNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1, y-1));
        cv::Vec3b directNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1,y));
        cv::Vec3b lowerNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1, y+1));

        if(upperNeighbour.val[1] < directNeighbour.val[1] && upperNeighbour.val[1] < lowerNeighbour.val[1]){
            smallestNeighbour = cv::Point(x-1, y-1);
        }else if(directNeighbour.val[1] < upperNeighbour.val[1] && directNeighbour.val[1] < lowerNeighbour.val[1]){
             smallestNeighbour = cv::Point(x-1, y);
        }else{
            smallestNeighbour = cv::Point(x-1,y+1);
        }

       // catch border cases
       }else if(y == 0){
           cv::Vec3b directNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1,y));
           cv::Vec3b lowerNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1, y+1));
            if(directNeighbour.val[1] < lowerNeighbour.val[1]){
               smallestNeighbour = cv::Point(x-1, y);
           }else{

               smallestNeighbour = cv::Point(x-1,y+1);
           }

       }else{
           // must be the case y = lower border --> no bottom
           cv::Vec3b directNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1,y));
           cv::Vec3b upperNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1, y-1));
           if(directNeighbour.val[1] < upperNeighbour.val[1]){
               smallestNeighbour = cv::Point(x-1, y);
           }else{
               smallestNeighbour = cv::Point(x-1, y-1);
           }
       }

       //Add to horizontalSeam
       it = horizontalSeam.begin();
       horizontalSeam.insert(it, smallestNeighbour);

       y_adjust = smallestNeighbour.y;

    }

    return horizontalSeam;
}

/**
 * @brief MainWindow::findSeamV
 *
 * Finds an vertical seam based on the current energyMap
 *
 * @return verticalSeam
 */
std::vector<cv::Point> MainWindow::findSeamV(){

    // Vector with points where our seam goes along
    std::vector<cv::Point> verticalSeam;
    std::vector<cv::Point>::iterator it;

    cv::Mat wayfindingMatrix = cv::Mat::zeros(energyMap.size(), energyMap.type());

    // Fill the first row with initial contour strength values
    for(int x = 0; x < wayfindingMatrix.size().width; x++){
        int value = energyMap.at<cv::Vec3b>(cv::Point(x,0)).val[1];
        wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,0)) = cv::Vec3b(0,value,0);
    }

    // Calculate the rest and fill the matrix..
    // We don't need calculate the first column, so x = 1;
    for(int y = 1; y < energyMap.size().height; y++){
        for(int x = 0; x < energyMap.size().width; x++){

           cv::Point currentLocation = cv::Point(x,y);

           // Exception if there is no upper neighbour (upper border)
           if(x == 0){
               // Set value of x,y to min(lowerNeighbour, directNeighbour) + SELF

               int directNeighbour  = energyMap.at<cv::Vec3b>(cv::Point(x,y-1)).val[1];
               int rightNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x+1,y-1)).val[1];

               int value = std::min(directNeighbour, rightNeighbour) + (int)energyMap.at<cv::Vec3b>(currentLocation).val[1];
               wayfindingMatrix.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,value,0);

           // Exception if there is no lower neighbour  (lower border)
           }else if (x == wayfindingMatrix.size().width-1){

               // Set value of x,y to min(leftNeighbour, directNeighbour) + SELF
               int directNeighbour  = energyMap.at<cv::Vec3b>(cv::Point(x,y-1)).val[1];
               int leftNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x-1,y-1)).val[1];

               int value = std::min(directNeighbour, leftNeighbour) + (int)energyMap.at<cv::Vec3b>(currentLocation).val[1];
               wayfindingMatrix.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,value,0);
           }else{

               // catch normal case..
               int directNeighbour  = energyMap.at<cv::Vec3b>(cv::Point(x,y-1)).val[1];
               int leftNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x-1,y-1)).val[1];
               int rightNeighbour   = energyMap.at<cv::Vec3b>(cv::Point(x+1,y-1)).val[1];

               int value = std::min(std::min(directNeighbour,leftNeighbour),rightNeighbour) + (int)energyMap.at<cv::Vec3b>(currentLocation).val[1];
               wayfindingMatrix.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,value,0);
           }
        }
    }

    cv::Point min_pos;
    int min = 756; // Cant be higher than 255*3..

    for(int x = 0; x < wayfindingMatrix.size().width; x++){
        // Set X to right border column
        int y = wayfindingMatrix.size().height-1;

        if(min > wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y)).val[1]){
            min = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y)).val[1];
            min_pos = cv::Point(x,y);
        }
    }

    it = verticalSeam.begin();
    verticalSeam.insert(it, cv::Point(min_pos.x,min_pos.y));

    // Start going upwards and add elements of the seam based on the wf-Matrix
    int x_adjust = min_pos.x;

    for(int y = wayfindingMatrix.size().height-1; y > 0; y--){

       int x = x_adjust;
       cv::Point smallestNeighbour;

        // normal procedure if not on the left or right border
       if(x != 0 && x != wayfindingMatrix.size().width-1){

           // Look at the Elements at X-1/Y , X-1/Y+1 and X-1/Y-1 and select lowest
        cv::Vec3b leftNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1, y-1));
        cv::Vec3b directNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y-1));
        cv::Vec3b rightNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x+1, y-1));

        if(leftNeighbour.val[1] < directNeighbour.val[1] && leftNeighbour.val[1] < rightNeighbour.val[1]){
            smallestNeighbour = cv::Point(x-1, y-1);
        }else if(directNeighbour.val[1] < rightNeighbour.val[1] && directNeighbour.val[1] < leftNeighbour.val[1]){
             smallestNeighbour = cv::Point(x, y-1);
        }else{
            smallestNeighbour = cv::Point(x+1,y-1);
        }

       // catch border cases --> x == 0 --> left border
       }else if(x == 0){
           cv::Vec3b directNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y-1));
           cv::Vec3b rightNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x+1, y-1));
            if(directNeighbour.val[1] < rightNeighbour.val[1]){
               smallestNeighbour = cv::Point(x, y-1);
           }else{
               smallestNeighbour = cv::Point(x+1,y-1);
           }

       }else{
           // must be the case x = right border
           cv::Vec3b directNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x,y-1));
           cv::Vec3b leftNeighbour = wayfindingMatrix.at<cv::Vec3b>(cv::Point(x-1, y-1));
           if(directNeighbour.val[1] < leftNeighbour.val[1]){
               smallestNeighbour = cv::Point(x, y-1);
           }else{
               smallestNeighbour = cv::Point(x-1, y-1);
           }
       }

       //Add to verticalSeam
       it = verticalSeam.begin();
       verticalSeam.insert(it, smallestNeighbour);

       x_adjust = smallestNeighbour.x;

    }

    return verticalSeam;
}

/**
 * @brief MainWindow::calculateEnergy
 *
 *  Calculates the EnergyMap for a given image
 *
 * @param inputImage    Image matrix for which the energy is written in the energyMap
 * @return void
 */
cv::Mat MainWindow::calculateEnergy(cv::Mat inputImage){

    energyMap = cv::Mat::zeros(inputImage.size(), inputImage.type());

    int imageWidth = inputImage.size().width;
    int imageHeight = inputImage.size().height;

    // Iterate over the whole image
    for(int x = 0; x < imageWidth; x++){
        for(int y = 0; y < imageHeight; y++){
            cv::Point currentLocation = cv::Point(x,y);
            int pixelEnergy = abs(sobelX(currentLocation)) + abs(sobelY(currentLocation));
            // Set the value of the energypixel to the energymap in fancy green (BGR Notation in openCV)
            energyMap.at<cv::Vec3b>(currentLocation) = cv::Vec3b(0,pixelEnergy,0);
        }
    }

    return energyMap;
}

/**
 * @brief MainWindow::sobelX
 *
 *      Applies the sobel filter in X-direction on the workingCopy at the given pixelLocation
 *
 * @param pixelLocation cv::Point Location of the pixel where the sobel filter in x-direction is applied
 * @return new pixel value as int
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

    return int((pixelEnergyB + pixelEnergyG + pixelEnergyR));
}

/**
 * @brief MainWindow::sobelY
 *
 *      Applies the sobel filter in Y-direction on the workingCopy at the given pixelLocation
 *
 * @param pixelLocation cv::Point Location of the pixel where the sobel filter in y-direction is applied
 * @return new pixel value as int
 */
int MainWindow::sobelY(cv::Point pixelLocation){

    cv::Vec3b lower_to_x;
    cv::Vec3b upper_to_x;

    int lower_border = workingCopy.size().height-1;

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

    return int((pixelEnergyB + pixelEnergyG + pixelEnergyR));
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
