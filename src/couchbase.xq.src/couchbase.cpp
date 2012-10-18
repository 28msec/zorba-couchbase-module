
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

#include <zorba/item_factory.h>
#include <zorba/singleton_item_sequence.h>
#include <zorba/empty_sequence.h>
#include <zorba/store_manager.h>
#include <zorba/user_exception.h>
#include <zorba/transcode_stream.h>
#include <zorba/serializer.h>
#include <zorba/vector_item_sequence.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>


#include <zorba/util/uuid.h>


#include <libcouchbase/couchbase.h>

#define LCB_WRITE_BUFF_SIZE 24

#include "couchbase.h"

namespace zorba { namespace couchbase {

/*******************************************************************************
 ******************************************************************************/

zorba::ExternalFunction*
  CouchbaseModule::getExternalFunction(const zorba::String& localname)
{
  FuncMap_t::iterator lIte = theFunctions.find(localname);

  ExternalFunction*& lFunc = theFunctions[localname];

  if (lIte == theFunctions.end())
  {
    if (localname == "connect")
    {
      lFunc = new ConnectFunction(this);
    }
    else if (localname == "find-text")
    {
      lFunc = new FindTextFunction(this);
    }
    else if (localname == "find-xml")
    {
      lFunc = new FindXmlFunction(this);
    }
    else if (localname == "find-binary")
    {
      lFunc = new FindBinaryFunction(this);
    }
    else if (localname == "store-text")
    {
      lFunc = new StoreTextFunction(this);
    }
    else if (localname == "store-json")
    {
      lFunc = new StoreJsonFunction(this);
    }
    else if (localname == "store-xml")
    {
      lFunc = new StoreXmlFunction(this);
    }
    else if (localname == "store-binary")
    {
      lFunc = new StoreBinaryFunction(this);
    }
    else if (localname == "remove")
    {
      lFunc = new RemoveFunction(this);
    }
    else if (localname == "destroy")
    {
      lFunc = new DestroyFunction(this);
    }
    else if (localname == "flush")
    {
      lFunc = new FlushFunction(this);
    }
    else if (localname == "touch")
    {
      lFunc = new TouchFunction(this);
    }
  }

  return lFunc;
}

void CouchbaseModule::destroy()
{
  delete this;
}

CouchbaseModule::~CouchbaseModule()
{
  for (FuncMap_t::const_iterator lIter = theFunctions.begin();
    lIter != theFunctions.end(); ++lIter)
  {
    delete lIter->second;
  }
  theFunctions.clear();
}

/*******************************************************************************
 ******************************************************************************/

CouchbaseFunction::CouchbaseFunction(const CouchbaseModule* aModule)
  : theModule(aModule)
{
}

CouchbaseFunction::~CouchbaseFunction()
{
}

String
CouchbaseFunction::getURI() const
{
  return theModule->getURI();
}

String
CouchbaseFunction::getOneStringArgument(const Arguments_t& aArgs, int aPos) const
{
  Item lItem;
  Iterator_t args_iter = aArgs[aPos]->getIterator();
  args_iter->open();
  args_iter->next(lItem);
  String lTmpString = lItem.getStringValue();
  args_iter->close();
  return lTmpString;
}

Item
CouchbaseFunction::getOneItemArgument(const Arguments_t& aArgs, int aPos) const
{
    Item lItem;
    Iterator_t args_iter = aArgs[aPos]->getIterator();
    args_iter->open();
    args_iter->next(lItem);
    args_iter->close();
    return lItem;
}

Iterator_t
CouchbaseFunction::getIterArgument(const Arguments_t& aArgs, int aPos) const
{
  Iterator_t args_iter = aArgs[aPos]->getIterator();
  return args_iter;
}

void
CouchbaseFunction::throwError(const char *aLocalName, const char* aErrorMessage)
{
  String errNS(COUCHBASE_MODULE_NAMESPACE);
  String errName(aLocalName);
  Item errQName = CouchbaseModule::getItemFactory()->createQName(errNS, errName);
  String errDescription(aErrorMessage);
  throw USER_EXCEPTION(errQName, errDescription);
}

lcb_t
CouchbaseFunction::getInstance(const DynamicContext* aDctx, const String& aIdent) const
{
  InstanceMap* lInstanceMap;
  if (!(lInstanceMap = dynamic_cast<InstanceMap*>(aDctx->getExternalFunctionParameter("couchbaseInstanceMap"))))
  {
    throwError("NoInstanceMatch", "No instance of couchbase with the given identifier was found.");
  }

  lcb_t lInstance;
  if(!(lInstance = lInstanceMap->getInstance(aIdent)))
  {
    throwError("NoInstanceMatch", "No instance of couchbase with the given identifier was found.");
  }

  return lInstance;
}

void 
  CouchbaseFunction::FindOptions::setOptions(Item aOptions)
{
  if(!aOptions.isJSONItem())
    throwError("CouchbaseModuleError", "Options must be a JSON object");

  Iterator_t lIter = aOptions.getObjectKeys();
  Item lItem;
  lIter->open();
  while (lIter->next(lItem))
  {
    String lStrKey = lItem.getStringValue();
    if (lStrKey == "type")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      String lStrValue = lValue.getStringValue();
      if (lStrValue == "text")
      {
        theType = LCB_TEXT;
      }
      else if (lStrValue == "json")
      {
        theType = LCB_JSON;
      }
      else if (lStrValue == "xml")
      {
        theType = LCB_XML;
      }
      else if (lStrValue == "binary")
      {
        theType = LCB_BASE64;
      }
    }
    else if (lStrKey == "expiration-time")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theExpTime = lValue.getUnsignedIntValue();
    }
    else if (lStrKey == "encoding")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theEncoding = lValue.getStringValue();
      std::transform(
        theEncoding.begin(), theEncoding.end(),
        theEncoding.begin(), toupper);
      if (!transcode::is_supported(theEncoding.c_str()))
      {
            std::ostringstream lMsg;
            lMsg << theEncoding << ": unsupported encoding";
            throwError("CouchbaseModuleError", lMsg.str().c_str());
      }
    }
  }
  lIter->close();

}

