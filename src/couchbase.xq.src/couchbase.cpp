
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
    else if (localname == "get-text")
    {
      lFunc = new GetTextFunction(this);
    }
    else if (localname == "get-binary")
    {
      lFunc = new GetBinaryFunction(this);
    }
    else if (localname == "put-text")
    {
      lFunc = new PutTextFunction(this);
    }
    else if (localname == "put-binary")
    {
      lFunc = new PutBinaryFunction(this);
    }
    else if (localname == "remove")
    {
      lFunc = new RemoveFunction(this);
    }
    else if (localname == "disconnect")
    {
      lFunc = new DisconnectFunction(this);
    }
    else if (localname == "flush")
    {
      lFunc = new FlushFunction(this);
    }
    else if (localname == "touch")
    {
      lFunc = new TouchFunction(this);
    }
    else if (localname == "view-text")
    {
      lFunc = new ViewFunction(this);
    }
    else if (localname == "create-view")
    {
      lFunc = new CreateViewFunction(this);
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
  Item lItem = getOneItemArgument(aArgs, aPos);
  String lTmpString = lItem.getStringValue();
  return lTmpString;
}

Item
CouchbaseFunction::getOneItemArgument(const Arguments_t& aArgs, int aPos) const
{
  Item lItem;
  Iterator_t args_iter = getIterArgument(aArgs, aPos);
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
  String errName(aLocalName);
  Item errQName = CouchbaseModule::getItemFactory()->createQName(COUCHBASE_MODULE_NAMESPACE, errName);
  String errDescription(aErrorMessage);
  throw USER_EXCEPTION(errQName, errDescription);
}

void 
CouchbaseFunction::isNotJSONError() 
{ 
  throwError("CB0002", "Options parameter is not a JSON object"); 
}

void
CouchbaseFunction::libCouchbaseError(lcb_t aInstance, lcb_error_t aError) 
{ 
  throwError("LCB0002", lcb_strerror(aInstance, aError)); 
} 


lcb_t
CouchbaseFunction::getInstance(const DynamicContext* aDctx, const String& aIdent) const
{
  InstanceMap* lInstanceMap;
  lcb_t lInstance;
  if ((lInstanceMap = dynamic_cast<InstanceMap*>(aDctx->getExternalFunctionParameter("couchbaseInstanceMap"))))
  {
    if ((lInstance = lInstanceMap->getInstance(aIdent)))
      return lInstance;
  }
  throwError("CB0000", "No instance of couchbase with the given identifier was found.");
  return NULL;
}

void
  CouchbaseFunction::ViewOptions::setOptions(Item& aOptions)
{
  if (!aOptions.isJSONItem())
    isNotJSONError();

  Iterator_t lIter = aOptions.getObjectKeys();
  Item lItem;
  lIter->open();
  while (lIter->next(lItem))
  {
    String lStrKey = lItem.getStringValue();
    std::transform(
      lStrKey.begin(), lStrKey.end(),
      lStrKey.begin(), tolower);
    if (lStrKey == "encoding")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theEncoding = lValue.getStringValue();
      if (!transcode::is_supported(theEncoding.c_str()))
      {
        std::ostringstream lMsg;
        lMsg << theEncoding << ": unsupported encoding";
        throwError("CB0006", lMsg.str().c_str());
      }
    }
    else
    {
      std::ostringstream lMsg;
      lMsg << lStrKey << ": option not supported";
      throwError("CB0007", lMsg.str().c_str());
    }
  }
  lIter->close();
}

void 
  CouchbaseFunction::GetOptions::setOptions(Item& aOptions)
{
  if (!aOptions.isJSONItem())
    isNotJSONError();

  Iterator_t lIter = aOptions.getObjectKeys();
  Item lItem;
  lIter->open();
  while (lIter->next(lItem))
  {
    String lStrKey = lItem.getStringValue();
    std::transform(
      lStrKey.begin(), lStrKey.end(),
      lStrKey.begin(), tolower);
    if (lStrKey == "type")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      String lStrValue = lValue.getStringValue();
      std::transform(
        lStrValue.begin(), lStrValue.end(),
        lStrValue.begin(), tolower);
      if (lStrValue == "text")
      {
        theType = LCB_TEXT;
      }
      else if (lStrValue == "binary")
      {
        theType = LCB_BASE64;
      }
    }
    else if (lStrKey == "expiration-time")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      try
      {      
        theExpTime = lValue.getUnsignedIntValue();
      }
      catch (ZorbaException& e)
      {
        throwError("CB0009", " expiration-time option must be an integer value");
      }
    }
    else if (lStrKey == "encoding")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theEncoding = lValue.getStringValue();
      if (!transcode::is_supported(theEncoding.c_str()))
      {
        std::ostringstream lMsg;
        lMsg << theEncoding << ": unsupported encoding";
        throwError("CB0006", lMsg.str().c_str());
      }
    }
    else
    {
      std::ostringstream lMsg;
      lMsg << lStrKey << ": option not supported";
      throwError("CB0007", lMsg.str().c_str());
    }
  }
  lIter->close();

}

