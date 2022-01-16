#pragma once

#include <QtSqlLib/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class InsertInto : public BaseInsert
{
public:
  InsertInto(unsigned int tableId);
  ~InsertInto() override;

  InsertInto& value(unsigned int columnId, const QVariant& value);

  QSqlQuery getSqlQuery(const SchemaConfigurator::Schema& schema) const override;

private:
  std::vector<QVariant> m_values;

};

}