#include "data.h"
#include <QDir>
#include <QCollator>
#include <QTextStream>
#include <QtGui/private/qzipreader_p.h>
#include <QApplication>

//Конструктор
Data::Data(const QString& path)
{
  //Инициализируем переменные, прописываем путь
  //---------------------------------
  if(path == "" || path == "/")
    root_path = QApplication::applicationDirPath();
  else
  {
    root_path = path;
  }
  curr_path = root_path;
  //---------------------------------
  history_mode = false;
  mkHistoyTree();
  //Заполняем дерево
  //---------------------------------
  //Инициализируем корень
  initRoot(root_path);
  //Подгружаем данные
  scanFolder(&tree, root_path);
  //---------------------------------
}
//Диструктор
Data::~Data()
{
    writeHistoryToFile();
    deleteTree(&tree);
    deleteTree(&history_tree);
}

//Возвращает N сына
const data_tree &Data::at(int son)
{
  //Ищем сына по номеру
  //---------------------------------
  data_tree* curr = search(tree, curr_path);
  if(son > -1 && son < curr->numSons)
    return *curr->sons[son];
  return *tree;
  //---------------------------------
}

void Data::history()
{
    deleteSons(&tree);
    copy();
}

void Data::copy()
{
    history_mode = true;
    for (int i(history_tree->numSons - 1); i > -1; i--)
    {
        addSon(&tree, history_tree->sons[i]->img, history_tree->sons[i]->name,history_tree->sons[i]->path, history_tree->sons[i]->isZip);
    }
}

//Подсчет сыновей
int Data::count()
{
  return search(tree, curr_path)->numSons;
}

//Движение по дереву
bool Data::move(const QString &parent_path)
{
  //Ищем нужного сына и спускаемся в него
  //---------------------------------
  curr_path = search(tree, parent_path)->path;
  return curr_path == parent_path;
  //---------------------------------
}
//Меняем директорию
void Data::cd(const QString &path)
{
  //Переписываем пути
  //---------------------------------
  root_path = path;
  curr_path = root_path;
  //---------------------------------

  //Очищаем дерево и заново заполняем
  //---------------------------------
  clear();
  scanFolder(&tree, root_path);
  //---------------------------------
}
//Очистка дерева
void Data::clear()
{
  //Удаляем дерево и переинициализируем корень
  //---------------------------------
  deleteTree(&tree);
  initRoot(root_path);
  //---------------------------------
}
//Подъём к предку
void Data::up()
{
  if(history_mode)
  {
      history_mode = false;
      deleteSons(&tree);
      scanFolder(&tree, root_path);
      this->move(curr_path);
  }
  else
  {
      curr_path = search(tree, curr_path)->parent->path;
  }
}

//Проверка на вершину
bool Data::isRoot()
{
  if(history_mode)
  {
      return false;
  }
  //Проверяем в корне ли дерева мы
  //---------------------------------
  return curr_path == root_path;
  //---------------------------------

}

void Data::addToHistory(const data_tree& new_son)
{
    addSon(&history_tree, new_son.img, new_son.name, new_son.path, new_son.isZip);
}

//Добавляем сына для папки с главами
void Data::addSon(data_tree **parent, const QString& path)
{
  //Заполняем ячейку и сканируем её папку
  //---------------------------------
  QDir dir(path);
  data_tree *root = *parent;
  auto *son = new data_tree;
  son->sons = new data_tree*[dir.count()];
  son->name = dir.dirName();
  son->path = dir.absolutePath();
  son->isLast = false;
  son->isZip = false;
  son->numSons = 0;
  son->parent = *parent;
  root->sons[root->numSons++] = son;
  scanFolder(&son, son->path);
  //---------------------------------
}
//Добавляем сына для папки с мангой
void Data::addSon(data_tree **parent, QString img, QString name, QString path, bool isZip)
{
  //Добавляем сына, вместо изоображения передаем путь к нему
  //---------------------------------
  data_tree *root = *parent;
  auto *son = new data_tree;
  son->isRoot = false;
  son->name = std::move(name);
  son->img = std::move(img);
  son->path = std::move(path);
  son->isLast = true;
  son->isZip = isZip;
  son->numSons = 0;
  son->parent = *parent;
  root->sons[root->numSons++] = son;
  //---------------------------------
}
//Функция Инициализация корня
//Принимает путь к корню
void Data::initRoot(const QString &path)
{
  //Создаем ячейку
  auto *temp = new data_tree;
  //Прописываем путь
  temp->path = path;
  //Отмечаем что это корень
  temp->isRoot = true;
  temp->numSons = 0;
  //Выделяем память под массив сылок
  QDir dir(path);
  temp->sons = new data_tree*[dir.count()];
  //Устанавливаем указатель на корень
  tree = temp;
}

