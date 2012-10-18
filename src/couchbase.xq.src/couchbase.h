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

    class FindOptions
    {
      protected:
        lcb_storage_type_t theType;
        unsigned int theExpTime;
        String theEncoding;

      public:
        Item theItem;

        FindOptions() : theType(LCB_JSON), theExpTime(0), theEncoding("") {}

        FindOptions(lcb_storage_type_t aType) : theType(aType), theExpTime(0) {} 

        void setOptions(Item aOptions);

        ~FindOptions() {}

        lcb_storage_type_t getFindType() { return theType; }

        unsigned int getExpTime() { return theExpTime; }

        String getEncoding() { return theEncoding; }

    };

    class StoreOptions
    {
      protected:
        
        lcb_storage_t theOperation;
        lcb_storage_type_t theType;
        unsigned int theExpTime;
        String theEncoding;

      public:

        StoreOptions() : theOperation(LCB_ADD), theType(LCB_JSON), theExpTime(0), theEncoding("") { }

        StoreOptions(lcb_storage_type_t aType) : theOperation(LCB_SET), theType(aType), theExpTime(0) { }

        void setOptions(Item aOptions);

        ~StoreOptions() {}

        lcb_storage_t getOperation() { return theOperation; }

        lcb_storage_type_t getOperationType() { return theType; }

        unsigned int getExmpTime() { return theExpTime; }

        String getEncoding() { return theEncoding; }
    };

    class FindItemSequence : public ItemSequence
    {
      protected:
        lcb_t theInstance;
        Iterator_t theKeys;
        FindOptions theOptions;
    
      public:

        class FindIterator : public Iterator
        {
          protected:            
            lcb_t theInstance;
            lcb_error_t theError;
            Iterator_t theKeys;
            FindOptions theOptions;

          public:
            FindIterator(lcb_t& aInstance, Iterator_t& aKeys, FindOptions& aOptions) 
              : theInstance(aInstance),
                theKeys(aKeys),
                theOptions(aOptions) {}

            virtual ~FindIterator() {}

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
        FindItemSequence(lcb_t& aInstance, Iterator_t& aKeys, FindOptions aOptions) 
          : theInstance(aInstance),
            theKeys(aKeys),
            theOptions(aOptions){}

        virtual ~FindItemSequence(){}

        zorba::Iterator_t
          getIterator() { return new FindIterator(theInstance, theKeys, theOptions); }

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
      store (lcb_t aInstance, Iterator_t aKeys, Iterator_t aValues, StoreOptions aOptions);

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

class FindTextFunction : public CouchbaseFunction
{
  public:
    FindTextFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) 
    {
    }

    virtual ~FindTextFunction(){}

    virtual zorba::String
      getLocalName() const { return "find-text"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;

    static void get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp);
    static std::vector<Item> theVectorItem;
};

/*******************************************************************************
 ******************************************************************************/

class FindXmlFunction : public CouchbaseFunction
{
  public:
    FindXmlFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) 
    {
    }

    virtual ~FindXmlFunction(){}

    virtual zorba::String
      getLocalName() const { return "find-xml"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;

    static void get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp);
    static std::vector<Item> theVectorItem;
};

/*******************************************************************************
 ******************************************************************************/

class FindBinaryFunction : public CouchbaseFunction
{
  public:
    FindBinaryFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) 
    {
    }

    virtual ~FindBinaryFunction(){}

    virtual zorba::String
      getLocalName() const { return "find-binary"; }

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

class StoreTextFunction : public CouchbaseFunction
{
  public:
    StoreTextFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~StoreTextFunction(){}

    virtual zorba::String
      getLocalName() const { return "store-text"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class StoreJsonFunction : public CouchbaseFunction
{
  public:
    StoreJsonFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~StoreJsonFunction(){}

    virtual zorba::String
      getLocalName() const { return "store-json"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class StoreXmlFunction : public CouchbaseFunction
{
  public:
    StoreXmlFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~StoreXmlFunction(){}

    virtual zorba::String
      getLocalName() const { return "store-xml"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

/*******************************************************************************
 ******************************************************************************/

class StoreBinaryFunction : public CouchbaseFunction
{
  public:
    StoreBinaryFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~StoreBinaryFunction(){}

    virtual zorba::String
      getLocalName() const { return "store-binary"; }

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

class DestroyFunction : public CouchbaseFunction
{
  public:
    DestroyFunction(const CouchbaseModule* aModule)
      : CouchbaseFunction(aModule) {}

    virtual ~DestroyFunction(){}

    virtual zorba::String
      getLocalName() const { return "destroy"; }

    virtual zorba::ItemSequence_t
      evaluate( const Arguments_t&,
                const zorba::StaticContext*,
                const zorba::DynamicContext*) const;
};

} /*namespace couchbase*/ } /*namespace zorba*/


#endif //_COM_ZORBA_WWW_MODULES_COUCHBASE_H_

