#pragma once

#include "QtSqlLib/Query/InsertInto.h"

namespace QtSqlLib::Query
{

class InsertIntoReferences : public InsertInto
{
public:
  InsertIntoReferences(API::IID::Type tableId);
  ~InsertIntoReferences() override;

  void addForeignKeyValue(const QVariant& value);

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariant> m_foreignKeyValues;

};

}