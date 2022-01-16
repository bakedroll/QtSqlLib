#pragma once

#include <QtSqlLib/BaseInsert.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

  class BatchInsertInto : public BaseInsert
  {
  public:
    BatchInsertInto(unsigned int tableId);
    ~BatchInsertInto() override;

    BatchInsertInto& values(unsigned int columnId, const QVariantList& values);

    QSqlQuery getSqlQuery(const SchemaConfigurator::Schema& schema) const override;
    bool isBatchExecution() const override;

  private:
    std::vector<QVariantList> m_values;

  };

}