void 
CouchbaseFunction::PutOptions::setOptions(Item& aOptions)
{
  if (!aOptions.isJSONItem())
    isNotJSONError();

  Iterator_t lIter = aOptions.getObjectKeys();
  Item lItem;
  lIter->open();
  while (lIter->next(lItem))
  {
    String lStrKey = lItem.getStringValue();
    std::transform(
      lStrKey.begin(), lStrKey.end(),
      lStrKey.begin(), tolower);
    if (lStrKey == "operation")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      String lStrValue = lValue.getStringValue();
      std::transform(
        lStrValue.begin(), lStrValue.end(),
        lStrValue.begin(), tolower);
      if (lStrValue == "add")
      {
        theOperation = LCB_ADD;
      }
      else if (lStrValue == "replace")
      {
        theOperation = LCB_REPLACE;
      }
      else if (lStrValue == "set")
      {
        theOperation = LCB_SET;
      }
      else if (lStrValue == "append")
      {
        theOperation = LCB_APPEND;
      }
      else if (lStrValue == "prepend")
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
      else if (lStrValue == "binary")
      {
        theType = LCB_BASE64;
      }
    }
    else if (lStrKey == "expiration-time")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);

      try
      {      
        theExpTime = lValue.getUnsignedIntValue();
      }
      catch (ZorbaException& e)
      {
        throwError("CB0009", " expiration-time option must be an integer value");
      }
    }
    else if (lStrKey == "encoding")
    {
      Item lValue = aOptions.getObjectValue(lStrKey);
      theEncoding = lValue.getStringValue();
      if (!transcode::is_supported(theEncoding.c_str()))
      {
            std::ostringstream lMsg;
            lMsg << theEncoding << ": unsupported encoding";
            throwError("CB0006", lMsg.str().c_str());
      }
    }
    else
    {
      std::ostringstream lMsg;
      lMsg << lStrKey << ": option not supported";
      throwError("CB0007", lMsg.str().c_str());
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
      else
      {
        std::ostringstream lMsg;
        lMsg << lStrKey << ": option not supported";
        throwError("CB0007", lMsg.str().c_str());
      }
    }
    lKeys->close();
  }
  else
  {
    isNotJSONError();
  }
  
  struct lcb_create_st create_options;
  memset(&create_options, 0, sizeof(create_options));
  
  if (lHost.isNull())
    throwError ("CB0001", "Missing declaration of the couchbase server host");
  String lStrHost = lHost.getStringValue();
  create_options.v.v0.host = lStrHost.c_str();

  if (lBucket.isNull())
    throwError ("CB0001", "Missing declaration of the couchbase bucket");
  String lStrBucket = lBucket.getStringValue();
  create_options.v.v0.bucket = lStrBucket.c_str();

  if (!lUserName.isNull())
  {
    String lStrUserName = lUserName.getStringValue();
    if (lStrUserName != "null")
      create_options.v.v0.user = lStrUserName.c_str();
  }

  if (!lPassword.isNull())
  {
    String lStrPassword = lPassword.getStringValue();
    if (lStrPassword != "null")
      create_options.v.v0.passwd = lStrPassword.c_str();
  }

  lcb_t lInstance;
  lcb_error_t lError;
  
  lError = lcb_create(&lInstance, &create_options);
  if (lError != LCB_SUCCESS)
  {
    throwError("LCB0001", "Error creating a libcouchbase Instance");
    lcb_strerror(NULL, lError);
  }

  //Connect to couchbase
  if ((lError = lcb_connect(lInstance)) != LCB_SUCCESS)
  {
    throwError("LCB0001", "Error connecting to the couchbase server");
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
    lCmd.v.v0.nkey = lStrKey.size(); 
    lcb_remove_cmd_st *lCommand[1] = {&lCmd};

    lError = lcb_remove(lInstance, NULL, 1, lCommand);
    if (lError != LCB_SUCCESS)
    {
      CouchbaseFunction::libCouchbaseError (lInstance, lError);
    } 
    
    lcb_wait(lInstance);
  }
  lKeys->close();

  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

