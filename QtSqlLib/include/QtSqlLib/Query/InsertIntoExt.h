#pragma once

#include <QtSqlLib/Query/QuerySequence.h>
#include "QtSqlLib/Query/InsertInto.h"

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class InsertIntoExt : public QuerySequence
{
public:
  InsertIntoExt(Schema::Id tableId);
  ~InsertIntoExt() override;

  InsertIntoExt& value(Schema::Id columnId, const QVariant& value);
  InsertIntoExt& linkTuple(Schema::Id relationshipId, const Schema::TableColumnValuesMap& tupleIdsMap);

  InsertIntoExt& returnIds();

  void prepare(Schema& schema) override;

private:
  class InsertIntoReferences : public InsertInto
  {
  public:
    InsertIntoReferences(Schema::Id tableId);
    ~InsertIntoReferences() override;

    void setForeignKeyValues(const std::vector<QVariant>& values);

  protected:
    void bindQueryValues(QSqlQuery& query) const override;

  private:
    std::vector<QVariant> m_foreignKeyValues;

  };

  class QueryInsertedIds : public Query
  {
  public:
    QueryInsertedIds(Schema::Id tableId);
    ~QueryInsertedIds() override;

    SqlQuery getSqlQuery(Schema& schema) override;
    QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

  private:
    Schema::Id m_tableId;

  };

  std::unique_ptr<InsertIntoReferences>& getOrCreateInsertQuery();

  std::unique_ptr<InsertIntoReferences> m_insertQuery;

  Schema::Id m_tableId;
  bool m_bIsReturningInsertedIds;
  std::map<Schema::Id, Schema::TableColumnValuesMap> m_linkedTuple;

};

}
