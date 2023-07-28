const { Library } = require("ffi-napi");
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
