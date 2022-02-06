#pragma once

#include <QtSqlLib/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class InsertInto : public BaseInsert
{
public:
  InsertInto(Schema::Id tableId);
  ~InsertInto() override;

  InsertInto& value(Schema::Id columnId, const QVariant& value);
  InsertInto& relatedEntity(Schema::Id relationshipId, const QueryDefines::ColumnResultMap& entryIdsMap);

  InsertInto& returnIds();

  QueryDefines::SqlQuery getSqlQuery(Schema& schema) override;
  QueryDefines::QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

private:
  enum class ReturnIdMode
  {
    Undefined,
    Yes,
    No
  };

  std::vector<QVariant> m_values;
  std::map<Schema::Id, QueryDefines::ColumnResultMap> m_relatedEntities;

  ReturnIdMode m_returnIdMode;

};

}