#pragma once

#include <QtSqlLib/Query/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class InsertInto : public BaseInsert
{
public:
  InsertInto(API::ISchema::Id tableId);
  ~InsertInto() override;

  InsertInto& value(API::ISchema::Id columnId, const QVariant& value);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariant> m_values;

};

}