void 
CouchbaseFunction::StoreOptions::setOptions(Item aOptions)
{
  if(!aOptions.isJSONItem())
    throwError("CouchbaseModuleError", "Options must be a JSON object");

  Iterator_t lIter = aOptions.getObjectKeys();
  Item lItem;
  lIter->open();
  while (lIter->next(lItem))
  {
    String lStrKey = lItem.getStringValue();
    if (lStrKey == "operation")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      String lValueStr = lValue.getStringValue();
      if (lValueStr == "add")
      {
        theOperation = LCB_ADD;
      }
      else if (lValueStr == "replace")
      {
        theOperation = LCB_REPLACE;
      }
      else if (lValueStr == "set")
      {
        theOperation = LCB_SET;
      }
      else if (lValueStr == "append")
      {
        theOperation = LCB_APPEND;
      }
      else if (lValueStr == "prepend")
      {
        theOperation = LCB_PREPEND;
      }
    }
    else if (lStrKey == "type")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      String lStrValue = lValue.getStringValue();
      if (lStrValue == "text")
      {
        theType = LCB_TEXT;
      }
      else if (lStrValue == "json")
      {
        theType = LCB_JSON;
      }
      else if (lStrValue == "xml")
      {
        theType = LCB_XML;
      }
      else if (lStrValue == "binary")
      {
        theType = LCB_BASE64;
      }
    }
    else if (lStrKey == "expiration-time")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theExpTime = lValue.getUnsignedIntValue();
    }
    else if (lStrKey == "encoding")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theEncoding = lValue.getStringValue();
      std::transform(
        theEncoding.begin(), theEncoding.end(),
        theEncoding.begin(), toupper);
      if (!transcode::is_supported(theEncoding.c_str()))
      {
            std::ostringstream lMsg;
            lMsg << theEncoding << ": unsupported encoding";
            throwError("CouchbaseModuleError", lMsg.str().c_str());
      }
    }
  }
  lIter->close();

}
/*******************************************************************************
 ******************************************************************************/

InstanceMap::InstanceMap()
{   
  InstanceMap::instanceMap = new InstanceMap_t();
}

bool
InstanceMap::storeInstance(const String& aKeyName, lcb_t aInstance)
{
  std::pair<InstanceMap_t::iterator, bool> ret;
  ret = instanceMap->insert(std::pair<String, lcb_t>(aKeyName, aInstance));
  return ret.second;
}

lcb_t
InstanceMap::getInstance(const String& aKeyName)
{
  InstanceMap::InstanceMap_t::iterator lIter = instanceMap->find(aKeyName);
  
  if (lIter == instanceMap->end())
    return NULL;
  
  lcb_t lInstance = lIter->second;

  return lInstance;
}

