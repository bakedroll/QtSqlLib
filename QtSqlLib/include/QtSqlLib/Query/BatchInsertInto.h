#pragma once

#include <QtSqlLib/Query/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class BatchInsertInto : public BaseInsert
{
public:
  BatchInsertInto(Schema::Id tableId);
  ~BatchInsertInto() override;

  BatchInsertInto& values(Schema::Id columnId, const QVariantList& values);

  SqlQuery getSqlQuery(Schema& schema) override;

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariantList> m_values;

};

}