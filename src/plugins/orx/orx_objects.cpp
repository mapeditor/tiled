#include "orx_objects.h"
#include <QFileInfo>
#include <QDir>

namespace Orx {

///////////////////////////////////////////////////////////////////////////////
std::string normalize_name(const std::string & name)
{
    QString ret(name.c_str());
    ret.replace(QString(" "), QString("_"));
    return ret.toStdString();
}

///////////////////////////////////////////////////////////////////////////////
std::string get_name_from_file(const std::string & name)
{
    QFileInfo fi(name.c_str());
    return normalize_name(fi.baseName().toStdString());
}



}


