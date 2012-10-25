import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:put-text($instance, "remove", "foo");
cb:remove($instance, "remove");
variable $result := cb:get-text($instance, "remove");
cb:disconnect($instance);
$result
