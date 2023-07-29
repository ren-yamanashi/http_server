const { Library, Callback } = require("ffi-napi");
const ref = require("ref-napi");
const ArrayType = require("ref-array-napi");
const StructType = require("ref-struct-napi");

const LIBRARY_FILE = "lib/target/libServer";

const Route = StructType({
  method: "string",
  path: "string",
  content_type: "string",
  file_path: "string",
  message: "string",
  handler: "pointer",
});

const RoutePtr = ref.refType(Route);
const RoutePtrArray = ArrayType(RoutePtr);

const KeyValue = StructType({
  key: "string",
  value: "string",
});

const HttpRequest = StructType({
  method: ArrayType("char", 32),
  target: ArrayType("char", 1024),
  version: ArrayType("char", 32),
  content_type: ArrayType("char", 128),
  body: ArrayType("char", 1024),
  param_kv: ArrayType(KeyValue, 10),
  param_kv_count: "uint",
  parsed_kv: ArrayType(KeyValue, 10),
  parsed_kv_count: "uint",
});

const HttpResponse = StructType({
  status: "int",
  content_type: ArrayType("char", 32),
  content_length: "int",
  body_size: "uint",
  body: ArrayType("char", 1024),
});

const libServer = Library(LIBRARY_FILE, {
  createRoute: [
    "void",
    [RoutePtr, "string", "string", "string", "string", "string", "pointer"],
  ],
  runServer: ["int", [RoutePtrArray, "int"]],
});

const routes = new Array(2);
for (let i = 0; i < routes.length; i++) {
  routes[i] = new Route().ref();
}

const requestHandler = Callback(
  "void",
  [ref.refType(HttpRequest), ref.refType(HttpResponse)],
  (req, res) => {
    console.log(req, res);
  }
);

libServer.createRoute(
  routes[0],
  "GET",
  "/",
  "text/html",
  "/index.html",
  "",
  requestHandler
);
// libServer.createRoute(
//   routes[1],
//   "GET",
//   "/user/:id",
//   "text/html",
//   "/index.html",
//   "",
//   requestHandler
// );

const res = libServer.runServer(routes[0], routes.length);

if (res < 0) {
  console.log("Failed to connect to the HTTP server. Error code:", res);
  return 1;
}
return 0;
