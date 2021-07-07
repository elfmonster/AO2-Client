#pragma once

class AOApplication;

#include <QObject>

class AOObject : public QObject
{
  Q_OBJECT

public:
  AOObject(AOApplication *p_ao_app, QObject *p_parent = nullptr);

protected:
  AOApplication *ao_app = nullptr;
};
