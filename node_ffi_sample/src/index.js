const { Library } = require("ffi-napi");

const LIBRARY_FILE = "lib/target/binding";

const dll = Library(LIBRARY_FILE, {
  add: ["int", ["int", "int"]],
});

const ret = dll.add(1, 2);

console.log(ret);
