import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:store-text($instance, "exp-time", "foo", { "expiration-time" : 1 });
variable $result := cb:find-text($instance, "exp-time");
cb:disconnect($instance);
$result
