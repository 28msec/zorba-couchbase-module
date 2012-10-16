import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $result := cb:find-text($instance, "key1", { "expiration-time" : 2 });
cb:destroy($instance);
$result

