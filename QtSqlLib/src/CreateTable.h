#pragma once

#include "QtSqlLib/Query/Query.h"

namespace QtSqlLib::Query
{

class CreateTable : public Query
{
public:
  CreateTable(const API::Table& table);
  ~CreateTable() override;

  API::IQuery::SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;

private:
  const API::Table& m_table;

};

}
