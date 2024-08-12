void MainWindow::InitPHD2()
{
    isGuideCapture = true;

    cmdPHD2 = new QProcess();
    cmdPHD2->start("pkill phd2");
    cmdPHD2->waitForStarted();
    cmdPHD2->waitForFinished();

    key_phd = ftok("../", 2015);
    key_phd = 0x90;

    if (key_phd == -1)
    {
        qDebug("ftok_phd");
    }

    // build the shared memory
    system("ipcs -m"); // 查看共享内存
    shmid_phd = shmget(key_phd, BUFSZ_PHD, IPC_CREAT | 0666);
    if (shmid_phd < 0)
    {
        qDebug("main.cpp | main | shared memory phd shmget ERROR");
        exit(-1);
    }

    // 映射
    sharedmemory_phd = (char *)shmat(shmid_phd, NULL, 0);
    if (sharedmemory_phd == NULL)
    {
        qDebug("main.cpp | main | shared memor phd map ERROR");
        exit(-1);
    }

    // 读共享内存区数据
    qDebug("data_phd = [%s]\n", sharedmemory_phd);

    cmdPHD2->start("phd2");

    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 10000)
    {
        usleep(10000);
        qApp->processEvents();
        if (connectPHD() == true)
            break;
    }
}

bool MainWindow::connectPHD(void)
{
    QString versionName = "";
    call_phd_GetVersion(versionName);

    qDebug() << "QSCOPE|connectPHD|version:" << versionName;
    if (versionName != "")
    {
        // init stellarium operation
        return true;
    }
    else
    {
        qDebug() << "QSCOPE|connectPHD|error:there is no openPHD2 running";
        return false;
    }
}

bool MainWindow::call_phd_GetVersion(QString &versionName)
{
    unsigned int baseAddress;
    unsigned int vendcommand;
    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x01;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }

    if (t.elapsed() >= 500)
    {
        versionName = "";
        return false;
    }
    else
    {
        unsigned char addr = 0;
        uint16_t length;
        memcpy(&length, sharedmemory_phd + baseAddress + addr, sizeof(uint16_t));
        addr = addr + sizeof(uint16_t);
        // qDebug()<<length;

        if (length > 0 && length < 1024)
        {
            for (int i = 0; i < length; i++)
            {
                versionName.append(sharedmemory_phd[baseAddress + addr + i]);
            }
            return true;
            // qDebug()<<versionName;
        }
        else
        {
            versionName = "";
            return false;
        }
    }
}

uint32_t MainWindow::call_phd_StartLooping(void)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x03;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }
    if (t.elapsed() >= 500)
        return false; // timeout
    else
        return true;
}

uint32_t MainWindow::call_phd_StopLooping(void)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x04;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }
    if (t.elapsed() >= 500)
        return false; // timeout
    else
        return true;
}

uint32_t MainWindow::call_phd_AutoFindStar(void)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x05;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }
    if (t.elapsed() >= 500)
        return false; // timeout
    else
        return true;
}

uint32_t MainWindow::call_phd_StartGuiding(void)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x06;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }
    if (t.elapsed() >= 500)
        return false; // timeout
    else
        return true;
}

uint32_t MainWindow::call_phd_checkStatus(unsigned char &status)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x07;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    // wait stellarium finished this task
    QElapsedTimer t;
    t.start();
    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    } // wait stellarium run end

    if (t.elapsed() >= 500)
    {
        // timeout
        status = 0;
        return false;
    }

    else
    {
        status = sharedmemory_phd[3];
        return true;
    }
}

uint32_t MainWindow::call_phd_setExposureTime(unsigned int expTime)
{
    unsigned int vendcommand;
    unsigned int baseAddress;
    qDebug() << "call_phd_setExposureTime" << expTime;
    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x0b;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    unsigned char addr = 0;
    memcpy(sharedmemory_phd + baseAddress + addr, &expTime, sizeof(unsigned int));
    addr = addr + sizeof(unsigned int);

    sharedmemory_phd[0] = 0x01; // enable command

    // wait stellarium finished this task
    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    } // wait stellarium run end

    if (t.elapsed() >= 500)
        return QHYCCD_ERROR; // timeout
    else
        return QHYCCD_SUCCESS;
}

