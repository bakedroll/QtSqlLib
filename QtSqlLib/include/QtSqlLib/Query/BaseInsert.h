#pragma once
#pragma once

#include <QtSqlLib/Query/Query.h>

#include <vector>

namespace QtSqlLib::Query
{

class BaseInsert : public Query
{
public:
  BaseInsert(const API::IID& tableId);
  ~BaseInsert() override;

  void addColumnId(const API::IID& id);

protected:
  QSqlQuery getQSqlQuery(const QSqlDatabase& db, API::ISchema& schema) const;
  virtual void bindQueryValues(QSqlQuery& query) const = 0;

private:
  void throwIfColumnIdAlreadyExisting(API::IID::Type id) const;

  API::IID::Type m_tableId;
  std::vector<API::IID::Type> m_columnIds;

};

}
