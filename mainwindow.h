#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "standartview.h"
#include "data.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void nextBook();
    void prevBook();
private slots:
    void on_close_action_triggered();
    void on_file_action_triggered();
    void on_dir_action_triggered();
    void on_root_dir_triggered();
    void clickedFromMenu_Folder();
    void clickedFromMenu_Dir();
    void clickedFromMenu_Zip();
    void clickedFromMenu_Back();
    void on_resize300_500_triggered();
    void on_resize500_800_triggered();
    void on_cln1_triggered();
    void on_cln2_triggered();
    void on_cln3_triggered();
    void on_cln4_triggered();
    void on_update_triggered();
    void on_history_triggered();

private:
    void readToFolder(const QString& path);
    void extractZip(const QString& path);
    QPixmap extractCover(const QString &path);
    void deleteTempFolder();
    void loadMenu();
    void clearMenu();
    void createBackButton(int row);
    void createLessMoreButton(bool isMore, int row);

private:
    Ui::MainWindow *ui;
    QList<QString> *pages;
    QList<QString> *link;
    QWidget *menu;
    QGridLayout *mainLay;
    StandartView *view;
    QString root_dir;
    Data *data;
    int cln, icon_width, icon_height;
    int now_reading;
};

#endif // MAINWINDOW_H