uint32_t MainWindow::call_phd_whichCamera(std::string Camera)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x0d;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    int length = Camera.length() + 1;

    unsigned char addr = 0;
    // memcpy(sharedmemory_phd + baseAddress + addr, &index, sizeof(int));
    // addr = addr + sizeof(int);
    memcpy(sharedmemory_phd + baseAddress + addr, &length, sizeof(int));
    addr = addr + sizeof(int);
    memcpy(sharedmemory_phd + baseAddress + addr, Camera.c_str(), length);
    addr = addr + length;

    // wait stellarium finished this task
    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    } // wait stellarium run end

    if (t.elapsed() >= 500)
        return QHYCCD_ERROR; // timeout
    else
        return QHYCCD_SUCCESS;
}

uint32_t MainWindow::call_phd_ChackControlStatus(int sdk_num)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x0e;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    unsigned char addr = 0;
    memcpy(sharedmemory_phd + baseAddress + addr, &sdk_num, sizeof(int));
    addr = addr + sizeof(int);

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }
    if (t.elapsed() >= 500)
        return false; // timeout
    else
        return true;
}

uint32_t MainWindow::call_phd_ClearCalibration(void)
{
    unsigned int vendcommand;
    unsigned int baseAddress;

    bzero(sharedmemory_phd, 1024); // 共享内存清空

    baseAddress = 0x03;
    vendcommand = 0x02;

    sharedmemory_phd[1] = Tools::MSB(vendcommand);
    sharedmemory_phd[2] = Tools::LSB(vendcommand);

    sharedmemory_phd[0] = 0x01; // enable command

    QElapsedTimer t;
    t.start();

    while (sharedmemory_phd[0] == 0x01 && t.elapsed() < 500)
    {
        // QCoreApplication::processEvents();
    }
    if (t.elapsed() >= 500)
        return false; // timeout
    else
        return true;
}

