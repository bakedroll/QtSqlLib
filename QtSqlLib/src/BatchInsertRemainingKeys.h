#pragma once

#include "QtSqlLib/Query/BatchInsertInto.h"

namespace QtSqlLib::Query
{

class BatchInsertRemainingKeys : public BatchInsertInto
{
public:
  BatchInsertRemainingKeys(
    API::IID::Type tableId,
    int numRelations,
    const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap);
  ~BatchInsertRemainingKeys() override;

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet_bak& previousQueryResults) override;

private:
  int m_numRelations;
  const API::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

};

}