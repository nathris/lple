#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void loadFile();
    void saveFile();
    void newFile();
    void addRow();
    void addExistingRom();
    void makeUniform();
    void deleteRow();

private:
    Ui::MainWindow *ui;

    QStandardItemModel* p_model;
    QString currentDir;

    void setHeaders();
};

#endif // MAINWINDOW_H
