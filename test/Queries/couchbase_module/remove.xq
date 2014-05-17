import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "zorba.iriscouch.com:8091",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});

cb:put-text($instance, "remove", "foo");
cb:remove($instance, "remove");
variable $result := cb:get-text($instance, "remove");
$result
