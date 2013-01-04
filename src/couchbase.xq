xquery version "3.0";

(:
 : Copyright 2012 The FLWOR Foundation.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

(:~
 : This module provides minimal functionality to interact with the
 : Couchbase NoSQL database.
 :
 : The module is built using the libcouchbase C client library and
 : exposes most of its functionality in XQuery with JSONiq extensions.
 : Beyond just allowing for basic key-value store operations (e.g.
 : put-/get-text or put-/get-binary, this module also allows to work
 : with Couchbase views in order to allow for complex JSON query
 : operations.
 :
 : @author Juan Zacarias
 :
 :)
module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

declare namespace an = "http://www.zorba-xquery.com/annotations";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.0";

(:~
 : Connect to the Couchbase server and return an opaque identifier
 : representing the established connection.
 :
 : @param $host address of the couchbase server (e.g. "localhost:8091")
 : @param $username username used for the connection
 : @param $password password used for the connection
 : @param $bucket name of the bucket to use (e.g. "default")
 :
 : @error cb:LCB0001 if the connection to the given host/bucket
 :   could not be established.
 :
 : @return an identifier for the established connection.
 :)

declare %an:sequential function cb:connect(
  $host as xs:string, 
  $username as xs:string?, 
  $password as xs:string?, 
  $bucket as xs:string)
    as xs:anyURI
{
  cb:connect({"host" : $host, 
              "username" : $username,
              "password" : $password, 
              "bucket" : $bucket })
};

(:~
 : Connect to the Couchbase server and return an opaque identifier
 : representing the established connection.
 :
 : @param $options a JSONiq object that contains the host, bucket,
 :   and user information.
 :
 : @option "host" endpoint of the Couchbase server (mandatory)
 : @option "username" username used for the connection (optional)
 : @option "password" password used for the connection (optional)
 : @option "bucket" name of an existing bucket (mandatory)
 :
 : @error cb:LCB0001 if the connection to the given host/bucket
 :   could not be established.
 : @error cb:CB0001 if mandatory connection information is missing.
 : @error cb:CB0007 if a given option is not supported.
 :
 : @return an identifier for the established connection.
 :
 : Example:
 : <code>
 : {
 :   "host": "localhost:8091",
 :   "username" : null,
 :   "password" : null,
 :   "bucket" : "default"
 : }
 : </code>
 :)
declare %an:sequential function cb:connect($options as object())
    as xs:anyURI external;

(:~
 : Disconnect from the server/bucket identified by the given
 : URI.
 :
 : @param $db connection reference
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :)
declare %an:sequential function cb:disconnect($db as xs:anyURI)
  as empty-sequence() external;

(:~
 : Return the values of the given keys (type xs:string) as string.
 : 
 : @param $db connection reference
 : @param $key the requested keys
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :
 : @return A sequence of string Items corresponding to the key
 :)

declare %an:sequential function cb:get-text(
  $db as xs:anyURI,
  $key as xs:string*)
as xs:string* external;

(:~
 : Return the values of the given keys (type xs:string) as string.
 : 
 : @param $db connection reference
 : @param $key the requested keys
 : @param $options JSONiq object with additional options
 :
 : @option "expiration-time" xs:integer value for refreshing the expiration
 :   time in seconds. 
 : @option "encoding" string with the name of the encoding of the returned
 :   string (if not UTF-8).
 : 
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0006 if the given encoding is not supported.
 : @error cb:CB0007 if any of the options is not supported.
 : @error cb:CB0009 if the given expiration time is not an xs:integer.
 :
 : @return a sequence of strings for the given keys.
 :)

declare %an:sequential function cb:get-text(
  $db as xs:anyURI,
  $key as xs:string*,
  $options as object())
as xs:string* external;

(:~
 : Return the values of the given keys (type xs:string) as base64Binary.
 : 
 : @param $db connection reference
 : @param $key the requested keys
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :
 : @return a sequence of xs:base64Binary items for the given keys.
 :)
declare %an:sequential function cb:get-binary(
  $db as xs:anyURI,
  $key as xs:string*)
as xs:base64Binary* external;

(:~
 : Return the values of the given keys (type xs:string) as base64Binary.
 : 
 : @param $db connection reference
 : @param $key the requested keys
 : @param $options JSONiq object with additional options
 :
 : @option "expiration-time" xs:integer value for refreshing the expiration
 :   time in seconds. 
 : 
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0007 if any of the options is not supported.
 : @error cb:CB0009 if the given expiration time is not an xs:integer.
 :
 : @return a sequence of xs:base64Binary items for the given keys.
 :)
declare %an:sequential function cb:get-binary(
  $db as xs:anyURI,
  $key as xs:string*,
  $options as object())
as xs:base64Binary* external;

(:~
 : Remove the values matching the given keys (xs:string) from the server.
 :
 : @param $db connection reference
 : @param $key the keys of the values that should be removed.
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :)
declare %an:sequential function cb:remove($db as xs:anyURI, $key as xs:string*)
    as empty-sequence() external;

(:~
 : Store the given key-value bindings.
 :
 : The values are stored with the UTF-8 encoding and a default
 : expiration time of 60 seconds.
 :
 : @param $db connection reference
 : @param $key the keys to store
 : @param $value the values (as xs:string) to be stored.
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0005 if the number of keys doesn't match the number
 :   of values.
 :)  
declare %an:sequential function cb:put-text(
  $db as xs:anyURI,
  $key as xs:string*,
  $value as xs:string*)
as empty-sequence()
{
  cb:put-text($db, $key, $value, { "expiration-time" : 60, "encoding" : "UTF-8" })
};

(:~
 : Store the given key-value bindings.
 :
 : @param $db connection reference
 : @param $key the keys to store
 : @param $value the values (as xs:string) to be stored.
 : @param $options JSONiq object with additional options
 :
 : @option "expiration-time" integer value that represent the 
 :         expiration time in seconds.
 : @option "operation" type of operation, possible values are 
 :         "add", "replace", "set", "append" and "prepend".
 : @option "encoding" the encoding that should be used for the
 :   value (default is UTF-8).
 : 
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0005 if the number of keys doesn't match the number
 :   of values.
 : @error cb:CB0006 if the given encoding is not supported.
 : @error cb:CB0007 if any of the options is not supported.
 : @error cb:CB0009 if the given expiration time is not an xs:integer.
 :)  
declare %an:sequential function cb:put-text(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:string*,
            $options as object())
    as empty-sequence() external;

(:~
 : Store the given key-value bindings.
 :
 : @param $db connection reference
 : @param $key the keys to store
 : @param $value the values (as xs:base64binary) to be stored.
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0005 if the number of keys doesn't match the number
 :   of values.
 :)  
declare %an:sequential function cb:put-binary(
  $db as xs:anyURI,
  $key as xs:string*,
  $value as xs:base64Binary*)
as empty-sequence() 
{
  cb:put-binary($db, $key, $value, { "expiration-time" : 60 })
};

(:~
 : Store the given key-value bindings.
 :
 : @param $db connection reference
 : @param $key the keys to store
 : @param $value the values (as xs:base64binary) to be stored.
 : @param $options JSONiq object with additional options

 : @option "expiration-time" integer value that represent the 
 :         expiration time in seconds.
 : @option "operation" type of operation, possible values are 
 :         "add", "replace", "set", "append" and "prepend".
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0005 if the number of keys doesn't match the number
 :   of values.
 : @error cb:CB0007 if any of the options is not supported.
 : @error cb:CB0009 if the given expiration time is not an xs:integer.
 :)  
declare %an:sequential function cb:put-binary(
  $db as xs:anyURI,
  $key as xs:string*,
  $value as xs:base64Binary*,
  $options as object())
as empty-sequence() external;


(:~
 : Remove all key/value pairs from the cluster
 :
 : @param $db connection reference
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :)
declare %an:sequential function cb:flush($db as xs:anyURI)
as empty-sequence() external;

(:~
 : Refresh the expiration time of the given keys.
 :
 : @param $db connection reference
 : @param $key the keys to touch
 : @param $exp-time new expieration time in seconds
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :)
declare %an:sequential function cb:touch(
  $db as xs:anyURI,
  $key as xs:string*,
  $exp-time as xs:integer)
as empty-sequence() external; 

(:~
 : Retrieve the content of existing views.
 :
 : @param $db connection reference
 : @param $path contains the string of a view path 
 :        (e.g. "_design/test/_view/vies").
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :
 : @return a sequence of strings (as JSON) containing information of the views.
 :)
declare %fn:private %an:sequential function cb:view-text($db as xs:anyURI, $path as xs:string*)
  as xs:string* external;

declare %an:sequential function cb:view($db as xs:anyURI, $path as xs:string*)
  as object()*
{
  jn:parse-json(cb:view-text($db, $path))
};

(:~
 : Retrieve the content of existing views.
 :
 : @param $db connection reference
 : @param $path contains the string of a view path 
 :        (e.g. "_design/test/_view/vies").
 : @param $options JSONiq object with additional options
 :
 : @option "encoding" string with the name of the encoding of the returned
 :   strings (if not UTF-8).
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0007 if any of the options is not supported.
 :
 : @return a sequence of strings (as JSON) containing information of the views.
 :)
declare %fn:private %an:sequential function cb:view-text(
  $db as xs:anyURI,
  $path as xs:string*,
  $options as object())
as xs:string* external; 

declare %an:sequential function cb:view(
  $db as xs:anyURI,
  $path as xs:string*,
  $options as object())
as object()*
{
  jn:parse-json(cb:view-text($db, $path, $options))
};

(:~
 : Create a document/view.
 
 : If the document already exists, it is replaced. A document can hold several
 : views that must be specified in the same call of cb:create-view. 
 :
 : @param $db connection reference
 : @param $doc-name name of the document to create.
 : @param $view-names names of the views to create in the document.
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 :
 : @return the names of the paths for the views that have been
 :   created.
 :)
declare %an:sequential function cb:create-view(
  $db as xs:anyURI,
  $doc-name as xs:string,
  $view-names as xs:string*)
as xs:string* external;

(:~
 : Create a document/view.
 
 : If the document already exists, it is replaced. A document can hold several
 : views that must be specified in the same call of cb:create-view. 
 :
 : @param $db connection reference
 : @param $doc-name name of the document to create.
 : @param $view-names names of the views to create in the document.
 : @param $options options describing how to create the view.
 :
 : @option "key" value must be a string containing the name of the value
 :         in the json object stored in couchbase that is wanted to be 
 :         displayed as the value of "key" in the resulting json.
 : @option "values" value must be a string or an array of strings containing
 :         the name of the value in the json object stored in couchbase that 
 :         is wanted to be displayed as the value of "value" in the resulting
 :         json.
 : @option "function" string with a javascript function describing the 
 :         map function of the view. cb:create-view won't throw an error
 :         if the javascript function is not compilable or functional. If
 :         this option is set the "key" and "values" options are ignored.
 :
 : @error cb:LCB0002 if any error occurs in the communication with
 :   the server.
 : @error cb:CB0005 if the number of options doesn't match the number of
 :   view-names.
 : @error cb:CB0007 if any of the options is not supported.
 : @error cb:CB0010 if any of the given options has an invalid type.
 :
 : @return the names of the paths for the views that have been
 :   created.
 :)
declare %an:sequential function cb:create-view(
  $db as xs:anyURI,
  $doc-name as xs:string,
  $view-names as xs:string*,
  $options as object()*)
as xs:string* external;
