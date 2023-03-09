var path = require('path');
var fs = require('fs');
const project = path.basename(path.dirname(__filename)) + "_bincc_data";
const base_path = __dirname + "\\bin\\";
const output_path = __dirname + "\\bincc\\";

var walk = function (p, fileCallback, errCallback) {
  var files = fs.readdirSync(p);
  files.forEach(function (f) {
    var fp = path.join(p, f);
    if (fs.statSync(fp).isDirectory()) {
      walk(fp, fileCallback);
    } else {
      fileCallback(fp);
    }
  });
};

var files = [];
walk(base_path, function (path) {
  files.push(path.slice(base_path.length));
}, function (err) {
  console.log("Receive err:" + err);
});


var header = [
  "#ifndef BINCC_H_INCLUDED",
  "#define BINCC_H_INCLUDED",
  "#include <stdint.h>",
  "namespace " + project + "{",
];
var source = [
  "file(" + project,
];

console.log('found ' + files.length + ' files');

var decl_name = [];
var decl_size = [];
var arrays = [];
var data_sizes = [];
var data_names = [];
arrays.push("static const uint8_t *data[]={");
data_sizes.push("static const int data_size[]={");
data_names.push("static const char* data_names[]={");

files.forEach(function (f) {
  const absolute_path = base_path + f;
  const relative_path = f;
  const name = relative_path.replace(/[-\ \(\)\,\.\\]/g, "_");//.replace(/[A-Z]+/, a => a.toLowerCase());
  var stat = fs.statSync(base_path + f);
  decl_name.push("extern const uint8_t " + name + "[];");
  decl_size.push("static const int " + name + "_size=" + stat.size + ";");
  arrays.push("  " + name + ",");
  data_sizes.push("  " + stat.size + ",");
  data_names.push("  \"" + relative_path.replace(/\\/g, "\\\\") + "\",");
  var output_name = name + ".cc";
  source.push("  " + output_name + "");

  console.log("read... " + f);
  fs.readFile(absolute_path, (err, buf) => {
    if (!err) {
      var hex = [];
      for (var i = 0; i < buf.length; i++) {
        if (0 == (i % 1024)) {
          hex.push("\r\n" + buf.readUInt8(i, true));
        }
        else {
          hex.push(buf.readUInt8(i, true));
        }
      }
      var code = "#include \"bincc.h\"\n"
        + "namespace " + project + "{\n"
        + "const uint8_t " + name + "[]={" + hex.join(",") + "};\n"
        + "}";
      fs.writeFileSync(output_path + output_name, code);
      console.log("done: " + name);
    }
  });
});

arrays.push("};");
data_sizes.push("};");
data_names.push("};");

header = header.concat(decl_name);
header = header.concat(decl_size);
header = header.concat(arrays);
header = header.concat(data_sizes);
header = header.concat(data_names);

header = header.concat([
  "}",
  "#endif"
]);

fs.writeFileSync(output_path + "bincc.h", header.join("\n"));

source.push(")");
fs.writeFileSync(output_path + "CMakeLists.txt", source.join("\n"));

console.log("done") 