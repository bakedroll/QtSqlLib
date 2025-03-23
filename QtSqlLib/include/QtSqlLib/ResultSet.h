#pragma once

#include <QSqlQuery>

namespace QtSqlLib
{

class ResultSet
{
public:
  ResultSet(const QSqlQuery& query);
  virtual ~ResultSet();

private:
  QSqlQuery m_sqlQuery;

};

}
