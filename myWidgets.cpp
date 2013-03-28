#include "myWidgets.h"
#include "data_provider.h"
#include <QHelpEvent>
#include <QToolTip>
#include <QPainter>

myMenu::myMenu (QWidget * parent) : QMenu(parent) {}

myMenu::myMenu (const QString & title, QWidget * parent) : QMenu(title, parent) {}

bool myMenu::event (QEvent * p) 
{
	QHelpEvent * ev = dynamic_cast<QHelpEvent *>(p);
	if (ev and ev->type() == QEvent::ToolTip and activeAction()) QToolTip::showText(ev->globalPos(), activeAction()->toolTip());
	else if (QToolTip::isVisible()) QToolTip::hideText();
	return QMenu::event(p);
}

dayInLine::dayInLine (QWidget * parent) : QWidget(parent)
{
	setMinimumSize(500, 150);
}

void dayInLine::paintEvent (QPaintEvent * event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect wrect = contentsRect();
	day last_day = data_provider::get_obj()->get_last_day();
	if (last_day.isValid()) {
		QPoint center_point = wrect.center();
		QRect line_rect(wrect.left(), center_point.y() - 50, wrect.width(), 100);
		painter.fillRect(line_rect, "#cfefff");
		double seconds_by_pixel = static_cast<double>(last_day.getSeconds()) / wrect.width();
		QDateTime now = QDateTime::currentDateTime();
		int now_line_x = line_rect.left() + ((now.toTime_t() - last_day.start.toTime_t()) / seconds_by_pixel);
		QPen now_line_pen(Qt::SolidLine);
		now_line_pen.setColor("#007700");
		now_line_pen.setWidth(3);
		painter.setPen(now_line_pen);
		painter.drawLine(now_line_x, wrect.top(), now_line_x, wrect.bottom());
		painter.setPen(QColor("#0000aa"));
		QRect begin_date_rect(line_rect.left(), line_rect.top(), 40, 50);
		painter.drawText(begin_date_rect, Qt::AlignLeft | Qt::AlignTop, last_day.start.toString("HH:mm"));
		QRect end_date_rect(line_rect.right() - 40, line_rect.top(), 40, 50);
		painter.drawText(end_date_rect, Qt::AlignRight | Qt::AlignTop, last_day.end.toString("HH:mm"));
	} else {
		painter.drawText(wrect, Qt::AlignCenter, tr("Last day is not exists."));
	}
}
