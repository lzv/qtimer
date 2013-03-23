#ifndef UTILS_H
#define UTILS_H

#include <QString>

class utils {
	public:
		static bool check_can_be_edit (const QString & full_name, QString & error_message);
};

#endif // UTILS_H