bool
InstanceMap::deleteInstance(const String& aKeyName)
{
  InstanceMap::InstanceMap_t::iterator lIter = instanceMap->find(aKeyName);

  if (lIter == instanceMap->end())
    return false;

  lcb_destroy(lIter->second);

  instanceMap->erase(lIter);

  return true;
}

/*******************************************************************************
 ******************************************************************************/

static void error_callback(lcb_t aInstance, lcb_error_t aError, const char* aErrorInfo)
{

}

zorba::ItemSequence_t
ConnectFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  DynamicContext* lDctx = const_cast<DynamicContext*>(aDctx);
  
  InstanceMap* lInstanceMap;
  if (!(lInstanceMap = dynamic_cast<InstanceMap*>(lDctx->getExternalFunctionParameter("couchbaseInstanceMap"))))
  {
    lInstanceMap = new InstanceMap();
    lDctx->addExternalFunctionParameter("couchbaseInstanceMap", lInstanceMap);
  }

  Item lHost;
  Item lUserName;
  Item lPassword;
  Item lBucket;

  if (aArgs.size() > 1)
  {
    lHost = getOneItemArgument(aArgs, 0);
    lUserName = getOneItemArgument(aArgs, 1);
    lPassword = getOneItemArgument(aArgs, 2);
    lBucket = getOneItemArgument(aArgs, 3);
  }
  else
  {
    Item lOptions = getOneItemArgument(aArgs, 0);

    if (lOptions.isJSONItem())
	   {
      Iterator_t lKeys = lOptions.getObjectKeys();
      lKeys->open();
      Item lKey;
      while (lKeys->next(lKey))
      {
        String lStrKey = lKey.getStringValue();
        if (lStrKey == "host")
        {
          lHost = lOptions.getObjectValue(lStrKey);
        }
        else if (lStrKey == "username")
        {
          lUserName = lOptions.getObjectValue(lStrKey);
        }
        else if (lStrKey == "password")
        {
          lPassword = lOptions.getObjectValue(lStrKey);
        }
        else if (lStrKey == "bucket")
        {
          lBucket = lOptions.getObjectValue(lStrKey);
        }
      }
      lKeys->close();
    }
    else
    {
      throwError("CouchbaseError", "Options sent in the connect function is not a JSON object");
    }
  }

  struct lcb_create_st create_options;
  memset(&create_options, 0, sizeof(create_options));

  if (lHost.isNull())
    throwError ("CouchbaseModuleError", "Missing declaration of the couchbase server host");
  String lStrHost = lHost.getStringValue();
  create_options.v.v0.host = lStrHost.c_str();

  if (lBucket.isNull())
    throwError ("CouchbaseModuleError", "Missing declaration of the couchbase bucket");
  String lStrBucket = lBucket.getStringValue();
  create_options.v.v0.bucket = lStrBucket.c_str();

  if (!lUserName.isNull())
  {
    String lStrUserName = lUserName.getStringValue();
    if (lStrUserName == "null")
      create_options.v.v0.user = NULL;
    else
      create_options.v.v0.user = lStrUserName.c_str();
  }

  if (!lPassword.isNull())
  {
    String lStrPassword = lPassword.getStringValue();
    if (lStrPassword == "null")
      create_options.v.v0.passwd = NULL;
    else
      create_options.v.v0.passwd = lStrPassword.c_str();
  }

  lcb_t lInstance;
  lcb_error_t lError;
  
  lError = lcb_create(&lInstance, &create_options);
  if (lError != LCB_SUCCESS)
  {
    throwError("LibCouchbaseError", "Error creating a libcouchbase Instance");
    lcb_strerror(NULL, lError);
  }

  //set callback for catching error
  lcb_set_error_callback(lInstance, error_callback);

  //Connect to couchbase
  if ((lError = lcb_connect(lInstance)) != LCB_SUCCESS)
  {
    throwError("LibCouchbaseError", "Error connecting to the couchbase server");
    lcb_strerror(NULL, lError);
  }

  lcb_wait(lInstance);
  
  uuid lUUID;
  uuid::create(&lUUID);
  
  std::stringstream lStream;
  lStream << lUUID;

  String lStrUUID = lStream.str();

  lInstanceMap->storeInstance(lStrUUID, lInstance);

  return ItemSequence_t(new SingletonItemSequence(CouchbaseModule::getItemFactory()->createAnyURI(lStrUUID)));   
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
RemoveFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);


  lcb_error_t lError;
  Item lKey;
  lKeys->open();
  while (lKeys->next(lKey))
  {
    lcb_remove_cmd_st lCmd;
    String lStrKey = lKey.getStringValue();
    lCmd.v.v0.key = lStrKey.c_str();
    lCmd.v.v0.nkey = strlen((const char*)(lCmd.v.v0.key)); 
    lcb_remove_cmd_st *lCommand[1] = {&lCmd};

    lError = lcb_remove(lInstance, NULL, 1, lCommand);
    if (lError != LCB_SUCCESS)
    {
      std::stringstream lErrorMessage;
      lErrorMessage << "Error finding key \"" << lStrKey  << "\" : " <<  lcb_strerror(lInstance, lError);
      throwError("LibCouchbaseError", lErrorMessage.str().c_str());
    } 
    
    lcb_wait(lInstance);
  }
  lKeys->close();

  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