void MainWindow::ShowPHDdata()
{
    unsigned int currentPHDSizeX = 1;
    unsigned int currentPHDSizeY = 1;
    unsigned int bitDepth = 1;

    unsigned char guideDataIndicator;
    unsigned int guideDataIndicatorAddress;
    double dRa, dDec, SNR, MASS, RMSErrorX, RMSErrorY, RMSErrorTotal, PixelRatio;
    int RADUR, DECDUR;
    char RADIR, DECDIR;
    unsigned char LossAlert;

    double StarX;
    double StarY;
    bool isSelected;

    bool showLockedCross;
    double LockedPositionX;
    double LockedPositionY;

    unsigned char MultiStarNumber;
    unsigned short MultiStarX[32];
    unsigned short MultiStarY[32];

    unsigned int mem_offset;
    int sdk_direction = 0;
    int sdk_duration = 0;
    int sdk_num;
    int zero = 0;

    bool StarLostAlert = false;

    if (sharedmemory_phd[2047] != 0x02)
        return; // if there is no image comes, return

    mem_offset = 1024;
    // guide image dimention data
    memcpy(&currentPHDSizeX, sharedmemory_phd + mem_offset, sizeof(unsigned int));
    mem_offset = mem_offset + sizeof(unsigned int);
    memcpy(&currentPHDSizeY, sharedmemory_phd + mem_offset, sizeof(unsigned int));
    mem_offset = mem_offset + sizeof(unsigned int);
    memcpy(&bitDepth, sharedmemory_phd + mem_offset, sizeof(unsigned char));
    mem_offset = mem_offset + sizeof(unsigned char);

    mem_offset = mem_offset + sizeof(int); // &sdk_num
    mem_offset = mem_offset + sizeof(int); // &sdk_direction
    mem_offset = mem_offset + sizeof(int); // &sdk_duration

    guideDataIndicatorAddress = mem_offset;

    // guide error data
    guideDataIndicator = sharedmemory_phd[guideDataIndicatorAddress];

    mem_offset = mem_offset + sizeof(unsigned char);
    memcpy(&dRa, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&dDec, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&SNR, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&MASS, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);

    memcpy(&RADUR, sharedmemory_phd + mem_offset, sizeof(int));
    mem_offset = mem_offset + sizeof(int);
    memcpy(&DECDUR, sharedmemory_phd + mem_offset, sizeof(int));
    mem_offset = mem_offset + sizeof(int);

    memcpy(&RADIR, sharedmemory_phd + mem_offset, sizeof(char));
    mem_offset = mem_offset + sizeof(char);
    memcpy(&DECDIR, sharedmemory_phd + mem_offset, sizeof(char));
    mem_offset = mem_offset + sizeof(char);

    memcpy(&RMSErrorX, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&RMSErrorY, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&RMSErrorTotal, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&PixelRatio, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&StarLostAlert, sharedmemory_phd + mem_offset, sizeof(bool));
    mem_offset = mem_offset + sizeof(bool);
    memcpy(&InGuiding, sharedmemory_phd + mem_offset, sizeof(bool));
    mem_offset = mem_offset + sizeof(bool);

    mem_offset = 1024 + 200;
    memcpy(&isSelected, sharedmemory_phd + mem_offset, sizeof(bool));
    mem_offset = mem_offset + sizeof(bool);
    memcpy(&StarX, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&StarY, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&showLockedCross, sharedmemory_phd + mem_offset, sizeof(bool));
    mem_offset = mem_offset + sizeof(bool);
    memcpy(&LockedPositionX, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&LockedPositionY, sharedmemory_phd + mem_offset, sizeof(double));
    mem_offset = mem_offset + sizeof(double);
    memcpy(&MultiStarNumber, sharedmemory_phd + mem_offset, sizeof(unsigned char));
    mem_offset = mem_offset + sizeof(unsigned char);
    memcpy(MultiStarX, sharedmemory_phd + mem_offset, sizeof(MultiStarX));
    mem_offset = mem_offset + sizeof(MultiStarX);
    memcpy(MultiStarY, sharedmemory_phd + mem_offset, sizeof(MultiStarY));
    mem_offset = mem_offset + sizeof(MultiStarY);

    sharedmemory_phd[guideDataIndicatorAddress] = 0x00; // have been read back

    glPHD_isSelected = isSelected;
    glPHD_StarX = StarX;
    glPHD_StarY = StarY;
    glPHD_CurrentImageSizeX = currentPHDSizeX;
    glPHD_CurrentImageSizeY = currentPHDSizeY;
    glPHD_LockPositionX = LockedPositionX;
    glPHD_LockPositionY = LockedPositionY;
    glPHD_ShowLockCross = showLockedCross;

    glPHD_Stars.clear();
    for (int i = 0; i < MultiStarNumber; i++)
    {
        if (i > 30)
            break;
        QPoint p;
        p.setX(MultiStarX[i]);
        p.setY(MultiStarY[i]);
        glPHD_Stars.push_back(p);
    }

    if (glPHD_StarX != 0 && glPHD_StarY != 0)
        glPHD_StartGuide = true;

    unsigned int byteCount;
    byteCount = currentPHDSizeX * currentPHDSizeY * (bitDepth / 8);

    mem_offset = 2048;

    unsigned char m = sharedmemory_phd[2047];

    if (sharedmemory_phd[2047] == 0x02 && bitDepth > 0 && currentPHDSizeX > 0 && currentPHDSizeY > 0)
    {
        // 导星过程中的数据
        // qDebug() << guideDataIndicator << "dRa:" << dRa << "dDec:" << dDec
        //          << "rmsX:" << RMSErrorX << "rmsY:" << RMSErrorY
        //          << "rmsTotal:" << RMSErrorTotal << "SNR:" << SNR;
                unsigned char phdstatu;
        call_phd_checkStatus(phdstatu);

        if (dRa != 0 && dDec != 0)
        {
            QPointF tmp;
            tmp.setX(-dRa * PixelRatio);
            tmp.setY(dDec * PixelRatio);
            glPHD_rmsdate.append(tmp);
            //   m_pToolbarWidget->guiderLabel->Series_err->append(-dRa * PixelRatio, -dDec * PixelRatio);
            emit wsThread->sendMessageToClient("AddScatterChartData:" + QString::number(-dRa * PixelRatio) + ":" + QString::number(-dDec * PixelRatio));

            // 曲线的数值
            // qDebug() << "Ra|Dec: " << -dRa * PixelRatio << "," << dDec * PixelRatio;

            // 图像中的小绿框
            if (InGuiding == true)
            {
                // m_pToolbarWidget->LabelMainStarBox->setStyleSheet("QLabel{border:2px solid rgb(0,255,0);border-radius:3px;background-color:transparent;}");
                // m_pToolbarWidget->LabelCrossX->setStyleSheet("QLabel{border:1px solid rgb(0,255,0);border-radius:3px;background-color:transparent;}");
                // m_pToolbarWidget->LabelCrossY->setStyleSheet("QLabel{border:1px solid rgb(0,255,0);border-radius:3px;background-color:transparent;}");
                emit wsThread->sendMessageToClient("InGuiding");
            }
            else
            {
                // m_pToolbarWidget->LabelMainStarBox->setStyleSheet("QLabel{border:2px solid rgb(255,255,0);border-radius:3px;background-color:transparent;}");
                // m_pToolbarWidget->LabelCrossX->setStyleSheet("QLabel{border:1px solid rgb(255,255,0);border-radius:3px;background-color:transparent;}");
                // m_pToolbarWidget->LabelCrossY->setStyleSheet("QLabel{border:1px solid rgb(255,255,0);border-radius:3px;background-color:transparent;}");
                emit wsThread->sendMessageToClient("InCalibration");
            }

            if (StarLostAlert == true)
            {
                // m_pToolbarWidget->LabelMainStarBox->setStyleSheet("QLabel{border:2px solid rgb(255,0,0);border-radius:3px;background-color:transparent;}");
                // m_pToolbarWidget->LabelCrossX->setStyleSheet("QLabel{border:1px solid rgb(255,0,0);border-radius:3px;background-color:transparent;}");
                // m_pToolbarWidget->LabelCrossY->setStyleSheet("QLabel{border:1px solid rgb(255,0,0);border-radius:3px;background-color:transparent;}");
                emit wsThread->sendMessageToClient("StarLostAlert");
            }

            emit wsThread->sendMessageToClient("AddRMSErrorData:" + QString::number(RMSErrorX, 'f', 3) + ":" + QString::number(RMSErrorX, 'f', 3));
        }
        // m_pToolbarWidget->guiderLabel->RMSErrorX_value->setPlainText(QString::number(RMSErrorX, 'f', 3));
        // m_pToolbarWidget->guiderLabel->RMSErrorY_value->setPlainText(QString::number(RMSErrorY, 'f', 3));

        // m_pToolbarWidget->guiderLabel->GuiderDataRA->clear();
        // m_pToolbarWidget->guiderLabel->GuiderDataDEC->clear();

        for (int i = 0; i < glPHD_rmsdate.size(); i++)
        {
            //   m_pToolbarWidget->guiderLabel->GuiderDataRA ->append(i, glPHD_rmsdate[i].x());
            //   m_pToolbarWidget->guiderLabel->GuiderDataDEC->append(i, glPHD_rmsdate[i].y());
            if (i == glPHD_rmsdate.size() - 1)
            {
                emit wsThread->sendMessageToClient("AddLineChartData:" + QString::number(i) + ":" + QString::number(glPHD_rmsdate[i].x()) + ":" + QString::number(glPHD_rmsdate[i].y()));
                if (i > 50)
                {
                    // m_pToolbarWidget->guiderLabel->AxisX_Graph->setRange(i-100,i);
                    emit wsThread->sendMessageToClient("SetLineChartRange:" + QString::number(i - 50) + ":" + QString::number(i));
                }
            }
        }

        unsigned char *srcData = new unsigned char[byteCount];
        mem_offset = 2048;

        memcpy(srcData, sharedmemory_phd + mem_offset, byteCount);
        sharedmemory_phd[2047] = 0x00; // 0x00= image has been read

        cv::Mat img8;
        cv::Mat PHDImg;

        img8.create(currentPHDSizeY, currentPHDSizeX, CV_8UC1);

        if (bitDepth == 16)
            PHDImg.create(currentPHDSizeY, currentPHDSizeX, CV_16UC1);
        else
            PHDImg.create(currentPHDSizeY, currentPHDSizeX, CV_8UC1);

        PHDImg.data = srcData;

        uint16_t B = 0;
        uint16_t W = 65535;

        cv::Mat image_raw8;
        image_raw8.create(PHDImg.rows, PHDImg.cols, CV_8UC1);

        if (AutoStretch == true)
        {
            Tools::GetAutoStretch(PHDImg, 0, B, W);
        }
        else
        {
            B = 0;
            W = 65535;
        }

        Tools::Bit16To8_Stretch(PHDImg, image_raw8, B, W);

        saveGuiderImageAsJPG(image_raw8);

        // saveGuiderImageAsJPG(PHDImg);

        // refreshGuideImage(PHDImg, "MONO");

        int centerX = glPHD_StarX; // Replace with your X coordinate
        int centerY = glPHD_StarY; // Replace with your Y coordinate

        int cropSize = 20; // Size of the cropped region

        // Calculate crop region
        int startX = std::max(0, centerX - cropSize / 2);
        int startY = std::max(0, centerY - cropSize / 2);
        int endX = std::min(PHDImg.cols - 1, centerX + cropSize / 2);
        int endY = std::min(PHDImg.rows - 1, centerY + cropSize / 2);

        // Crop the image using OpenCV's ROI (Region of Interest) functionality
        cv::Rect cropRegion(startX, startY, endX - startX + 1, endY - startY + 1);
        cv::Mat croppedImage = PHDImg(cropRegion).clone();

        // strechShowImage(croppedImage, m_pToolbarWidget->guiderLabel->ImageLable,m_pToolbarWidget->histogramLabel->hisLabel,"MONO",false,false,0,0,65535,1.0,1.7,100,false);
        // m_pToolbarWidget->guiderLabel->ImageLable->setScaledContents(true);

        delete[] srcData;
        img8.release();
        PHDImg.release();
    }
}

