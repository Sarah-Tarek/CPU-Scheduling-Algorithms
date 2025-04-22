#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QDialog>
#include <QVector>
#include "process.h"
#include <QTimer>
#include <QGraphicsScene>
#include <thread>


namespace Ui {
class SecondWindow;
}

class SecondWindow : public QDialog
{
    Q_OBJECT

public:
    static SecondWindow *instance;
    explicit SecondWindow(QWidget *parent = nullptr);
    //void runAlgorithm(const QString &algorithmName);
    void setProcesses(const QVector<Process>& p );
    void updateGanttChart();
    void startSimulation(const QString &algorithm);

    ~SecondWindow();



signals:
    /// emitted any time the averages change
    void statsUpdated(double avgWaiting, double avgTurnaround);

private slots:
    void onStatsUpdated(double avgWaiting, double avgTurnaround);

    void on_finishButton_clicked();



    void on_resetButton_clicked();

private:
    Ui::SecondWindow *ui;
    QVector<Process> localProcesses;

    QGraphicsScene *scene;
    QTimer *timer;
    int currentX = 0;
    int chartX = 0;
    QMap<QString, QColor> processColors;
    int totalChartWidth = 0; // Track total width of the Gantt chart

    QColor getColorForProcess(const QString &name);

    std::thread readyQueueThread;



    std::thread liveTableThread;
    std::thread schedulerThread;

    void setupLiveTable();
    void fillTable();
    void liveTableChart();
    void dynamicAddLoop();
    void runAlgorithm(const QString &algorithm);
    void onAddProcessClicked();
    void updateRemainingTimes();
    void updateGanttChart1();
};

#endif // SECONDWINDOW_H
