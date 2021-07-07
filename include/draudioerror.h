#pragma once

#include <QString>

class DRAudioError
{
public:
  DRAudioError();
  DRAudioError(QString p_error);

  QString what();

private:
  QString m_error;
};
