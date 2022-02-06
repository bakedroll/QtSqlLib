#include "QtSqlLib/DatabaseException.h"

#include <QSqlDatabase>

namespace QtSqlLib
{

DatabaseException::DatabaseException(Type type, const QString& message)
  : std::exception(message.toStdString().c_str())
  , m_type(type)
{
  if (m_type == Type::QueryError)
  {
    QSqlDatabase::database().rollback();
  }
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
