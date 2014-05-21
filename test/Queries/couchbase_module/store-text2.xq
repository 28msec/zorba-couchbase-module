import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "74.93.6.105:8091",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});

cb:put-text($instance, "store2", "foo", { "encoding" : "foo" });
variable $result := cb:get-text($instance, "store2", { "encoding" : "UTF-8" });
$result