void
CouchbaseFunction::GetItemSequence::get_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_get_resp_t *resp)
{
  if (error != LCB_SUCCESS)
  {
    libCouchbaseError (instance, error);
  }
  
  GetOptions* lRes = (GetOptions*)cookie;
  
  lcb_storage_type_t lType = lRes->getGetType();

  if (lType == LCB_TEXT)
  {
    String lEncoding = lRes->getEncoding();
    String lTmp((const char*)resp->v.v0.bytes, resp->v.v0.nbytes);
    if (lEncoding != "" && transcode::is_necessary(lEncoding.c_str()))
    {    
      transcode::stream<std::istringstream> lTranscoder(lEncoding.c_str(), lTmp.c_str());
      lTmp.clear();

      char buf[1024];
      while (lTranscoder.good())
      {
        lTranscoder.read(buf, 1024);
        lTmp.append(buf, lTranscoder.gcount());
      }      
    }
    lRes->theItem = CouchbaseModule::getItemFactory()->createString(lTmp);
  }
  else if (lType == LCB_BASE64)
  {
    //unsigned char lData[resp->v.v0.nbytes];
    //size_t lLen = resp->v.v0.nbytes;
    //memcpy(lData, resp->v.v0.bytes, lLen);
    lRes->theItem = CouchbaseModule::getItemFactory()->createBase64Binary((const unsigned char*)resp->v.v0.bytes, resp->v.v0.nbytes);
  }
  else
  {
    throwError ("CB0004", "The requested collection has a not recognized type");
  }
}

void
CouchbaseFunction::GetItemSequence::GetIterator::open()
{
  lcb_set_get_callback(theInstance, GetItemSequence::get_callback);
  theKeys->open();
}

void
CouchbaseFunction::GetItemSequence::GetIterator::close()
{
  theKeys->close();
}

bool
CouchbaseFunction::GetItemSequence::GetIterator::next(Item& aItem)
{
  Item lKey;
  if (theKeys->next(lKey))
  {
    GetOptions* lValue = &theOptions;
    String lStrKey = lKey.getStringValue();
    lcb_get_cmd_st lGet;
    lGet.v.v0.key = lStrKey.c_str();
    lGet.v.v0.nkey = lStrKey.size();
    unsigned int lExpTime = theOptions.getExpTime();
    if (lExpTime > 0)
    {
      lGet.v.v0.exptime = lExpTime;
    }

    lcb_get_cmd_st *lCommand[1] = {&lGet};

    theError = lcb_get(theInstance, lValue, 1, lCommand);

    if (theError != LCB_SUCCESS)
    {
      libCouchbaseError (theInstance, theError);
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
GetTextFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  GetOptions lOptions(LCB_TEXT);
  if (aArgs.size() > 2)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 2);
    lOptions.setOptions(lOptionsArg);
  }
 
  return ItemSequence_t(new GetItemSequence(lInstance,lKeys, lOptions));   
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
GetBinaryFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  GetOptions lOptions(LCB_BASE64);
  if (aArgs.size() > 2)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 2);
    lOptions.setOptions(lOptionsArg);
  }
 
  return ItemSequence_t(new GetItemSequence(lInstance,lKeys, lOptions));   
}

/*******************************************************************************
 ******************************************************************************/

