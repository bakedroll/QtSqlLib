#pragma once
#pragma once

#include <QtSqlLib/IQuery.h>
#include <QtSqlLib/Schema.h>

#include <vector>

namespace QtSqlLib
{

class BaseInsert : public IQuery
{
public:
  BaseInsert(Schema::Id tableId);
  ~BaseInsert() override;

protected:
  QString getSqlQueryString(Schema& schema) const;
  void addColumnId(Schema::Id id);
  void checkColumnIdExisting(Schema::Id id) const;
  void checkTableExisting(Schema& schema) const;

  Schema::Id getTableId() const;

private:
  Schema::Id m_tableId;
  std::vector<Schema::Id> m_columnIds;

};

}