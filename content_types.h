#ifndef CONTENT_TYPES_H
#define CONTENT_TYPES_H

#include <QString>
#include <QDateTime>

// Количество часов, минут и секунд. В отличии от QTime, количество часов может быть меньше 0 и больше 24-х.
class time_count
{
	public:
		long int h;
		short int m;
		short int s;

		explicit time_count (long int seconds = 0); // Создание из количества секунд. Может быть отрицательным.
		explicit time_count (long int hours, long int minutes, long int seconds); // Значения могут быть любыми.

		void set_from_seconds (long int seconds = 0);
		bool set_from_string (const QString & HH_MM_SS); // Возврат true при успешном парсинге строки HH:MM:SS, иначе false. Объект меняется только при успехе. Вместо двоеточия может быть любой символ, так как значения смотрятся по позиции в строке.
		QString get_string_for_print (char type = 'n') const; // n - нормальный вид, c - компактный, w - широкий.
		long int get_in_seconds () const;

		bool operator == (const time_count & val) const {return h == val.h and m == val.m and s == val.s;}
		bool operator != (const time_count & val) const {return h != val.h or m != val.m or s != val.s;}
		bool operator < (const time_count & val) const {return h < val.h or (h == val.h and m < val.m) or (h == val.h and m == val.m and s < val.s);}
		bool operator <= (const time_count & val) const {return h < val.h or (h == val.h and m < val.m) or (h == val.h and m == val.m and s <= val.s);}
		bool operator > (const time_count & val) const {return h > val.h or (h == val.h and m > val.m) or (h == val.h and m == val.m and s > val.s);}
		bool operator >= (const time_count & val) const {return h > val.h or (h == val.h and m > val.m) or (h == val.h and m == val.m and s >= val.s);}
		time_count operator - (const time_count & val) const;
};

time_count operator - (const QDateTime & val_left, const QDateTime & val_right);

// Для данных, которые хранятся в БД.
class data_need_id 
{
	public:
		int id;

		explicit data_need_id (int var_id = 0) : id(var_id) {}
		virtual ~data_need_id () {}

		bool isValid () {return id > 0;}
		bool operator < (const data_need_id & val) const {return id < val.id;}
		virtual bool isCorrect () const = 0; // Проверяется корректность только самих данных, без привязки к другим данным или логике программы. id не учитывается.
};

// Класс для учитываемого дня.
class day : public data_need_id 
{
	public:
		QDateTime start;
		QDateTime end;

		explicit day (int day_id = 0, QDateTime day_start = QDateTime(), QDateTime day_end = QDateTime()) : data_need_id(day_id), start(day_start), end(day_end) {}

		bool isCorrect () const;
		bool isIn (const QDateTime & moment = QDateTime::currentDateTime()) const;
		long int getSeconds () const;
};

// Родительский класс для типа данных - работ. 
class work : public data_need_id {

	public:
		QString name;
		int plan;

		explicit work (int var_id = 0, const QString & var_name = "", int var_plan = 0) : data_need_id(var_id), name(var_name), plan(var_plan) {}

		bool isCorrect () const {return !name.isEmpty() and plan > 0;}
};

#endif // CONTENT_TYPES_H