void CouchbaseFunction::put (lcb_t aInstance, Iterator_t aKeys, Iterator_t aValues, PutOptions aOptions)
{
  lcb_error_t lError;
  Item lKey;
  Item lValue;

  aKeys->open();
  aValues->open();
  while (aKeys->next(lKey))
  {
    if (!aValues->next(lValue))
      throwError("CB0005", "The number of key/value's on the save function is not the same.");

    String lStrKey = lKey.getStringValue();

    //TODO: add more options
    lcb_store_cmd_st lPut;
    lPut.v.v0.key = lStrKey.c_str();
    lPut.v.v0.nkey = lStrKey.size();
    
    const char* lData;
    size_t lLen = 0;
    lPut.v.v0.datatype = aOptions.getOperationType();
    String lStrValue;
    if (lPut.v.v0.datatype == LCB_TEXT)
    {
      String lEncoding = aOptions.getEncoding();
      lStrValue = lValue.getStringValue();
      if (lEncoding != "" && transcode::is_necessary(lEncoding.c_str()))
      {
        std::stringstream lStream;        
        transcode::attach(lStream, lEncoding.c_str());
        lStream << lStrValue.c_str();
        lStrValue.clear();
        lStrValue = lStream.str();
        lData = lStrValue.c_str();
        lLen = lStrValue.size();
      }
      else
      {
        lData = lStrValue.c_str();
        lLen = lStrValue.size();
      }
    }
    else if (lPut.v.v0.datatype == LCB_BASE64)
    {
      lData = lValue.getBase64BinaryValue(lLen);
    }
    else
    {
      throwError ("CB0004", " Storing type not recognized");
    }

    lPut.v.v0.bytes = lData;
    lPut.v.v0.nbytes = lLen;
    lPut.v.v0.operation = aOptions.getOperation();
    unsigned int lExpTime = aOptions.getExmpTime();
    if (lExpTime > 0)
    {
      lPut.v.v0.exptime = lExpTime;
    }

    lcb_store_cmd_st *lCommands[1] = {&lPut};
    lError = lcb_store(aInstance, NULL, 1, lCommands);
    
    if (lError != LCB_SUCCESS)
    {
      libCouchbaseError (aInstance, lError);
    } 


    lcb_wait(aInstance);
  }

  if (aValues->next(lValue))
    throwError("CB0005", "The number of key/value's on the save function is not the same.");

  aKeys->close();
  aValues->close();

}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
PutTextFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Iterator_t lValues = getIterArgument(aArgs, 2);
  
  PutOptions lOptions(LCB_TEXT);
  if (aArgs.size() > 3)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 3);
    lOptions.setOptions(lOptionsArg);
  }

  put(lInstance, lKeys, lValues, lOptions);
  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
PutBinaryFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = CouchbaseFunction::getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);
  Iterator_t lKeys = getIterArgument(aArgs, 1);
  Iterator_t lValues = getIterArgument(aArgs, 2);
  
  PutOptions lOptions(LCB_BASE64);
  if (aArgs.size() > 3)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 3);
    lOptions.setOptions(lOptionsArg);
  }

  put(lInstance, lKeys, lValues, lOptions);
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
    throwError("LCB0003", lErrorMessage.str().c_str());
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
  unsigned int lInt;
  try
  {
    lInt = lExp.getUnsignedIntValue();
  }
  catch (ZorbaException& e)
  {
    throwError("CB0009", " expiration-time option must be an integer value");
  }
  lcb_error_t lError;
  Item lKey;
  lKeys->open();
  while (lKeys->next(lKey))
  {
    lcb_touch_cmd_t lCmd;
    String lStrKey = lKey.getStringValue();
    lCmd.v.v0.key = lStrKey.c_str();
    lCmd.v.v0.nkey = lStrKey.size(); 
    lCmd.v.v0.exptime = lInt;
    lcb_touch_cmd_t *lCommand[1] = {&lCmd};

    lError = lcb_touch(lInstance, NULL, 1, lCommand);
    if (lError != LCB_SUCCESS)
    {
      libCouchbaseError (lInstance, lError);
    } 
    
    lcb_wait(lInstance);
  }
  lKeys->close();

  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

zorba::ItemSequence_t
DisconnectFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  
  InstanceMap* lInstanceMap;
  if ((lInstanceMap = dynamic_cast<InstanceMap*>(aDctx->getExternalFunctionParameter("couchbaseInstanceMap"))))
  {
    if (lInstanceMap->deleteInstance(lInstanceID))
      return ItemSequence_t(new EmptySequence());
  }

  throwError("CB0000", "No instance of couchbase with the given identifier was found.");

  return ItemSequence_t(new EmptySequence());  
}

/*******************************************************************************
 ******************************************************************************/

