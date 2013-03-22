import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "12492873009",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});
if ($instance)
 then true()
 else false()

