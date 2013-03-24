#ifndef UTILS_H
#define UTILS_H

#include <QString>

class utils {
	public:
		static bool check_can_be_edit (const QString & full_name, QString & error_message);
		static QString num_declination (unsigned int count, const QString & str1, const QString & str2, const QString & str5);
};

#endif // UTILS_H
