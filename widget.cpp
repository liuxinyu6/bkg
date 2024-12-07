#include "widget.h"
#include "ui_widget.h"
#include <string>
#include <opencv2/opencv.hpp>
#include <QMouseEvent>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFileDialog>
#include <windows.h>
#include <cstdlib>
#include <cwchar>
#include <thread>
#include <QDebug>
#include <QThread>
#include <QProcess>

const std::string Widget::pic[] = {"jpg", "bmp", "png", "jpeg", "gif"};
const std::string Widget::vdo[] = {"mp4", "avi", "rmvb", "mpg", "mpeg", "mkv"};
const std::string Widget::exe[] = {"exe"};

int Widget::timeval = 200;
int Widget::timevalpic = 5;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),videoPathafter(QString::fromStdString(std::string(""))),videoPath(QString::fromStdString(std::string(""))),
      beStart{false},beChanged{false}, beOffsetDir{false},f_()
{
    ui->setupUi(this);

    connect(ui->selPath, &QPushButton::clicked, this, &Widget::sclicked);
    connect(ui->timectrl, &QPushButton::clicked, this, &Widget::onButtonStart);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::sclicked()
{
    videoPathafter = QFileDialog::getExistingDirectory(this, "选择背景目录");
    ui->textEdit->setText(videoPathafter);
}

void Widget::onButtonStart()
{
    beStart = !beStart;
    if(beStart)
    {
        if(videoPathafter != videoPath)
        {
            beOffsetDir = true;
            videoPath = videoPathafter;

            QString str = videoPath + QString("\\bg\\");
            QDir dir(str);
            if(!dir.exists())
            {
                dir.mkpath(str);
            }
        }
        if(!beChanged)
        {
            onButtonTimeOut();
        }
        ui->timectrl->setText("停用");
    }
    else
    {
        ui->timectrl->setText("启用");
    }
}
void Widget::onButtonTimeOut()
{
   auto func = [&](){
        if(videoPath != QString::fromStdString(std::string("")))
        {
            beChanged = true;
            while(true)
            {    
                while(!beStart)
                {
                    QThread::sleep(1);
                }
                QDir dir(videoPath);

                // 设置过滤器，只显示文件（不包括目录）
                dir.setFilter(QDir::Files);

                // 获取当前目录下的所有文件信息
                QFileInfoList fileList = dir.entryInfoList();

                QString str = videoPath + QString("\\bg\\bg.jpg");
                // 遍历文件列表
                int n = 0;
                for (const QFileInfo &file : fileList)
                {
                    n++;
                    while(!beStart)
                    {
                        QThread::sleep(1);
                    }
                    if(beOffsetDir)
                    {
                        beOffsetDir = false;
                        break;
                    }

                    const char* narrowStr = str.toStdString().c_str();
                    size_t len = std::strlen(narrowStr) + 1; // +1 for the null terminator
                    size_t wcharCount = 0;
                    wchar_t* wideStr = nullptr;

                    // First call to determine the required buffer size
                    mbstowcs_s(&wcharCount, nullptr, 0, narrowStr, len);

                    // Allocate the buffer
                    wideStr = new wchar_t[wcharCount];

                    // Perform the conversion
                    mbstowcs_s(nullptr, wideStr, wcharCount, narrowStr, len);

                    std::string path = file.absoluteFilePath().toStdString();
                    auto pos = path.find_last_of('.');
                    std::string substr = path.substr(pos + 1);
                    bool bpic = false;

                    for (auto& x : Widget::pic)
                    {
                        if (x == substr)
                        {
                            bpic = true;
                            break;
                        }
                    }
                    if(bpic)
                    {
                        cv::Mat mat = cv::imread(file.absoluteFilePath().toStdString());
                        if(!mat.empty())
                        {
                            cv::imwrite(str.toStdString(), mat);

                            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)wideStr, SPIF_UPDATEINIFILE);
                            QThread::sleep(Widget::timevalpic);
                        }
                    }
                    else
                    {
                        bool bvdo = false;
                        for (auto& x : Widget::vdo)
                        {
                            if (x == substr)
                            {
                                bvdo = true;
                                break;
                            }
                        }
                        if(bvdo)
                        {
                            cv::VideoCapture cap(file.absoluteFilePath().toStdString());

                            cv::Mat frame;
                            cap>>frame;
                            double fps = 1;
                            if(!frame.empty())
                            {
                                fps = cap.get(cv::CAP_PROP_FPS);
                            }
                            else
                            {
                                continue;
                            }
                            while(true)
                            {
                                while(!beStart)
                                {
                                    QThread::sleep(1);
                                }
                                if(beOffsetDir)
                                {
                                    if(n == fileList.size())
                                    {
                                        beOffsetDir = false;
                                    }
                                    break;
                                }
                                if (frame.empty())
                                {
                                   break;
                                }

                                cv::imwrite(str.toStdString(), frame);

                                SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)wideStr, SPIF_UPDATEINIFILE);

                                QThread::msleep(Widget::timeval);
                                cap >> frame;
                            }
                            cap.release();

                            QThread::sleep(Widget::timevalpic);
                        }
                        else
                        {
#ifdef _WINDOWS_
                            bool getexe = false;
                            for (auto& x : Widget::exe)
                            {
                                if (x == substr)
                                {
                                    getexe = true;
                                    break;
                                }
                            }
                            if(getexe)
                            {
                                QProcess th;
                                th.start(file.absoluteFilePath(), nullptr);
                                th.waitForStarted();
                                th.waitForFinished();
                                QThread::sleep(Widget::timevalpic);
                            }
#endif
                        }
                    }
                    delete[] wideStr; // Don't forget to clean up
                }
           }
        }
   };
   f_ = std::thread(func);
   f_.detach();
}
