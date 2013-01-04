import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $view-name := cb:create-view($instance, "zip", "zip", {"key" : "doc.state", "values" : "doc.pop"});
variable $data := cb:view($instance, $view-name)("rows");
for $d in jn:members($data)
let $state := $d("key")
group by $state
where $d("value") > 0
let $zip := $d("id")
let $population := sum($d("value"))
let $number-zip := count($zip)
let $avarage-pop := $population div $number-zip
order by $avarage-pop descending
return { "state" : $state , "avarage population" : round-half-to-even($avarage-pop, 2) }


