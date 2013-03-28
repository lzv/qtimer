#ifndef MYMENU_H
#define MYMENU_H

#include <QMenu>

// Класс для организации всплывающих подсказок на пунктах меню.

class myMenu : public QMenu 
{
	Q_OBJECT
		
	public:
		myMenu (QWidget * parent = 0);
		myMenu (const QString & title, QWidget * parent = 0);

	protected:
		virtual bool event (QEvent *);
};

class dayInLine : public QWidget
{
	Q_OBJECT

	protected:
		void paintEvent (QPaintEvent *);

	public:
		dayInLine (QWidget * parent = 0);
};

#endif // MYMENU_H
