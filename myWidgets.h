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

#endif // MYMENU_H