void CouchbaseFunction::ViewItemSequence::view_callback( lcb_http_request_t request, lcb_t instance, const void* cookie, lcb_error_t error, const lcb_http_resp_t* resp)
{
  if (error != LCB_SUCCESS)
  {
    libCouchbaseError (instance, error);
  }

  ViewOptions* lRes = (ViewOptions*) cookie;

  String lEncoding = lRes->getEncoding();
  String lTmp((const char*)resp->v.v0.bytes, resp->v.v0.nbytes);
  if (transcode::is_necessary(lEncoding.c_str()))
  {
    transcode::stream<std::istringstream> lTranscoder(lEncoding.c_str(), lTmp.c_str());
    lTmp.clear();

    char buf[1024];
    while (lTranscoder.good())
    {
      lTranscoder.read(buf, 1024);
      lTmp.append(buf, lTranscoder.gcount());
    }
  }
  lRes->theItem = CouchbaseModule::getItemFactory()->createString(lTmp);
}

void 
CouchbaseFunction::ViewItemSequence::ViewIterator::open()
{
  lcb_set_http_complete_callback(theInstance, ViewItemSequence::view_callback);
  thePaths->open();
}

void
CouchbaseFunction::ViewItemSequence::ViewIterator::close()
{
  thePaths->close();
}

bool
CouchbaseFunction::ViewItemSequence::ViewIterator::next(Item& aItem)
{
  Item lPath;
  if (thePaths->next(lPath))
  {
    ViewOptions* lOptions = &theOptions;

    lcb_http_request_t lReq;
    lcb_http_cmd_t lCmd;
    lCmd.v.v0.path = lPath.getStringValue().c_str();
    lCmd.v.v0.npath = lPath.getStringValue().size();
    lCmd.v.v0.body = NULL;
    lCmd.v.v0.nbody = 0;
    lCmd.v.v0.method = LCB_HTTP_METHOD_GET;
    lCmd.v.v0.chunked = 0;
    lCmd.v.v0.content_type = "application/json";
    lcb_error_t err = lcb_make_http_request(theInstance, lOptions, LCB_HTTP_TYPE_VIEW, &lCmd, &lReq);
  
    if (err != LCB_SUCCESS)
    {
      libCouchbaseError (theInstance, err);
    }

    lcb_wait(theInstance);

    if (lOptions->theItem.isNull())
      return false;

    aItem = lOptions->theItem;

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
ViewFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);

  Iterator_t lPaths = getIterArgument(aArgs, 1);
  ViewOptions lOptions;

  if (aArgs.size() > 2)
  {
    Item lOptionsArg = getOneItemArgument(aArgs, 2);
    lOptions.setOptions(lOptionsArg);
  }

  return ItemSequence_t(new ViewItemSequence(lInstance, lPaths, lOptions));  
}

/*******************************************************************************
 ******************************************************************************/


