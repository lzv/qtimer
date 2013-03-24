#include "myMenu.h"
#include <QHelpEvent>
#include <QToolTip>

myMenu::myMenu (QWidget * parent) : QMenu(parent) {}

myMenu::myMenu (const QString & title, QWidget * parent) : QMenu(title, parent) {}

bool myMenu::event (QEvent * p) 
{
	QHelpEvent * ev = dynamic_cast<QHelpEvent *>(p);
	if (ev and ev->type() == QEvent::ToolTip and activeAction()) QToolTip::showText(ev->globalPos(), activeAction()->toolTip());
	else if (QToolTip::isVisible()) QToolTip::hideText();
	return QMenu::event(p);
}
