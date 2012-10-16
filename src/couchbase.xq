module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.0";

(:~
 : Connect to the couchbase server
 :
 : @param $host address of the couchbase server. (example: "localhost:8091")
 : @param $username username of the couchbase server.
 : @param $password password corresponding to the username in the couchbase server.
 : @param $bucket name of the bucket to use for the couchbase opperations.
 :                (example: "default")
 :
 :)
declare function cb:connect($host as xs:string, 
                            $username as xs:string?, 
                            $password as xs:string?, 
                            $bucket as xs:string)
    as xs:anyURI external;

(:~
 : Connect to the couchbase server
 :
 : @param $options json object that has the host and user informations
 : Example:
 : {
 :   "host": "localhost:8091",
 :   "username" : null,
 :   "password" : null,
 :   "bucket" : "default"
 : }
 :
 :)

declare function cb:connect($options as object())
    as xs:anyURI external;

(:~
 : Get Value of the corresponding key as text
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 : @return A sequence of string Items corresponding to the key
 :)

declare function cb:find-text($db as xs:anyURI, $key as xs:string*)
    as xs:string* external;

(:~
 : Get Value of the corresponding key as text
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 : @param $options json object with adition options
 :  Example:
 :    { "expiration-time" : 60 } (expiration time value is in seconds)
 :
 : @return A sequence of string Items corresponding to the key
 :)

declare function cb:find-text($db as xs:anyURI, $key as xs:string*, $options as object())
    as xs:string* external;

(:~
 : Get Value of the corresponding key as json
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 : @return A sequence of json objects to the corresponding to the key
 :)

declare function cb:find-json($db as xs:anyURI, $key as xs:string*)
    as object()*
{
  jn:parse-json(cb:find-text($db, $key))
};

(:~
 : Get Value of the corresponding key as json
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 : @param $options json object with aditional options
 :  Example:
 :    { "expiration-time" : 60 } (expiration time value is in seconds)
 :
 : @return A sequence of json objects to the corresponding to the key
 :)

declare function cb:find-json($db as xs:anyURI, $key as xs:string*, $options as object())
    as object()*
{
  jn:parse-json(cb:find-text($db, $key, $options))
};

(:~
 : Get Value of the corresponding key as xml
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 : @return A sequence of xml elements to the corresponding to the key
 :)

declare function cb:find-xml($db as xs:anyURI, $key as xs:string*)
    as element()* external;

(:~
 : Get Value of the corresponding key as xml
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 : @param $options json object with aditional options
 :  Example:
 :    { "expiration-time" : 60 } (expiration time value is in seconds)
 :
 : @return A sequence of xml elements to the corresponding to the key
 :)

declare function cb:find-xml($db as xs:anyURI, $key as xs:string*, $options as object())
    as element()* external;

(:~
 : Get Value of the corresponding key as base64binary
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 : @return A sequence of base64binary to the corresponding to the key
 :)

declare function cb:find-binary($db as xs:anyURI, $key as xs:string*)
    as xs:base64Binary* external;

(:~
 : Get Value of the corresponding key as base64binary
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 : @param $options json object with aditional options
 :  Example:
 :    { "expiration-time" : 60 } (expiration time value is in seconds)
 :
 : @return A sequence of base64binary to the corresponding to the key
 :)

declare function cb:find-binary($db as xs:anyURI, $key as xs:string*, $options as object())
    as xs:base64Binary* external;

(:~
 : Remove key/value form the couchbase server
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 :)
declare function cb:remove($db as xs:anyURI, $key as xs:string*)
    as empty-sequence() external;


(:~
 : Stores key/value(text) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 :
 :)  

declare function cb:store-text(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:string*)
    as empty-sequence() external;

(:~
 : Stores key/value(text) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 : @param $options json object with aditional options
 :  Example:
 :    {
 :      "expiration-time" : 60
 :      "operation" : "add"
 :    }
 :    expiration-time value is in seconds.
 :    operation possible values are "add", "replace", "set", "append" 
 :      and "prepend".
 :
 :)  

declare function cb:store-text(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:string*,
            $options as object())
    as empty-sequence() external;

(:~
 : Stores key/value(json object) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 :
 :)  

declare function cb:store-json(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as object()*)
    as empty-sequence() external;

(:~
 : Stores key/value(json object) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 : @param $options json object with aditional options
 :  Example:
 :    {
 :      "expiration-time" : 60
 :      "operation" : "add"
 :    }
 :    expiration-time value is in seconds.
 :    operation possible values are "add", "replace", "set", "append" 
 :      and "prepend".
 :
 :)  

declare function cb:store-json(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as object()*,
            $options as object())
    as empty-sequence() external;

(:~
 : Stores key/value(xml element) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 :
 :)  

declare function cb:store-xml(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as element()*)
    as empty-sequence() external;

(:~
 : Stores key/value(xml element) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 : @param $options json object with aditional options
 :  Example:
 :    {
 :      "expiration-time" : 60
 :      "operation" : "add"
 :    }
 :    expiration-time value is in seconds.
 :    operation possible values are "add", "replace", "set", "append" 
 :      and "prepend".
 :
 :)  

declare function cb:store-xml(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as element()*,
            $options as object())
    as empty-sequence() external;

(:~
 : Stores key/value(base64binary) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 :
 :)  

declare function cb:store-binary(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:base64Binary*)
    as empty-sequence() external;

(:~
 : Stores key/value(base64binary) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 : @param $options json object with aditional options
 :  Example:
 :    {
 :      "expiration-time" : 60
 :      "operation" : "add"
 :    }
 :    expiration-time value is in seconds.
 :    operation possible values are "add", "replace", "set", "append" 
 :      and "prepend".
 :
 :)  

declare function cb:store-binary(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:base64Binary*,
            $options as object())
    as empty-sequence() external;


(:~
 : Remove all key/value pairs from the entire cluster
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 :)

declare function cb:flush($db as xs:anyURI)
    as empty-sequence() external;

(:~
 : Refresh expiration time of selected keys
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $exp-time time in seconds for new expiration time
 :
 :)

declare function cb:touch($db as xs:anyURI, $key as xs:string, $exp-time as xs:integer)
    as empty-sequence() external; 

(:~
 : Destroy couchbase instance
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 :
 :)
 
declare function cb:destroy($db as xs:anyURI)
  as empty-sequence() external; 

