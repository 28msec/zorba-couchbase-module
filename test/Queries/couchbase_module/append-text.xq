import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});

cb:put-text($instance, "append", "foo");
cb:put-text($instance, "append", "foo2", { "operation" : "append" });
variable $result := cb:get-text($instance, "append");
$result
