#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QList>
#include <QLabel>
#include <QtGui/private/qzipreader_p.h>
#include <QFileDialog>
#include <QMessageBox>
#include "standartview.h"

//Конструктор
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Узнаем размер экрана
    //---------------------------------
    QScreen* screen = QApplication::screens().at(0);
    QSize screen_size = screen->availableSize();
    //---------------------------------

    //Настраиваем графический интерфейс
    //---------------------------------
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->statusBar->setStyleSheet("QStatusBar { color: white }");
    //Задаем размеры и надписи окна
    this->setMinimumHeight(screen_size.height() / 100 * 60);
    this->setWindowTitle("Read Manga");
    QPixmap img(":/img/icon.png");
    this->setWindowIcon(img);
    this->setStyleSheet("QMainWindow { background: url(:/img/background.jpg)}");
    cln = 4;
    icon_width = 300;
    icon_height = 500;
    now_reading = -1;
    //---------------------------------

    //Подгружаем настройки
    //---------------------------------
    QFile settings(QApplication::applicationDirPath() + "/settings");
    QTextStream stream(&settings);
    settings.open(QFile::ReadOnly);
    if(settings.exists())
    {
        root_dir = stream.readAll();
        settings.close();
    }
    else //Если настроек нету
    {
        root_dir = "";
    }
    //---------------------------------

    //Выделяем память под страницы и дерево
    //---------------------------------
    pages = new QList<QString>;
    link = new QList<QString>;
    data = new Data(root_dir);
    //---------------------------------

    //Подгружаем меню
    //---------------------------------
    loadMenu();
    //Выводим сообщение о готовности
    ui->statusBar->showMessage("Готов к чтению!");
    //---------------------------------
    QMessageBox::information(this, "Спасибо!", "Спасибо за интерес к приложению!\nПрошу, все пожелания и замечания, а также ошибки отправлять мне. \n"
                                               "Без вас, это приложение не сможет развиваться!\n"
                                               "Помогите мне сделать то, чего хотите именно ВЫ.\n\n"
                                               "P.S. Приложение в будущем сможет работать с сайтами, но до этого момента, надо настроить его локальную версию.");
}
//Диструктор
MainWindow::~MainWindow()
{
    clearMenu();
    delete link;
    delete data;
    delete ui;
}

void MainWindow::nextBook()
{
    if(data->at(++now_reading).isZip)
    {
        //Извлекаем архив
        extractZip(link->at(now_reading));

        //Читаем извлеченное
        //Удаляем временную папку
        //---------------------------------
        readToFolder(QApplication::applicationDirPath() + "/" + link->back());
        deleteTempFolder();
        //---------------------------------
    }
    else if(data->at(now_reading).isLast)
    {
        //Читаем из папки по ссылке
        readToFolder(link->at(now_reading));
    }
}

void MainWindow::prevBook()
{
    if(data->at(--now_reading).isZip)
    {
        //Извлекаем архив
        extractZip(link->at(now_reading));

        //Читаем извлеченное
        //Удаляем временную папку
        //---------------------------------
        readToFolder(QApplication::applicationDirPath() + "/" + link->back());
        deleteTempFolder();
        //---------------------------------
    }
    else if(data->at(now_reading).isLast)
    {
        //Читаем из папки по ссылке
        readToFolder(link->at(now_reading));
    }
}

//Метод распаковки обложки
QPixmap MainWindow::extractCover(const QString& path)
{
    QPixmap img;
    QZipReader zip(path);
    if(zip.exists())
    {
      //Просматриваем файлы архива
      for(const auto& zipExtract : zip.fileInfoList())
      {
        //Ищем первое иззображение
        if(zipExtract.filePath == "001.png" ||
           zipExtract.filePath == "001.jpg" ||
           zipExtract.filePath == "01.png" ||
           zipExtract.filePath == "01.jpg" ||
           zipExtract.filePath == "1.png" ||
           zipExtract.filePath == "1.jpg")
        {
          //Извлекаем первое изоображение
          img.loadFromData((zip.fileData(zipExtract.filePath)));
          //Выходим из цикла поиска по архиву
          break;
        }
      }
    }
    zip.close();
    return img;
}
//Метод распаковки архивов во временную папку
void MainWindow::extractZip(const QString& path)
{
    //Создаем переменные для работы с архивом и папками
    //---------------------------------
    QDir dir(QApplication::applicationDirPath());
    QZipReader zip(path);
    //---------------------------------

    //Если архив существует и исправен
    //---------------------------------
    ui->statusBar->showMessage("Шарюсь в архиве...");
    if(zip.exists())
    {
        //Создаем папку
        dir.mkdir(QFileInfo(path).fileName());
        //Расспаковываем в нее
        dir.cd(QFileInfo(path).fileName());
        link->push_back(QFileInfo(path).fileName());
        zip.extractAll(dir.path());
    }
    else
    {
        ui->statusBar->showMessage("Ошибка загрузки архива... :( ");
    }
    //---------------------------------
}
//Метод удаления временного хранилища
void MainWindow::deleteTempFolder()
{
    //Извлекаем последнее принятую ссылку и по ней находим папку
    //---------------------------------
    QString name(link->back());
    link->pop_back();
    QDir dir(QApplication::applicationDirPath() + "/" + name);
    //---------------------------------

    //Рекурсивно удаляем
    //---------------------------------
    if(dir.dirName() == name)
    {
        for(const QFileInfo& temp : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot))
        {
            QFile file(temp.absoluteFilePath());
            file.remove();
        }
        dir.cdUp();
        dir.rmdir(name);
    }
    //---------------------------------
}

