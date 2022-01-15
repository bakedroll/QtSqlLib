#pragma once

#include <exception>

#include <QString>

namespace QtSqlLib
{

class DatabaseException : public std::exception
{
public:
  enum class Type
  {
    UnableToLoad,
    InvalidQuery
  };

  DatabaseException(Type type, const QString& message);
  ~DatabaseException() override;

  QString getMessage() const;
  Type getType() const;

private:
  Type m_type;

};

}