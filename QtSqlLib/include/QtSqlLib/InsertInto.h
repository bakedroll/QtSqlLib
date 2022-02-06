#pragma once

#include <QtSqlLib/QuerySequence.hpp>
#include <QtSqlLib/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class InsertInto : public QuerySequence
{
public:
  InsertInto(Schema::Id tableId);
  ~InsertInto() override;

  InsertInto& value(Schema::Id columnId, const QVariant& value);
  InsertInto& relatedEntity(Schema::Id relationshipId, const QueryDefines::ColumnResultMap& entryIdsMap);

  InsertInto& returnIds();

private:
  class QueryInsert : public BaseInsert
  {
  public:
    QueryInsert(Schema::Id tableId);
    ~QueryInsert() override;

    QueryDefines::SqlQuery getSqlQuery(Schema& schema) override;

    std::vector<QVariant>& values();
    std::map<Schema::Id, QueryDefines::ColumnResultMap>& relatedEntities();

  private:
    std::vector<QVariant> m_values;
    std::map<Schema::Id, QueryDefines::ColumnResultMap> m_relatedEntities;

  };

  class QueryInsertedIds : public IQuery
  {
  public:
    QueryInsertedIds(Schema::Id tableId);
    ~QueryInsertedIds() override;

    QueryDefines::SqlQuery getSqlQuery(Schema& schema) override;
    QueryDefines::QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

  private:
    Schema::Id m_tableId;

  };
};

}
