import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:put-text($instance, "key1", "foo");
cb:disconnect($instance);
variable $result := cb:get-text($instance, "key1");
$result
