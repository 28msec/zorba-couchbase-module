/*
 * Copyright 2012 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _COM_ZORBA_WWW_MODULES_COUCHBASE_H_
#define _COM_ZORBA_WWW_MODULES_COUCHBASE_H_

#include <map>

#include <zorba/zorba.h>
#include <zorba/external_module.h>
#include <zorba/function.h>
#include <zorba/dynamic_context.h>

#define COUCHBASE_MODULE_NAMESPACE "http://www.zorba-xquery.com/modules/couchbase"

namespace zorba { namespace couchbase {

/*******************************************************************************
 ******************************************************************************/

class CouchbaseModule : public ExternalModule {
  protected:
    class ltstr
    {
    public:
      bool operator()(const String& s1, const String& s2) const
      {
        return s1.compare(s2) < 0;
      }
    };

    typedef std::map<String, ExternalFunction*, ltstr> FuncMap_t;
    FuncMap_t theFunctions;

  public:
    
    virtual ~CouchbaseModule();

    virtual String
      getURI() const { return COUCHBASE_MODULE_NAMESPACE; }  

    virtual zorba::ExternalFunction*
      getExternalFunction(const String& localName);

    virtual void destroy();

    static ItemFactory*
      getItemFactory()
    {
      return Zorba::getInstance(0)->getItemFactory();
    }

};

/*******************************************************************************
 ******************************************************************************/

class CouchbaseFunction : public ContextualExternalFunction
{
  protected:
    const CouchbaseModule* theModule;

    String
      getOneStringArgument(const Arguments_t&, int) const;
    
    Item
      getOneItemArgument(const Arguments_t&, int) const;

    Iterator_t
      getIterArgument(const Arguments_t&, int) const;

    static void
      throwError(const char*, const char*);

  public:
    
    CouchbaseFunction(const CouchbaseModule* module);

    virtual ~CouchbaseFunction();

    virtual String
      getURI() const;
 
};

/*******************************************************************************
 ******************************************************************************/

class ConnectFunction : public CouchbaseFunction
{
  public:
    ConnectFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~ConnectFunction(){}

    virtual zorba::String
      getLocalName() const { return "connect"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};


} /*namespace couchbase*/ } /*namespace zorba*/


#endif //_COM_ZORBA_WWW_MODULES_COUCHBASE_H_

