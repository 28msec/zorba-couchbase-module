import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});

cb:put-text($instance, "encoding2", "äüö", { "encoding" : "ISO-8859-1" });
variable $result := cb:get-text($instance, "encoding2", { "encoding" : "UTF-8" });
if ($result = "äüö")
then true()
else false()
