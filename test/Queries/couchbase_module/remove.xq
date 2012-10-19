import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:store-text($instance, "remove", "foo");
cb:remove($instance, "remove");
variable $result := cb:find-text($instance, "remove");
cb:disconnect($instance);
$result
