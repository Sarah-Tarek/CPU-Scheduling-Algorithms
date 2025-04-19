#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "secondwindow.h"
#include "process.h"
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->PriorityLineEdit->setVisible(false);
    ui->priorityLabel->setVisible(false);
    ui->quantumLineEdit->setVisible(false);
    ui->quantumLabel->setVisible(false);

    ui->processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_start_clicked()
{
    secondWindow = new SecondWindow(this);
    secondWindow->show();


    if (processes.isEmpty()) {
        QMessageBox::warning(this, "No Processes", "Please add at least one process.");
        return;
    }

    secondWindow->setProcesses(processes);

    secondWindow->startSimulation(algorithm);


    this->hide();//hide main window
}



void MainWindow::on_addProcessButton_clicked()
{
    QString id = ui->idLineEdit->text();
    int arrival = ui->arrivalLineEdit->text().toInt();
    int burst = ui->burstLineEdit->text().toInt();
    priority = 0;
    quantum = 0;
    priority = ui->PriorityLineEdit->text().toInt();
    quantum = ui->quantumLineEdit->text().toInt();


     algorithm = ui->algorithmComboBox->currentText();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter Process ID.");
        return;
    }

    if (id.isEmpty() || arrival < 0 || burst <= 0 || priority <0 || quantum <0) {
        QMessageBox::warning(this, "Invalid Input", "Please enter valid process data.");
        return;
    }


    if (storedQuantum == -1) {
        storedQuantum = quantum;
    } else if (quantum != storedQuantum) {
            QMessageBox::warning(this, "Invalid Quantum", "Quantum must match the first entered value.");
            return;
    }



    Process p(id.toStdString(), arrival, burst); // the right function Process p(id, arrival, burst);
    p.priority = priority;
    p.quantum = quantum ;
    processes.append(p);

    // Lock the algorithm after adding the first process
    if (processes.size() == 1) {
        ui->algorithmComboBox->setEnabled(false);
    }

    if (processes.isEmpty()) {
        ui->algorithmComboBox->setEnabled(true);  // allow selecting another algorithm
        ui->quantumLineEdit->setEnabled(true);    // allow setting new quantum value
    }

    int row = ui->processTable->rowCount();
    ui->processTable->insertRow(row);
    ui->processTable->setItem(row, 0, new QTableWidgetItem(id));
    ui->processTable->setItem(row, 1, new QTableWidgetItem(QString::number(arrival)));
    ui->processTable->setItem(row, 2, new QTableWidgetItem(QString::number(burst)));
    //ui->processTable->setItem(row, 3, new QTableWidgetItem(QString::number(priority)));
   // ui->processTable->setItem(row, 4, new QTableWidgetItem(QString::number(quantum)));

    if (algorithm == "Priority Preemptive" || algorithm == "Priority Nonpreemptive") {
        ui->processTable->setItem(row, 3, new QTableWidgetItem(QString::number(priority)));
    }
    else if (algorithm == "Round Robin") {
        ui->processTable->setItem(row, 3, new QTableWidgetItem(QString::number(quantum)));
    }

    // Clear inputs
    ui->idLineEdit->clear();
    ui->arrivalLineEdit->clear();
    ui->burstLineEdit->clear();
    ui->PriorityLineEdit->clear();
    ui->quantumLineEdit->clear();
}


void MainWindow::on_deleteProcessButton_clicked()
{
    int row = ui->processTable->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a process to delete.");
        return;
    }

    // Remove from UI table
    ui->processTable->removeRow(row);

    // Safely remove from internal list
    if (row >= 0 && row < processes.size()) {
        processes.removeAt(row);
    }

    // Optional: re-enable controls if list is empty
    if (processes.isEmpty()) {
        ui->algorithmComboBox->setEnabled(true);
        ui->quantumLineEdit->setEnabled(true);
        storedQuantum =-1;

    }
}


void MainWindow::on_algorithmComboBox_currentTextChanged(const QString &text)
{
    // Show/hide input fields based on selected algorithm
    bool isPriority = (text == "Priority Preemptive" || text == "Priority Nonpreemptive");
    bool isRR = (text == "Round Robin");

    ui->PriorityLineEdit->setVisible(isPriority);
    ui->priorityLabel->setVisible(isPriority);

    ui->quantumLineEdit->setVisible(isRR);
    ui->quantumLabel->setVisible(isRR);


    if(isPriority){
        ui->processTable->setColumnCount(4);
        ui->processTable->setHorizontalHeaderItem(3, new QTableWidgetItem("Priority"));
        //ui->processTable->setItem(row, 3, new QTableWidgetItem(QString::number(priority)));
    }

    if(isRR){
        ui->processTable->setColumnCount(4);
        ui->processTable->setHorizontalHeaderItem(3, new QTableWidgetItem("quantum"));
        //ui->processTable->setItem(row, 3, new QTableWidgetItem(QString::number(quantum)));
    }




}




