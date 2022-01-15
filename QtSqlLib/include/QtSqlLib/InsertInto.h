#pragma once

#include <QtSqlLib/IQuery.h>

#include <QVariant>

#include <map>

namespace QtSqlLib
{

class InsertInto : public IQuery
{
public:
  InsertInto(unsigned int tableId);
  ~InsertInto() override;

  InsertInto& value(unsigned int columnId, const QVariant& value);

  QSqlQuery getQueryString(const SchemaConfigurator::Schema& schema) const override;

private:
  unsigned int m_tableId;
  std::map<unsigned int, QVariant> m_values;

};

}