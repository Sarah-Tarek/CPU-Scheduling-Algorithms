#include "secondwindow.h"
#include "ui_secondwindow.h"
#include "FCFS.h"
#include "SJF_NonPreemptive.h"
#include "SJF_Preemptive.h"
#include "priority_nonpreemptive.h"
#include "priority_preemptive.h"
#include "RoundRoubin.h"
#include <QMessageBox>
#include "global_variables.h"
#include "job_to_ready.h"
#include<thread>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QString>
#include <QScrollBar>

SecondWindow::SecondWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SecondWindow)
{
    ui->setupUi(this);

    this->setSizeGripEnabled(true); // allows resizing
    this->setWindowFlags(windowFlags() & ~Qt::MSWindowsFixedSizeDialogHint);

    // Match the size of the parent (MainWindow)
    if (parent)
        this->resize(parent->size());
    connect(ui->addProcessButton, &QPushButton::clicked, this, &SecondWindow::onAddProcessClicked);
}

SecondWindow::~SecondWindow()
{
    delete ui;
}
void SecondWindow::startSimulation(const QString &algorithm){
    setupLiveTable();

    fillTable();

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    if (algorithm == "Priority Preemptive" || algorithm == "Priority Nonpreemptive") {
        ui->Process_Priority->setVisible(true);
        ui->label_Priority->setVisible(true);
    } else {
        ui->Process_Priority->setVisible(false);
        ui->label_Priority->setVisible(false);
    }

    readyQueueThread = std::thread([]() {
        addToReadyQueue();
    });



    schedulerThread = std::thread([this, algorithm]() {
        runAlgorithm(algorithm);
    });
/*
    liveTableThread = std::thread([this]() {
        liveTableChart();
    });
*/
    // Start thread : Gantt chart via Qt Timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SecondWindow::updateGanttChart);
    timer->start(1000);  // every 1s

    readyQueueThread.detach();
    schedulerThread.detach();
}

void SecondWindow::runAlgorithm(const QString &algorithmName)
{
    if (algorithmName == "FCFS") {
        FCFS();
    }
    else if (algorithmName == "SJF Nonpreemptive") {
        SJF_NonPreemptive();
    }
    else if (algorithmName == "SJF Preemptive") {
        Sjf_Preemptive_Schedular();
    }
    else if (algorithmName == "Priority Preemptive") {
        priority_preemptive();
    }
    else if (algorithmName == "Priority Nonpreemptive") {
        Priority_NonPreemptive();
    }
    else if (algorithmName == "Round Robin") {
        roundRobin();
    }
    else {
        QMessageBox::warning(this, "Error", "Unknown algorithm selected!");
    }


}

void SecondWindow::setProcesses(const QVector<Process>& p){
    localProcesses = p;
    for (const Process &proc : localProcesses)
        jobQueue.push(proc);
}

void SecondWindow::onAddProcessClicked()
{
    bool burstOk, priorityOk = true;

    QString pid = ui->Process_ID->text();
    int burst = ui->Process_Burst->text().toInt(&burstOk);

    int priority = -1;

    // If priority is visible, read value
    if (ui->Process_Burst->isVisible()) {
        priority = ui->Process_Burst->text().toInt(&priorityOk);
        if (!priorityOk) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a valid Priority value.");
            return;
        }
    }

    if (pid.isEmpty() || !burstOk || burst <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid string PID and positive Burst Time.");
        return;
    }
    std::string Spid = pid.toStdString();
    Process newprocess(Spid, currentTime, burst);
    newprocess.priority=priority;

    localProcesses.append( newprocess);
    { // Lock the readyQueue to safely add the new process (thread-safe access)
        lock_guard<mutex> lock(mtx_readyQueue);

        // Add the new process to the readyQueue
        readyQueue.push(newprocess);
    }
     cv_readyQueue.notify_one();

    // now update the table
    int row = ui->liveTable->rowCount();
    ui->liveTable->insertRow(row);
    // convert back to QString for display:
    ui->liveTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(Spid)));

    ui->liveTable->setItem(row, 1, new QTableWidgetItem(QString::number(newprocess.arrivalTime)));
    ui->liveTable->setItem(row, 2, new QTableWidgetItem(QString::number(newprocess.burstTime)));

}

void SecondWindow::setupLiveTable()
{
    ui->liveTable->setColumnCount(3);
    ui->liveTable->setHorizontalHeaderLabels({"PID","Arrival Time", "Remaining Time"});
    ui->liveTable->horizontalHeader()->setStretchLastSection(true);
}

void SecondWindow::fillTable()
{
    ui->liveTable->setRowCount(localProcesses.size());
    for (int i = 0; i < localProcesses.size(); ++i) {
        const Process &p = localProcesses[i];
        ui->liveTable->setItem(i, 0,
                               new QTableWidgetItem(QString::fromStdString(p.id)));
        ui->liveTable->setItem(i, 1,
                               new QTableWidgetItem(QString::number(p.arrivalTime)));
        ui->liveTable->setItem(i,2,
                               new QTableWidgetItem(QString::number(p.burstTime)));
    }
}

