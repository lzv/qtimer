#ifndef CONTENT_TYPES_H
#define CONTENT_TYPES_H

#include <QString>

// Количество часов, минут и секунд. В отличии от QTime, количество часов может быть меньше 0 и больше 24-х.
class time_count {

	public:
		long int h;
		short int m;
		short int s;
		
		time_count (long int seconds = 0); // Создание из количества секунд. Может быть отрицательным.
		time_count (long int hours, long int minutes, long int seconds); // Значения могут быть любыми.

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

#endif // CONTENT_TYPES_H
