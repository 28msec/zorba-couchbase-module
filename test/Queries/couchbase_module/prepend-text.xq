import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:store-text($instance, "key32", "foo", { "operation" : "add" });
cb:store-text($instance, "key32", "foo3", { "operation" : "prepend" });
cb:store-text($instance, "key32", "foo4", { "operation" : "prepend" });
variable $result := cb:find-text($instance, "key32");
cb:disconnect($instance);
$result
