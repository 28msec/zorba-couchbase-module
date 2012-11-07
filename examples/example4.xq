import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";
import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $view-name := cb:create-view($instance, "zip", "zip", {"key" : "doc.state", "values" : ["doc.pop", "doc.city"]});
variable $data := jn:parse-json(cb:view($instance, $view-name))("rows");


let $city-pop :=
  for $d in jn:members($data)
  let $state := $d("key")
  let $pop := $d("value")(1)
  let $city := $d("value")(2)
  where $pop > 0
  group by $city, $state
  let $total-pop := sum($pop)
  return {"state" : $state, "city" : $city, "pop" : $total-pop} 
let $result :=
  for $i in $city-pop
  let $state := $i("state")
  let $city := $i("city")
  let $pop := $i("pop")
  group by $state
  let $largest-city := max($pop)
  let $smallest-city := min ($pop)
  return 
    { 
      "state" : $state , 
      "largest city" : 
        for $e in $city-pop
        where $e("pop") eq $largest-city
        return $e("city"),
      "smallest city" : 
        for $e in $city-pop
        where $e("pop") eq $smallest-city
        return $e("city")
    }
return $result

