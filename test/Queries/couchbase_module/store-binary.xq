import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace f = "http://expath.org/ns/file";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $binary := f:read-binary(resolve-uri("connect.xq"));
cb:put-binary($instance, "binary-file", $binary);
variable $result := cb:get-binary($instance, "binary-file");
$result
