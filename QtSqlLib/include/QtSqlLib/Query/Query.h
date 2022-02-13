#pragma once

#include <QtSqlLib/API/IQuery.h>
#include <QtSqlLib/API/IQueryElement.h>

namespace QtSqlLib::Query
{

class Query : public API::IQueryElement,
              public API::IQuery
{
public:
  void accept(API::IQueryVisitor& visitor) override;

};

}
