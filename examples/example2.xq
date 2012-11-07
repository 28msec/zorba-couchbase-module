import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $view-name := cb:create-view($instance, "zip", "zip", {"key" : "doc.state", "values" : "doc.pop"});
variable $data := jn:parse-json(cb:view($instance, $view-name))("rows");
for $d in jn:members($data)
let $state := $d("key")
group by $state
let $zip := $d("id")
let $number-zip := count($zip)
order by $number-zip descending
return { "state" : $state, "zip count" : $number-zip }

