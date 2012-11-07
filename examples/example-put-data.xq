import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $json-zip := jn:parse-json(http:get-text("http://media.mongodb.org/zips.json")[2]);
for $result in $json-zip
  return cb:put-text($instance, $result("_id"), fn:serialize($result), {"expiration-time" : 60*60*24});
