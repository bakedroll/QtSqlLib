#pragma once

#include <QtSqlLib/Query/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class BatchInsertInto : public BaseInsert
{
public:
  BatchInsertInto(API::ISchema::Id tableId);
  ~BatchInsertInto() override;

  BatchInsertInto& values(API::ISchema::Id columnId, const QVariantList& values);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariantList> m_values;

};

}