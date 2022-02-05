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
  InsertInto& returnIds();

  QSqlQuery getSqlQuery(Schema& schema) const override;
  QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

private:
  enum class ReturnIdMode
  {
    Undefined,
    Yes,
    No
  };

  std::vector<QVariant> m_values;
  ReturnIdMode m_returnIdMode;

};

}