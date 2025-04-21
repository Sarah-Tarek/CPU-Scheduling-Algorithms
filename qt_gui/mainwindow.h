#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "secondwindow.h"
#include <QVector>
#include "process.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void resetProcessTable();

    ~MainWindow();


private slots:
    void on_nonLiveButton_clicked();

    void on_pushButton_start_clicked();

    void on_addProcessButton_clicked();

    void on_deleteProcessButton_clicked();

    void on_algorithmComboBox_currentTextChanged(const QString &arg1);



private:
    Ui::MainWindow *ui;
    SecondWindow *secondWindow = nullptr;
    //SecondWindow *secondWindow;
    QVector<Process> processes;
    QString algorithm;
    int priority;
    int quantum;
    int row;
    int storedQuantum = -1;


    void show_second_window();

};
#endif // MAINWINDOW_H