void Data::mkHistoyTree()
{
    //Создаем ячейку
    auto *temp = new data_tree;
    //Прописываем путь
    temp->path = "";
    //Отмечаем что это корень
    temp->isRoot = true;
    temp->numSons = 0;
    //Выделяем память под массив сылок
    temp->sons = new data_tree*[100];
    //Устанавливаем указатель на корень
    history_tree = temp;
    QFile history_file(QApplication::applicationDirPath() + "/history");
    if(history_file.open(QIODevice::ReadOnly |QIODevice::Text))
    {
        while(!history_file.atEnd())
        {
            //читаем строку
            QString str = history_file.readLine();
            //Делим строку на слова разделенные пробелом
            QStringList lst = str.split(" | ");
            // выводим первых три слова
            addSon(&history_tree, lst.at(0), lst.at(1),lst.at(1), lst.at(2).toInt());
        }
    }
    history_file.close();
}

void Data::writeHistoryToFile()
{
    QFile history_file(QApplication::applicationDirPath() + "/history");
    QTextStream stream(&history_file);
    if(history_file.open(QIODevice::WriteOnly | QIODevice::Text) && history_tree->numSons != 0)
    {
        for (int i(0); i < history_tree->numSons; i++)
        {
            if(history_tree->sons[i]->isZip)
            {
                stream << history_tree->sons[i]->img <<  " | "  << history_tree->sons[i]->path << " | " << 1 << '\n';
            }
            else
            {
                stream << history_tree->sons[i]->img << " | " << history_tree->sons[i]->path << " | " << 0 << '\n';
            }
        }
    }
    history_file.close();
}

//Сканирование дерикторий
void Data::scanFolder(data_tree **root, const QString& path)
{
  QDir dir(path);
  for(const auto& temp : dir.entryInfoList(QDir::NoDotAndDotDot |
                                    QDir::Dirs |
                                    QDir::Files))
  {
    //Если директория
    if(temp.isDir())
    {
      //Проверяем что в ней
      QDir tempDir(temp.absoluteFilePath());
      tempDir.setSorting(QDir::LocaleAware);
      for (const auto& scan : tempDir.entryInfoList(QDir::NoDotAndDotDot |
                                                    QDir::Dirs |
                                                    QDir::Files))
      {
        //Если директория
        if(scan.isDir())
        {
          //Добавление сына с косвенной рекурсией на эту функцию
          addSon(root, temp.absoluteFilePath());
          break;
        }
        //Если архив
        if(scan.suffix() == "zip")
        {
          //Добавление сына с косвенной рекурсией на эту функцию
          addSon(root, temp.absoluteFilePath());
          break;
        }
        //Если картинка
        if(scan.suffix() == "png" || scan.suffix() == "jpg")
        {
          //Добавление сына БЕЗ косвенной рекурсией на эту функцию
          addSon(root, scan.absoluteFilePath(), tempDir.dirName(), scan.absolutePath(), false);
          break;
        }
      }
    }
    //Если есть архив
    else if(temp.suffix() == "zip")
    {
        //Добавляем в дерево
        addSon(root, temp.absoluteFilePath(), temp.fileName(), temp.absoluteFilePath(),true);
    }
  }
}

//Удаление дерева
void Data::deleteTree(data_tree **parent)
{
  if(parent != nullptr)
  {
    data_tree *temp = *parent;
    for(int i(0); i < temp->numSons; i++)
    {
      deleteTree(&temp->sons[i]);
    }
    delete temp;
  }
}
//Удаление сыновей предка
void Data::deleteSons(data_tree **parent)
{
  if(parent != nullptr)
  {
    data_tree *temp = *parent;
    for(int i(0); i < temp->numSons; i++)
    {
      deleteTree(&temp->sons[i]);
    }
    temp->numSons = 0;
  }
}
//Поиск по дереву
data_tree *Data::search(data_tree *parent, const QString& key)
{
  if(parent->name == key || parent->path == key)
    return parent;
  for(int i(0); i < parent->numSons; i++)
  {
    if(search(parent->sons[i], key)->name == key || search(parent->sons[i], key)->path == key)
    return search(parent->sons[i], key);
  }
  return tree;
}
