#pragma once

#include <QtSqlLib/Query/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class InsertInto : public BaseInsert
{
public:
  InsertInto(Schema::Id tableId);
  ~InsertInto() override;

  InsertInto& value(Schema::Id columnId, const QVariant& value);

  SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariant> m_values;

};

}
