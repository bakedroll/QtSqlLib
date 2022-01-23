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

  QSqlQuery getSqlQuery(Schema& schema) const override;

private:
  std::vector<QVariant> m_values;

};

}