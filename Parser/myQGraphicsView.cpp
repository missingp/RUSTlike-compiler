

//@func   : myQGraphicsView执行函数
#include "myQGraphicsView.h"
#include <cmath>
#include <QDebug>
#include <qmath.h>
#include <QWheelEvent>
#include <qscrollbar.h>

//@name  : 构造函数
//@para  : 
//@return: 
myQGraphicsView::myQGraphicsView(QWidget* parent) : QGraphicsView(parent)
{
}

//@name  : 鼠标滚轮事件
//@para  : 
//@return: 
void myQGraphicsView::wheelEvent(QWheelEvent* event)
{
	auto step = event->angleDelta().y();
	QPointF cursorPoint = event->position();
	QPointF scenePos = this->mapToScene(cursorPoint.x(), cursorPoint.y());
	qreal factor = 1 + ((qreal)step / (qreal)viewport()->height()); scale(factor, factor);

	QPointF viewPoint = this->transform().map(scenePos);
	horizontalScrollBar()->setValue(viewPoint.x() - cursorPoint.x());
	verticalScrollBar()->setValue(viewPoint.y() - cursorPoint.y());

}

//@name  : 画面缩小
//@para  : 
//@return: 
void myQGraphicsView::shrink()
{
	zoom(1.0 / 1.2);
}
//@name  : 画面增大
//@para  : 
//@return: 
void myQGraphicsView::expand()
{
	zoom(1.2);
}
//@name  : 画面改变
//@para  : 
//@return: 
void myQGraphicsView::zoom(double scaleFactor)
{
	//缩放函数
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.1 || factor > 50)
		return;
	scale(scaleFactor, scaleFactor);
}

//@name  : 刷新界面
//@para  : 
//@return: 
void myQGraphicsView::refresh()
{
	this->viewport()->update();
}

