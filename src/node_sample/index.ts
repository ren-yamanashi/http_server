import ffi from 'ffi-napi';
import ref from 'ref-napi';
import Struct from 'ref-struct-napi';
import ArrayType from "ref-array-napi"

const Route = Struct({
  method: 'string',
  path: 'string',
  content_type: 'string',
  file_path: 'string',
  message: 'string',
  handler: 'pointer',
});
const RoutePtr = ref.refType(Route);
const RoutePtrArray = ArrayType(RoutePtr)

const libServer = ffi.Library('./libServer.so', {
  createRoute: ['void', [RoutePtr, 'string', 'string', 'string', 'string', 'string', 'pointer']],
  runServer: ['int', [RoutePtrArray, 'int']],
});

// Define a handler function
const requestHandler = ffi.Callback('void', [], () => {
  console.log('Request handler called');
});

const routes = new Array(2);
for (let i = 0; i < routes.length; i++) {
  routes[i] = new Route();
}

libServer.createRoute(routes[0], 'GET', '/', 'text/html', '/index.html', '', requestHandler);
libServer.createRoute(routes[1], 'GET', '/user/:id', 'text/html', '/index.html', '', requestHandler);

const result = libServer.runServer(routes, routes.length);
console.log(result);
