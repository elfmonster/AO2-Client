#include "aoobject.h"

AOObject::AOObject(AOApplication *p_ao_app, QObject *p_parent) : QObject(p_parent), ao_app(p_ao_app)
{}