void
CouchbaseFunction::FindItemSequence::get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp)
{
  if (error != LCB_SUCCESS)
  {
    char lKey[resp->v.v0.nkey+1];
    lKey[resp->v.v0.nkey]=0;
    memcpy(lKey, resp->v.v0.key, resp->v.v0.nkey);
    std::stringstream lErrorMessage;
    lErrorMessage << "Error finding key \"" << lKey  << "\" : " <<  lcb_strerror(instance, error);
    throwError("LibCouchbaseError", lErrorMessage.str().c_str());
  }
  
  FindOptions* lRes = (FindOptions*)cookie;
  
  lcb_storage_type_t lType = lRes->getFindType();

  if(lType == LCB_TEXT || lType == LCB_JSON)
  {
    String lEncoding = lRes->getEncoding();
    String lStrValue ((const char*)resp->v.v0.bytes);
    if (transcode::is_necessary(lEncoding.c_str()))
    {
      std::stringstream lStream;
      lStream << lStrValue.c_str();
      transcode::attach(lStream, lEncoding.c_str());
      lRes->theItem = CouchbaseModule::getItemFactory()->createString(lStream.str());
    }
    else
    {
      lRes->theItem = CouchbaseModule::getItemFactory()->createString(lStrValue);
    }

    //const char* lData = (const char*)resp->v.v0.bytes;
    //lRes->theItem = CouchbaseModule::getItemFactory()->createString(lData);
  }
  else if (lType == LCB_XML)
  {
    const char* lData = (const char*)resp->v.v0.bytes;
    std::stringstream lStream;
    lStream << lData;
    Item lDoc = CouchbaseModule::getXmlDataManager()->parseXML(lStream);
    Iterator_t lIter = lDoc.getChildren();
    lIter->open();
    Item lItem;
    lIter->next(lItem);
    lIter->close();
    lRes->theItem = lItem;
  }
  else if (lType == LCB_BASE64)
  {
    const char* lData = (const char*)resp->v.v0.bytes;
    size_t lLen = resp->v.v0.nbytes;
    lRes->theItem = CouchbaseModule::getItemFactory()->createBase64Binary(lData, lLen);
  }
  else
  {
    throwError ("CouchbaseModuleError", "The requested collection has a not recognized type");
  }
}

void
CouchbaseFunction::FindItemSequence::FindIterator::open()
{
  lcb_set_get_callback(theInstance, FindItemSequence::get_callback);
  theKeys->open();
}

void
CouchbaseFunction::FindItemSequence::FindIterator::close()
{
  theKeys->close();
}

