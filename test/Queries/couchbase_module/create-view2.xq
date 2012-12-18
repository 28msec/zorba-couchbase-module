import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

variable $instance := cb:connect({
  "host": "localhost:8091",
  "username" : null,
  "password" : null,
  "bucket" : "default"});

variable $view-name := cb:create-view($instance, "dev_view2", ("test1", "test2"), ({"key" : "meta.id", "values" : "doc.value"},{ "key" : "meta.id", "values" : ["doc.value", "doc.value2"] }));
$view-name