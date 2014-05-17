#Zorba Couchbase Module [![Build Status](https://travis-ci.org/28msec/zorba-couchbase-module.svg?branch=master)](https://travis-ci.org/28msec/zorba-couchbase-module)

## Query Example
```jsoniq
jsoniq version "1.0";

import module namespace cb = "http://www.zorba-xquery.com/modules/couchbase";

let $db := cb:connect({ "host": "localhost:8091", … })
let $view-name := cb:create-view($instance, "document-name", "view-name", { key : "doc.state", values : "doc.pop"})
for $zip in jn:parse-json(cb:view($instance, $view-name)).rows
let $state := $zip.key
group by $state
let $pop := sum($d.value)
where $pop > 10000000
order by $pop descending
return {
    state : $state,
    population : $pop
}
```

## Documentation
http://www.zorba.io/documentation/latest/modules/connectors/couchbase

## Who's using it
* 28.io: http://www.28.io/datasources/couchbase/
