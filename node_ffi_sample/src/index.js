const { Library, Callback } = require("ffi-napi");
const ref = require("ref-napi");
const ArrayType = require("ref-array-napi");
const StructType = require("ref-struct-napi");

/**
 *
 * sample ffi
 *
 */
if (process.env.DEV_TYPE == "TEST") {
  const LIBRARY_FILE = "lib/target/binding";
  const dll = Library(LIBRARY_FILE, {
    add: ["int", ["int", "int"]],
  });
  const ret = dll.add(1, 2);
  console.log(ret);
  return;
}

/**
 *
 * httpServer ffi
 *
 */
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

const libServer = Library(LIBRARY_FILE, {
  createRoute: [
    "void",
    [RoutePtr, "string", "string", "string", "string", "string", "pointer"],
  ],
  runServer: ["int", [RoutePtrArray, "int"]],
});

const routes = new Array(2);
for (let i = 0; i < routes.length; i++) {
  routes[i] = new Route();
}

const requestHandler = Callback("void", [], () => {
  console.log("Request handler called");
});

libServer.createRoute(
  routes[0],
  "GET",
  "/",
  "text/html",
  "/index.html",
  "",
  requestHandler
);
libServer.createRoute(
  routes[1],
  "GET",
  "/user/:id",
  "text/html",
  "/index.html",
  "",
  requestHandler
);

const result = libServer.runServer(routes, routes.length);
console.log(result);