//Метод тригера кнопки из меню (закрытия приложения)
void MainWindow::on_close_action_triggered()
{
    close();
}
//Метод тригера кнопки из меню (открытия архива)
void MainWindow::on_file_action_triggered()
{
    //Принимаем архив и распаковываем
    //---------------------------------
    QString path;
    path = QFileDialog::getOpenFileName(this, "Выберете архив в формате Zip", "", "*.zip");
    if (path == "")
    {
        return;
    }
    extractZip(path);
    //---------------------------------

    //Читаем извлеченное
    //Удаляем временную папку
    //---------------------------------
    readToFolder(QApplication::applicationDirPath() + "/" + link->back());
    deleteTempFolder();
    //---------------------------------
}
//Метод тригера кнопки из меню (открытия директории)
void MainWindow::on_dir_action_triggered()
{
    //Принимаем дерикторию и читаем из неё
    //---------------------------------
    QString path;
    path = QFileDialog::getExistingDirectory(this, "Выберете директорию");
    if(path == "")
    {
        return;
    }
    readToFolder(path);
    //---------------------------------
}
//Метод тригера кнопки из меню (смена каталога меню)
void MainWindow::on_root_dir_triggered()
{
    //Принимаем дерикторию и читаем из неё
    //---------------------------------
    QString path;
    path = QFileDialog::getExistingDirectory(this, "Выберете директорию") + "//";
    if (path == "//" || path == root_dir)
    {
        return;
    }
    //---------------------------------

    //Устанавливаем корень и записываем в файл настроек
    //Меняем директорию Дерева
    //---------------------------------
    root_dir = path;
    QFile settings(QApplication::applicationDirPath() + "/settings");
    settings.open(QFile::WriteOnly);
    if(settings.exists())
    {
        QTextStream stream(&settings);
        stream << root_dir;
        settings.close();
    }
    else
    {
        QMessageBox::information(this, "Ошибка записи.", "Ошибка записи: данный путь не будет сохранён.");
    }
    ui->statusBar->showMessage("Переезжаю... Это может занять некоторое время.");
    data->cd(root_dir);
    //---------------------------------

    //Очищаем и строим меню
    //---------------------------------
    clearMenu();
    loadMenu();
    //---------------------------------
}

//Метод тригера кнопки из ПАНЕЛЬНОГО меню (папка с комиксом)
void MainWindow::clickedFromMenu_Folder()
{
    now_reading = dynamic_cast<QPushButton*>(sender())->text().toInt();
    //Читаем из папки по ссылке
    readToFolder(link->at(now_reading));
}
//Метод тригера кнопки из ПАНЕЛЬНОГО меню (переход в папку)
void MainWindow::clickedFromMenu_Dir()
{
    //Перемещаемся по дереву по ссылке
    data->move(link->at(dynamic_cast<QPushButton*>(sender())->text().toInt()));

    //Очищаем и создаем меню
    //---------------------------------
    clearMenu();
    loadMenu();
    //---------------------------------
}
//Метод тригера кнопки из ПАНЕЛЬНОГО меню (архив)
void MainWindow::clickedFromMenu_Zip()
{
    now_reading = dynamic_cast<QPushButton*>(sender())->text().toInt();
    //Извлекаем архив
    extractZip(link->at(now_reading));

    //Читаем извлеченное
    //Удаляем временную папку
    //---------------------------------
    readToFolder(QApplication::applicationDirPath() + "/" + link->back());
    deleteTempFolder();
    //---------------------------------
}
//Метод тригера кнопки из ПАНЕЛЬНОГО меню (назад)
void MainWindow::clickedFromMenu_Back()
{
    //Идем по дереву вверх
    //Очищаем и перезаписываем меню
    //---------------------------------
    data->up();
    clearMenu();
    loadMenu();
    //---------------------------------
}

