#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

DatabaseException::DatabaseException(Type type, const QString& message)
  : std::exception(message.toStdString().c_str())
  , m_type(type)
{
}

DatabaseException::~DatabaseException() = default;

QString DatabaseException::getMessage() const
{
  return what();
}

DatabaseException::Type DatabaseException::getType() const
{
  return m_type;
}

}
