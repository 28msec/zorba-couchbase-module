import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $view-name := cb:create-view($instance, "zip", "zip", {"key" : "doc.state", "values" : ["doc.pop", "doc.city"]});
variable $data := jn:parse-json(cb:view($instance, $view-name))("rows");
for $d in jn:members($data)
let $state := $d("key")
let $pop := $d("value")(1)
group by $state
let $largest-city := max($pop)
let $smallest-city := min($pop)
return 
{ "state" : $state , 
 "largest city" : 
for $e in jn:members($data)
where $e("value")(1) eq $largest-city
return $e("value")(2)
, "smallest city" : 
for $e in jn:members($data)
where $e("value")(1) eq $smallest-city
return $e("value")(2)
}