//Метод открытия комикса лежащего в дериктории
void MainWindow::readToFolder(const QString& path)
{
    //Если есть страницы очищаем список
    //---------------------------------
    if(pages->count() > 0)
        pages->clear();
    //---------------------------------

    //Объявляем временные переменные и подгружаем страницы
    //---------------------------------
    //Создаем строковую переменную для статус бара
    QString str;
    //Создаем объект для работы с дерикторией
    QDir dir(path);
    //---------------------------------

    //Проходим по каталогу в поисках картинок
    //---------------------------------
    for(const QFileInfo& temp : dir.entryInfoList())
    {
        //Проходим по дериктории выбирая только файлы изоображений
        if(temp.suffix() == "jpg" || temp.suffix() == "png")
        {
            //Закидываем пути изоображений в список
            pages->push_back(temp.absoluteFilePath());
        }
    }
    //---------------------------------

    //Создаем объкт класса Standartview для просмотра
    //---------------------------------
    if(!pages->empty())
    {
        ui->statusBar->showMessage("Сшиваю страницы...");
        //Создаем окно просмотра
        view = new StandartView(this,*pages,dir.dirName());
        //Выводим
        view->show();
        str.setNum(pages->count());
        data->addToHistory(data->at(now_reading));
        ui->statusBar->showMessage("Успешно загруженно " + str + " страниц.");
    }
    //---------------------------------
}

//Метод подгрузки меню
void MainWindow::loadMenu()
{
    //Узнаем размер экрана
    //---------------------------------
    QScreen* screen = QApplication::screens().at(0);
    //---------------------------------

    //Выделяем память, устанавливаем фон
    //---------------------------------
    link->clear();
    menu = new QWidget(this);
    mainLay = new QGridLayout(menu);
    menu->setObjectName("menu");
    menu->setStyleSheet("QWidget#menu { background: url(:/img/background.jpg)}");
    //---------------------------------

    //Настраиваем GUI
    //---------------------------------
    ui->scrollArea->setWidget(menu);
    ui->statusBar->showMessage("Ищу мангу...");
    ui->progressBar->setRange(0, data->count());
    ui->progressBar->show();
    //---------------------------------

    //Создаем временные переменные (для строк и столбцов)
    //И кнопку "назад" при необходимости
    //---------------------------------
    int colon = 0;
    int row = 0;
    if(!data->isRoot())
    {
        createBackButton(row);
        colon++;
    }
    //---------------------------------

    //Создаем пункты меню
    for(int i(0); i < data->count(); i++)
    {
        //Создаем строковый счетчик для отображения статуса
        //---------------------------------
        QString num;
        num.setNum(i);
        ui->progressBar->setValue(i);
        //---------------------------------

        //Если строка заполнена перенос на новую
        //---------------------------------
        if(colon == cln)
        {
            colon = 0;
            row++;
        }
        //---------------------------------

        //Подгружаем иззображение
        //В случае папки изоображение передается через путь к нему
        //В случае архива передается само изоображение
        //---------------------------------
        QPixmap img;
        if(data->at(i).isZip)
        {
            img = extractCover(data->at(i).img);
            if(img.isNull())
            {
                link->push_back("Null");
                continue;
            }
        }
        else if(!data->at(i).isZip && data->at(i).isLast)
        {
            img.load(data->at(i).img);
        }
        //---------------------------------

        //Адаптация изоображения под размеры меню
        //---------------------------------
        if(!img.isNull())
            img = img.scaled(icon_width, icon_height, Qt::KeepAspectRatio);
        QSize size(img.size());
        //---------------------------------

        //Выделение памяти под элементы пункта меню
        //---------------------------------
        auto *cover = new QLabel(menu);
        auto *text = new QLabel(menu);
        auto *menu_item = new QPushButton(menu);
        auto *item_lay = new QVBoxLayout(menu_item);
        QFont font("Area", 15, -1 ,true);
        font.setBold(true);
        //---------------------------------

        //Заполнение и компановка
        //---------------------------------
        cover->setPixmap(img);
        text->setText(data->at(i).name);
        text->setAlignment(Qt::AlignCenter);
        text->setFont(font);
        text->setWordWrap(true);
        item_lay->addWidget(cover);
        item_lay->addWidget(text);
        menu_item->setText(num);
        menu_item->setStyleSheet("QPushButton:hover { border: 5px solid #FF6500; border-color: #FF6500; border-radius: 6px; background: white }"
                                 "QPushButton { border: 5px solid #888888; border-radius: 6px; background: white }");
        menu_item->setFixedSize(size.width() + 15, size.height() + 75);
        mainLay->addWidget(menu_item,row,colon++);
        link->push_back(data->at(i).path);
        //---------------------------------

        //Подключение кнопок
        //---------------------------------
        if(data->at(i).isZip)
        {
            connect(menu_item, SIGNAL(clicked()), SLOT(clickedFromMenu_Zip()));
        }
        else if(!data->at(i).isLast)
        {
            QPixmap icon(":/img/icon.png");
            icon = icon.scaled(icon_width - 20, icon_height, Qt::KeepAspectRatio);
            size = icon.size();
            cover->setPixmap(icon);
            menu_item->setFixedSize(icon_width, icon_height);
            connect(menu_item, SIGNAL(clicked()), SLOT(clickedFromMenu_Dir()));
        }
        else
        {
            connect(menu_item, SIGNAL(clicked()), SLOT(clickedFromMenu_Folder()));
        }
        //---------------------------------

        //Регулировка размера окна и вывод статуса
        //---------------------------------
        if(size.width() * (cln + 1) > screen->size().width())
        {
            this->setMinimumWidth(screen->size().width());
        }
        else
        {
            this->setMinimumWidth(size.width() * (cln + 1));
        }
        num.setNum(i + 1);
        ui->statusBar->showMessage("Нашел " + num + " томов в этой папке...");
        //---------------------------------
    }
    if(data->count() == 0)
    {
        QLabel *info = new QLabel(menu);
        info->setText("Здесь пусто! \n"
                      "Либо в этой папке ничего нету, \n"
                      "либо необходимо указать папку с мангой: \n"
                      "Настройки -> Установить корневую папку");
        QFont font("Area", 20, -1 ,true);
        info->setFont(font);
        info->setStyleSheet("QLabel { color: white; }");
        mainLay->addWidget(info, row, colon);
    }
    ui->progressBar->hide();
}
//Очистка меню
void MainWindow::clearMenu()
{
    delete mainLay;
    delete menu;
}
//Создание кнопки "назад" для меню
void MainWindow::createBackButton(int row)
{
    QPixmap img(":/img/back.png");
    auto *menu_item = new QPushButton(menu);
    menu_item->setIcon(img);
    QSize size(275, 275);
    menu_item->setIconSize(size);
    menu_item->setFixedSize(icon_width, icon_height);
    menu_item->setStyleSheet("QPushButton:hover { border: 5px solid #FF6500; border-color: #FF6500; border-radius: 6px; background: white }"
                             "QPushButton { border: 5px solid #FFFFF0; border-radius: 6px; background: white }");
    connect(menu_item, SIGNAL(clicked()), SLOT(clickedFromMenu_Back()));
    mainLay->addWidget(menu_item, row, 0);
}
//Создание кнопок "больше" и "меньше" для меню
void MainWindow::createLessMoreButton(bool isMore, int row)
{
    if(isMore)
    {
        auto *menu_item = new QPushButton(menu);
        menu_item->setText("Больше");
        menu_item->setStyleSheet("QPushButton:hover { border: 5px solid #FF6500; border-color: #FF6500; border-radius: 6px; background: white }"
                                 "QPushButton { border: 5px solid #FFFFF0; border-radius: 6px; background: white }");
        //connect(menu_item, SIGNAL(clicked()), SLOT(clickedFromMenu_More()));
        mainLay->addWidget(menu_item, row, 0, 1, 4);
    }
    else
    {
        auto *menu_item = new QPushButton(menu);
        menu_item->setText("Предыдущие");
        menu_item->setStyleSheet("QPushButton:hover { border: 5px solid #FF6500; border-color: #FF6500; border-radius: 6px; background: white }"
                                 "QPushButton { border: 5px solid #FFFFF0; border-radius: 6px; background: white }");
        //connect(menu_item, SIGNAL(clicked()), SLOT(clickedFromMenu_Less()));
        mainLay->addWidget(menu_item, 0, 0, 1, 4);
    }
}

