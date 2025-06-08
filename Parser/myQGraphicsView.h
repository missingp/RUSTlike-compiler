#pragma once
#ifndef MYQGRAPHICSVIEW_H
#define MYQGRAPHICSVIEW_H

#include <QGraphicsView>

//绘图单元类，设置图像画面的相关操作
class myQGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit myQGraphicsView(QWidget* parent = nullptr);
    void wheelEvent(QWheelEvent* event);                  // 重写鼠标缩放
    void expand();                                        // 放大
    void shrink();                                        // 缩小
    void refresh();                                       // 刷新

private:
    void zoom(double);                                    // 尺寸变化

signals:

};

#endif // MYQGRAPHICSVIEW_H