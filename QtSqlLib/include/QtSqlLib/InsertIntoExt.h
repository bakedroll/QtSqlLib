#pragma once

#include <QtSqlLib/QuerySequence.h>
#include "QtSqlLib/InsertInto.h"

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class InsertIntoExt : public QuerySequence
{
public:
  InsertIntoExt(Schema::Id tableId);
  ~InsertIntoExt() override;

  InsertIntoExt& value(Schema::Id columnId, const QVariant& value);
  InsertIntoExt& relatedEntity(Schema::Id relationshipId, const QueryDefines::ColumnResultMap& entryIdsMap);

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

  std::map<Schema::Id, QueryDefines::ColumnResultMap> m_relatedEntities;
};

}
