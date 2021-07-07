#pragma once

#include <QString>
#include <QStringList>

class DRPacket
{
public:
  static QString encode(QString data);
  static QString decode(QString data);

  DRPacket(QString header);
  DRPacket(QString header, QStringList content);

  const QString &get_header() const;
  const QStringList &get_content() const;
  QString to_string(const bool encode = false) const;

private:
  QString m_header;
  QStringList m_content;
};
