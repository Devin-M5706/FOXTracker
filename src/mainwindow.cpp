#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <FlightAgxSettings.h>
#include <QAction>
#include <QMenu>
#include "ekfconfig.h"
#include "agentxconfig.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("FOXTracker");

    this->hd.main_window = this;

    is_running = false;
    this->on_endbutton_clicked();
    config_menu = new AgentXConfig;


    connect(&remapper, &PoseRemapper::send_mapped_posedata, this, &MainWindow::on_pose6d_data);
    connect(&remapper, &PoseRemapper::send_mapped_posedata, &data_sender, &PoseDataSender::on_pose6d_data);

    connect(&hd, &HeadPoseDetector::on_detect_pose, &remapper, &PoseRemapper::on_pose_data, Qt::QueuedConnection);

    connect(config_menu, &AgentXConfig::reset_camera, &hd, &HeadPoseDetector::reset);
    connect(config_menu, &AgentXConfig::recenter_hotkey_pressed, this, &MainWindow::on_center_keyboard_event);
    connect(config_menu, &AgentXConfig::set_camera_gain, &hd, &HeadPoseDetector::set_gain);
    connect(config_menu, &AgentXConfig::set_camera_expo, &hd, &HeadPoseDetector::set_expo);
    connect(config_menu, &AgentXConfig::set_camera_auto_expo, &hd, &HeadPoseDetector::set_auto_expo);

//    connect(&hd, &HeadPoseDetector::on_detect_pose6d, config_menu->ekf_config_menu(),
//            &EKFConfig::on_detect_pose6d);
//    connect(&hd, &HeadPoseDetector::on_detect_pose6d_raw, config_menu->ekf_config_menu(),
//           &EKFConfig::on_detect_pose6d_raw);
//    connect(&hd, &HeadPoseDetector::on_detect_twist, config_menu->ekf_config_menu(),
//            &EKFConfig::on_detect_twist);

    ui->time_disp->setDigitCount(5);
    ui->time_disp->setSmallDecimalPoint(false);
    ui->x_disp->setDigitCount(4);
    ui->y_disp->setDigitCount(4);
    ui->z_disp->setDigitCount(4);
    ui->yaw_disp->setDigitCount(3);
    ui->pitch_disp->setDigitCount(3);
    ui->roll_disp->setDigitCount(3);

    ui->fps_disp->setDigitCount(3);

    hotkeyManager = new UGlobalHotkeys();
    hotkeyManager->registerHotkey("alt+c", 1);

    connect(hotkeyManager, &UGlobalHotkeys::activated, this, &MainWindow::handle_global_hotkeys);


}

void MainWindow::handle_global_hotkeys(unsigned int _id) {
    if (_id == 1) {
        this->on_center_keyboard_event();
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_show() {
   //start_camera_preview();
   QTimer::singleShot(0, this, SLOT(show()));
}

void MainWindow::on_exit() {
   QCoreApplication::quit();
}

void MainWindow::create_tray_icon() {
    if (m_tray_icon == nullptr) {
        m_tray_icon = new QSystemTrayIcon(QIcon(":/icon.png"), this);

        connect( m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_show_hide(QSystemTrayIcon::ActivationReason)) );

        QAction *quit_action = new QAction( "Exit", m_tray_icon );
        connect( quit_action, SIGNAL(triggered()), this, SLOT(on_exit()) );

        QAction *hide_action = new QAction( "Show", m_tray_icon );
        connect( hide_action, SIGNAL(triggered()), this, SLOT(on_show()) );

        connect(m_tray_icon, SIGNAL(DoubleClick()), this, SLOT(on_show()));
        connect(m_tray_icon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this ,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

        QMenu *tray_icon_menu = new QMenu;
        tray_icon_menu->addAction( hide_action );
        tray_icon_menu->addAction( quit_action );

        m_tray_icon->setContextMenu( tray_icon_menu );

        m_tray_icon->show();
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
       case QSystemTrayIcon::Trigger:
       case QSystemTrayIcon::DoubleClick:
            this->on_show();
       default:
        break;
    }
}

void MainWindow::DisplayImage() {
    cv::Mat & img = hd.get_preview_image();
//    if (!img.empty()) {
//        cv::imshow("FOXTracker Preview", img);
//        cv::waitKey(10);
//    }
    QImage imdisplay((uchar*)img.data, img.cols, img.rows, img.step, QImage::Format_BGR888); //Converts the CV image into Qt standard format
    ui->preview_camera->setPixmap(QPixmap::fromImage(imdisplay));//display the image in label that is created earlier
}


void MainWindow::on_pose6d_data(double t, Pose6DoF _pose) {
//    qDebug() << "Pose 6D!!!" << t;
    static double t_last = 0;
    static double fps = 0;
    if (t_last != 0) {
        fps = 1/(t - t_last)*0.05 + fps*0.95;
    }
    t_last = t;
    ui->time_disp->display(t);
    ui->x_disp->display(_pose.second.x() * 100);
    ui->y_disp->display(_pose.second.y() * 100);
    ui->z_disp->display(_pose.second.z() * 100);

    ui->yaw_disp->display(_pose.first.x());
    ui->pitch_disp->display(_pose.first.y());
    ui->roll_disp->display(_pose.first.z());

    ui->fps_disp->display(fps);
}

void MainWindow::on_endbutton_clicked()
{
    if(is_running) {
        emit hd.stop();
        this->stop_camera_preview();
        is_running = false;
        ui->endbutton->setText("START");
    } else {
        emit hd.start();
        this->remapper.reset_center();
        this->start_camera_preview();
        is_running = true;
        ui->endbutton->setText("STOP");
    }
}

void MainWindow::start_camera_preview() {
    Timer = new QTimer(this);
    settings->enable_preview = true;
    connect(Timer, SIGNAL(timeout()), this, SLOT(DisplayImage()));
    Timer->start(30);
    camera_preview_enabled = true;
}

void MainWindow::stop_camera_preview() {
    settings->enable_preview = false;
    camera_preview_enabled = false;
    Timer->stop();
}

void MainWindow::on_toggle_preview_clicked()
{
    if (camera_preview_enabled) {
        this->stop_camera_preview();
    } else {
        this->start_camera_preview();
    }
}

void MainWindow::on_gamemode_clicked()
{
    this->stop_camera_preview();
    create_tray_icon();
    QTimer::singleShot(10, this, SLOT(hide()));
}

void MainWindow::on_config_button_clicked()
{
    config_menu->show();
}



void MainWindow::on_center_keyboard_event() {
//    hd.reset_detect();
    remapper.reset_center();
}


void MainWindow::on_pause_clicked()
{
    hd.pause();
    if(Timer->isActive()) {
        Timer->stop();
    } else {
        Timer->start();
    }
}

void MainWindow::on_centerButton_clicked()
{
    on_center_keyboard_event();
}

void MainWindow::on_always_on_top_clicked()
{
    if (is_always_on_top) {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        this->setWindowFlags(Qt::WindowTitleHint);
        is_always_on_top = false;
        this->show();
    } else {
        QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
        this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        is_always_on_top = true;
        this->show();
    }
}

void MainWindow::on_quit_clicked()
{
    QCoreApplication::quit();
}
