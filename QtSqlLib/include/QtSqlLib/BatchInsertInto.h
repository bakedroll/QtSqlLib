#pragma once

#include <QtSqlLib/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class BatchInsertInto : public BaseInsert
{
public:
  BatchInsertInto(Schema::Id tableId);
  ~BatchInsertInto() override;

  BatchInsertInto& values(Schema::Id columnId, const QVariantList& values);

  QueryDefines::SqlQuery getSqlQuery(Schema& schema) override;

private:
  std::vector<QVariantList> m_values;

};

}