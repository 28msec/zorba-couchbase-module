import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

cb:put-text($instance, "view", '{ "view" : 1 }');

variable $view-name := cb:create-view($instance, "test-view", "test", {"key":"doc.view"});

variable $data := jn:parse-json(cb:view($instance, $view-name))("rows");
for $d in jn:members($data)
where $d("key") > 0
return $d
