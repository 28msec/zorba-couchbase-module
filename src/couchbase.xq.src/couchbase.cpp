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
#include <zorba/diagnostic_list.h>
#include <zorba/empty_sequence.h>
#include <zorba/user_exception.h>
#include <stdio.h>

#include "couchbase.h"
#include <libcouchbase/couchbase.h>

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

/*******************************************************************************
 ******************************************************************************/
static void error_callback(
  libcouchbase_t instance,
  libcouchbase_error_t error,
  const char* errinfo)
{
  (void)instance;
  std::cout << "error";
}

static void get_callback(
  libcouchbase_t instance,
  const void *cookie,
  libcouchbase_error_t error,
  const void *key, size_t nkey,
  const void *bytes, size_t nbytes,
  uint32_t flags, uint64_t cas)
{
  char str[nbytes];
  memcpy (str, bytes, nbytes+1);
  std::cout << "get callback recieved : "<<str<<std::endl;
}


zorba::ItemSequence_t
ConnectFunction::evaluate(
  const Arguments_t& aArgs,
  const zorba::StaticContext* aSctx,
  const zorba::DynamicContext* aDctx) const
{
  libcouchbase_t instance; // libcouchbase instance
  libcouchbase_error_t oprc; // for checking responses

  //TODO: make modificable
  const char *host = "localhost:8091";
  const char *username = NULL;
  const char *passwd = NULL;
  const char *bucket = "default";

  instance = libcouchbase_create(host, username, passwd, bucket, NULL);

  if (instance == NULL)
  {
    throwError("LibcouchbaseError", "Failed to create libcouchbase instance");
  }
  
  (void)libcouchbase_set_error_callback(instance, error_callback);
  (void)libcouchbase_set_get_callback(instance, get_callback);

  if (libcouchbase_connect(instance) != LIBCOUCHBASE_SUCCESS) 
  {
    throwError("LibcouchbaseError", "Failed to connect libcouchbase instance to server");
  }

  libcouchbase_wait(instance);

  char* key ="hello2";
  char *doc ="{\"message\":\"world\"}";

  oprc = libcouchbase_store(instance,
                              NULL,
                              LIBCOUCHBASE_SET,
                              key, /* the key or _id of the document */
                              strlen(key), /* the key length */
                              doc,
                              strlen(doc), /* length of */
                              0,  /* flags,  */
                              0,  /* expiration */
                              0); /* and CAS values, see API reference */
  
  if (oprc != LIBCOUCHBASE_SUCCESS)
  {
    throwError("LibcouchbaseError", "Failed to create store operation");
  }

  
  libcouchbase_wait(instance);

  oprc = libcouchbase_get_last_error(instance);
  if (oprc == LIBCOUCHBASE_SUCCESS)
  {
    std::cout << "success";
  }
  else
  {
    throwError("LibcouchbaseError", "Could not set key.");
  }

  const char* keys[1];
  size_t nkey[1];
  keys[0] = key;
  nkey[0] = std::strlen(key);
  oprc = libcouchbase_mget(instance,
                           NULL,
                           1,
                           (const void*const*)keys,
                           nkey,
                           NULL);

  if (oprc != LIBCOUCHBASE_SUCCESS) 
  {
    throwError("LibcouchbaseError", "Failed to retrieve key.");
  }

  libcouchbase_wait(instance);


  return ItemSequence_t(new SingletonItemSequence(CouchbaseModule::getItemFactory()->createString("test")));
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
