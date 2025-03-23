#pragma once

#include "QtSqlLib/Query/Query.h"

namespace QtSqlLib::Query
{

class CreateIndex : public Query
{
public:
  CreateIndex(const API::Index& index);
  ~CreateIndex() override;

  API::IQuery::SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet_bak& previousQueryResults) override;

private:
  const API::Index& m_index;

};

}
