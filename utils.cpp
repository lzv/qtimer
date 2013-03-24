#include "utils.h"

#include <QFile>
#include <QFileInfo>

bool utils::check_can_be_edit (const QString & full_name, QString & error_message) {
	bool result = false;
	if (full_name.isEmpty()) {
		error_message = QObject::tr("File name is empty");
	} else {
		QFileInfo finfo(full_name);
		if (finfo.fileName().isEmpty()) {
			error_message = QObject::tr("File name is directory");
		} else {
			QFile new_file(finfo.absoluteFilePath());
			bool file_exists = new_file.exists();
			if (new_file.open(QIODevice::ReadWrite)) {
				new_file.close();
				if (!file_exists) new_file.remove();
				result = true;
			} else {
				error_message = QObject::tr("Can not open file for write: ") + new_file.errorString();
			}
		}
	}
	return result;
}

QString utils::num_declination (unsigned int count, const QString & str1, const QString & str2, const QString & str5)
{
	unsigned short int n1 = count % 100, n2 = count % 10;
	if (n1 >= 10 and n1 <= 20) return str5;
	else if (n2 >= 2 and n2 <= 4) return str2;
	else if (n2 == 1) return str1;
	else return str5;
}