bool
CouchbaseFunction::FindItemSequence::FindIterator::next(Item& aItem)
{
  Item lKey;
  if (theKeys->next(lKey))
  {
    FindOptions* lValue = &theOptions;
    String lStrKey = lKey.getStringValue();
    lcb_get_cmd_st lGet;
    lGet.v.v0.key = lStrKey.c_str();
    lGet.v.v0.nkey = strlen((const char*) lGet.v.v0.key);
    unsigned int lExpTime = theOptions.getExpTime();
    if (lExpTime > 0)
    {
      lGet.v.v0.exptime = lExpTime;
    }

    lcb_get_cmd_st *lCommand[1] = {&lGet};

    theError = lcb_get(theInstance, lValue, 1, lCommand);

    if (theError != LCB_SUCCESS)
    {
      std::stringstream lErrorMessage;
      lErrorMessage << "Error finding key \"" << lStrKey  << "\" : " <<  lcb_strerror(theInstance, theError);
      throwError("LibCouchbaseError", lErrorMessage.str().c_str());
    } 
    
    lcb_wait(theInstance);

    if (lValue->theItem.isNull())
      return false;

    aItem = lValue->theItem;

    return true;
  }
  else
  {
    return false;
  }
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
FindTextFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  FindOptions lOptions(LCB_TEXT);
  if (aArgs.size() > 2)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 2);
    lOptions.setOptions(lOptionsArg);
  }
 
  return ItemSequence_t(new FindItemSequence(lInstance,lKeys, lOptions));   
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
FindXmlFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  FindOptions lOptions(LCB_XML);
  if (aArgs.size() > 2)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 2);
    lOptions.setOptions(lOptionsArg);
  }
 
  return ItemSequence_t(new FindItemSequence(lInstance,lKeys, lOptions));   
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
FindBinaryFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  FindOptions lOptions(LCB_BASE64);
  if (aArgs.size() > 2)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 2);
    lOptions.setOptions(lOptionsArg);
  }
 
  return ItemSequence_t(new FindItemSequence(lInstance,lKeys, lOptions));   
}

/*******************************************************************************
 ******************************************************************************/

void CouchbaseFunction::store (lcb_t aInstance, Iterator_t aKeys, Iterator_t aValues, StoreOptions aOptions)
{
  lcb_error_t lError;
  Item lKey;
  Item lValue;

  aKeys->open();
  aValues->open();
  while (aKeys->next(lKey))
  {
    if (!aValues->next(lValue))
      throwError("CouchbaseModuleError", "The number of key/value's on the save function is not the same.");

    String lStrKey = lKey.getStringValue();

    //TODO: add more options
    lcb_store_cmd_st lStore;
    lStore.v.v0.key = lStrKey.c_str();
    lStore.v.v0.nkey = strlen((const char*)(lStore.v.v0.key));
    
    const char* lData;
    size_t lLen = 0;
    if ((lStore.v.v0.datatype = aOptions.getOperationType()) == LCB_JSON)
    {
      Zorba_SerializerOptions lSerOpt;
      lSerOpt.ser_method = ZORBA_SERIALIZATION_METHOD_JSON;
      lSerOpt.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
      Serializer_t lSer = Serializer::createSerializer(lSerOpt);
      std::stringstream lSerResult;
      lSer->serialize(ItemSequence_t(new SingletonItemSequence(lValue)), lSerResult);
      lData = lSerResult.str().c_str();
      lLen = strlen(lData)+1;
      
    }
    else if (lStore.v.v0.datatype == LCB_TEXT)
    {
      String lEncoding = aOptions.getEncoding();
      String lStrValue = lValue.getStringValue();
      if (transcode::is_necessary(lEncoding.c_str()))
      {
        std::stringstream lStream;
        lStream << lStrValue.c_str();
        transcode::attach(lStream, lEncoding.c_str());
        lData = lStream.str().c_str();
        lLen = strlen(lData)+1;
      }
      else
      {
        lData = lStrValue.c_str();
        lLen = strlen(lData)+1;
      }
    }
    else if (lStore.v.v0.datatype == LCB_XML)
    {
      Zorba_SerializerOptions lSerOpt;
      lSerOpt.ser_method = ZORBA_SERIALIZATION_METHOD_XML;
      lSerOpt.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
      Serializer_t lSer = Serializer::createSerializer(lSerOpt);
      std::stringstream lSerResult;
      lSer->serialize(ItemSequence_t(new SingletonItemSequence(lValue)), lSerResult);
      lData = lSerResult.str().c_str();
      lLen = strlen(lData)+1;
    }
    else if (lStore.v.v0.datatype == LCB_BASE64)
    {
      lData = const_cast<char*>(lValue.getBase64BinaryValue(lLen));
    }
    else
    {
      throwError ("CouchbaseModuleError", "Storing type not recognized");
    }

    lStore.v.v0.bytes = lData;
    lStore.v.v0.nbytes = lLen;
    lStore.v.v0.operation = aOptions.getOperation();
    unsigned int lExpTime = aOptions.getExmpTime();
    if (lExpTime > 0)
    {
      lStore.v.v0.exptime = lExpTime;
    }

    lcb_store_cmd_st *lCommands[1] = {&lStore};
    lError = lcb_store(aInstance, NULL, 1, lCommands);
    
    if (lError != LCB_SUCCESS)
    {
      std::stringstream lErrorMessage;
      lErrorMessage << "Error storing key \"" << lStrKey  << "\" : " <<  lcb_strerror(aInstance, lError);
      throwError("LibCouchbaseError", lErrorMessage.str().c_str());
    } 


    lcb_wait(aInstance);
  }

  if (aValues->next(lValue))
    throwError("CouchbaseModuleError", "The number of key/value's on the save function is not the same.");

  aKeys->close();
  aValues->close();

}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
StoreTextFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Iterator_t lValues = getIterArgument(aArgs, 2);
  
  StoreOptions lOptions(LCB_TEXT);
  if (aArgs.size() > 3)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 3);
    lOptions.setOptions(lOptionsArg);
  }

  store(lInstance, lKeys, lValues, lOptions);
  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
StoreJsonFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Iterator_t lValues = getIterArgument(aArgs, 2);
  
  StoreOptions lOptions(LCB_JSON);
  if (aArgs.size() > 3)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 3);
    lOptions.setOptions(lOptionsArg);
  }

  store(lInstance, lKeys, lValues, lOptions);
  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
StoreXmlFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Iterator_t lValues = getIterArgument(aArgs, 2);
  
  StoreOptions lOptions(LCB_XML);
  if (aArgs.size() > 3)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 3);
    lOptions.setOptions(lOptionsArg);
  }

  store(lInstance, lKeys, lValues, lOptions);
  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
StoreBinaryFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Iterator_t lValues = getIterArgument(aArgs, 2);
  
  StoreOptions lOptions(LCB_BASE64);
  if (aArgs.size() > 3)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 3);
    lOptions.setOptions(lOptionsArg);
  }

  store(lInstance, lKeys, lValues, lOptions);
  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
FlushFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  lcb_error_t lError;


  lcb_flush_cmd_st lFlush;
  lFlush.version=0;
  lcb_flush_cmd_st* lCommand[] = {&lFlush};

  lError =  lcb_flush(lInstance, NULL, 1, lCommand);
  if (lError != LCB_SUCCESS)
  {
    std::stringstream lErrorMessage;
    lErrorMessage << "Flush error : " <<  lcb_strerror(lInstance, lError);
    throwError("LibCouchbaseError", lErrorMessage.str().c_str());
  } 
    
  lcb_wait(lInstance);

  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
TouchFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Item lExp = getOneItemArgument(aArgs, 2);
  unsigned int lInt = lExp.getUnsignedIntValue();

  lcb_error_t lError;
  Item lKey;
  lKeys->open();
  while (lKeys->next(lKey))
  {
    lcb_touch_cmd_t lCmd;
    String lStrKey = lKey.getStringValue();
    lCmd.v.v0.key = lStrKey.c_str();
    lCmd.v.v0.nkey = strlen((const char*)(lCmd.v.v0.key)); 
    lCmd.v.v0.exptime = lInt;
    lcb_touch_cmd_t *lCommand[1] = {&lCmd};

    lError = lcb_touch(lInstance, NULL, 1, lCommand);
    if (lError != LCB_SUCCESS)
    {
      std::stringstream lErrorMessage;
      lErrorMessage << "Error finding key \"" << lStrKey  << "\" : " <<  lcb_strerror(lInstance, lError);
      throwError("LibCouchbaseError", lErrorMessage.str().c_str());
    } 
    
    lcb_wait(lInstance);
  }
  lKeys->close();

  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
DestroyFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  
  InstanceMap* lInstanceMap;
  if (!(lInstanceMap = dynamic_cast<InstanceMap*>(aDctx->getExternalFunctionParameter("couchbaseInstanceMap"))))
  {
    throwError("NoInstanceMatch", "No instance of couchbase with the given identifier was found.");
  }

  if (!lInstanceMap->deleteInstance(lInstanceID))
  {
    throwError("NoInstanceMatch", "No instance of couchbase with the given identifier was found.");
  }

  return ItemSequence_t(new EmptySequence());  
}


} /*namespace couchbase*/ } /*namespace zorba*/


#ifdef WIN32
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT __attribute__ ((visibility("default")))
#endif

extern "C" DLL_EXPORT zorba::ExternalModule* createModule() {
  return new zorba::couchbase::CouchbaseModule();
}
