#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H

class QString;
class QStringList;

QStringList animated_or_static_extensions();
QStringList animated_extensions();
QStringList audio_extensions(bool no_suffix = false);

bool file_exists(QString file_path);
bool dir_exists(QString file_path);

#endif // FILE_FUNCTIONS_H
