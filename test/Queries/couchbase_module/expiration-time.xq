import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});

cb:put-text($instance, "exp-time", "foo", { "expiration-time" : 1 });
variable $result := cb:get-text($instance, "exp-time");
$result
