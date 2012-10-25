import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:store-text($instance, "prepend", "foo");
cb:store-text($instance, "prepend", "foo2", { "operation" : "prepend" });
variable $result := cb:find-text($instance, "prepend");
cb:disconnect($instance);
$result
