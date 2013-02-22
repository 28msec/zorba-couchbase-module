import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:put-text($instance, "replace", "foo");
cb:put-text($instance, "replace", "foo2", { "operation" : "replace" });
variable $result := cb:get-text($instance, "replace");
$result
