#ifndef STANDARTVIEW_H
#define STANDARTVIEW_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>

namespace Ui {
  class StandartView;
}

class StandartView : public QDialog
{
  Q_OBJECT

public:
  explicit StandartView(QWidget *parent, const QList<QString>& pages, const QString& name);
  ~StandartView();

signals:
  void inc();
  void range(int);
private:
  Ui::StandartView *ui;
  QLabel *lbl;
  QVBoxLayout *lay;
  QWidget *widget;
  QTimer *timer;
private slots:
  void setSpeed(int);
  void autoReadOn();
  void autoReadOff();
  void step();
};

#endif // STANDARTVIEW_H
