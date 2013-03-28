#include "content_types.h"
#include <QRegExp>
#include <QStringList>

time_count::time_count (long int seconds) : h(0), m(0), s(0) {
	set_from_seconds(seconds);
}

time_count::time_count (long int hours, long int minutes, long int seconds) : h(0), m(0), s(0) {
	set_from_seconds(hours * 3600 + minutes * 60 + seconds);
}

void time_count::set_from_seconds (long int seconds) {
	h = m = s = 0;
	if (seconds != 0) {
		bool flag_minus = seconds < 0; // Если операнд меньше 0, то поведение операции % зависит от реализации. Выберем надежный путь. 
		if (flag_minus) seconds = -seconds;
		seconds -= (s = seconds % 60);
		if (seconds > 0) {
			seconds /= 60;
			seconds -= (m = seconds % 60);
			if (seconds > 0) h = seconds / 60;
		}
		if (flag_minus) {
			h = -h;
			m = -m;
			s = -s;
		}
	}
}

bool time_count::set_from_string (const QString & HH_MM_SS) {
	bool result = false;
	QRegExp exp("^(\\d{2}).(\\d{2}).(\\d{2})");
	if (HH_MM_SS.contains(exp)) {
		QStringList list = exp.capturedTexts();
		h = list[1].toLong();
		m = list[2].toShort();
		s = list[3].toShort();
	}
	return result;
}

QString time_count :: get_string_for_print (char type) const {
	QString result("");
	if (type == 'w') {
		// Как нормальный вывод, но если первые значения нулевые, их место заполняется пробелами. Так же добавляется пробел к значениям меньшим 10.
		result = QString("%1 %2 %3").arg(h == 0 ? "    " : (h < 10 ? " " : "") + QString().setNum(h) + "ч.").arg(h == 0 and m == 0 ? "    " : (m < 10 ? " " : "") + QString().setNum(m) + "м.").arg((s < 10 ? " " : "") + QString().setNum(s) + "с.");
	} else if (type == 'c') {
		// Показываются только ненулевые значения.
		result = QString("%1%2%3").arg(h == 0 ? "" : QString().setNum(h) + "ч." + (m == 0 and s == 0 ? "" : " ")).arg(m == 0 ? "" : QString().setNum(m) + "м." + (s == 0 ? "" : " ")).arg(s == 0 and (h != 0 or m != 0) ? "" : QString().setNum(s) + "с.");
	} else {
		// Нулевые значения показываются, если перед ними был вывод.
		result = QString("%1%2%3").arg(h == 0 ? "" : QString().setNum(h) + "ч. ").arg(h == 0 and m == 0 ? "" : QString().setNum(m) + "м. ").arg(QString().setNum(s) + "с.");
	}
	return result;
}

long int time_count::get_in_seconds () const {
	return h * 3600 + m * 60 + s;
}

time_count time_count::operator - (const time_count & val) const {
	return time_count(get_in_seconds() - val.get_in_seconds());
}

time_count operator - (const QDateTime & val_left, const QDateTime & val_right) {
	return time_count(val_left.isValid() and val_right.isValid() ? val_left.secsTo(val_right) : 0);
}

bool day::isCorrect () const {
	return start.isValid() and end.isValid() and start < end;
}

bool day::isIn (const QDateTime & moment) const {
	return isCorrect() and moment.isValid() and moment >= start and moment <= end;
}

long int day::getSeconds () const {
	long int result = 0;
	if (isCorrect()) result = end.toTime_t() - start.toTime_t();
	return result;
}
