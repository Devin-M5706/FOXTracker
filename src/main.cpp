#include "mainwindow.h"
#include "HeadPoseDetector.h"
#include <QApplication>
#include <opencv2/opencv.hpp>
#include<QDebug>
#include "FlightAgxSettings.h"

FlightAgxSettings * settings = nullptr;

#ifdef Q_OS_WIN
#   ifdef main
#      undef main
#   endif
#endif

int main(int argc, char *argv[])
{

    qRegisterMetaType<Pose_>("Pose_");
    qRegisterMetaType<Pose6DoF>("Pose6DoF");
    qRegisterMetaType<Eigen::Vector3d>("Eigen::Vector3d");
    qRegisterMetaType<Matrix19d>("Matrix19d");

    qInfo() << "Welcome! pliots";
    if (argc > 1 && std::string(argv[1]) == "small") {
    } else {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }

    QApplication a(argc, argv);

    settings = new FlightAgxSettings;

    MainWindow w;
    w.show();
    return a.exec();
}