void MainWindow::on_resize500_800_triggered()
{
    icon_width = 500;
    icon_height = 800;
    if(cln == 4)
    {
        cln = 3;
    }
    clearMenu();
    loadMenu();
}

void MainWindow::on_resize300_500_triggered()
{
    icon_width = 300;
    icon_height = 500;
    clearMenu();
    loadMenu();
}

void MainWindow::on_cln1_triggered()
{
    cln = 1;
    clearMenu();
    loadMenu();
}

void MainWindow::on_cln2_triggered()
{
    cln = 2;
    clearMenu();
    loadMenu();
}

void MainWindow::on_cln3_triggered()
{
    cln = 3;
    clearMenu();
    loadMenu();
}

void MainWindow::on_cln4_triggered()
{
    if(icon_width == 500)
    {
        QMessageBox::information(this, "Не стоит", "Данное количество столбцов не доступно при размере иконок 500х800.");
    }
    else
    {
        cln = 4;
        clearMenu();
        loadMenu();
    }
}

void MainWindow::on_update_triggered()
{
    data->cd(root_dir);
    //---------------------------------

    //Очищаем и строим меню
    //---------------------------------
    clearMenu();
    loadMenu();
    //---------------------------------
}

void MainWindow::on_history_triggered()
{
    clearMenu();
    data->history();
    loadMenu();
}