void MainWindow::ControlGuide(int Direction, int Duration)
{
    qDebug() << "\033[32m"
             << "ControlGuide: "
             << "\033[0m" << Direction << "," << Duration;
    switch (Direction)
    {
    case 1:
    {
        if (dpMount != NULL)
        {
            indi_Client->setTelescopeGuideNS(dpMount, Direction, Duration);
        }
        break;
    }
    case 0:
    {
        if (dpMount != NULL)
        {
            indi_Client->setTelescopeGuideNS(dpMount, Direction, Duration);
        }
        break;
    }
    case 2:
    {
        if (dpMount != NULL)
        {
            indi_Client->setTelescopeGuideWE(dpMount, Direction, Duration);
        }
        break;
    }
    case 3:
    {
        if (dpMount != NULL)
        {
            indi_Client->setTelescopeGuideWE(dpMount, Direction, Duration);
        }
        break;
    }
    default:
        break; //
    }
}

void MainWindow::getTimeNow(int index)
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 将当前时间点转换为毫秒
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // 将毫秒时间戳转换为时间类型（std::time_t）
    std::time_t time_now = ms / 1000; // 将毫秒转换为秒

    // 使用 std::strftime 函数将时间格式化为字符串
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&time_now));

    // 添加毫秒部分
    std::string formatted_time = buffer + std::to_string(ms % 1000);

    // 打印带有当前时间的输出
    // std::cout << "TIME(ms): " << formatted_time << "," << index << std::endl;
}

