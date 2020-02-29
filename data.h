#ifndef DATA_H
#define DATA_H

#include <QList>
#include <QString>
#include <QPixmap>

struct data_tree
{
  QString img, name, path;
  bool isRoot, isZip, isLast;
  int numSons;
  data_tree **sons;
  data_tree *parent;
};

class Data
{
public:
  explicit Data(const QString& path);
  ~Data();
  const data_tree& at(int son);
  void history();
  int count();
  bool move(const QString& parent_path);
  void cd(const QString& path);
  void clear();
  void up();
  bool isRoot();
  void addToHistory(const data_tree& new_son);
private:
  void addSon(data_tree **parent, const QString& path);
  void addSon(data_tree **parent, QString img, QString name, QString path, bool isZip);
  void initRoot(const QString& path);
  void mkHistoyTree();
  void writeHistoryToFile();
  void copy();
  void scanFolder(data_tree **root, const QString& path);
  void deleteTree(data_tree **parent);
  void deleteSons(data_tree **parent);
  data_tree* search(data_tree *parent, const QString& key);
private:
  data_tree *tree;
  data_tree *history_tree;
  QString root_path;
  QString curr_path;
  bool history_mode;
};

#endif // DATA_H
