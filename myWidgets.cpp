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
		time_count to_day_end(now.secsTo(last_day.end));
		
		QMap<work, int> planned_works = data_provider::get_obj()->get_planned_works_for_last_day();
		int total_planned_secs = 0;
		for (QMap<work, int>::const_iterator i = planned_works.begin(); i != planned_works.end(); ++i)
			total_planned_secs += i.value();
		time_count free_to_day_end(to_day_end.get_in_seconds() - total_planned_secs);
		
		if (total_planned_secs > 0) {
			if (free_to_day_end.get_in_seconds() > 0) {
				time_period last_tp = data_provider::get_obj()->get_last_time_period();
				bool work_is_running = last_tp.isValid() and last_tp.isOpened();
				int pieces_count = (total_planned_secs - (total_planned_secs % 2700)) / 2700 + 1;
				int pixels_in_piece = (total_planned_secs / seconds_by_pixel) / pieces_count;
				int pixels_between_pieces = (wrect.width() - now_line_x - total_planned_secs / seconds_by_pixel) / (work_is_running ? pieces_count : pieces_count + 1);
				for (int i = 0; i < pieces_count; ++i) {
					QRect piece(now_line_x + pixels_between_pieces * (work_is_running ? i : i + 1) + pixels_in_piece * i + i, line_rect.top(), pixels_in_piece, 100);
					painter.fillRect(piece, "#ff7777");
				}
			} else {
				QRect right_part(now_line_x, line_rect.top(), wrect.right() - now_line_x, 100);
				painter.fillRect(right_part, "#ff7777");
			}
		}
		
		QList<time_period> time_periods = data_provider::get_obj()->get_time_periods_for_current_day();
		for (QList<time_period>::const_iterator i = time_periods.begin(); i != time_periods.end(); ++i) {
			int left_side = wrect.left() + last_day.start.secsTo(i->start) / seconds_by_pixel;
			int right_side = (i->end.isValid() ? wrect.left() + last_day.start.secsTo(i->end) / seconds_by_pixel : now_line_x);
			QRect green_piece(left_side, line_rect.top(), right_side - left_side + 1, 100);
			painter.fillRect(green_piece, "#77ff77");
		}
		
		painter.setPen(QColor("#000000"));
		painter.drawLine(wrect.left(), center_point.y(), wrect.right(), center_point.y());
		
		QPen hours_label_pen(Qt::SolidLine);
		hours_label_pen.setColor(QColor("#000000"));
		hours_label_pen.setWidth(3);
		painter.setPen(hours_label_pen);
		QDateTime hour_label(last_day.start.date(), QTime(last_day.start.time().hour(), 0, 0));
		hour_label = hour_label.addSecs(3600);
		while (hour_label < last_day.end) {
			int label_x = wrect.left() + last_day.start.secsTo(hour_label) / seconds_by_pixel;
			painter.drawLine(label_x, center_point.y() - 5, label_x, center_point.y() + 5);
			hour_label = hour_label.addSecs(3600);
		}
		
		QPen now_line_pen(Qt::SolidLine);
		now_line_pen.setColor("#007700");
		now_line_pen.setWidth(3);
		painter.setPen(now_line_pen);
		painter.drawLine(now_line_x, wrect.top(), now_line_x, wrect.bottom());
		
		painter.setPen(QColor("#0000aa"));
		QRect total_time_left_text(wrect.right() - 270, wrect.top(), 270, 21);
		painter.fillRect(total_time_left_text, painter.background());
		painter.drawText(total_time_left_text, Qt::AlignRight | Qt::AlignVCenter, tr("Total time left:") + "   " + to_day_end.get_string_for_print('c'));
		
		painter.setPen(QColor(free_to_day_end.get_in_seconds() < 0 ? "#aa0000" : "#0000aa"));
		QRect total_free_time_left_text(wrect.right() / 2 - 185, wrect.bottom() - 20, 370, 21);
		painter.fillRect(total_free_time_left_text, painter.background());
		painter.drawText(total_free_time_left_text, Qt::AlignCenter, tr("Total free time left:") + "   " + free_to_day_end.get_string_for_print('c'));
		
		painter.setPen(QColor("#0000aa"));
		QRect begin_date_rect(line_rect.left(), line_rect.top(), 40, 50);
		painter.drawText(begin_date_rect, Qt::AlignLeft | Qt::AlignTop, last_day.start.toString("HH:mm"));
		QRect end_date_rect(line_rect.right() - 40, line_rect.top(), 40, 50);
		painter.drawText(end_date_rect, Qt::AlignRight | Qt::AlignTop, last_day.end.toString("HH:mm"));
	} else {
		painter.setPen(QColor("#0000aa"));
		painter.drawText(wrect, Qt::AlignCenter, tr("Last day is not exists."));
	}
}
