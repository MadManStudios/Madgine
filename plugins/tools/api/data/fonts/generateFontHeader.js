const fs = require('fs')

iconsFile = process.argv[2]

let rawdata = fs.readFileSync(iconsFile);
let icons = JSON.parse(rawdata);

file = process.argv[3]

code = "#pragma once\n\n"

for ([name, value] of Object.entries(icons)) {
    byte0 = (value & 0x3F) | 0x80
    value >>= 6
    byte1 = (value & 0x3F) | 0x80
    value >>= 6
    byte2 = (value & 0x0F) | 0xe0

    code += "#define IMGUI_ICON_" + name.toUpperCase() + " \"\\x" + byte2.toString(16) + "\\x" + byte1.toString(16) + "\\x" + byte0.toString(16) + "\"\n"
}

fs.writeFileSync(file, code)
console.log("Generated  " + file)