void MainWindow::onPHDControlGuideTimeout()
{
    GetPHD2ControlInstruct();
}

void MainWindow::GetPHD2ControlInstruct()
{
    std::lock_guard<std::mutex> lock(receiveMutex);

    unsigned int mem_offset;

    int sdk_direction = 0;
    int sdk_duration = 0;
    int sdk_num = 0;
    int zero = 0;
    mem_offset = 1024;

    mem_offset = mem_offset + sizeof(unsigned int);
    mem_offset = mem_offset + sizeof(unsigned int);
    mem_offset = mem_offset + sizeof(unsigned char);

    int ControlInstruct = 0;

    memcpy(&ControlInstruct, sharedmemory_phd + mem_offset, sizeof(int));
    int mem_offset_sdk_num = mem_offset;
    mem_offset = mem_offset + sizeof(int);

    sdk_num = (ControlInstruct >> 24) & 0xFFF;       // 取前12位
    sdk_direction = (ControlInstruct >> 12) & 0xFFF; // 取中间12位
    sdk_duration = ControlInstruct & 0xFFF;          // 取后12位

    if (sdk_num != 0)
    {
        getTimeNow(sdk_num);
        std::cout << "\033[31m"
                  << "PHD2ControlTelescope: "
                  << "\033[0m" << sdk_num << "," << sdk_direction << ","
                  << sdk_duration << std::endl;
    }
    if (sdk_duration != 0)
    {
        MainWindow::ControlGuide(sdk_direction, sdk_duration);

        memcpy(sharedmemory_phd + mem_offset_sdk_num, &zero, sizeof(int));

        call_phd_ChackControlStatus(sdk_num); // set pFrame->ControlStatus = 0;
    }
}