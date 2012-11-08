module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

declare namespace ver = "http://www.zorba-xquery.com/options/versioning";
declare option ver:module-version "1.0";

(:~
 : Connect to the couchbase server.
 :
 : @param $host address of the couchbase server. (example: "localhost:8091")
 : @param $username username of the couchbase server.
 : @param $password password corresponding to the username in the couchbase server.
 : @param $bucket name of the bucket to use for the couchbase opperations.
 :                (example: "default")
 :
 : @error LCB0001 Error connecting to the couchbase server.
 :
 : @return Return and identifier for the couchbase server.
 :)

declare function cb:connect($host as xs:string, 
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
 : Connect to the couchbase server.
 :
 : @param $options json object that has the host and user informations.
 :
 : @option "host" URL of the couchbase server, this option must be specified.
 : @option "username" username for the couchbase server, this option is optional.
 : @option "password" password for the couchbase server, this option is optional.
 : @option "bucket" name of the bucket that is going to manage the data, this
 :         option must be specified. The bucket must be previously created, this
 :         function does not create new buckets.
 :
 : @error LCB0001 Error connecting to the couchbase server.
 : @error CB0001  Error missing important information for the couchbase 
 :                connection.
 : @error CB0007  Option not supported.
 :
 : @return Return and identifier for the couchbase server.
 :
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
 : @error LCB0002 Libcouchbase error on operation.
 :
 : @return A sequence of string Items corresponding to the key
 :)

declare function cb:get-text($db as xs:anyURI, $key as xs:string*)
    as xs:string* external;

(:~
 : Get Value of the corresponding key as text
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 : @param $options json object with adition options
 :
 : @option "expiration-time" Integer value for refreshing the expiration
 :         time in seconds. 
 : @option "encoding" string with the name of the encoding.
 : 
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0006  Encoding not supported.
 : @error CB0007  Option not supported.
 : @error CB0009  expiration time must be integer value.
 :
 : @return A sequence of string Items corresponding to the key
 :)

declare function cb:get-text($db as xs:anyURI, $key as xs:string*, $options as object())
    as xs:string* external;


(:~
 : Get Value of the corresponding key as base64binary
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 : @error LCB0002 Libcouchbase error on operation.
 :
 : @return A sequence of base64binary to the corresponding to the key
 :)

declare function cb:get-binary($db as xs:anyURI, $key as xs:string*)
    as xs:base64Binary* external;

(:~
 : Get Value of the corresponding key as base64binary
 : 
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 : @param $options json object with aditional options
 :
 : @option "expiration-time" Integer value for refreshing the expiration
 :         time in seconds. 
 : 
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0007  Option not supported.
 : @error CB0009  expiration time must be integer value.
 :
 : @return A sequence of base64binary to the corresponding to the key
 :)

declare function cb:get-binary($db as xs:anyURI, $key as xs:string*, $options as object())
    as xs:base64Binary* external;

(:~
 : Remove key/value form the couchbase server
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier's of the asked value
 :
 : @error LCB0002 Libcouchbase error on operation.
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
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0005 Number of values mismatch number of keys.
 :
 :)  

declare function cb:put-text(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:string*)
    as empty-sequence()
    {
      cb:put-text($db, $key, $value, { "expiration-time" : 60, "encoding" : "UTF-8" })
    };

(:~
 : Stores key/value(text) into the couchbase server's cache.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance
 : @param $key the identifier of the stored value
 : @param $value json to be stored in couchbase server
 : @param $options json object with aditional options
 :
 : @option "expiration-time" integer value that represent the 
 :         expiration time in seconds.
 : @option "operation" type of operation, possible values are 
 :         "add", "replace", "set", "append" and "prepend".
 : @option "encoding" string with the name of the encoding.
 : 
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0005 Number of values mismatch number of keys.
 : @error CB0006  Encoding not supported.
 : @error CB0007  Option not supported.
 : @error CB0009  expiration time must be integer value.
 :
 :)  

declare function cb:put-text(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:string*,
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
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0005 Number of values mismatch number of keys.
 :
 :)  

declare function cb:put-binary(
            $db as xs:anyURI,
            $key as xs:string*,
            $value as xs:base64Binary*)
    as empty-sequence() 
    {
      cb:put-binary($db, $key, $value, { "expiration-time" : 60 })
    };

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
 : @option "expiration-time" integer value that represent the 
 :         expiration time in seconds.
 : @option "operation" type of operation, possible values are 
 :         "add", "replace", "set", "append" and "prepend".
 :
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0005 Number of values mismatch number of keys.
 : @error CB0007  Option not supported.
 : @error CB0009  expiration time must be integer value.
 :
 :)  

declare function cb:put-binary(
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
 :
 : @error LCB0002 Libcouchbase error on operation.
 :
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
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0009  expiration time must be integer value.
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
 : @error LCB0002 Libcouchbase error on operation.
 :
 :)
 
declare function cb:disconnect($db as xs:anyURI)
  as empty-sequence() external;

(:~
 : View content of the couchbase database.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance.
 : @param $path containins the string of a view path 
 :        Example "_design/test/_view/vies".
 :
 : @error LCB0002 Libcouchbase error on operation.
 :
 : @return a xs:string containing json information of the couchbase database
 :         described by the view.
 :
 :)

declare function cb:view($db as xs:anyURI, $path as xs:string*)
  as xs:string* external;

(:~
 : View content of the couchbase database.
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance.
 : @param $path containins the string of a view path 
 :        Example "_design/test/_view/vies".
 : @param $options json object containing options for the view.
 :
 : @option "encoding" string with the name of the encoding.
 :
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0007 Option not supported.
 :
 : @return a xs:string containing json information of the couchbase database
 :         described by the view.
 :
 :)

declare function cb:view($db as xs:anyURI, $path as xs:string*, $options as object())
  as xs:string* external; 

(:~
 : Create a document/view in the couchbase server, if the document already
 : exists in the couchbase server it is replaced. A document can hold several
 : views that must be specified in the same call of cb:create-view. 
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance.
 : @param $doc-name name of the document to create.
 : @param $view-name names of the views to create in the document.
 :
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0005 Number of options mismatch number of views.
 : @error CB0010 Invalid type.
 :
 : @return Return the name of the paths for the views.
 :
 :)
declare function cb:create-view($db as xs:anyURI, $doc-name as xs:string, $view_name as xs:string*)
  as xs:string* external;

(:~
 : Create a document/view in the couchbase server, if the document already
 : exists in the couchbase server it is replaced. A document can hold several
 : views that must be specified in the same call of cb:create-view. 
 :
 : @param $db variable holding the xs:anyURI of a connected 
 :        couchbase instance.
 : @param $doc-name name of the document to create.
 : @param $view-name names of the views to create in the document.
 : @param $options options describing how to create de view.
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
 : @error LCB0002 Libcouchbase error on operation.
 : @error CB0005 Number of options mismatch number of views.
 : @error CB0007 Option not supported.
 : @error CB0010 Invalid type.
 :
 : @return Return the name of the paths for the views.
 :
 :)
declare function cb:create-view($db as xs:anyURI, $doc-name as xs:string, $view_name as xs:string*, $options as object()*)
  as xs:string* external;

