#include "myqtreeview.h"

MyQTreeView::MyQTreeView(QWidget * parent) : QTreeView(parent) {}

void MyQTreeView::closeEvent(QCloseEvent * event)
{
    emit changeState();
    QTreeView::closeEvent(event);
}