zorba::ItemSequence_t
CreateViewFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  String lInstanceID = getOneStringArgument(aArgs, 0);
  lcb_t lInstance = getInstance(aDctx, lInstanceID);

  String lDocName = getOneStringArgument(aArgs, 1);
  String lPath = "_design/" + lDocName;

  String lBody = "{\"views\": {";
  Iterator_t lViewNames = getIterArgument(aArgs, 2);
  Item lView;

  std::vector<Item> lResult;

  Iterator_t lOptions;
  if (aArgs.size() > 3)
  {
    lOptions = getIterArgument(aArgs, 3);
    Item lOption;
    lOptions->open();
    lViewNames->open();
    bool lIsFirstView = true;
    while (lViewNames->next(lView))
    {
      if (!lOptions->next(lOption))
        throwError("CB0005", "The number of options is not the same as the number of views.");

      //variables used to form the body
      String lBodyKey;
      String lBodyValues;
      String lBodyFunction;

      String lViewName = lView.getStringValue();
      if (lOption.isJSONItem())
      {
        Iterator_t lKeys = lOption.getObjectKeys();
        lKeys->open();
        Item lKey;
        while (lKeys->next(lKey))
        {
          String lStrKey = lKey.getStringValue();
          std::transform(
            lStrKey.begin(), lStrKey.end(),
            lStrKey.begin(), tolower);
          if (lStrKey == "key")
          {
            Item lValue = lOption.getObjectValue(lStrKey);
            if (lValue.isJSONItem())
            {
              std::ostringstream lMsg;
              lMsg << lStrKey << ": value must be of type string.";
              throwError("CB0010", lMsg.str().c_str());
            }
            String lStrValue = lValue.getStringValue();
            std::transform(
              lStrValue.begin(), lStrValue.end(),
              lStrValue.begin(), tolower);
            lBodyKey = lStrValue;
          }
          else if (lStrKey == "values")
          {
            Item lValue = lOption.getObjectValue(lStrKey);
            if (lValue.isJSONItem())
            { 
              lBodyValues = "[";
              bool lIsFirstView = true;
              int lSize = lValue.getArraySize()+1;
              for (int i = 1; i < lSize; i++)
              {
                Item lArrValue = lValue.getArrayValue(i);
                String lStrArrValue = lArrValue.getStringValue();
                std::transform(
                  lStrArrValue.begin(), lStrArrValue.end(),
                  lStrArrValue.begin(), tolower);

                if (lIsFirstView)
                  lIsFirstView = false;
                else
                  lBodyValues += ", ";

                lBodyValues += lStrArrValue;
              }
              lBodyValues += "]";
            }
            else
            {
              String lStrValue = lValue.getStringValue();
              std::transform(
                lStrValue.begin(), lStrValue.end(),
                lStrValue.begin(), tolower);
              lBodyValues = lStrValue;
            }
          }
          else if (lStrKey == "function")
          {
            Item lValue = lOption.getObjectValue(lStrKey);
            if (lValue.isJSONItem())
            {
              std::ostringstream lMsg;
              lMsg << lStrKey << ": value must be of type string.";
              throwError("CB0010", lMsg.str().c_str());
            }
            String lStrValue = lValue.getStringValue();
            std::transform(
              lStrValue.begin(), lStrValue.end(),
              lStrValue.begin(), tolower);
            lBodyFunction = lStrValue;
          }
          else
          {
            std::ostringstream lMsg;
            lMsg << lStrKey << ": option not supported";
            throwError("CB0007", lMsg.str().c_str());
          }
        }
        lKeys->close();
      }
      if (lIsFirstView)
        lIsFirstView = false;
      else
        lBody += " , ";
      if (lBodyFunction.size() < 1)
      {
        if (lBodyKey.size() < 1)
          lBodyKey = "null";
        if (lBodyValues.size() <1)
          lBodyValues = "null";
        lBodyFunction = "function(doc, meta) { emit("+lBodyKey+", "+lBodyValues+");}";
      }
      lBody += "\"" + lViewName + "\": {\"map\": \""+lBodyFunction+"\"}";

      String lStrRes = "_design/" + lDocName + "/_view/" + lViewName;
      Item lRes = CouchbaseModule::getItemFactory()->createString(lStrRes);
      lResult.push_back(lRes);

    }
    if (lOptions->next(lOption))
      throwError("CB0005", "The number of options is not the same as the number of views.");
    lViewNames->close();
    lOptions->close();
    lBody += "}}";
  }
  else
  {
    bool lIsFirstView = true;
    lViewNames->open();
    while (lViewNames->next(lView))
    {
      if (lIsFirstView)
        lIsFirstView = false;
      else
        lBody += " , ";
      String lViewName = lView.getStringValue();
      lBody += "\"" + lViewName + "\": {\"map\": \"function(doc, meta) { emit(meta.id, null);}\"}";

      String lStrRes = "_design/" + lDocName + "/_view/" + lViewName;
      Item lRes = CouchbaseModule::getItemFactory()->createString(lStrRes);
      lResult.push_back(lRes);

    }
    lViewNames->close();
    lBody += "}}";
  }

  lcb_http_request_t lReq;
  lcb_http_cmd_t lCmd;
  lCmd.v.v0.path = lPath.c_str();
  lCmd.v.v0.npath = lPath.size();
  lCmd.v.v0.content_type = "application/json";
  lCmd.v.v0.method = LCB_HTTP_METHOD_PUT;
  lCmd.v.v0.body = lBody.c_str();
  lCmd.v.v0.nbody = lBody.size();
  lCmd.v.v0.chunked = 0;
  lcb_error_t err = lcb_make_http_request(lInstance, NULL, LCB_HTTP_TYPE_VIEW, &lCmd, &lReq);

  if (err != LCB_SUCCESS)
    libCouchbaseError (lInstance, err);

  lcb_wait(lInstance);

  return ItemSequence_t(new VectorItemSequence(lResult));  
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
