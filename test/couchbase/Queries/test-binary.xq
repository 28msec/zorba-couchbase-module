import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace a = "http://www.zorba-xquery.com/modules/archive";
import schema namespace as = "http://www.zorba-xquery.com/modules/archive";
import module namespace f = "http://expath.org/ns/file";


variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $binary := f:read-binary(resolve-uri("simple.zip"));
cb:store-binary($instance, "archive", $binary);
variable $binary2 := cb:find-binary($instance, "archive");

for $a in a:extract-text($binary2)
return <text>{ $a }</text>