void SecondWindow::liveTableChart() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Sleep before checking

        std::lock_guard<std::mutex> lock(mtx_table);

        if (!table.empty()) {
            for (const auto& entry : table) {
                const std::string& pid = entry.second.id;
                int remTime = entry.second.remainingTime;

                /************************** Aziza *******************************/
                int blockWidth = 50;

                // Add a new rectangle for the process
                QColor color = getColorForProcess(QString::fromStdString(pid));
                QGraphicsRectItem* block = scene->addRect(chartX * blockWidth, 0, blockWidth, 50, QPen(Qt::black), QBrush(color));

                // Add text labels for the process
                QGraphicsTextItem* label = scene->addText(QString::fromStdString(pid));
                label->setPos(chartX * blockWidth + 10, 10);

                QGraphicsTextItem* timeLabel = scene->addText(QString::number(chartX));
                timeLabel->setPos(chartX * blockWidth, 55);

                ++chartX;

                totalChartWidth = chartX * blockWidth;

                // Automatically scroll the chart to the right if the total width exceeds the view width
                if (totalChartWidth > ui->graphicsView->width()) {
                    // Scroll the view horizontally to the right
                    int shiftAmount = totalChartWidth - ui->graphicsView->width();
                    ui->graphicsView->horizontalScrollBar()->setValue(shiftAmount);
                }

                // Ensure the scene is large enough to accommodate the growing chart
                scene->setSceneRect(0, 0, totalChartWidth, 100);  // Adjust the height as necessary

                /***************************************************************/




                // Update the table on the GUI thread
                QMetaObject::invokeMethod(this, [=]() {
                    // Loop over the rows to find matching PID
                    for (int row = 0; row < ui->liveTable->rowCount(); ++row) {
                        QTableWidgetItem* pidItem = ui->liveTable->item(row, 0);  // PID column
                        if (pidItem && pidItem->text() == QString::fromStdString(pid)) {
                            QTableWidgetItem* remItem = ui->liveTable->item(row, 2);  // Remaining time col
                            if (!remItem) {
                                remItem = new QTableWidgetItem();
                                ui->liveTable->setItem(row, 2, remItem);
                            }
                            remItem->setText(QString::number(remTime));  // Update remaining time
                            break;
                        }
                    }
                }, Qt::QueuedConnection);
            }


            table.clear();
        }
    }
}

void SecondWindow::updateGanttChart()
{
    std::lock_guard<std::mutex> lock(mtx_table);

    if (table.count(chartX)) {
        Process p = table[chartX];

        int blockWidth = 50;

        // Add a new rectangle for the process
        QColor color = getColorForProcess(QString::fromStdString(p.id));
        QGraphicsRectItem* block = scene->addRect(chartX * blockWidth, 0, blockWidth, 50, QPen(Qt::black), QBrush(color));

        // Add text labels for the process
        QGraphicsTextItem* label = scene->addText(QString::fromStdString(p.id));
        label->setPos(chartX * blockWidth + 10, 10);

        QGraphicsTextItem* timeLabel = scene->addText(QString::number(chartX));
        timeLabel->setPos(chartX * blockWidth, 55);



        /************************** doaa *******************************/

        int remTime = p.remainingTime;

        // Update the table on the GUI thread
        QMetaObject::invokeMethod(this, [=]() {
            // Loop over the rows to find matching PID
            for (int row = 0; row < ui->liveTable->rowCount(); ++row) {
                QTableWidgetItem* pidItem = ui->liveTable->item(row, 0);  // PID column
                if (pidItem && pidItem->text() == QString::fromStdString(p.id)) {
                    QTableWidgetItem* remItem = ui->liveTable->item(row, 2);  // Remaining time col
                    if (!remItem) {
                        remItem = new QTableWidgetItem();
                        ui->liveTable->setItem(row, 2, remItem);
                    }
                    remItem->setText(QString::number(remTime));  // Update remaining time
                    break;
                }
            }
        }, Qt::QueuedConnection);

        /************************** **** *******************************/

        table.erase(chartX);
        ++chartX;

        totalChartWidth = chartX * blockWidth;

        // Automatically scroll the chart to the right if the total width exceeds the view width
       if (totalChartWidth > ui->graphicsView->width()) {
            // Scroll the view horizontally to the right
            int shiftAmount = totalChartWidth - ui->graphicsView->width();
            ui->graphicsView->horizontalScrollBar()->setValue(shiftAmount);
        }

        // Ensure the scene is large enough to accommodate the growing chart
        scene->setSceneRect(0, 0, totalChartWidth, 100);  // Adjust the height as necessary
    }
}





QColor SecondWindow::getColorForProcess(const QString& processName)
{
    if (!processColors.contains(processName)) {
        QColor color = QColor::fromHsv(processColors.size() * 50 % 360, 200, 255);
        processColors[processName] = color;
    }
    return processColors[processName];
}
