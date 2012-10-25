import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:store-text($instance, "encoding", "äüö", { "encoding" : "ISO-8859-1" });
variable $result := cb:find-text($instance, "encoding", { "encoding" : "ISO-8859-1" });
cb:disconnect($instance);
$result
