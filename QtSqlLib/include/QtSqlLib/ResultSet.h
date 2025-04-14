#pragma once

#include <QSqlQuery>

#include "QtSqlLib/TupleView.h"

namespace QtSqlLib
{

class ResultSet
{
public:
  ResultSet(const QSqlQuery& query);
  virtual ~ResultSet();

  bool hasNextTuple();
  bool hasNextJoinedTuple();

  TupleView nextTuple();
  TupleView nextJoinedTuple();

private:
  QSqlQuery m_sqlQuery;

};

}
