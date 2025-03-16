#pragma once

#include "QtSqlLib/Query/Query.h"

namespace QtSqlLib::Query
{

class QueryInsertedIDs : public Query
{
public:
  QueryInsertedIDs(API::IID::Type tableId);
  ~QueryInsertedIDs() override;

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, ResultSet& previousQueryResults) override;
  ResultSet getQueryResults(API::ISchema& schema, QSqlQuery&& query) override;

private:
  API::IID::Type m_tableId;

};

}
