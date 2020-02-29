#include "standartview.h"
#include "ui_standartview.h"
#include <QScrollBar>
#include <QScreen>
#include <QPushButton>
#include <QTimer>

StandartView::StandartView(QWidget *parent, const QList<QString>& pages, const QString& name) :
    QDialog (parent),
    ui(new Ui::StandartView)
{
    //Узнаем размер экрана
    //---------------------------------
    QScreen* screen = QApplication::screens().at(0);
    QSize screen_size = screen->availableSize();
    //---------------------------------

    //Настраиваем GUI
    //---------------------------------
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    if(pages.count() == 0)
        close();
    //Пишем имя окна
    this->setWindowTitle(name);
    //Выставляем максимальнодоступную высоту окна
    this->setFixedHeight(screen_size.height() - 45);
    //Отключаем горизантальный скролл
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->horizontalSlider->setRange(ui->scrollArea->verticalScrollBar()->singleStep(), 100);
    ui->AutoRead->setText("Включить");
    connect(ui->AutoRead,SIGNAL(clicked()), SLOT(autoReadOn()));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), SLOT(setSpeed(int)));
    connect(ui->commandLinkButton, SIGNAL(clicked()), parent, SLOT(nextBook()));
    connect(ui->commandLinkButton, SIGNAL(clicked()), SLOT(close()));
    //connect(ui->pushButton_2, SIGNAL(clicked()), parent, SLOT(prevBook()));
    //connect(ui->pushButton_2, SIGNAL(clicked()), SLOT(close()));
    //---------------------------------

    //Настраиваем таймер
    //---------------------------------
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(step()));
    //---------------------------------

    //Создаем виджет и слой для него
    //Настраиваем
    //---------------------------------
    widget = new QWidget(this);
    lay = new QVBoxLayout(widget);
    lay->setSpacing(1);
    ui->scrollArea->setWidget(widget);
    //---------------------------------

    //Натягиваем картинки на лейбл и отправляем их на слой
    //---------------------------------
    //Ссылка на предыдущий лейбл
    QPixmap t_img(pages.at(pages.size()/2));
    QPixmap st_img(pages.at((pages.size()/2) + 1));
    if(t_img.width() > st_img.width())
    {
        t_img = st_img;
    }
    if(t_img.width() > screen_size.width() - 20)
    {
        t_img = t_img.scaled(screen_size.width() - 35, t_img.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    this->setFixedWidth(t_img.width() + 50);
    for(const auto & page : pages)
    {
        //Создаем лейбл для натягивания картинок
        //И загружаем саму картинку
        //---------------------------------
        lbl = new QLabel(widget);
        QPixmap img(page);
        //---------------------------------

        //Адаптируем картинки по размеру
        //---------------------------------
        img = img.scaled(t_img.width(), img.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        lbl->setPixmap(img);
        lay->addWidget(lbl);
        //---------------------------------
    }
    ui->scrollArea->verticalScrollBar()->setRange(0, 4000);
    ui->AutoReadSpeed->setRange(0, 10);
    ui->AutoReadSpeed->setSingleStep(1);
    //---------------------------------
}

StandartView::~StandartView()
{
    delete widget;
    delete ui;
}

void StandartView::setSpeed(int value)
{
    ui->scrollArea->verticalScrollBar()->setSingleStep(value);
}

void StandartView::autoReadOn()
{
    timer->start(20);
    ui->AutoRead->setText("Отключить");
    disconnect(ui->AutoRead,SIGNAL(clicked()), this, SLOT(autoRaedOn()));
    connect(ui->AutoRead,SIGNAL(clicked()), SLOT(autoReadOff()));
}

void StandartView::autoReadOff()
{
    timer->stop();
    ui->AutoRead->setText("Включить");
    disconnect(ui->AutoRead,SIGNAL(clicked()), this, SLOT(autoRaedOff()));
    connect(ui->AutoRead,SIGNAL(clicked()), SLOT(autoReadOn()));
}

void StandartView::step()
{
    ui->scrollArea->verticalScrollBar()->setValue(
                ui->scrollArea->verticalScrollBar()->value()
                + ui->AutoReadSpeed->value());
    timer->start(20);
}
