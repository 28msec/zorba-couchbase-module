import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "none",
  "username" : null,
  "password" : null,
  "bucket" : "default"});
if ($instance)
 then true()
 else false()

