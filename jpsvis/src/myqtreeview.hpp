#pragma once
#include <QTreeView>

class MyQTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MyQTreeView(QWidget * parent = nullptr);

    void closeEvent(QCloseEvent *) override;
signals:
    void changeState();
};
