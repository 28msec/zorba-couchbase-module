import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});
variable $view-name := cb:create-view($instance, "zip", "zip", {"key" : "doc.state", "values" : "doc.pop"});
variable $data := cb:view($instance, $view-name, {"stale" : "false"});
for $d in jn:members($data("rows"))
let $state := $d("key")
group by $state
where $d("value") > 0
let $population := sum($d("value"))
order by $population
where $population > 10000000
return { "state" : $state, "total population" : $population }

