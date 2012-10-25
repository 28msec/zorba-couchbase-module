import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:store-text($instance, "append", "foo");
cb:store-text($instance, "append", "foo2", { "operation" : "append" });
variable $result := cb:find-text($instance, "append");
cb:disconnect($instance);
$result
