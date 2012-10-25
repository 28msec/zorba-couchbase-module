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

    static XmlDataManager*
      getXmlDataManager()
    {
      return Zorba::getInstance(0)->getXmlDataManager();
    }
};

/*******************************************************************************
 ******************************************************************************/

class CouchbaseFunction : public ContextualExternalFunction
{
  protected:

    typedef enum
    {
      LCB_TEXT = 0x01,
      LCB_JSON = 0x02,
      LCB_XML = 0x03,
      LCB_BASE64 = 0x04

    } lcb_storage_type_t;

    class GetOptions
    {
      protected:
        lcb_storage_type_t theType;
        unsigned int theExpTime;
        String theEncoding;

      public:
        Item theItem;

        GetOptions() : theType(LCB_JSON), theExpTime(0), theEncoding("") {}

        GetOptions(lcb_storage_type_t aType) : theType(aType), theExpTime(0) {} 

        void setOptions(Item aOptions);

        ~GetOptions() {}

        lcb_storage_type_t getGetType() { return theType; }

        unsigned int getExpTime() { return theExpTime; }

        String getEncoding() { return theEncoding; }

    };

    class PutOptions
    {
      protected:
        
        lcb_storage_t theOperation;
        lcb_storage_type_t theType;
        unsigned int theExpTime;
        String theEncoding;

      public:

        PutOptions() : theOperation(LCB_ADD), theType(LCB_JSON), theExpTime(0), theEncoding("") { }

        PutOptions(lcb_storage_type_t aType) : theOperation(LCB_SET), theType(aType), theExpTime(0) { }

        void setOptions(Item aOptions);

        ~PutOptions() {}

        lcb_storage_t getOperation() { return theOperation; }

        lcb_storage_type_t getOperationType() { return theType; }

        unsigned int getExmpTime() { return theExpTime; }

        String getEncoding() { return theEncoding; }
    };

    class GetItemSequence : public ItemSequence
    {
      protected:
        lcb_t theInstance;
        Iterator_t theKeys;
        GetOptions theOptions;
    
      public:

        class GetIterator : public Iterator
        {
          protected:            
            lcb_t theInstance;
            lcb_error_t theError;
            Iterator_t theKeys;
            GetOptions theOptions;

          public:
            GetIterator(lcb_t& aInstance, Iterator_t& aKeys, GetOptions& aOptions) 
              : theInstance(aInstance),
                theKeys(aKeys),
                theOptions(aOptions) {}

            virtual ~GetIterator() {}

            void
              open();

            bool 
              next(zorba::Item &aItem);

            void
              close();

            bool
              isOpen() const { return theKeys->isOpen(); }
        };

      public:
        GetItemSequence(lcb_t& aInstance, Iterator_t& aKeys, GetOptions aOptions) 
          : theInstance(aInstance),
            theKeys(aKeys),
            theOptions(aOptions){}

        virtual ~GetItemSequence(){}

        zorba::Iterator_t
          getIterator() { return new GetIterator(theInstance, theKeys, theOptions); }

      protected:
        static void 
          get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp);
    };

    const CouchbaseModule* theModule;

    String
      getOneStringArgument(const Arguments_t&, int) const;
    
    Item
      getOneItemArgument(const Arguments_t&, int) const;

    Iterator_t
      getIterArgument(const Arguments_t&, int) const;

    static void
      throwError(const char*, const char*);

    lcb_t
      getInstance (const DynamicContext*, const String& aIdent) const;

    static void
      store (lcb_t aInstance, Iterator_t aKeys, Iterator_t aValues, PutOptions aOptions);

  public:
    
    CouchbaseFunction(const CouchbaseModule* module);

    virtual ~CouchbaseFunction();

    virtual String
      getURI() const;
 
};


/*******************************************************************************
 ******************************************************************************/

class InstanceMap : public ExternalFunctionParameter
{
  private:
    typedef std::map<String, lcb_t> InstanceMap_t;
    InstanceMap_t* instanceMap;

  public:
    InstanceMap();
    
    bool
    storeInstance(const String&, lcb_t);

    lcb_t
    getInstance(const String&);

    bool 
    deleteInstance(const String&);

    virtual void
    destroy() throw()
    {
      if (instanceMap)
      {
        for (InstanceMap_t::const_iterator lIter = instanceMap->begin();
             lIter != instanceMap->end(); ++lIter)
        {
          lcb_destroy(lIter->second);
        }
        instanceMap->clear();
        delete instanceMap;
      }
      delete this;
    };

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

/*******************************************************************************
 ******************************************************************************/

class GetTextFunction : public CouchbaseFunction
{
  public:
    GetTextFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) 
    {
    }

    virtual ~GetTextFunction(){}

    virtual zorba::String
      getLocalName() const { return "get-text"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;

    static void get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp);
    static std::vector<Item> theVectorItem;
};

/*******************************************************************************
 ******************************************************************************/

class GetBinaryFunction : public CouchbaseFunction
{
  public:
    GetBinaryFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) 
    {
    }

    virtual ~GetBinaryFunction(){}

    virtual zorba::String
      getLocalName() const { return "get-binary"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;

    static void get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp);
    static std::vector<Item> theVectorItem;
};

/*******************************************************************************
 ******************************************************************************/

class RemoveFunction : public CouchbaseFunction
{
  public:
    RemoveFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~RemoveFunction(){}

    virtual zorba::String
      getLocalName() const { return "remove"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class PutTextFunction : public CouchbaseFunction
{
  public:
    PutTextFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~PutTextFunction(){}

    virtual zorba::String
      getLocalName() const { return "put-text"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class PutBinaryFunction : public CouchbaseFunction
{
  public:
    PutBinaryFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~PutBinaryFunction(){}

    virtual zorba::String
      getLocalName() const { return "put-binary"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class FlushFunction : public CouchbaseFunction
{
  public:
    FlushFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~FlushFunction(){}

    virtual zorba::String
      getLocalName() const { return "flush"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class TouchFunction : public CouchbaseFunction
{
  public:
    TouchFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~TouchFunction(){}

    virtual zorba::String
      getLocalName() const { return "touch"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class DisconnectFunction : public CouchbaseFunction
{
  public:
    DisconnectFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~DisconnectFunction(){}

    virtual zorba::String
      getLocalName() const { return "disconnect"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

} /*namespace couchbase*/ } /*namespace zorba*/


#endif //_COM_ZORBA_WWW_MODULES_COUCHBASE_H_

