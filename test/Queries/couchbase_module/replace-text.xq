import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "office.go.dyndns.org:8091",
  "username" : jn:null(),
  "password" : jn:null(),
  "bucket" : "default"});

cb:put-text($instance, "replace", "foo");
cb:put-text($instance, "replace", "foo2", { "operation" : "replace" });
variable $result := cb:get-text($instance, "replace");
$result
